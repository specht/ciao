#include "obj.h"
#include "quadtree.h"
#include "scene.h"
#include <stdio.h>

r_obj::r_obj(r_scene* _scene)
    : shading(NULL),
      scene(_scene)
{
    sprintf(lm_path, "lm_%d.lm", scene->objects.size());
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
