#pragma once

#include "hdr_image.h"
#include "obj.h"
#include <vector>

struct r_obj;

struct r_scene {
    std::vector<r_obj*> objects;
    r_scene();
    virtual ~r_scene();
    
    hdr_image* backdrop;
    void add_backdrop(hdr_image* _backdrop);
};
