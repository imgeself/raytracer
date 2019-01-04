#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdint.h>

struct Vector3 {
    float x,y,z;
    
    Vector3();
    Vector3(float x, float y, float z);
    Vector3(const Vector3 &v);
    
    float operator[](const uint32_t index);
    Vector3 operator-();
    Vector3 operator=(const Vector3 v);
    Vector3 operator+=(const Vector3 v);
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
    
    float Lenght();
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

#endif
