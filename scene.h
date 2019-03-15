#ifndef _SCENE_H_
#define _SCENE_H_

#include "math_util.h"

struct Material {
    float refractiveIndex; // Refractive index of material. 0 means no refraction.
    float reflection; // 0 is pure diffuse, 1 is mirror.
    Vector3 color;
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

struct World {
    uint32_t materialCount;
    Material* materials;
    uint32_t sphereCount;
    Sphere* spheres;
    uint32_t planeCount;
    Plane* planes;
    Camera* camera;
};

Camera CreateCamera(Vector3 cameraPosition) {
    Vector3 cameraZ = Normalize(cameraPosition);
    Vector3 cameraX = Normalize(CrossProduct(Vector3(0.0f, 1.0f, 0.0f), cameraZ));
    Vector3 cameraY = Normalize(CrossProduct(cameraZ, cameraX));

    Camera result = {};
    result.position = cameraPosition;
    result.zVec = cameraZ;
    result.yVec = cameraY;
    result.xVec = cameraX;

    return result;
} 

#endif
