#ifndef _MATH_H_
#define _MATH_H_

#include <math.h>
#include <float.h>
#include "math_vector.h"

#define F32Max FLT_MAX

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

inline uint32_t RGBPackToUInt32(Vector3 color) {
  return (255 << 24 |
	  (int32_t) (255 * color.x) << 16 |
	  (int32_t) (255 * color.y) << 8 |
	  (int32_t) (255 * color.z) << 0);
}

#endif
