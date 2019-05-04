#include "color.h"
#include <math.h>
#include <stdio.h>

r_color::r_color() 
    : r(0), g(0), b(0) 
{}

r_color::r_color(const r_color& o) 
    : r(o.r), g(o.g), b(o.b) 
{}

r_color::r_color(float _r, float _g, float _b) 
    : r(_r), g(_g), b(_b) 
{} 

uint32_t r_color::to_i() 
{ 
    int _r = round(r * 255);
    int _g = round(g * 255);
    int _b = round(b * 255);
    if (_r < 0) _r = 0;
    if (_r > 255) _r = 255;
    if (_g < 0) _g = 0;
    if (_g > 255) _g = 255;
    if (_b < 0) _b = 0;
    if (_b > 255) _b = 255;
    return (_b << 16) | (_g << 8) | _r;
}

void r_color::multiply(float x) 
{ 
    r *= x; g *= x; b *= x; 
}

float r_color::lightness() 
{ 
    return r * 0.299 + g * 0.587 + b * 0.114; 
}

void r_color::rgb_to_xyz(r_color* out) 
{
    r_color temp(*this);
    for (int i = 0; i < 3; i++)
    {
        if (temp.c[i] > 0.04045)
            temp.c[i] = pow(((temp.c[i] + 0.055) / 1.055), 2.4);
        else
            temp.c[i] /= 12.92;
    }
    out->x = temp.r * 0.4124 + temp.g * 0.3576 + temp.b * 0.1805;
    out->y = temp.r * 0.2126 + temp.g * 0.7152 + temp.b * 0.0722;
    out->z = temp.r * 0.0193 + temp.g * 0.1192 + temp.b * 0.9505;
}

void r_color::xyz_to_rgb(r_color* out) 
{
    out->r = x * 3.2406 + y * -1.5372 + z * -0.4986;
    out->g = x * -0.9689 + y * 1.8758 + z * 0.0415;
    out->b = x * 0.0557 + y * -0.2040 + z * 1.0570;
    for (int i = 0; i < 3; i++)
    {
        if (out->c[i] > 0.0031308)
            out->c[i] = 1.055 * pow(out->c[i], 1.0 / 2.4) - 0.055;
        else
            out->c[i] *= 12.92;
    }
}

void r_color::xyz_to_lab(r_color* out)
{
    r_color temp(*this);
    for (int i = 0; i < 3; i++)
    {
        if (temp.c[i] > 0.008856)
            temp.c[i] = pow(temp.c[i], 1.0 / 3);
        else
            temp.c[i] = (temp.c[i] * 7.787) + (16.0 / 116);
    }
    out->l = (116 * temp.y) - 16;
    out->a = 500 * (temp.x - temp.y);
    out->b = 200 * (temp.y - temp.z);
}

void r_color::lab_to_xyz(r_color* out)
{
    out->y = (l + 16) / 116.0;
    out->x = a / 500.0 + out->y;
    out->z = out->y - b / 200.0;
    for (int i = 0; i < 3; i++)
    {
        if (pow(out->c[i], 3) > 0.008856)
            out->c[i] = pow(out->c[i], 3);
        else
            out->c[i] = (out->c[i] - 16.0 / 116) / 7.787;
    }
}

void r_color::rgb_to_lab(r_color* out)
{
    r_color xyz;
    rgb_to_xyz(&xyz);
    xyz.xyz_to_lab(out);
}

void r_color::lab_to_rgb(r_color* out)
{
    r_color xyz;
    lab_to_xyz(&xyz);
    xyz.xyz_to_rgb(out);
}

void r_color::print()
{
    fprintf(stderr, "[%1.1f, %1.1f, %1.1f]", r, g, b);
}
