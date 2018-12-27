#pragma once

#include "obj.h"
#include <vector>
#include <stdint.h>

// struct r_obj;

#pragma pack(push, 1)
struct r_node {
    uint32_t parent;
    uint32_t children[4];
    uint8_t v[4];
    
    r_node();
    r_node(const r_node& other);
    virtual ~r_node();
};
#pragma pack(pop)

struct r_quadtree {
    std::vector<r_node*> nodes;
    unsigned char min_level, max_level, extra_levels;
    float xs, ys, scale1, dim;
    r_obj* obj;
    
    r_quadtree(r_obj* _obj, float x0, float x1, float y0, float y1, float scale, int _extra_levels);
    void insert(float _x, float _y);
    float query(float _x, float _y, bool insert_value = true);
    
    void load_from_file(const char* path);
    void save_to_file(const char* path);
    void dump();
    void save_to_texture_recurse(int mag, unsigned char* buffer, uint32_t p, int level, int x, int y);
    void save_to_texture(const char* path);
};
