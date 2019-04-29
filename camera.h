#pragma once

#include "hdr_image.h"
#include "obj.h"
#include "scene.h"
#include "vec3d.h"
#include <stdio.h>
#include <SDL2/SDL.h>

const int LIGHT_FRAME_SIZE = 64;
const int LIGHT_FRAME_ANTIALIASING = 0;
static unsigned char* LIGHT_FRAME_MASK = NULL;
static unsigned int LIGHT_FRAME_SUM = 0;

struct r_camera {
    r_vec3d from, at, up;
    r_vec3d dir, right;
    float fov;
    int width, height;
    float w2, h2, w_2, h_2;
    float dx, dy, dz;
    r_scene* scene;
    bool shading_pass;
    r_obj* ignore_object;
    
    r_camera(int _width, int _height, float _fov, 
             const r_vec3d& _from,
             const r_vec3d& _at,
             const r_vec3d& _up
            );
    void mkray(r_vec3d* v, float x, float y);
    float trace(const r_vec3d& from, const r_vec3d& dir, int recursions_left, r_obj* additional_ignore = 0);
    float get_color(float sx, float sy, int level, int max_level, int recursions_left);
    void render(FILE* f = NULL, int aa_level = 0, int recursions_left = 0, float* _sum = NULL, SDL_Window* window = NULL, SDL_Surface *surface = NULL);
    void ignore(r_obj* _ignore_object);
};
