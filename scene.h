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

// XY plane axis-aligned rectangle
// RULE: Default Rectangle are 2 unit square and it's center point always on the origin.
// Use transform matrices to manupilate position, scale and orientation.
static const Vector3 rectDefaultMinPoint{ -1.0f, -1.0f, 0.0f };
static const Vector3 rectDefaultMaxPoint{  1.0f,  1.0f, 0.0f };
struct RectangleXY {
    Matrix4 scaleMatrix;
    Matrix4 translateMatrix;
    Matrix4 rotationMatrix;
    uint32_t materialIndex;
};

static RectangleXY CreateRectangle(Vector3 position, Vector3 scale, uint32_t materialIndex) {
    RectangleXY result = {};
    Matrix4 scaleMatrixRect = IdentityMatrix;
    Matrix4 translateMatrixRect = IdentityMatrix;
    Matrix4 rotateMatrix = IdentityMatrix;

    ScaleMatrix(scaleMatrixRect, scale);
    TranslateMatrix(translateMatrixRect, position);

    result.scaleMatrix = scaleMatrixRect;
    result.translateMatrix = translateMatrixRect;
    result.rotationMatrix = rotateMatrix;
    result.materialIndex = materialIndex;

    return result;
}

struct Box {
    RectangleXY rectangles[6];
};

static Box CreateBox(Vector3 position, Vector3 scale, uint32_t materialIndex) {
    Box result = {};

    Vector3 topRectPosition = position;
    topRectPosition.y += scale.y;
    RectangleXY topRect = CreateRectangle(topRectPosition, Vector3(scale.x, scale.z, 1.0f), materialIndex);
    RotateMatrixXAxis(topRect.rotationMatrix, -HALF_PI);

    Vector3 bottomRectPosition = position;
    bottomRectPosition.y -= scale.y;
    RectangleXY bottomRect = CreateRectangle(bottomRectPosition, Vector3(scale.x, scale.z, 1.0f), materialIndex);
    RotateMatrixXAxis(bottomRect.rotationMatrix, -HALF_PI);

    Vector3 rightRectPosition = position;
    rightRectPosition.x += scale.x;
    RectangleXY rightRect = CreateRectangle(rightRectPosition, Vector3(scale.z, scale.y, 1.0f), materialIndex);
    RotateMatrixYAxis(rightRect.rotationMatrix, -HALF_PI);

    Vector3 leftRectPosition = position;
    leftRectPosition.x -= scale.x;
    RectangleXY leftRect = CreateRectangle(leftRectPosition, Vector3(scale.z, scale.y, 1.0f), materialIndex);
    RotateMatrixYAxis(leftRect.rotationMatrix, HALF_PI);

    Vector3 backRectPosition = position;
    backRectPosition.z -= scale.z;
    RectangleXY backRect = CreateRectangle(backRectPosition, Vector3(scale.x, scale.y, 1.0f), materialIndex);

    Vector3 frontRectPosition = position;
    frontRectPosition.z += scale.z;
    RectangleXY frontRect = CreateRectangle(frontRectPosition, Vector3(scale.x, scale.y, 1.0f), materialIndex);

    result.rectangles[0] = frontRect;
    result.rectangles[1] = backRect;
    result.rectangles[2] = rightRect;
    result.rectangles[3] = leftRect;
    result.rectangles[4] = topRect;
    result.rectangles[5] = bottomRect;

    return result;
}

static void RotateBoxZAxis(Box* box, float angle) {
    for (uint32_t rectangleIndex = 0; rectangleIndex < 4; ++rectangleIndex) {
        RectangleXY* rect = box->rectangles + rectangleIndex;
        Matrix4 newRotationMatrix = IdentityMatrix;
        RotateMatrixZAxis(newRotationMatrix, angle);
        rect->rotationMatrix = newRotationMatrix * rect->rotationMatrix;
    }
}

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
    uint32_t rectangleCount;
    RectangleXY* rectangles;
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
    //    defaultMaterial.emitColor = Vector3(0.1f, 0.2f, 0.4f);

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
    sphere5Material.emitColor = Vector3(20.0f, 15.0f, 10.0f) * 0.2f;

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

    // Initialize rectangles
    RectangleXY* rect = new RectangleXY;
    Matrix4 scaleMatrixRect = IdentityMatrix;
    Matrix4 translateMatrixRect = IdentityMatrix;
    Matrix4 rotateMatrix = IdentityMatrix;

    ScaleMatrix(scaleMatrixRect, Vector3(1.0f, 0.5f, 0.0f));
    TranslateMatrix(translateMatrixRect, Vector3(-1.0f, 1.0f, 2.5f));
    RotateMatrixXAxis(rotateMatrix, -3.14f * 0.5f);
    
    rect->scaleMatrix = scaleMatrixRect;
    rect->translateMatrix = translateMatrixRect;
    rect->rotationMatrix = rotateMatrix;

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
    world->rectangleCount = 1;
    world->rectangles = rect;
    world->camera = camera;

    return world;
}

World* CreateCornellBoxScene() {
    Vector3 globalUpVector = Vector3(0.0f, 1.0f, 0.0f);

    Material defaultMaterial = {};
    //defaultMaterial.emitColor = Vector3(0.1f, 0.2f, 0.4f);

    Material whiteDiffuseMaterial = {};
    whiteDiffuseMaterial.color = Vector3(0.73f, 0.73f, 0.73f);

    Material greenDiffuseMaterial = {};
    greenDiffuseMaterial.color = Vector3(0.12f, 0.45f, 0.15f);

    Material redDiffuseMaterial = {};
    redDiffuseMaterial.color = Vector3(0.65f, 0.05f, 0.05f);

    Material whiteLightMaterial = {};
    whiteLightMaterial.emitColor = Vector3(15.0f, 15.0f, 15.0f);

    uint32_t materialCount = 5;
    Material* materials = new Material[materialCount];
    materials[0] = defaultMaterial;
    materials[1] = whiteDiffuseMaterial;
    materials[2] = greenDiffuseMaterial;
    materials[3] = redDiffuseMaterial;
    materials[4] = whiteLightMaterial;

    // Initialize rectangles
    RectangleXY lightRect = CreateRectangle(Vector3(0.0f, 7.99f, -6.0f), Vector3(2.0f, 2.0f, 1.0f), 4);
    RotateMatrixXAxis(lightRect.rotationMatrix, -HALF_PI);

    RectangleXY bottomRect = CreateRectangle(Vector3(0.0f, -8.0f, -8.0f), Vector3(8.0f, 10.0f, 1.0f), 1);
    RotateMatrixXAxis(bottomRect.rotationMatrix, -HALF_PI);

    RectangleXY rightRect = CreateRectangle(Vector3(8.0f, 0.0f, -8.0f), Vector3(10.0f, 8.0f, 1.0f), 3);
    RotateMatrixYAxis(rightRect.rotationMatrix, -HALF_PI);

    RectangleXY leftRect = CreateRectangle(Vector3(-8.0f, 0.0f, -8.0f), Vector3(10.0f, 8.0f, 1.0f), 2);
    RotateMatrixYAxis(leftRect.rotationMatrix, HALF_PI);

    RectangleXY backRect = CreateRectangle(Vector3(0.0f, 0.0f, -14.0f), Vector3(8.0f, 8.0f, 1.0f), 1);

    RectangleXY topRect = CreateRectangle(Vector3(0.0f, 8.0f, -8.0f), Vector3(8.0f, 10.0f, 1.0f), 1);
    RotateMatrixXAxis(topRect.rotationMatrix, -HALF_PI);


    Box box = CreateBox(Vector3(2.0f, -6.0f, -3.0f), Vector3(2.0f, 2.0f, 2.0f), 1);
    //RotateBoxZAxis(&box, -0.3f);

    Box box2 = CreateBox(Vector3(-2.0f, -4.0f, -8.0f), Vector3(2.0f, 4.0f, 2.0f), 1);
    //RotateBoxZAxis(&box2, 0.2f);

    uint32_t rectangleCount = 18;
    RectangleXY* rectangles = new RectangleXY[rectangleCount];
    rectangles[0] = lightRect;
    rectangles[1] = bottomRect;
    rectangles[2] = rightRect;
    rectangles[3] = leftRect;
    rectangles[4] = backRect;
    rectangles[5] = topRect;
    rectangles[6] = box.rectangles[0];
    rectangles[7] = box.rectangles[1];
    rectangles[8] = box.rectangles[2];
    rectangles[9] = box.rectangles[3];
    rectangles[10] = box.rectangles[4];
    rectangles[11] = box.rectangles[5];
    rectangles[12] = box2.rectangles[0];
    rectangles[13] = box2.rectangles[1];
    rectangles[14] = box2.rectangles[2];
    rectangles[15] = box2.rectangles[3];
    rectangles[16] = box2.rectangles[4];
    rectangles[17] = box2.rectangles[5];


    Camera* camera = new Camera(Vector3(0.0f, 1.0f, 20.0f));

    // I used raw pointers for scene objects. Freeing heap memory is callers responsibilty.
    // TODO: I should use smart pointers for scene objects but I don't want to do that now. 
    // Memory automatically will be freed after program terminated.
    World* world = new World;
    world->materialCount = materialCount;
    world->materials = materials;
    world->planeCount = 0;
    world->sphereCount = 0;
    world->sphereSoAArrayCount = 0;
    world->rectangleCount = rectangleCount;
    world->rectangles = rectangles;
    world->camera = camera;

    return world;
}

#endif
