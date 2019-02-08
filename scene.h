#ifndef _SCENE_H_
#define _SCENE_H_

#include "math_util.h"

struct Material {
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

struct World {
    uint32_t materialCount;
    Material* materials;
    uint32_t sphereCount;
    Sphere* spheres;
    uint32_t planeCount;
    Plane* planes;
};

#endif
