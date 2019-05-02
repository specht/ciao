#include <math.h>
#include <stdio.h>
#include "render.h"

int width = 860;
int height = 480;

struct r_test_renderer: r_renderer {
    r_test_renderer(int width, int height)
        : r_renderer(width, height)
    {
    }
    
    void get_color(float x, float y, rgb* color) {
        x -= width / 2;
        y -= height / 2;
        bool f0 = x * x + y * y < (height * 0.4) * (height * 0.4);
        bool f1 = fmod(fabs(x * 1 + y * 4), 200) < 100;
    //     bool f2 = fmod(fabs(x * 4 + y * -1), 200) < 100;
        bool f2 =0;
        if (f0 ^ f1 ^ f2)
            *color = rgb(0.0, 0.5, 0.5);
        else
            *color = rgb(1, 1, 1);
    }
};

int main(int argc, char** argv)
{
    r_test_renderer r(width, height);
    r.scale = 2;
    r.aa_level = 2;
    r.render_to_window = true;
    r.render();
    return 0;
}
