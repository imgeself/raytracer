#ifndef _SIMD_H_
#define _SIMD_H_

#include <x86intrin.h>

#define ALIGN(X) alignas(X)

#define LANE_WIDTH 4

// wide 32-bit floating point number operations
#if LANE_WIDTH == 8
#define ALIGN_LANE ALIGN(32)
struct LaneF32 {
    __m256 m;

    LaneF32();
    LaneF32(float value);
    LaneF32(const float* value);
    LaneF32(const LaneF32 &copy);
};

inline LaneF32::LaneF32() {
    
};

inline LaneF32::LaneF32(float value) {
    this->m = _mm256_set1_ps(value);
};

inline LaneF32::LaneF32(const float* value) {
    this->m = _mm256_load_ps(value);
};

inline LaneF32::LaneF32(const LaneF32 &copy) {
    this->m = copy.m;
};

inline LaneF32 operator-(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_sub_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator+(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_add_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator*(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_mul_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator/(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_div_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator>(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_cmp_ps(left.m, right.m, _CMP_GT_OQ);

    return result;
};

inline LaneF32 operator<(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_cmp_ps(left.m, right.m, _CMP_LT_OQ);

    return result;
};

inline LaneF32 operator|(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_or_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator&(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_and_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator-(const LaneF32 value) {
    LaneF32 result;
    result.m = _mm256_xor_ps(value.m, _mm256_set1_ps(-0.0f));

    return result;
};

inline void StoreLane(float* dest, LaneF32 lane) {
    _mm256_store_ps(dest, lane.m);
};

inline LaneF32 SquareRoot(LaneF32 value) {
    LaneF32 result;
    result.m = _mm256_sqrt_ps(value.m);

    return result;
};

inline LaneF32 RSquareRoot(LaneF32 value) {
    LaneF32 result;
    result.m = _mm256_rsqrt_ps(value.m);

    return result;
};

inline bool MaskIsZeroed(LaneF32 mask) {
    bool result = _mm256_movemask_ps(mask.m) == 0;

    return result;
};

inline void Select(LaneF32* dest, LaneF32 mask, LaneF32 right) {
    dest->m = _mm256_blendv_ps(dest->m, right.m, mask.m);
};

#elif LANE_WIDTH == 4
#define ALIGN_LANE ALIGN(32)
struct LaneF32 {
    __m128 m;

    LaneF32();
    LaneF32(float value);
    LaneF32(const float* value);
    LaneF32(const LaneF32 &copy);
};

inline LaneF32::LaneF32() {
    
};

inline LaneF32::LaneF32(float value) {
    this->m = _mm_set_ps1(value);
};

inline LaneF32::LaneF32(const float* value) {
    this->m = _mm_load_ps(value);
};

inline LaneF32::LaneF32(const LaneF32 &copy) {
    this->m = copy.m;
};

inline LaneF32 operator-(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_sub_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator+(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_add_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator*(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_mul_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator/(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_div_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator>(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_cmpgt_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator<(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_cmplt_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator|(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_or_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator&(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm_and_ps(left.m, right.m);

    return result;
};

inline LaneF32 operator-(const LaneF32 value) {
    LaneF32 result;
    result.m = _mm_xor_ps(value.m, _mm_set1_ps(-0.0f));

    return result;
};

inline void StoreLane(float* dest, LaneF32 lane) {
    _mm_store_ps(dest, lane.m);
};

inline LaneF32 SquareRoot(LaneF32 value) {
    LaneF32 result;
    result.m = _mm_sqrt_ps(value.m);

    return result;
};

inline LaneF32 RSquareRoot(LaneF32 value) {
    LaneF32 result;
    result.m = _mm_rsqrt_ps(value.m);

    return result;
};

inline bool MaskIsZeroed(LaneF32 mask) {
    bool result = _mm_movemask_ps(mask.m) == 0;

    return result;
};

inline void Select(LaneF32* dest, LaneF32 mask, LaneF32 right) {
    dest->m = _mm_blendv_ps(dest->m, right.m, mask.m);
};
#endif

struct LaneVector3 {
    LaneF32 x;
    LaneF32 y;
    LaneF32 z;

    LaneVector3();
    LaneVector3(LaneF32 x, LaneF32 y, LaneF32 z);
    LaneVector3(Vector3 vector);
};

inline LaneVector3::LaneVector3() {
    this->x = LaneF32();
    this->y = LaneF32();
    this->z = LaneF32();
};

inline LaneVector3::LaneVector3(LaneF32 x, LaneF32 y, LaneF32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
};

inline LaneVector3::LaneVector3(Vector3 vector) {
    this->x = LaneF32(vector.x);
    this->y = LaneF32(vector.y);
    this->z = LaneF32(vector.z);
};


inline LaneVector3 operator+(const LaneVector3 left, const LaneVector3 right) {
    return LaneVector3(left.x + right.x, left.y + right.y, left.z + right.z);
};

inline LaneVector3 operator-(const LaneVector3 left, const LaneVector3 right) {
    return LaneVector3(left.x - right.x, left.y - right.y, left.z - right.z);
};

inline LaneVector3 operator*(const LaneVector3 left, const LaneF32 right) {
    return LaneVector3(left.x * right, left.y * right, left.z * right);
};

inline LaneF32 DotProduct(const LaneVector3 left, const LaneVector3 right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
};

inline LaneVector3 Normalize(const LaneVector3 v) {
  const LaneF32 dot = DotProduct(v, v);
  const LaneF32 factor = RSquareRoot(dot);
  return LaneVector3(v.x * factor, v.y * factor, v.z * factor);
};

inline void Select(LaneVector3* dest, LaneF32 mask, LaneVector3 right) {
    Select(&(dest->x), mask, right.x);
    Select(&(dest->y), mask, right.y);
    Select(&(dest->z), mask, right.z);
};

#endif
