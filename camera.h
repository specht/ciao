#pragma once

#include "hdr_image.h"
#include "obj.h"
#include "render.h"
#include "scene.h"
#include "vec3d.h"
#include <stdio.h>
#include <SDL2/SDL.h>

const int LIGHT_FRAME_SIZE = 64;
const int LIGHT_FRAME_ANTIALIASING = 0;
static unsigned char* LIGHT_FRAME_MASK = NULL;
static unsigned int LIGHT_FRAME_SUM = 0;

struct r_camera: r_renderer {
    r_vec3d from, at, up;
    r_vec3d dir, right;
    int width, height;
    float fov;
    float w2, h2, w_2, h_2;
    float dx, dy, dz;
    r_scene* scene;
    bool shading_pass;
    float* shading_backdrop_pass;
    float* use_shading_backdrop;
    r_obj* ignore_object;
    
    r_camera();
    r_camera(int _width, int _height, float _fov, 
             const r_vec3d& _from,
             const r_vec3d& _at,
             const r_vec3d& _up
            );
    void mkray(r_vec3d* v, float x, float y);
    void trace(const r_vec3d& from, const r_vec3d& dir, rgb* color, int recursions_left, r_obj* additional_ignore = 0);
    void ignore(r_obj* _ignore_object);
    virtual void get_color(float x, float y, rgb* color);
};
