#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "image.h"
#include "math_util.h"
#include "scene.h"

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct WorldIntersectionResult {
    float t = F32Max;
    uint32_t hitMetarialIndex;
    Vector3 hitNormal;
};

bool 
IntersectWorld(World* world, Ray* ray, WorldIntersectionResult* intersectionResult) {
    float hitTolerance = 0.001;
    float minHitDistance = 0.0001;
    
    for (int planeIndex = 0; planeIndex < world->planeCount; ++planeIndex) {
        Plane plane = world->planes[planeIndex];
        
        float denom = DotProduct(plane.normal, ray->direction);
        if ((denom < -hitTolerance) || (denom > hitTolerance)) {
            float hitDistance = (-plane.d - DotProduct(plane.normal, ray->origin)) / denom;
            if (hitDistance > minHitDistance && hitDistance < intersectionResult->t) {
                intersectionResult->t = hitDistance;
                intersectionResult->hitMetarialIndex = plane.metarialIndex;
                intersectionResult->hitNormal = plane.normal;
            }
        }
    }
    
    for (int sphereIndex = 0; sphereIndex < world->sphereCount; ++sphereIndex) {
        Sphere sphere = world->spheres[sphereIndex];
        Vector3 centerToOrigin = ray->origin - sphere.position;
        float a = DotProduct(ray->direction, ray->direction);
        float b = 2 * DotProduct(ray->direction, centerToOrigin);
        float c = DotProduct(centerToOrigin, centerToOrigin) - (sphere.radius * sphere.radius);
        float discriminant = b * b - 4 * a * c;
        float denom = 2 * a;
        if (discriminant >= 0) {
            float tp = (-b + sqrtf(discriminant)) / denom;
            float tn = (-b - sqrtf(discriminant)) / denom;
            
            float hitDistance = tp;
            if (tn > minHitDistance && tn < tp) {
                hitDistance = tn;
            }
            
            if (hitDistance > minHitDistance && hitDistance < intersectionResult->t) {
                intersectionResult->t = hitDistance;
                intersectionResult->hitMetarialIndex = sphere.metarialIndex;
                
                Vector3 hitPosition = ray->origin + ray->direction * hitDistance;
                intersectionResult->hitNormal = Normalize(hitPosition - sphere.position);
            }
        }
    }
    
    return intersectionResult->t < F32Max;
}

// Checks for word intersection. 
// If there is, iterate through lights and calculate the sum of lights then add to the color.
// This method of light blending not physically correct it all.
// But its okay for our purposes now.
Vector3 RaytraceWorld(World* world, Ray* ray) {
    Vector3 result = world->metarials[0].color;
    
    WorldIntersectionResult intersectionResult = {};
    bool isIntersect = IntersectWorld(world, ray, &intersectionResult);
    
    if (isIntersect) {
        Vector3 finalColor = Vector3(0.0f, 0.0f, 0.0f);
        for (int lightIndex = 0; lightIndex < world->lightCount; ++lightIndex) {
            Light light = world->lights[lightIndex];
            
            Vector3 hitPosition = ray->origin + ray->direction * intersectionResult.t;
            Vector3 lightVector = Normalize(light.position - hitPosition);
            float factor = DotProduct(lightVector, intersectionResult.hitNormal);
            if (factor < 0.0001f) {
                factor = 0.0001f;
            }
            Vector3 color = world->metarials[intersectionResult.hitMetarialIndex].color * factor;
            
            Ray shadowRay = {};
            shadowRay.origin = hitPosition;
            shadowRay.direction = lightVector;
            
            WorldIntersectionResult shadowIntersection = {};
            bool isShadowIntersect = IntersectWorld(world, &shadowRay, &shadowIntersection);
            if (isShadowIntersect) {
                color = color * 0.1f;
            }
            
            finalColor += color;
        }
        
        result = finalColor / world->lightCount;
    }
    
    return result;
}

int main(int argc, char** argv) {
    Image image = CreateImage(1280, 720);
    float imageAspectRatio = (float) image.width / (float) image.height;
    
    // Y is up.
    const Vector3 globalUpVector = Vector3(0.0f, 1.0f, 0.0f);

	// Generate scene
    Plane plane = {};
    plane.normal = globalUpVector;
    plane.d = 0;
    plane.metarialIndex = 1;
    
    Sphere sphere = {};
    sphere.position = Vector3(0.0f, 1.0f, 0.0f);
    sphere.radius = 1.0f;
    sphere.metarialIndex = 2;
    
    Sphere sphere2 = {};
    sphere2.position = Vector3(2.0f, 2.0f, 3.0f);
    sphere2.radius = 1.0f;
    sphere2.metarialIndex = 2;
    
    Sphere sphere3 = {};
    sphere3.position = Vector3(-2.0f, 1.0f, 4.0f);
    sphere3.radius = 1.0f;
    sphere3.metarialIndex = 2;
    
    Sphere spheres[3];
    spheres[0] = sphere;
    spheres[1] = sphere2;
    spheres[2] = sphere3;
    
    Metarial defaultMetarial = {};
    defaultMetarial.color = Vector3(0.1f, 0.1f, 0.1f);
    
    Metarial planeMetarial = {};
    planeMetarial.color = Vector3(1.0f, 0.0f, 0.0f);
    
    Metarial sphereMetarial = {};
    sphereMetarial.color = Vector3(0.0f, 0.0f, 1.0f);
    
    Metarial metarials[3] = {};
    metarials[0] = defaultMetarial;
    metarials[plane.metarialIndex] = planeMetarial;
    metarials[sphere.metarialIndex] = sphereMetarial;
    
    Light light = {};
    light.position = Vector3(2.0f, 3.0f, 8.0f);
    light.color = Vector3(1.0f, 1.0f, 1.0f);
    light.density = 1;
    
    Light light2 = {};
    light2.position = Vector3(-2.0f, 3.0f, 3.0f);
    light2.color = Vector3(1.0f, 1.0f, 1.0f);
    light2.density = 1;
    
    Light light3 = {};
    light3.position = Vector3(-3.0f, 3.0f, 5.0f);
    light3.color = Vector3(1.0f, 1.0f, 1.0f);
    light3.density = 1;
    
    Light lights[3] = {};
    lights[0] = light;
    lights[1] = light2;
    lights[2] = light3;
    
    World world = {};
    world.metarialCount = 2;
    world.metarials = metarials;
    world.planeCount = 1;
    world.planes = &plane;
    world.sphereCount = 3;
    world.spheres = spheres;
    world.lightCount = 3;
    world.lights = lights;
    
    Vector3 cameraPosition = Vector3(0.0f, 1.0f, 10.0f);
    Vector3 cameraZ = Normalize(cameraPosition);
    Vector3 cameraX = Normalize(CrossProduct(globalUpVector, cameraZ));
    Vector3 cameraY = Normalize(CrossProduct(cameraZ, cameraX));
    
    float filmDistance = 1.0;
    Vector3 filmCenter = cameraPosition - cameraZ * filmDistance;
    
    float filmWidth = 1.0f * imageAspectRatio;
    float filmHeight = 1.0f;
    
    float halfFilmWidth = filmWidth * 0.5f;
    float halfFilmHeight = filmHeight * 0.5f;
    
    uint32_t *frameBuffer = image.pixelData;  
    for (int32_t y = 0; y < image.height; ++y) {
        float filmY = ((float) y / (float) image.height) * -2.0f + 1.0f;
        for (int32_t x = 0; x < image.width; ++x) {
            float filmX = (((float) x / (float) image.width) * 2.0f - 1.0f);
            
            Vector3 filmPosition = filmCenter + cameraX * filmX * halfFilmWidth + cameraY * halfFilmHeight * filmY;
            
            Ray ray = {};
            ray.origin = cameraPosition;
            ray.direction = Normalize(filmPosition - cameraPosition);
            
            Vector3 color = RaytraceWorld(&world, &ray);
            
            *frameBuffer++ = RGBPackToUInt32(color);
            
        }
    }
    
    WriteImageFile(&image, "render.bmp");
    FreeImage(&image);
    return 0;
}
