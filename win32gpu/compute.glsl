#version 450 core

#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#define Matrix4 mat4
#define Vector3 vec3
#define Vector4 vec4
#define uint32_t uint

// Function defines
#define Clamp(A, c, B) clamp(c, A, B)
#define DotProduct(A, B) dot(A, B)
#define Normalize(V) normalize(V)
#define CrossProduct(A, B) cross(A, B)
#define Lerp(A, c, B) mix(A, B, c)

#define F32Max 3.402823466e+38F
#define PI 3.141592653

layout(rgba32f, location = 0) readonly uniform image2D srcImage;
layout(rgba32f, location = 1) writeonly uniform image2D destImage;
layout(location = 2) uniform Vector3 cameraPosition;
layout(location = 3) uniform uint32_t time;
layout(location = 4) uniform uint32_t frameIndex;

struct Material {
    Vector3 color;
    float refractiveIndex; // Refractive index of material. 0 means no refraction.
    Vector3 emitColor;
    float reflection; // 0 is pure diffuse, 1 is mirror.
};

struct Sphere {
    Vector3 position;
    float radius;
    uint32_t materialIndex;
};

struct Plane {
    Vector3 normal;
    float d;
    uint32_t materialIndex;
};

struct Camera {
    Vector3 position;
    Vector3 zVec;
    Vector3 yVec;
    Vector3 xVec;
};

const Vector3 rectDefaultMinPoint = Vector3(-1.0f, -1.0f, 0.0f);
const Vector3 rectDefaultMaxPoint = Vector3(1.0f,  1.0f, 0.0f);
struct RectangleXY {
    Matrix4 transformMatrix;
    Vector3 normal;
    uint32_t materialIndex;
};

// There can only be one array of variable size per SSBO and it has to be the bottommost in the layout definition.
// So I created 1 SSBO for every scene element buffer.
// TODO: This is dumb! Because scene never change on runtime, all of those buffers actually fixed size.
// We can make 1 SSBO and put all of the buffer in it with fixed sizes.
layout(binding = 0) buffer MaterialBuffer {
    Material gMaterials[];
}; 

layout(binding = 1) buffer SphereBuffer {
    Sphere gSpheres[];
};

layout(binding = 2) buffer PlaneBuffer {
    Plane gPlanes[];
};

layout(binding = 3) buffer RectangleBuffer {
    RectangleXY gRectangles[];
};

layout(binding = 4) buffer Counter { 
    uint32_t gBounceCount; 
};

Camera CreateCamera(Vector3 cameraPos) {
    Vector3 cameraZ = Normalize(cameraPos);
    Vector3 cameraX = Normalize(CrossProduct(Vector3(0.0f, 1.0f, 0.0f), cameraZ));
    Vector3 cameraY = Normalize(CrossProduct(cameraZ, cameraX));

    Camera result;
    result.position = cameraPos;
    result.zVec = cameraZ;
    result.yVec = cameraY;
    result.xVec = cameraX;

    return result;
} 

// The state must be initialized to non-zero value
uint32_t XOrShift32(inout uint32_t state)
{
    // https://en.wikipedia.org/wiki/Xorshift
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    return x;
}

float RandomUnilateral(inout uint32_t state) {
    return float(XOrShift32(state)) / float(uint32_t(4294967295));
}

float RandomBilateral(inout uint32_t state) {
    return 2.0f * RandomUnilateral(state) - 1.0f;
}

bool Refract(in Vector3 incidentVector, in Vector3 normal,
            in float refractiveIndex, inout Vector3 refractionDirection) {
    // Clamp cos value for avoiding any NaN errors;
    float cosIncidentAngle = Clamp(-1.0f, DotProduct(incidentVector, normal), 1.0f);
    Vector3 hitNormal = normal;
    float refractiveIndexRatio;
    // NOTE: we assume incident ray comes from the air which has refraction index equals 1.
    if (cosIncidentAngle < 0) {
        // We are coming from outside the surface
        cosIncidentAngle = -cosIncidentAngle;
        refractiveIndexRatio = 1.0 / refractiveIndex;
    } else {
        hitNormal = -normal;
        refractiveIndexRatio = refractiveIndex; // / 1.0f
    }

    float discriminant = 1 - refractiveIndexRatio * refractiveIndexRatio *
                        (1 - cosIncidentAngle * cosIncidentAngle);
    // If discriminant lower than 0 we cannot refract the other medium.
    // This is called total internal reflection.
    if (discriminant < 0) {
        return false;
    } else {
        refractionDirection = incidentVector * refractiveIndexRatio + hitNormal *
                    (refractiveIndexRatio * cosIncidentAngle - sqrt(discriminant));
        return true;
    }
}

float Schlick(in Vector3 incidentVector, in Vector3 normal, in float refractiveIndex) {
    float cosIncidentAngle = Clamp(-1.0f, DotProduct(incidentVector, normal), 1.0f);
    float cosine;
    if (cosIncidentAngle > 0) {
        cosine = cosIncidentAngle;
    } else {
        cosine = -cosIncidentAngle;
    }
    float r0 = (1 - refractiveIndex) / (1 + refractiveIndex);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct WorldIntersectionResult {
    float t;
    uint32_t hitMaterialIndex;
    Vector3 hitNormal;
};

bool IntersectWorld(in Ray ray, inout WorldIntersectionResult intersectionResult) {
    float hitTolerance = 0.001;
    float minHitDistance = 0.001;
    
    for (int planeIndex = 0; planeIndex < gPlanes.length(); ++planeIndex) {
        Plane plane = gPlanes[planeIndex];
        
        float denom = DotProduct(plane.normal, ray.direction);
        if ((denom < -hitTolerance) || (denom > hitTolerance)) {
            float hitDistance = (-plane.d - DotProduct(plane.normal, ray.origin)) / denom;
            if (hitDistance > minHitDistance && hitDistance < intersectionResult.t) {
                intersectionResult.t = hitDistance;
                intersectionResult.hitMaterialIndex = plane.materialIndex;
                intersectionResult.hitNormal = plane.normal;
            }
        }
    }
    
    for (int sphereIndex = 0; sphereIndex < gSpheres.length(); ++sphereIndex) {
        Sphere sphere = gSpheres[sphereIndex];
        Vector3 centerToOrigin = ray.origin - sphere.position;
        float a = DotProduct(ray.direction, ray.direction);
        float b = 2 * DotProduct(ray.direction, centerToOrigin);
        float c = DotProduct(centerToOrigin, centerToOrigin) - (sphere.radius * sphere.radius);
        float discriminant = b * b - 4 * a * c;
        float denom = 2 * a;
        if (discriminant > 0) {
            float sqrtDiscriminant = sqrt(discriminant);
            float tp = (-b + sqrtDiscriminant) / denom;
            float tn = (-b - sqrtDiscriminant) / denom;

            float hitDistance = tp;
            if (tn > minHitDistance && tn < tp) {
                hitDistance = tn;
            }

            if (hitDistance > minHitDistance && hitDistance < intersectionResult.t) {
                intersectionResult.t = hitDistance;
                intersectionResult.hitMaterialIndex = sphere.materialIndex;

                Vector3 hitPosition = ray.origin + ray.direction * hitDistance;
                intersectionResult.hitNormal = Normalize(hitPosition - sphere.position);
            }
        }
    }

    for (int rectangleIndex = 0; rectangleIndex < gRectangles.length(); ++rectangleIndex) {
        RectangleXY rect = gRectangles[rectangleIndex];

        // rectangle's transform matrix is already inverted when creating scene
        Matrix4 rayMatrix = transpose(rect.transformMatrix);
        Vector3 rayOrigin = (rayMatrix * Vector4(ray.origin, 1.0f)).xyz;
        Vector3 rayDirection = (rayMatrix * Vector4(ray.direction, 0.0f)).xyz;

        float t = (-rayOrigin.z) / rayDirection.z;
        Vector3 hitPoint = rayOrigin + rayDirection * t;

        bool hit = hitPoint.x <= rectDefaultMaxPoint.x && hitPoint.x >= rectDefaultMinPoint.x &&
            hitPoint.y <= rectDefaultMaxPoint.y && hitPoint.y >= rectDefaultMinPoint.y;
        if (hit && t < intersectionResult.t && t > minHitDistance) {
            intersectionResult.t = t;
            intersectionResult.hitMaterialIndex = rect.materialIndex;
            Vector3 rectNormal = rect.normal;
            // Check for incident ray direction vector direction
            // If it's coming to back side of rectangle
            // Flip the normal vector
            float dot = DotProduct(rectNormal, ray.direction);
            if (dot > 0) {
                intersectionResult.hitNormal = -rectNormal;
            }
            else {
                intersectionResult.hitNormal = rectNormal;
            }
        }
    }

    return intersectionResult.t < F32Max;
}

// Main ray trace function.
// I use a loop-based tracing instead of recursion-based trace function.
// You can write clean code by using recursion but I find recursion hard to understand.
// This way is more straightforward and understandable for me.
Vector3 RaytraceWorld(in Ray ray, inout uint32_t randomState, inout uint32_t bounceCount) {
    Vector3 result = Vector3(0.0f, 0.0f, 0.0f);
    
    Ray bounceRay;
    bounceRay.origin = ray.origin;
    bounceRay.direction = ray.direction;

    uint32_t bouncesComputed = 0;

    Vector3 attenuation = Vector3(1.0f, 1.0f, 1.0f);
    for (uint32_t bounceIndex = 0; bounceIndex < 8; ++bounceIndex) {
        // Default memory for structs are not zeroed.
        // We have to specify default variables.
        WorldIntersectionResult intersectionResult;
        intersectionResult.t = F32Max;
        intersectionResult.hitMaterialIndex = 0;
        intersectionResult.hitNormal = Vector3(0.0f, 0.0f, 0.0f);
        bool isIntersect = IntersectWorld(bounceRay, intersectionResult);
        ++bouncesComputed;

        Material mat = gMaterials[intersectionResult.hitMaterialIndex];
        if (isIntersect) {
            result += attenuation * mat.emitColor;
            attenuation *= mat.color;
            bounceRay.origin = bounceRay.origin + bounceRay.direction * intersectionResult.t;
        
            Vector3 mirrorBounce = bounceRay.direction - intersectionResult.hitNormal *
                                    DotProduct(intersectionResult.hitNormal, bounceRay.direction) * 2.0f;
            Vector3 randomBounce = intersectionResult.hitNormal + Vector3(RandomBilateral(randomState),
                                                                    RandomBilateral(randomState),
                                                                    RandomBilateral(randomState));
            Vector3 reflectedRay = Normalize(Lerp(randomBounce, mat.reflection, mirrorBounce));

             // Fresnel coefficient is between 0 and 1. We start with 1 which is full reflection, no refraction.
            float fresnelCoefficient = 1.0;
            Vector3 refractedRay = reflectedRay;

            if (mat.refractiveIndex != 0.0f) {
                bool isRefract = Refract(bounceRay.direction, intersectionResult.hitNormal,
                                         mat.refractiveIndex, refractedRay);
                if (isRefract) {
                    // Refractive material
                    refractedRay = Normalize(refractedRay);

                    // We use the Schlick Approximation for getting fresnel coefficient. It's okay for our purposes.
                    // NOTE: We can use actual Fresnel Equations for making the image little bit more realistic.
                    fresnelCoefficient = Schlick(bounceRay.direction, intersectionResult.hitNormal,
                                                 mat.refractiveIndex);
                }
            }

            // We use the Russian Roulette method for determining which way to go. It fits our architecture.
            // We might do calculate reflected and refracted ray separately and apply linear interpolation
            // between them by coefficient given from the Fresnel Equations.
            if (RandomUnilateral(randomState) <= fresnelCoefficient) {
                bounceRay.direction = reflectedRay;
            } else {
                bounceRay.direction = refractedRay;
            }
        } else {
            // Hit nothing (sky)
            // We just return attenuation for now. No sky color or sky emmiter.
            result += attenuation * mat.emitColor;
            break;
        }
    }

    bounceCount += bouncesComputed;
    return result;
}

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main() {
    
    uint32_t x = gl_GlobalInvocationID.x;
    uint32_t y = gl_GlobalInvocationID.y;
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    
    uint32_t width = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
    uint32_t height = gl_NumWorkGroups.y * gl_WorkGroupSize.y;

    float imageAspectRatio = float(width) / float(height);

    
    // We get camera position as uniform variable.
    // TODO: We can get camera as a SSBO. So we don't have to calculate camera axis for every pixel.
    Camera camera = CreateCamera(cameraPosition);
    
    float filmDistance = 1.0;
    Vector3 filmCenter = cameraPosition - camera.zVec * filmDistance;

    float filmWidth = 1.0f * imageAspectRatio;
    float filmHeight = 1.0f;
    
    float halfFilmWidth = filmWidth * 0.5f;
    float halfFilmHeight = filmHeight * 0.5f;

    float halfPixelWidth = 0.5f / width;
    float halfPixelHeight = 0.5f / height;

    uint32_t sampleSize = 8;
    uint32_t randomState = time * (x + y * 36) + 1;
    uint32_t totalBounces = 0;

    Vector3 color = Vector3(0.0f, 0.0f, 0.0f);
    float filmX = (float(x) / float(width) * 2.0f - 1.0f);
    float filmY = (float(y) / float(height) * 2.0f - 1.0f);
    for (uint32_t sampleIndex = 0; sampleIndex < sampleSize; ++sampleIndex) {
        float offsetX = filmX + RandomBilateral(randomState) * halfPixelWidth;
        float offsetY = filmY + RandomBilateral(randomState) * halfPixelHeight;
        
        Vector3 filmPosition = filmCenter + camera.xVec * offsetX * halfFilmWidth + camera.yVec * halfFilmHeight * offsetY;
        
        Ray ray;
        ray.origin = cameraPosition;
        ray.direction = Normalize(filmPosition - cameraPosition);
        
        Vector3 result = RaytraceWorld(ray, randomState, totalBounces);
        color += result;
    }
 
    // We can use Atomic Counters for bounce count and use atomicCounterAddARB operation for interlocked adding operation.
    // But the atomicCounterAddARB function only supported in OpenGL version 4.6.
    // So instead of Atomic Counters, I used SSBO for counter and use classic atomicAdd function.
    // That way we don't have use version 4.6 and performance is same as Atomic Counters.
    atomicAdd(gBounceCount, totalBounces);

    Vector3 prevColor = imageLoad(srcImage, pix).xyz;

    float factor = 1.0f / float(sampleSize);
    color *= factor;
    // Calcualte the sum of all previous samples
    Vector3 sumColor = prevColor * frameIndex;
    // Add new color to sum, then re-average it.
    Vector3 finalColor = (color + sumColor) / (frameIndex + 1);
    
    imageStore(destImage, pix, vec4(finalColor, 1.0));
}