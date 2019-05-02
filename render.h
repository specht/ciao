#pragma once

#include "rgb.h"
#include <stdint.h>
#include <SDL2/SDL.h>

struct r_renderer {
    int width, height, scale, aa_level;
    bool render_to_window;
    r_renderer* render_to_renderer;
    bool accumulate;
    float accumulated;
    float *lightness_target;
    float *use_lightmap;
    r_renderer(int _width, int _height);
    virtual void get_color(float x, float y, rgb* color) = 0;
    void render();
    void create_window(int width, int height, int scale);
    void destroy_window();
    void subdivide(float x, float y, rgb c00, rgb c20, 
                   rgb c02, rgb c22, float d, int aa_level, rgb* result);
    
    SDL_Window* window;
    SDL_Surface* surface;
};
