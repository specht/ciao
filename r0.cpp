#include <math.h>
#include <stdio.h>
#include "render.h"

int width = 600;
int height = 320;

struct r_test_renderer: r_renderer {
    r_test_renderer(int width, int height)
        : r_renderer(width, height)
    {
        for (int i = 0; i < 49; i++)
        {
            colors[i] = r_color((float)rand() / RAND_MAX,
                                (float)rand() / RAND_MAX,
                                (float)rand() / RAND_MAX);
        }
    }
    
    void get_color(float x, float y, r_color* color) {
        int ix = floor(x * 7 / width);
        int iy = floor(y * 7 / height);
        *color = colors[iy * 7 + ix];
        color->l += (((float)rand() / RAND_MAX) - 0.5) * 2.0 / 64;
        color->a += (((float)rand() / RAND_MAX) - 0.5) * 2.0 / 32;
        color->b += (((float)rand() / RAND_MAX) - 0.5) * 2.0 / 32;
    }
    
    r_color colors[49];
};

int main(int argc, char** argv)
{
    r_test_renderer r(width, height);
    r.scale = 2;
    r.aa_level = 0;
    r.render_to_window = true;
    r.render();
    return 0;
}
