#ifndef _SCENE_H_
#define _SCENE_H_

#include "math_util.h"

struct Metarial {
    Vector3 color;
};

struct Sphere {
    Vector3 position;
    float radius;
    uint32_t metarialIndex;
};

struct Plane {
    Vector3 normal;
    float d;
    uint32_t metarialIndex;
};

struct Light {
    Vector3 position;
    Vector3 color;
    float density = 1;
};

struct World {
    uint32_t metarialCount;
    Metarial* metarials;
    uint32_t sphereCount;
    Sphere* spheres;
    uint32_t planeCount;
    Plane* planes;
    uint32_t lightCount;
    Light* lights;
};

#endif
