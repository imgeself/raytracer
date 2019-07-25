#ifndef _SIMD_SSE_H_
#define _SIMD_SSE_H_

struct LaneF32 {
    __m128 m;

    LaneF32();
    LaneF32(float value);
    LaneF32(const float* value);
    LaneF32(const LaneF32& copy);
};

inline LaneF32::LaneF32() {

};

inline LaneF32::LaneF32(float value) {
    this->m = _mm_set_ps1(value);
};

inline LaneF32::LaneF32(const float* value) {
    this->m = _mm_load_ps(value);
};

inline LaneF32::LaneF32(const LaneF32& copy) {
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

inline LaneF32 FMulAdd(const LaneF32 left, const LaneF32 right, const LaneF32 addend) {
    LaneF32 result;
#ifdef __FMA__
    result.m = _mm_fmadd_ps(left.m, right.m, addend.m);
#else
    result = (left * right) + addend;
#endif
    return result;
};

inline LaneF32 FMulSub(const LaneF32 left, const LaneF32 right, const LaneF32 sub) {
    LaneF32 result;
#ifdef __FMA__
    result.m = _mm_fmsub_ps(left.m, right.m, sub.m);
#else
    result = (left * right) - sub;
#endif

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