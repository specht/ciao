#include "obj.h"
#include "camera.h"
#include "quadtree.h"
#include "scene.h"
#include <math.h>
#include <stdio.h>

r_obj::r_obj(r_scene* _scene)
    : shading(NULL),
      scene(_scene),
      light_emitted(0)
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
}

float r_obj::calculate_occlusion(const r_vec3d& p, r_scene* scene)
{
    float x, y;
    calculate_uv(p, &x, &y);
    
    r_vec3d s_from(p);
    r_vec3d at(s_from);
    r_vec3d n, tangent;
    calculate_n(p, &n);
    calculate_tangent(p, &tangent);
    at.add(n);
    // TODO experiment with size and antialiasing of this image
    r_camera lc(LIGHT_FRAME_SIZE, LIGHT_FRAME_SIZE, 0.4 * M_PI,
        s_from, at, tangent);
    lc.scene = scene;
    lc.shading_pass = true;
    lc.ignore(this);

    FILE* f = NULL;
//     if ((float)rand() / RAND_MAX < 0.001)
//     {
//         char filename[256];
//         sprintf(filename, "%s_lc_%1.5f_%1.5f.pgm", lm_path, x, y);
//         f = fopen(filename, "w");
//     }
    float sum = 0.0;
    lc.render(f, LIGHT_FRAME_ANTIALIASING, 0, &sum, 0, 0);
    if (f) fclose(f);
//         fprintf(stderr, "%1.4f\n", sum);
    return sum;
}
