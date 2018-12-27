#pragma once

struct r_quadtree;

#include "vec3d.h"
// #include "scene.h"
#include <vector>

struct r_scene;

struct r_obj {
    r_quadtree* shading;
    r_scene* scene;
    
    r_obj(r_scene* _scene);
    virtual ~r_obj();
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir) = 0;
    virtual float shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left) = 0;
    virtual float calculate_occlusion(float x, float y, r_scene* scene) = 0;
};
