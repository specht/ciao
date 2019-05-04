#pragma once

#include "color.h"
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
    virtual void get_color(float x, float y, r_color* color) = 0;
    void render();
    void create_window(int width, int height, int scale);
    void destroy_window();
    void subdivide(float x, float y, r_color c00, r_color c20, 
                   r_color c02, r_color c22, float d, int aa_level, r_color* result);
    void update();
    void set_pixel(int x, int y, uint32_t color);
    
    SDL_Window* window;
    SDL_Surface* surface;
    long last_update;
    bool cancel;
};
