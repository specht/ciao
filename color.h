#pragma once

#include <stdint.h>

struct r_color
{
    union { 
        float c[3]; 
        struct { float r, g, b; }; 
        struct { float x, y, z; };
        struct { float l, a, b_; };
    };
    r_color();
    r_color(const r_color& o);
    r_color(float _r, float _g, float _b);
    uint32_t to_i();
    void multiply(float x);
    float lightness();
    void rgb_to_xyz(r_color* out);
    void xyz_to_rgb(r_color* out);
    void xyz_to_lab(r_color* out);
    void lab_to_xyz(r_color* out);
    void rgb_to_lab(r_color* out);
    void lab_to_rgb(r_color* out);
    void print();
};
