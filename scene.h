#ifndef _SCENE_H_
#define _SCENE_H_

#include "math_util.h"
#include "simd.h"

#if defined(PLATFORM_WIN32) && defined(WIN32_GPU)
#define ALIGN_GPU __declspec(align(16))
#else
#define ALIGN_GPU 
#endif // PLATFORM_WIN32

ALIGN_GPU struct Material {
    Vector3 color{ 0.0f, 0.0f, 0.0f };
    float refractiveIndex; // Refractive index of material. 0 means no refraction.
    Vector3 emitColor{ 0.0f, 0.0f, 0.0f };
    float reflection; // 0 is pure diffuse, 1 is mirror.
};

ALIGN_GPU struct Sphere {
    Vector3 position;
    float radius;
    uint32_t materialIndex;
};

struct SphereSoALane {
    LaneVector3 position;
    LaneF32 radiusSquared;
    LaneF32 materialIndex;
};

ALIGN_GPU struct Plane {
    Vector3 normal;
    float d;
    uint32_t materialIndex;
};

struct Camera {
    Vector3 position;
    Vector3 zVec;
    Vector3 yVec;
    Vector3 xVec;

    Camera(Vector3 cameraPosition) {
        position = cameraPosition;
        zVec = Normalize(cameraPosition);
        xVec = Normalize(CrossProduct(Vector3(0.0f, 1.0f, 0.0f), zVec));
        yVec = Normalize(CrossProduct(zVec, xVec));
    }
};

struct World {
    uint32_t materialCount;
    Material* materials;
    uint32_t sphereCount;
    Sphere* spheres;
    uint32_t sphereSoAArrayCount;
    SphereSoALane* sphereSoAArray;
    uint32_t planeCount;
    Plane* planes;
    Camera* camera;
};

World* createScene() {
    // Y is up.
    Vector3 globalUpVector = Vector3(0.0f, 1.0f, 0.0f);

    // Generate scene
    Plane* plane = new Plane;
    plane->normal = globalUpVector;
    plane->d = 0;
    plane->materialIndex = 1;

    Sphere sphere = {};
    sphere.position = Vector3(0.0f, 1.0f, 0.0f);
    sphere.radius = 1.0f;
    sphere.materialIndex = 2;

    Sphere sphere2 = {};
    sphere2.position = Vector3(-2.0f, 1.0f, 0.0f);
    sphere2.radius = 1.0f;
    sphere2.materialIndex = 3;

    Sphere sphere3 = {};
    sphere3.position = Vector3(-4.0f, 2.0f, 1.0f);
    sphere3.radius = 1.0f;
    sphere3.materialIndex = 4;

    Sphere sphere4 = {};
    sphere4.position = Vector3(2.0f, 1.0f, -1.0f);
    sphere4.radius = 1.0f;
    sphere4.materialIndex = 5;

    Sphere sphere5 = {};
    sphere5.position = Vector3(-2.0f, 1.0f, 5.0f);
    sphere5.radius = 1.0f;
    sphere5.materialIndex = 2;

    Sphere sphere6 = {};
    sphere6.position = Vector3(5.0f, 2.0f, -6.0f);
    sphere6.radius = 2.0f;
    sphere6.materialIndex = 3;

    Sphere sphere7 = {};
    sphere7.position = Vector3(-4.0f, 4.0f, 5.0f);
    sphere7.radius = 1.0f;
    sphere7.materialIndex = 6;

    Sphere sphere8 = {};
    sphere8.position = Vector3(2.0f, 1.0f, 4.0f);
    sphere8.radius = 1.0f;
    sphere8.materialIndex = 5;

    const uint32_t sphereCount = 8;
    Sphere* spheres = new Sphere[sphereCount];
    spheres[0] = sphere;
    spheres[1] = sphere2;
    spheres[2] = sphere3;
    spheres[3] = sphere4;
    spheres[4] = sphere5;
    spheres[5] = sphere6;
    spheres[6] = sphere7;
    spheres[7] = sphere8;


    // We use AoSoA layout for sphere data. fixed simd-lane size arrays of each member.
    const uint32_t sphereSoAArrayCount = (sphereCount + LANE_WIDTH - 1) / LANE_WIDTH;
    SphereSoALane* sphereSoAArray = new SphereSoALane[sphereSoAArrayCount];

    for (int i = 0; i < sphereSoAArrayCount; ++i) {
        ALIGN_LANE float spheresPositionX[LANE_WIDTH];
        ALIGN_LANE float spheresPositionY[LANE_WIDTH];
        ALIGN_LANE float spheresPositionZ[LANE_WIDTH];
        ALIGN_LANE float spheresRadiusSquared[LANE_WIDTH];
        ALIGN_LANE float spheresMaterialIndex[LANE_WIDTH];

        uint32_t remainingSpheres = sphereCount - i * LANE_WIDTH;
        uint32_t len = (remainingSpheres / LANE_WIDTH) > 0 ? LANE_WIDTH : remainingSpheres % LANE_WIDTH;
        for (int j = 0; j < len; ++j) {
            uint32_t sphereIndex = j + i * LANE_WIDTH;
            Sphere s = spheres[sphereIndex];
            spheresPositionX[j] = s.position.x;
            spheresPositionY[j] = s.position.y;
            spheresPositionZ[j] = s.position.z;
            spheresRadiusSquared[j] = s.radius * s.radius;
            spheresMaterialIndex[j] = s.materialIndex;
        }

        SphereSoALane sphereSoA = {};
        sphereSoA.position = LaneVector3(LaneF32(spheresPositionX),
            LaneF32(spheresPositionY),
            LaneF32(spheresPositionZ));
        sphereSoA.radiusSquared = LaneF32(spheresRadiusSquared);
        sphereSoA.materialIndex = LaneF32(spheresMaterialIndex);

        sphereSoAArray[i] = sphereSoA;
    }

    Material defaultMaterial = {};
    defaultMaterial.emitColor = Vector3(0.1f, 0.2f, 0.4f);

    Material planeMaterial = {};
    planeMaterial.color = Vector3(0.8f, 0.8f, 0.8f);

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

    Material sphere5Material = {};
    sphere5Material.emitColor = Vector3(20.0f, 15.0f, 10.0f);

    Material blueLightMaterial = {};
    blueLightMaterial.emitColor = Vector3(1.0f, 1.0f, 18.0f);

    uint32_t materialCount = 8;
    Material* materials = new Material[materialCount];
    materials[0] = defaultMaterial;
    materials[1] = planeMaterial;
    materials[2] = sphereMaterial;
    materials[3] = sphere2Material;
    materials[4] = sphere3Material;
    materials[5] = sphere4Material;
    materials[6] = sphere5Material;
    materials[7] = blueLightMaterial;


    Camera* camera = new Camera(Vector3(0.0f, 4.0f, 10.0f));

    // I used raw pointers for scene objects. Freeing heap memory is callers responsibilty.
    // TODO: I should use smart pointers for scene objects but I don't want to do that now. 
    // Memory automatically will be freed after program terminated.
    World* world = new World;
    world->materialCount = materialCount;
    world->materials = materials;
    world->planeCount = 1;
    world->planes = plane;
    world->sphereCount = sphereCount;
    world->sphereSoAArrayCount = sphereSoAArrayCount;
    world->sphereSoAArray = sphereSoAArray;
    world->spheres = spheres;
    world->camera = camera;

    return world;
}

#endif
