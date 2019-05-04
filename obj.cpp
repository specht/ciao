#include "obj.h"
#include "camera.h"
#include "quadtree.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>

r_obj::r_obj(r_scene* _scene)
    : shading(NULL),
      scene(_scene),
      light_emitted(0),
      lightmap_backdrop(0)
{
    sprintf(lm_path, "lm_%ld.lm", scene->objects.size());
}

r_obj::~r_obj()
{
    if (shading)
    {
        fprintf(stderr, "Object had a quadtree with %ld nodes.\n", shading->nodes.size());
        shading->save_to_file(lm_path);
        char path[32];
        sprintf(path, "%s.pgm", lm_path);
        shading->save_to_texture(path);
        delete shading;
    }
    if (lightmap_backdrop)
        delete [] lightmap_backdrop;
}

float r_obj::calculate_occlusion(const r_vec3d& p, r_scene* scene)
{
    if (!lightmap_backdrop)
    {
        lightmap_backdrop = new float[LIGHT_FRAME_SIZE * LIGHT_FRAME_SIZE];
        float x, y;
        calculate_uv(p, &x, &y);
        
        r_vec3d s_from(p);
        r_vec3d at(s_from);
        r_vec3d n, tangent;
        calculate_n(p, &n);
        calculate_tangent(p, &tangent);
        at.add(n);
        
        r_camera lc(LIGHT_FRAME_SIZE, LIGHT_FRAME_SIZE, 0.45 * M_PI, s_from, at, tangent);
        lc.aa_level = 2;
        lc.scene = scene;
        lc.shading_backdrop_pass = lightmap_backdrop;
        lc.lightness_target = lightmap_backdrop;
        lc.render();
    }
    float x, y;
    calculate_uv(p, &x, &y);
    
    r_vec3d s_from(p);
    r_vec3d at(s_from);
    r_vec3d n, tangent;
    calculate_n(p, &n);
    calculate_tangent(p, &tangent);
    at.add(n);
    
    // TODO experiment with size and antialiasing of this image
    r_camera lc(LIGHT_FRAME_SIZE, LIGHT_FRAME_SIZE, 0.45 * M_PI,
        s_from, at, tangent);
    lc.aa_level = LIGHT_FRAME_ANTIALIASING;
    lc.accumulate = true;
    lc.scene = scene;
    lc.shading_pass = true;
    lc.ignore(this);
    lc.render_to_renderer = scene->camera;
    lc.use_shading_backdrop = lightmap_backdrop;
    lc.use_lightmap = lightmap_backdrop;
    lc.scale = 3;
    lc.render();

//     return 1.0 - exp(-lc.accumulated.l());
    return pow(lc.accumulated, 5);
}
