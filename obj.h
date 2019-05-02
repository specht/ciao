#pragma once

struct r_quadtree;

#include "vec3d.h"
#include <vector>
#include "rgb.h"

struct r_scene;

struct r_obj {
    r_quadtree* shading;
    r_scene* scene;
    float light_emitted;
    char lm_path[16];
    float* lightmap_backdrop;
    
    r_obj(r_scene* _scene);
    virtual ~r_obj();
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir) = 0;
    virtual void shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left, rgb* result) = 0;
    float calculate_occlusion(const r_vec3d& p, r_scene* scene);
    virtual void calculate_uv(const r_vec3d& p, float* u, float *v) = 0;
    virtual void calculate_p_from_uv(float u, float v, r_vec3d* p) = 0;
    virtual void calculate_n(const r_vec3d& p, r_vec3d* n) = 0;
    virtual void calculate_tangent(const r_vec3d& p, r_vec3d* tangent) = 0;
};
