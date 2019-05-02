#pragma once

#include <stdint.h>
#include <math.h>

struct rgb
{
    union { float c[3]; struct { float r, g, b; }; };
    rgb() : r(0), g(0), b(0) {}
    rgb(const rgb& o) : r(o.r), g(o.g), b(o.b) {}
    rgb(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
    uint32_t to_i() { 
        return ((int)(round(r * 255.0))) << 16 |
                ((int)(round(g * 255.0))) << 8 |
                ((int)(round(b * 255.0)));
    }
    void multiply(float x) { r *= x; g *= x; b *= x; }
    float l() { return r * 0.299 + g * 0.587 + b * 0.114; }
};
