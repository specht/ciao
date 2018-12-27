#include "obj.h"
#include "quadtree.h"
#include <stdio.h>

r_obj::r_obj(r_scene* _scene)
    : shading(NULL),
      scene(_scene)
{
}

r_obj::~r_obj()
{
    if (shading)
    {
        fprintf(stderr, "Object had a quadtree with %ld nodes.\n", shading->nodes.size());
        delete shading;
    }
}
