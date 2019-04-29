#include "scene.h"

r_scene::r_scene()
    : backdrop(0)
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

void r_scene::add_backdrop(hdr_image* _backdrop)
{
    backdrop = _backdrop;
}
