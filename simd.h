#ifndef _SIMD_H_
#define _SIMD_H_

#ifdef PLATFORM_WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define ALIGN(X) alignas(X)

// If compiler supperts AVX2, it's very likely support FMA instructions too.
#if !defined(__FMA__) && defined(__AVX2__)
    #define __FMA__ 1
#endif

#define LANE_WIDTH 8

// wide 32-bit floating point number operations
#if LANE_WIDTH == 8
#define ALIGN_LANE ALIGN(32)
#include "simd_avx2.h"
#elif LANE_WIDTH == 4
#define ALIGN_LANE ALIGN(16)
#include "simd_sse.h"
#endif

// Wide vector3 struct
struct LaneVector3 {
    LaneF32 x;
    LaneF32 y;
    LaneF32 z;

    LaneVector3();
    LaneVector3(LaneF32 x, LaneF32 y, LaneF32 z);
    LaneVector3(float arr[3][LANE_WIDTH]);
    LaneVector3(Vector3 vector);

    LaneF32& operator[](int index);
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

inline LaneVector3::LaneVector3(float arr[3][LANE_WIDTH]) {
    this->x = LaneF32(arr[0]);
    this->y = LaneF32(arr[1]);
    this->z = LaneF32(arr[2]);
}

inline LaneVector3::LaneVector3(Vector3 vector) {
    this->x = LaneF32(vector.x);
    this->y = LaneF32(vector.y);
    this->z = LaneF32(vector.z);
};

inline LaneF32& LaneVector3::operator[](int index) {
    return (&x)[index];
};

inline LaneVector3 operator-(const LaneVector3 v) {
    LaneVector3 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;

    return result;
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
    return FMulAdd(left.x, right.x, FMulAdd(left.y, right.y, (left.z * right.z)));
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

inline LaneVector3 FMulAdd(const LaneVector3 left, const LaneVector3 right, const LaneVector3 addend) {
    LaneVector3 result;
    result.x = FMulAdd(left.x, right.x, addend.x);
    result.y = FMulAdd(left.y, right.y, addend.y);
    result.z = FMulAdd(left.z, right.z, addend.z);

    return result;
}

inline LaneVector3 FMulAdd(const LaneVector3 left, const LaneF32 right, const LaneVector3 addend) {
    LaneVector3 result;
    result.x = FMulAdd(left.x, right, addend.x);
    result.y = FMulAdd(left.y, right, addend.y);
    result.z = FMulAdd(left.z, right, addend.z);

    return result;
}

// Wide vector4 struct
struct LaneVector4 {
    LaneF32 x;
    LaneF32 y;
    LaneF32 z;
    LaneF32 w;

    LaneVector4();
    LaneVector4(LaneF32 x, LaneF32 y, LaneF32 z, LaneF32 w);
    LaneVector4(LaneVector3 v, LaneF32 w);
    LaneVector4(Vector4 vector);

    LaneF32& operator[](int index);
    LaneVector3 xyz();
};

inline LaneVector4::LaneVector4() {
    this->x = LaneF32();
    this->y = LaneF32();
    this->z = LaneF32();
    this->w = LaneF32();
};

inline LaneVector4::LaneVector4(LaneF32 x, LaneF32 y, LaneF32 z, LaneF32 w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
};

inline LaneVector4::LaneVector4(LaneVector3 v, LaneF32 w) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = w;
};


inline LaneVector4::LaneVector4(Vector4 vector) {
    this->x = LaneF32(vector.x);
    this->y = LaneF32(vector.y);
    this->z = LaneF32(vector.z);
    this->w = LaneF32(vector.w);
};

inline LaneF32& LaneVector4::operator[](int index) {
    return (&x)[index];
};

inline LaneVector3 LaneVector4::xyz() {
    return LaneVector3(x, y, z);
}

inline LaneVector4 operator-(const LaneVector4 v) {
    LaneVector4 result;
    result.x = -v.x;
    result.y = -v.y;
    result.z = -v.z;
    result.w = -v.w;

    return result;
};

inline LaneVector4 operator+(const LaneVector4 left, const LaneVector4 right) {
    return LaneVector4(left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w);
};

inline LaneVector4 operator-(const LaneVector4 left, const LaneVector4 right) {
    return LaneVector4(left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w);
};

inline LaneVector4 operator*(const LaneVector4 left, const LaneF32 right) {
    return LaneVector4(left.x * right, left.y * right, left.z * right, left.w * right);
};

inline LaneF32 DotProduct(const LaneVector4 left, const LaneVector4 right) {
    return FMulAdd(left.x, right.x, FMulAdd(left.y, right.y, FMulAdd(left.z, right.z, (left.w * right.w))));
};

inline LaneVector4 Normalize(const LaneVector4 v) {
    const LaneF32 dot = DotProduct(v, v);
    const LaneF32 factor = RSquareRoot(dot);
    return LaneVector4(v.x * factor, v.y * factor, v.z * factor, v.w * factor);
};

inline void Select(LaneVector4* dest, LaneF32 mask, LaneVector4 right) {
    Select(&(dest->x), mask, right.x);
    Select(&(dest->y), mask, right.y);
    Select(&(dest->z), mask, right.z);
    Select(&(dest->w), mask, right.w);
};

inline LaneVector4 FMulAdd(const LaneVector4 left, const LaneVector4 right, const LaneVector4 addend) {
    LaneVector4 result;
    result.x = FMulAdd(left.x, right.x, addend.x);
    result.y = FMulAdd(left.y, right.y, addend.y);
    result.z = FMulAdd(left.z, right.z, addend.z);
    result.w = FMulAdd(left.w, right.w, addend.w);

    return result;
}

inline LaneVector4 FMulAdd(const LaneVector4 left, const LaneF32 right, const LaneVector4 addend) {
    LaneVector4 result;
    result.x = FMulAdd(left.x, right, addend.x);
    result.y = FMulAdd(left.y, right, addend.y);
    result.z = FMulAdd(left.z, right, addend.z);
    result.w = FMulAdd(left.w, right, addend.w);

    return result;
}

// Wide matrix4 struct
struct LaneMatrix4 {
    LaneVector4 data[4];

    LaneMatrix4();
    LaneMatrix4(LaneVector4 v1, LaneVector4 v2, LaneVector4 v3, LaneVector4 v4);
    LaneMatrix4(float array[4][4][LANE_WIDTH]);
    LaneMatrix4(const LaneMatrix4& m);

    LaneVector4& operator[](int index);
};

inline LaneMatrix4::LaneMatrix4() {
    data[0] = LaneVector4(0.0f, 0.0f, 0.0f, 0.0f);
    data[1] = LaneVector4(0.0f, 0.0f, 0.0f, 0.0f);
    data[2] = LaneVector4(0.0f, 0.0f, 0.0f, 0.0f);
    data[3] = LaneVector4(0.0f, 0.0f, 0.0f, 0.0f);
}

inline LaneMatrix4::LaneMatrix4(LaneVector4 v1, LaneVector4 v2, LaneVector4 v3, LaneVector4 v4) {
    data[0] = v1;
    data[1] = v2;
    data[2] = v3;
    data[3] = v4;
}

inline LaneMatrix4::LaneMatrix4(float arr[4][4][LANE_WIDTH]) {
    for (uint32_t row = 0; row < 4; ++row) {
        for (uint32_t column = 0; column < 4; ++column) {
            data[row][column] = LaneF32(arr[row][column]);
        }
    }
}

inline LaneMatrix4::LaneMatrix4(const LaneMatrix4& m) {
    data[0] = m.data[0];
    data[1] = m.data[1];
    data[2] = m.data[2];
    data[3] = m.data[3];
}

inline LaneVector4& LaneMatrix4::operator[](int index) {
    return data[index];
}

inline LaneVector4 operator*(LaneMatrix4& left, LaneVector4& right) {
    LaneVector4 result;
    result.x = DotProduct(left[0], right);
    result.y = DotProduct(left[1], right);
    result.z = DotProduct(left[2], right);
    result.w = DotProduct(left[3], right);

    return result;
}

inline LaneMatrix4 operator*(LaneMatrix4& left, LaneMatrix4& right) {
    LaneMatrix4 result;
    for (uint32_t row = 0; row < 4; ++row) {
        LaneVector4 rowVector = left[row];
        for (uint32_t column = 0; column < 4; ++column) {
            LaneVector4 columnVector = LaneVector4(right[0][column], right[1][column], right[2][column], right[3][column]);
            LaneF32 value = DotProduct(rowVector, columnVector);

            result[row][column] = value;
        }
    }

    return result;
}

inline LaneMatrix4 Transpose(LaneMatrix4& mat) {
    LaneMatrix4 result;
    for (uint32_t row = 0; row < 4; ++row) {
        for (uint32_t column = 0; column < 4; ++column) {
            result[row][column] = mat[column][row];
        }
    }
    return result;
}

#endif
