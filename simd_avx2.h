#ifndef _SIMD_AVX2_H_
#define _SIMD_AVX2_H_

struct LaneF32 {
    __m256 m;

    LaneF32();
    LaneF32(float value);
    LaneF32(const float* value);
    LaneF32(const LaneF32& copy);
};

inline LaneF32::LaneF32() {

};

inline LaneF32::LaneF32(float value) {
    this->m = _mm256_set1_ps(value);
};

inline LaneF32::LaneF32(const float* value) {
    this->m = _mm256_load_ps(value);
};

inline LaneF32::LaneF32(const LaneF32& copy) {
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

inline LaneF32 operator<=(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_cmp_ps(left.m, right.m, _CMP_LE_OQ);

    return result;
}

inline LaneF32 operator>=(const LaneF32 left, const LaneF32 right) {
    LaneF32 result;
    result.m = _mm256_cmp_ps(left.m, right.m, _CMP_GE_OQ);

    return result;
}

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

inline LaneF32 FMulAdd(const LaneF32 left, const LaneF32 right, const LaneF32 addend) {
    LaneF32 result;
#ifdef __FMA__
    result.m = _mm256_fmadd_ps(left.m, right.m, addend.m);
#else
    result = (left * right) + addend;
#endif

    return result;
};

inline LaneF32 FMulSub(const LaneF32 left, const LaneF32 right, const LaneF32 sub) {
    LaneF32 result;
#ifdef __FMA__
    result.m = _mm256_fmsub_ps(left.m, right.m, sub.m);
#else
    result = (left * right) - sub;
#endif

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

#endif