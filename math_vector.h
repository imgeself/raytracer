#ifndef _VECTOR_H_
#define _VECTOR_H_

// TODO: Make flexible vector class using templates someday!
#include <stdint.h>

struct Vector3 {
    float x,y,z = 0.0f;
    
    Vector3();
    Vector3(float x, float y, float z);
    Vector3(const Vector3 &v);
    
    float operator[](const uint32_t index);
    Vector3 operator-();
    Vector3 operator=(const Vector3 v);
    Vector3 operator+=(const Vector3 v);
    Vector3 operator*=(const Vector3 v);
    Vector3 operator-=(const Vector3 v);
    Vector3 operator/=(const Vector3 v);
    Vector3 operator-(const Vector3 v);
    Vector3 operator-(const Vector3 v) const;
    Vector3 operator*(const Vector3 v);
    Vector3 operator*(const Vector3 v) const;
    Vector3 operator/(const Vector3 v);
    Vector3 operator/(const Vector3 v) const;
    Vector3 operator+(const Vector3 v);
    Vector3 operator+(const Vector3 v) const;
    
    Vector3 operator*(const float factor);
    Vector3 operator+(const float factor);
    Vector3 operator-(const float factor);
    Vector3 operator/(const float factor);

    bool operator==(const Vector3 v);
    bool operator!=(const Vector3 v);
};

inline Vector3::Vector3() {
}

inline Vector3::Vector3(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

inline Vector3::Vector3(const Vector3 &v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
}

inline float Vector3::operator[](const uint32_t index) {
    return (&x)[index];
}

inline Vector3 Vector3::operator-() {
    return Vector3(-x, -y, -z);
}

inline Vector3 Vector3::operator=(const Vector3 v) {
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

inline Vector3 Vector3::operator+=(const Vector3 v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

inline Vector3 Vector3::operator*=(const Vector3 v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

inline Vector3 Vector3::operator-=(const Vector3 v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

inline Vector3 Vector3::operator/=(const Vector3 v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    return *this;
}

inline Vector3 Vector3::operator*(const Vector3 v) {
    return Vector3(x * v.x, y * v.y, z * v.z);
}

inline Vector3 Vector3::operator*(const Vector3 v) const {
    return Vector3(x * v.x, y * v.y, z * v.z);
}

inline Vector3 Vector3::operator/(const Vector3 v) {
    return Vector3(x / v.x, y / v.y, z / v.z);
}

inline Vector3 Vector3::operator/(const Vector3 v) const {
    return Vector3(x / v.x, y / v.y, z / v.z);
}

inline Vector3 Vector3::operator+(const Vector3 v) {
    return Vector3(x + v.x, y + v.y, z + v.z);
}

inline Vector3 Vector3::operator+(const Vector3 v) const {
    return Vector3(x + v.x, y + v.y, z + v.z);
}

inline Vector3 Vector3::operator-(const Vector3 v) {
    return Vector3(x - v.x, y - v.y, z - v.z);
}

inline Vector3 Vector3::operator-(const Vector3 v) const {
    return Vector3(x - v.x, y - v.y, z - v.z);
}

inline Vector3 Vector3::operator*(const float factor) {
    return Vector3(x * factor, y * factor, z * factor);
}

inline Vector3 Vector3::operator+(const float factor) {
    return Vector3(x + factor, y + factor, z + factor);
}

inline Vector3 Vector3::operator-(const float factor) {
    return Vector3(x - factor, y - factor, z - factor);
}

inline Vector3 Vector3::operator/(const float factor) {
    return Vector3(x / factor, y / factor, z / factor);
}

inline bool Vector3::operator==(const Vector3 v) {
    return ((x == v.x) && (y == v.y) && (z == v.z));
}

inline bool Vector3::operator!=(const Vector3 v) {
    return ((x != v.x) || (y != v.y) || (z != v.z));
} 


// Vector4
struct Vector4 {
    float x, y, z, w = 0.0f;

    Vector4();
    Vector4(float x, float y, float z, float w);
    Vector4(Vector3 v, float w);
    Vector4(const Vector4& v);

    Vector3 xyz();

    float operator[](const uint32_t index);
    Vector4 operator-();
    Vector4 operator=(const Vector4 v);
    Vector4 operator+=(const Vector4 v);
    Vector4 operator*=(const Vector4 v);
    Vector4 operator-=(const Vector4 v);
    Vector4 operator/=(const Vector4 v);
    Vector4 operator-(const Vector4 v);
    Vector4 operator-(const Vector4 v) const;
    Vector4 operator*(const Vector4 v);
    Vector4 operator*(const Vector4 v) const;
    Vector4 operator/(const Vector4 v);
    Vector4 operator/(const Vector4 v) const;
    Vector4 operator+(const Vector4 v);
    Vector4 operator+(const Vector4 v) const;

    Vector4 operator*(const float factor);
    Vector4 operator+(const float factor);
    Vector4 operator-(const float factor);
    Vector4 operator/(const float factor);

    bool operator==(const Vector4 v);
    bool operator!=(const Vector4 v);
};

inline Vector4::Vector4() {
}

inline Vector4::Vector4(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

inline Vector4::Vector4(Vector3 v, float w) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = w;
}

inline Vector4::Vector4(const Vector4& v) {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = v.w;
}

inline float Vector4::operator[](const uint32_t index) {
    return (&x)[index];
}

inline Vector3 Vector4::xyz() {
    return Vector3(x, y, z);
}

inline Vector4 Vector4::operator-() {
    return Vector4(-x, -y, -z, -w);
}

inline Vector4 Vector4::operator=(const Vector4 v) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
    return *this;
}

inline Vector4 Vector4::operator+=(const Vector4 v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

inline Vector4 Vector4::operator*=(const Vector4 v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    w *= v.w;
    return *this;
}

inline Vector4 Vector4::operator-=(const Vector4 v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

inline Vector4 Vector4::operator/=(const Vector4 v) {
    x /= v.x;
    y /= v.y;
    z /= v.z;
    w /= v.w;
    return *this;
}

inline Vector4 Vector4::operator*(const Vector4 v) {
    return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

inline Vector4 Vector4::operator*(const Vector4 v) const {
    return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

inline Vector4 Vector4::operator/(const Vector4 v) {
    return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
}

inline Vector4 Vector4::operator/(const Vector4 v) const {
    return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
}

inline Vector4 Vector4::operator+(const Vector4 v) {
    return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

inline Vector4 Vector4::operator+(const Vector4 v) const {
    return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

inline Vector4 Vector4::operator-(const Vector4 v) {
    return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

inline Vector4 Vector4::operator-(const Vector4 v) const {
    return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

inline Vector4 Vector4::operator*(const float factor) {
    return Vector4(x * factor, y * factor, z * factor, w * factor);
}

inline Vector4 Vector4::operator+(const float factor) {
    return Vector4(x + factor, y + factor, z + factor, w + factor);
}

inline Vector4 Vector4::operator-(const float factor) {
    return Vector4(x - factor, y - factor, z - factor, w - factor);
}

inline Vector4 Vector4::operator/(const float factor) {
    return Vector4(x / factor, y / factor, z / factor, w / factor);
}

inline bool Vector4::operator==(const Vector4 v) {
    return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
}

inline bool Vector4::operator!=(const Vector4 v) {
    return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
}
#endif
