#include <stdio.h>
#include "render.h"

int width = 320;
int height = 240;
int scale = 1;
int aa_level = 3;

unsigned char get_color(float x, float y)
{
    x -= width / 2;
    y -= height / 2;
    bool f0 = x * x + y * y < (height * 0.4) * (height * 0.4);
    bool f1 = x * 1 + y * 4 < 0;
    return f0 ^ f1 ? 255 : 0;
}

int main(int argc, char** argv)
{
    render(width, height, scale, aa_level, &get_color);
    return 0;
}
