#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "platform.h"
#include "math_util.h"
#include "simd.h"

#include "scene.h"

#include "image.cpp"

struct Ray {
    Vector3 origin;
    Vector3 direction;
};

struct WorldIntersectionResult {
    float t = F32Max;
    uint32_t hitMaterialIndex;
    Vector3 hitNormal;
};

inline
bool IntersectWorldWide(World* world, Ray* ray, WorldIntersectionResult* intersectionResult) {
    float hitTolerance = 0.001;
    float minHitDistance = 0.001;

    float closestHitDistance = F32Max;
    uint32_t hitMaterialIndex = 0;
    Vector3 hitNormal;
    bool anyHit = false;

    // We have only 1 plane in our scene. So we are calculate plane intersection in scalar.
    for (int planeIndex = 0; planeIndex < world->planeCount; ++planeIndex) {
        Plane plane = world->planes[planeIndex];
        
        float denom = DotProduct(plane.normal, ray->direction);
        if ((denom < -hitTolerance) || (denom > hitTolerance)) {
            float hitDistance = (-plane.d - DotProduct(plane.normal, ray->origin)) / denom;
            if (hitDistance > minHitDistance && hitDistance < closestHitDistance) {
                closestHitDistance = hitDistance;
                hitMaterialIndex = plane.materialIndex;
                hitNormal = plane.normal;
                anyHit = true;
            }
        }
    }

    // Broadcast scalar values into lanes.
    LaneVector3 rayOrigin(ray->origin);
    LaneVector3 rayDirection(ray->direction);

    LaneF32 closestHitDistanceLane = LaneF32(closestHitDistance);
    LaneF32 hitMaterialIndexLane = LaneF32(hitMaterialIndex);
    LaneVector3 hitNormalLane = LaneVector3(hitNormal);

    for (int sphereLaneIndex = 0; sphereLaneIndex < world->sphereSoAArrayCount; ++sphereLaneIndex) {	
        SphereSoALane sphereSoA = world->sphereSoAArray[sphereLaneIndex];
    
        LaneVector3 centerToOrigin = rayOrigin - sphereSoA.position;
        LaneF32 a = DotProduct(rayDirection, rayDirection);
        LaneF32 b = 2.0f * DotProduct(rayDirection, centerToOrigin);
        LaneF32 c = DotProduct(centerToOrigin, centerToOrigin) - (sphereSoA.radiusSquared);
        LaneF32 discriminant = FMulSub(b, b, 4.0f * a * c); //b * b - 4.0f * a * c;
        LaneF32 denom = 2.0f * a;
    
        LaneF32 squareRootMask = discriminant > 0.0f;
        if (!MaskIsZeroed(squareRootMask)) {
            LaneF32 tp = (-b + SquareRoot(discriminant)) / denom;
            LaneF32 tn = (-b - SquareRoot(discriminant)) / denom;
            
            LaneF32 hitDistance = tp;
            LaneF32 pickMask = (tn > minHitDistance & tn < tp);
            Select(&hitDistance, pickMask, tn);   
            
            LaneF32 tMask = (hitDistance > minHitDistance & hitDistance < closestHitDistanceLane);
            LaneF32 hitMask = (squareRootMask & tMask);

            if (!MaskIsZeroed(hitMask)) {	
                Select(&closestHitDistanceLane, hitMask, hitDistance);
                Select(&hitMaterialIndexLane, hitMask, sphereSoA.materialIndex);
            
                LaneVector3 hitPosition = FMulAdd(rayDirection, hitDistance, rayOrigin);
                Select(&hitNormalLane, hitMask, Normalize(hitPosition - sphereSoA.position));
                anyHit = true;
            }
        }
    }

    if (anyHit) {
        // After calculating n sphere ray intersection, we have to find which one is closer.
        // This is probably the most naive way to do it. We store lane values into an array and iterating through to find any close hit. 
        ALIGN_LANE float closestHitDistanceLaneUnpacked[LANE_WIDTH];
        ALIGN_LANE float hitMaterialIndexLaneUnpacked[LANE_WIDTH];
        ALIGN_LANE float hitNormalLaneXUnpacked[LANE_WIDTH];
        ALIGN_LANE float hitNormalLaneYUnpacked[LANE_WIDTH];
        ALIGN_LANE float hitNormalLaneZUnpacked[LANE_WIDTH];
        StoreLane(closestHitDistanceLaneUnpacked, closestHitDistanceLane);
        StoreLane(hitMaterialIndexLaneUnpacked, hitMaterialIndexLane);
        StoreLane(hitNormalLaneXUnpacked, hitNormalLane.x);
        StoreLane(hitNormalLaneYUnpacked, hitNormalLane.y);
        StoreLane(hitNormalLaneZUnpacked, hitNormalLane.z);
        for (int i = 0; i < LANE_WIDTH; ++i) {
            float t = closestHitDistanceLaneUnpacked[i];
            if (t < closestHitDistance) {
                closestHitDistance = t;
                hitMaterialIndex = hitMaterialIndexLaneUnpacked[i];
                hitNormal = Vector3(hitNormalLaneXUnpacked[i],
                                    hitNormalLaneYUnpacked[i],
                                    hitNormalLaneZUnpacked[i]);
            }
        }
    }

    // Pz = Oz + Dz * t
    // Pz is fixed z component of one of the vectors in rectangle struct
    // t = (Pz - Oz) / Dz
    for (uint32_t rectangleIndex = 0; rectangleIndex < world->rectangleCount; ++rectangleIndex) {
        RectangleXY* rect = world->rectangles + rectangleIndex;

        Vector3 minPoint = rectDefaultMinPoint;
        Vector3 maxPoint = rectDefaultMaxPoint;
        float rectZ = minPoint.z;

        Matrix4 transformMatrix = rect->translateMatrix * rect->rotationMatrix * rect->scaleMatrix;
        Matrix4 rayMatrix = Inverse(transformMatrix);
        Vector3 rayOrigin = (rayMatrix * Vector4(ray->origin, 1.0f)).xyz();
        Vector3 rayDirection = (rayMatrix * Vector4(ray->direction, 0.0f)).xyz();

        float t = (rectZ - rayOrigin.z) / rayDirection.z;
        Vector3 hitPoint = rayOrigin + rayDirection * t;

        bool hit = hitPoint.x <= maxPoint.x && hitPoint.x >= minPoint.x &&
            hitPoint.y <= maxPoint.y && hitPoint.y >= minPoint.y;
        if (hit && t < closestHitDistance && t > minHitDistance) {
            closestHitDistance = t;
            hitMaterialIndex = rect->materialIndex;
            Vector3 positiveZ = (rect->rotationMatrix * Vector4(0.0f, 0.0f, 1.0f, 0.0f)).xyz();
            // Check for incident ray direction vector direction
            // If it's coming to back side of rectangle
            // Flip the normal vector
            float dot = DotProduct(positiveZ, ray->direction);
            if (dot > 0) {
                hitNormal = -positiveZ;
            } else {
                hitNormal = positiveZ;
            }
            anyHit = true;
        }
    }

    intersectionResult->t = closestHitDistance;
    intersectionResult->hitMaterialIndex = hitMaterialIndex;
    intersectionResult->hitNormal = hitNormal;

    return anyHit;
}

bool
IntersectWorld(World* world, Ray* ray, WorldIntersectionResult* intersectionResult) {
    float hitTolerance = 0.001;
    float minHitDistance = 0.001;
    
    for (int planeIndex = 0; planeIndex < world->planeCount; ++planeIndex) {
        Plane plane = world->planes[planeIndex];
        
        float denom = DotProduct(plane.normal, ray->direction);
        if ((denom < -hitTolerance) || (denom > hitTolerance)) {
            float hitDistance = (-plane.d - DotProduct(plane.normal, ray->origin)) / denom;
            if (hitDistance > minHitDistance && hitDistance < intersectionResult->t) {
                intersectionResult->t = hitDistance;
                intersectionResult->hitMaterialIndex = plane.materialIndex;
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
        if (discriminant > 0) {
            float tp = (-b + sqrtf(discriminant)) / denom;
            float tn = (-b - sqrtf(discriminant)) / denom;

            float hitDistance = tp;
            if (tn > minHitDistance && tn < tp) {
                hitDistance = tn;
            }

            if (hitDistance > minHitDistance && hitDistance < intersectionResult->t) {
                intersectionResult->t = hitDistance;
                intersectionResult->hitMaterialIndex = sphere.materialIndex;

                Vector3 hitPosition = ray->origin + ray->direction * hitDistance;
                intersectionResult->hitNormal = Normalize(hitPosition - sphere.position);
            }
        }
    }
    
    return intersectionResult->t < F32Max;
}

struct WorkOrder {
    Image* image;
    World* world;
    uint32_t startRowIndex;
    uint32_t endRowIndex;
    uint32_t sampleSize;
};

struct WorkQueue {
    uint32_t workOrderCount;
    WorkOrder* workOrders;

    volatile uint64_t nextOrderToDo;
    volatile uint64_t finishedOrderCount;
    volatile uint64_t totalBouncesComputed;
};

// Main ray trace function.
// I use a loop-based tracing instead of recursion-based trace function.
// You can write clean code by using recursion but I find recursion hard to understand.
// This way is more straightforward and understandable for me.
Vector3 RaytraceWorld(World* world, Ray* ray, uint32_t* randomState, WorkQueue* workQueue, uint64_t* bounceCount) {
    Vector3 result(0.0f, 0.0f, 0.0f);

    Ray bounceRay = {};
    bounceRay.origin = ray->origin;
    bounceRay.direction = ray->direction;

    uint64_t bouncesComputed = 0;

    Vector3 attenuation(1.0f, 1.0f, 1.0f);
    for (uint32_t bounceIndex = 0; bounceIndex < 8; ++bounceIndex) {    
        WorldIntersectionResult intersectionResult = {};
        bool isIntersect = IntersectWorldWide(world, &bounceRay, &intersectionResult);
        ++bouncesComputed;

        Material mat = world->materials[intersectionResult.hitMaterialIndex];
        if (isIntersect) {

            //result = attenuation;
            //attenuation *= mat.color;
            result += attenuation * mat.emitColor;
            attenuation *= mat.color;
            bounceRay.origin = bounceRay.origin + bounceRay.direction * intersectionResult.t;
        
            Vector3 mirrorBounce = bounceRay.direction - intersectionResult.hitNormal *
            DotProduct(intersectionResult.hitNormal, bounceRay.direction) * 2.0f;
            Vector3 randomBounce = intersectionResult.hitNormal +
            Vector3(RandomBilateral(randomState),
                RandomBilateral(randomState),
                RandomBilateral(randomState));
            Vector3 reflectedRay = Normalize(Lerp(randomBounce, mat.reflection, mirrorBounce));

             // Fresnel coefficient is between 0 and 1. We start with 1 which is full reflection, no refraction.
            float fresnelCoefficient = 1.0;
            Vector3 refractedRay = reflectedRay;

            if (mat.refractiveIndex != 0.0f) {
                bool isRefract = Refract(bounceRay.direction, intersectionResult.hitNormal,
                                 mat.refractiveIndex, &refractedRay);
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
            //result = attenuation;
            break;
        }
    }

    *bounceCount += bouncesComputed;
    return result;
}

bool RaytraceWork(WorkQueue* workQueue) {

    uint32_t nextOrderToDo = InterlockedAddAndReturnPrevious(&workQueue->nextOrderToDo, 1);
    if (nextOrderToDo >= workQueue->workOrderCount) {
        return false;
    }

    WorkOrder workOrder = workQueue->workOrders[nextOrderToDo];
    Image* image = workOrder.image;
    World* world = workOrder.world;
    uint32_t startRowIndex = workOrder.startRowIndex;
    uint32_t endRowIndex = workOrder.endRowIndex;
    uint32_t sampleSize = workOrder.sampleSize;

    float imageAspectRatio = (float) image->width / (float) image->height;

    Camera* camera = world->camera;
    Vector3 cameraPosition = camera->position;
    Vector3 cameraZ = camera->zVec;
    Vector3 cameraX = camera->xVec;
    Vector3 cameraY = camera->yVec;

    float filmDistance = 1.0;
    Vector3 filmCenter = cameraPosition - cameraZ * filmDistance;

    float filmWidth = 1.0f * imageAspectRatio;
    float filmHeight = 1.0f;
    
    float halfFilmWidth = filmWidth * 0.5f;
    float halfFilmHeight = filmHeight * 0.5f;

    float halfPixelWidth = 0.5f / image->width;
    float halfPixelHeight = 0.5f / image->height;

    uint32_t randomState = 262346 * (startRowIndex + endRowIndex * 36);
    uint64_t totalBounces = 0;
    
    uint32_t* frameBuffer = image->pixelData + (startRowIndex * image->width);
    for (int32_t y = startRowIndex; y < endRowIndex; ++y) {
        float filmY = ((float) y / (float) image->height) * -2.0f + 1.0f;
        for (int32_t x = 0; x < image->width; ++x) {
            float filmX = (((float) x / (float) image->width) * 2.0f - 1.0f);
        
            Vector3 color(0.0f, 0.0f, 0.0f);
            for (uint32_t sampleIndex = 0; sampleIndex < sampleSize; ++sampleIndex) {
                float offsetX = filmX + RandomBilateral(&randomState) * halfPixelWidth;
                float offsetY = filmY + RandomBilateral(&randomState) * halfPixelHeight;
        
                Vector3 filmPosition = filmCenter + cameraX * offsetX * halfFilmWidth + cameraY * halfFilmHeight * offsetY;
        
                Ray ray = {};
                ray.origin = cameraPosition;
                ray.direction = Normalize(filmPosition - cameraPosition);

                color += RaytraceWorld(world, &ray, &randomState, workQueue, &totalBounces);
            }
            
            *frameBuffer++ = RGBPackToUInt32WithsRGB(color / sampleSize);
        }
    }

    InterlockedAddAndReturnPrevious(&workQueue->totalBouncesComputed, totalBounces);
    InterlockedAddAndReturnPrevious(&workQueue->finishedOrderCount, 1);
    return true;
}

THREAD_PROC_RET ThreadProc(void* arguments) {
    WorkQueue* workQueue = (WorkQueue*) arguments;
    while (RaytraceWork(workQueue));
    
    return 0;
}

int main(int argc, char** argv) {
    Image image = CreateImage(1280, 720);

    World* world = CreateCornellBoxScene();

    uint64_t startClock = GetTimeMilliseconds();
    const uint32_t sampleSize = 16;

#if SINGLE_THREAD
    uint32_t totalWorkOrderCount = 1;
    WorkQueue workQueue = {};
    workQueue.workOrders = (WorkOrder*) malloc(totalWorkOrderCount * sizeof(WorkOrder));
    workQueue.workOrderCount = totalWorkOrderCount;

    WorkOrder* workOrder = workQueue.workOrders;
    workOrder->image = &image;
    workOrder->world = world;
    workOrder->startRowIndex = 0;
    workOrder->endRowIndex = image.height;
    workOrder->sampleSize = sampleSize;

    RaytraceWork(&workQueue);
    
#else
    uint32_t stride = 1;
    uint32_t totalWorkOrderCount = image.height / stride;
    WorkQueue workQueue = {};
    workQueue.workOrders = (WorkOrder*) malloc(totalWorkOrderCount * sizeof(WorkOrder));
    workQueue.workOrderCount = totalWorkOrderCount;

    uint32_t counter = 0;
    for (uint32_t rowIndex = 0; rowIndex < totalWorkOrderCount; ++rowIndex) {
        WorkOrder* workOrder = workQueue.workOrders + rowIndex;
        workOrder->image = &image;
        workOrder->world = world;
        workOrder->startRowIndex = counter;
        counter += stride;
        workOrder->endRowIndex = counter;
        workOrder->sampleSize = sampleSize;
    }

    uint32_t threadCount = GetNumberOfProcessors();
    for (uint32_t threadIndex = 1; threadIndex < threadCount; ++threadIndex) {
        Thread thread = CreateWorkThread(ThreadProc, &workQueue);
        CloseThreadHandle(thread);
    }

    while (workQueue.finishedOrderCount < totalWorkOrderCount) {
        if (RaytraceWork(&workQueue)) {
            fprintf(stdout, "Raytracing %.0f%%...\r", 100 * ((float) workQueue.finishedOrderCount / totalWorkOrderCount));
            fflush(stdout);
        }
    }
#endif

    uint64_t endClock =  GetTimeMilliseconds();
    
    uint64_t timeElapsedMs = endClock - startClock;
    uint64_t bouncesComputed = workQueue.totalBouncesComputed;
    printf("Raytracing time: %llums\n", timeElapsedMs);
    printf("Total computed rays: %llu\n", bouncesComputed);
    printf("Performance: %.1fMray/s, %fms/ray\n", (bouncesComputed / 1000.0) / timeElapsedMs,
       (double) timeElapsedMs / (double) bouncesComputed);
    
    WriteImageFile(&image, "render.bmp");
    return 0;
}
