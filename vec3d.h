#pragma once

#define EPSILON 0.00001

struct r_vec3d {
    float x, y, z;
    
    r_vec3d();
    r_vec3d(const r_vec3d& other);
    r_vec3d(float _x, float _y, float _z);
    void add(const r_vec3d& other);
    void subtract(const r_vec3d& other);
    void multiply(const float s);
    float dot(const r_vec3d& other) const;
    float length() const;
    void normalize();
    void cross(const r_vec3d& other);
};

// END OF FILE
