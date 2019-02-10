#ifndef _MATH_H_
#define _MATH_H_

#include <math.h>
#include <float.h>
#include "math_vector.h"

#define F32Max FLT_MAX

inline float Clamp(float low, float value, float high) {
    if (value < low) {
	return low;
    } else if (value > high) {
	return high;
    } else {
	return value;
    }
}

inline float DotProduct(const Vector3 v1, const Vector3 v2) {
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vector3 CrossProduct(const Vector3 v1, const Vector3 v2) {
  return Vector3(v1.y * v2.z - v1.z * v2.y,
	       v1.z * v2.x - v1.x * v2.z,
	       v1.x * v2.y - v1.y * v2.x);
}

inline float Lenght(const Vector3 v) {
  return sqrtf(DotProduct(v, v));
}

inline Vector3 Normalize(const Vector3 v) {
  const float dot = DotProduct(v, v);
  const float factor = 1 / sqrtf(dot);
  return Vector3(v.x * factor, v.y * factor, v.z * factor);
}

inline Vector3 Lerp(Vector3 left, float factor, Vector3 right) {
    return left * (1.0f - factor) + right * factor;
}

float RandomUnilateral() {
    return (float) rand() / (float) RAND_MAX;
}

float RandomBilateral() {
    return 2.0f * RandomUnilateral() - 1.0f;
}

bool Refract(Vector3 incidentVector, Vector3 normal,
	     float refractiveIndex, Vector3* refractionDirection) {
    // Clamp cos value for avoiding any NaN errors;
    float cosIncidentAngle = Clamp(-1.0f, DotProduct(incidentVector, normal), 1.0f);
    Vector3 hitNormal = normal;
    float refractiveIndexRatio;
    // NOTE: we assume incident ray comes from the air which has refraction index equals 1
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
    if (discriminant < 0) {
	return false;
    } else {
	*refractionDirection = incidentVector * refractiveIndexRatio + hitNormal *
	    (refractiveIndexRatio * cosIncidentAngle - sqrtf(discriminant));
	return true;
    }
    
}

inline uint32_t RGBPackToUInt32(Vector3 color) {
  return (255 << 24 |
	  (int32_t) (255 * color.x) << 16 |
	  (int32_t) (255 * color.y) << 8 |
	  (int32_t) (255 * color.z) << 0);
}

inline uint32_t RGBPackToUInt32WithGamma2(Vector3 color) {
  return (255 << 24 |
	  (int32_t) (255 * sqrtf(color.x)) << 16 |
	  (int32_t) (255 * sqrtf(color.y)) << 8 |
	  (int32_t) (255 * sqrtf(color.z)) << 0);
}

#endif
