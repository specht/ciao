#pragma once

#include "obj.h"
#include <vector>
// #include "RefPtr.h"

struct r_obj;

struct r_scene {
    std::vector<r_obj*> objects;
    r_scene();
    virtual ~r_scene();
};
