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
    uint32_t hitMaterialIndex;
    Vector3 hitNormal;
};

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

// Main ray trace function.
// I use a loop-based tracing instead of recursion-based trace function.
// You can write clean code by using recursion but I find recursion hard to understand.
// This way is more straightforward and understandable for me.
Vector3 RaytraceWorld(World* world, Ray* ray) {
    Vector3 result(0.0f, 0.0f, 0.0f);

    Ray bounceRay = {};
    bounceRay.origin = ray->origin;
    bounceRay.direction = ray->direction;

    Vector3 attenuation(1.0f, 1.0f, 1.0f);
    for (uint32_t bounceIndex = 0; bounceIndex < 8; ++bounceIndex) {    
	WorldIntersectionResult intersectionResult = {};
	bool isIntersect = IntersectWorld(world, &bounceRay, &intersectionResult);


	if (isIntersect) {
	    Material mat = world->materials[intersectionResult.hitMaterialIndex];

	    result = attenuation;
	    attenuation *= mat.color;
	    bounceRay.origin = bounceRay.origin + bounceRay.direction * intersectionResult.t;

	    // Fresnel coefficient is between 0 and 1. We start with 1 which is full reflection, no refraction.
	    float fresnelCoefficient = 1.0;
	    Vector3 refractedRay;
	    bool isRefract = Refract(bounceRay.direction, intersectionResult.hitNormal,
	    			     mat.refractiveIndex, &refractedRay);

	    if (mat.refractiveIndex != 0.0f && isRefract) {
		// Refractive material
	    	refractedRay = Normalize(refractedRay);

		// We use the Schlick Approximation for getting fresnel coefficient. It's okay for our purposes.
		// NOTE: We can use actual Fresnel Equations for making the image little bit more realistic.
		fresnelCoefficient = Schlick(bounceRay.direction, intersectionResult.hitNormal,
				      mat.refractiveIndex);
	    }
	    
	    Vector3 mirrorBounce = bounceRay.direction - intersectionResult.hitNormal *
		DotProduct(intersectionResult.hitNormal, bounceRay.direction) * 2.0f;
	    Vector3 randomBounce = intersectionResult.hitNormal +
		Vector3(RandomBilateral(), RandomBilateral(), RandomBilateral());
	    Vector3 reflectedRay = Normalize(Lerp(randomBounce, mat.reflection, mirrorBounce));

	    // We use the Russian Roulette method for determining which way to go. It fits our architecture.
	    // We might do calculate reflected and refracted ray separately and apply linear interpolation
	    // between them by coefficient given from the Fresnel Equations.
	    if (RandomUnilateral() < fresnelCoefficient) {
		bounceRay.direction = reflectedRay;
	    } else {
		bounceRay.direction = refractedRay;
	    }
	    	
	    
	} else {
	    // Hit nothing (sky)
	    // We just return attenuation for now. No sky color or sky emmiter.
	    result = attenuation;
	    break;
	}
	
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
    plane.materialIndex = 1;
    
    Sphere sphere = {};
    sphere.position = Vector3(0.0f, 1.0f, 0.0f);
    sphere.radius = 1.0f;
    sphere.materialIndex = 2;

    Sphere sphere2 = {};
    sphere2.position = Vector3(-2.0f, 1.0f, 0.0f);
    sphere2.radius = 1.0f;
    sphere2.materialIndex = 4;

    Sphere sphere3 = {};
    sphere3.position = Vector3(-4.0f, 2.0f, 1.0f);
    sphere3.radius = 1.0f;
    sphere3.materialIndex = 3;
    
    Sphere sphere4 = {};
    sphere4.position = Vector3(2.0f, 1.0f, -1.0f);
    sphere4.radius = 1.0f;
    sphere4.materialIndex = 5;
    
    Sphere spheres[4];
    spheres[0] = sphere;
    spheres[1] = sphere2;
    spheres[2] = sphere3;
    spheres[3] = sphere4;
    
    Material defaultMaterial = {};
    defaultMaterial.color = Vector3(1.0f, 1.0f, 1.0f);
    
    Material planeMaterial = {};
    planeMaterial.color = Vector3(0.8f, 0.8f, 0.0f);
    
    Material sphereMaterial = {};
    sphereMaterial.color = Vector3(0.8f, 0.3f, 0.3f);

    Material sphere2Material = {};
    sphere2Material.color = Vector3(1.0f, 1.0f, 1.0f) * 0.9f;
    sphere2Material.reflection = 1.0f;

    Material sphere3Material = {};
    sphere3Material.color = Vector3(0.8f, 0.6f, 0.2f);
    sphere3Material.reflection = 0.9f;

    Material sphere4Material = {};
    sphere4Material.color = Vector3(1.0f, 1.0f, 1.0f) * 0.9f;
    sphere4Material.refractiveIndex = 1.5f;
    sphere4Material.reflection = 1.0f;
    
    Material materials[6] = {};
    materials[0] = defaultMaterial;
    materials[plane.materialIndex] = planeMaterial;
    materials[sphere.materialIndex] = sphereMaterial;
    materials[sphere2.materialIndex] = sphere2Material;
    materials[sphere3.materialIndex] = sphere3Material;
    materials[sphere4.materialIndex] = sphere4Material;
    
    World world = {};
    world.materialCount = 6;
    world.materials = materials;
    world.planeCount = 1;
    world.planes = &plane;
    world.sphereCount = 4;
    world.spheres = spheres;
    
    Vector3 cameraPosition = Vector3(0.0f, 3.0f, 10.0f);
    Vector3 cameraZ = Normalize(cameraPosition);
    Vector3 cameraX = Normalize(CrossProduct(globalUpVector, cameraZ));
    Vector3 cameraY = Normalize(CrossProduct(cameraZ, cameraX));
    
    float filmDistance = 1.0;
    Vector3 filmCenter = cameraPosition - cameraZ * filmDistance;
    
    float filmWidth = 1.0f * imageAspectRatio;
    float filmHeight = 1.0f;
    
    float halfFilmWidth = filmWidth * 0.5f;
    float halfFilmHeight = filmHeight * 0.5f;

    float halfPixelWidth = 0.5f / image.width;
    float halfPixelHeight = 0.5f / image.height;

    uint32_t sampleSize = 32;
    uint32_t *frameBuffer = image.pixelData;  
    for (int32_t y = 0; y < image.height; ++y) {
        float filmY = ((float) y / (float) image.height) * -2.0f + 1.0f;
        for (int32_t x = 0; x < image.width; ++x) {
            float filmX = (((float) x / (float) image.width) * 2.0f - 1.0f);

	    Vector3 color(0.0f, 0.0f, 0.0f);
	    for (uint32_t sampleIndex = 0; sampleIndex < sampleSize; ++sampleIndex) {
		float offsetX = filmX + RandomBilateral() * halfPixelWidth;
		float offsetY = filmY + RandomBilateral() * halfPixelHeight;
            
		Vector3 filmPosition = filmCenter + cameraX * offsetX * halfFilmWidth + cameraY * halfFilmHeight * offsetY;
		
		Ray ray = {};
		ray.origin = cameraPosition;
		ray.direction = Normalize(filmPosition - cameraPosition);
            
		color += RaytraceWorld(&world, &ray);
	    }
            
            *frameBuffer++ = RGBPackToUInt32WithGamma2(color / sampleSize);
            
        }
    }
    
    WriteImageFile(&image, "render.bmp");
    FreeImage(&image);
    return 0;
}
