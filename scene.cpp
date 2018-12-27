#include "scene.h"

r_scene::r_scene()
{
}

r_scene::~r_scene()
{
    for (std::vector<r_obj*>::iterator i = objects.begin(); i != objects.end(); i++)
    {
        delete *i;
    }
    
    objects.clear();
}

