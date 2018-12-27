#include "vec3d.h"
#include <math.h>

r_vec3d::r_vec3d()
    : x(0), y(0), z(0)
{}

r_vec3d::r_vec3d(const r_vec3d& other)
    : x(other.x), y(other.y), z(other.z)
{}

r_vec3d::r_vec3d(float _x, float _y, float _z)
    : x(_x), y(_y), z(_z)
{}

void r_vec3d::add(const r_vec3d& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
}

void r_vec3d::subtract(const r_vec3d& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
}

void r_vec3d::multiply(const float s)
{
    x *= s;
    y *= s;
    z *= s;
}

float r_vec3d::dot(const r_vec3d& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

float r_vec3d::length() const
{
    return sqrt(dot(*this));
}

void r_vec3d::normalize()
{
    multiply(1.0 / length());
}

void r_vec3d::cross(const r_vec3d& other)
{
    r_vec3d t(*this);
    x = t.y * other.z - t.z * other.y;
    y = t.z * other.x - t.x * other.z;
    z = t.x * other.y - t.y * other.x;
}
