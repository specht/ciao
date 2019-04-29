#include "quadtree.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

r_node::r_node()
    : parent(0)
{
    for (int i = 0; i < 4; i++)
    {
        children[i] = 0;
        v[i] = 0;
    }
}

r_node::r_node(const r_node& other)
    : parent(other.parent)
{
    for (int i = 0; i < 4; i++)
    {
        children[i] = other.children[i];
        v[i] = other.v[i];
    }
}

r_node::~r_node()
{
}

r_quadtree::r_quadtree(r_obj* _obj, float x0, float x1, float y0, float y1, float scale, int _extra_levels)
    : obj(_obj)
{
//     fprintf(stderr, "sizeof r_node: %d\n", sizeof(r_node));
    float w = x1 - x0;
    float h = y1 - y0;
    dim = (w > h) ? w : h;
    xs = x0;
    ys = y0;
    scale1 = 1.0 / scale;
    extra_levels = _extra_levels;
    min_level = 0;
    while (min_level < 32 && (1 << min_level) < dim * scale1)
        min_level += 1;
    max_level = min_level + extra_levels;
//     fprintf(stderr, "levels: %d - %d, scale1: %1.2f\n", min_level, min_level + extra_levels, scale1);
    r_node* root = new r_node();
    for (int y = 0; y < 2; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            int offset = y * 2 + x;
            float fx = xs + (x << min_level) / scale1;
            float fy = ys + (y << min_level) / scale1;
            r_vec3d p;
            obj->calculate_p_from_uv(fx, fy, &p);
            float v = obj->calculate_occlusion(p, obj->scene);
            root->v[offset] = v;
        }
    }
    nodes.push_back(root);
    // load existing quadtree data if available
    load_from_file(obj->lm_path);
}

r_quadtree::~r_quadtree()
{
}

void r_quadtree::insert(float _x, float _y)
{
    int x = (int)(floor((_x - xs) * scale1 * (1 << extra_levels)));
    int y = (int)(floor((_y - ys) * scale1 * (1 << extra_levels)));
    int level = 1;
    int level1 = max_level - 1;
    unsigned int node_index = 0;
    r_node* node = nodes.at(node_index);
    float step = dim * 0.5;
    float px = xs;
    float py = ys;
//     fprintf(stderr, "inserting at (%1.4f, %1.4f) => (%d, %d), step = %1.4f\n", _x, _y, x, y, step);
    while (level <= max_level) 
    {
        int ix = x >> level1;
        int iy = y >> level1;
        int cx = (x >> level1) & 1;
        int cy = (y >> level1) & 1;
        unsigned char offset = cy * 2 + cx;
        px += cx * step;
        py += cy * step;
//         fprintf(stderr, "  [%d,%d] (at (%d, %d) / (%1.4f, %1.4f) @ level %d, step = %1.4f)\n", 
//                 cx, cy, ix, iy,
//                 px, py, level, step);
        if (node->children[offset])
        {
            // child already exists
//             fprintf(stderr, ".");
            node_index = node->children[offset];
            node = nodes.at(node_index);
        }
        else
        {
//             fprintf(stderr, "+");
            r_node* parent = node;
            node->children[offset] = nodes.size();
            node = new r_node();
//                 fprintf(stderr, "adding node #%d\n", nodes.size());
            nodes.push_back(node);
            node->parent = node_index;
            node_index = nodes.size() - 1;
            for (int cy = 0; cy < 2; cy++)
            {
                for (int cx = 0; cx < 2; cx++)
                {
                    unsigned char co = cy * 2 + cx;
                    if (co == offset)
                    {
                        // re-use value from parent
//                             fprintf(stderr, "re-using occlusion for %1.4f, %1.4f\n", px + cx * step, py + cy * step);
                        node->v[co] = parent->v[offset];
                    }
                    else
                    {
//                             fprintf(stderr, "calculate_occlusion for %1.4f, %1.4f\n", px + cx * step, py + cy * step);
                        r_vec3d temp;
                        obj->calculate_p_from_uv(px + cx * step, py + cy * step, &temp);
                        float v = obj->calculate_occlusion(temp, obj->scene);
                        node->v[co] = v;
                    }
                }
            }
        }
        float min = node->v[0];
        float max = node->v[0];
        for (int i = 1; i < 4; i++)
        {
            if (node->v[i] < min) min = node->v[i];
            if (node->v[i] > max) max = node->v[i];
        }
//         fprintf(stderr, "%d %d\n", level, max - min);
        if ((level >= min_level) && ((max - min) < 0.05))
            break;
//             fprintf(stderr, "  %d %d\n", cx, cy);
        step *= 0.5;
        level += 1;
        level1 -= 1;
    }
}

float r_quadtree::query(float _x, float _y, bool insert_value)
{
    if (insert_value)
        insert(_x, _y);
    
    int x = (int)(floor((_x - xs) * scale1 * (1 << extra_levels << 8)));
    int y = (int)(floor((_y - ys) * scale1 * (1 << extra_levels << 8)));
    int level = 0;
    int level1 = max_level;
    unsigned int node_index = 0;
    r_node* node = nodes.at(node_index);
    float step = 1 << min_level;
    float px = xs;
    float py = ys;
    while (true) 
    {
        int cx = (x >> (level1 - 1) >> 8) & 1;
        int cy = (y >> (level1 - 1) >> 8) & 1;
        unsigned char offset = cy * 2 + cx;
        px += cx * step;
        py += cy * step;
        step *= 0.5;
        if (node->children[offset])
            node = nodes.at(node->children[offset]);
        else
            break;
        level += 1;
        level1 -= 1;
    }
    float fx = (float)((x >> (level1)) & 0xff) / 256.0;
    float fy = (float)((y >> (level1)) & 0xff) / 256.0;
    float lv = node->v[2] * fy + node->v[0] * (1.0 - fy);
    float rv = node->v[3] * fy + node->v[1] * (1.0 - fy);
    float v = rv * fx + lv * (1.0 - fx);
    return v;
//     return (node->v[0] + node->v[1] + node->v[2] + node->v[3]) * 0.25 / 255.0;
}

void r_quadtree::load_from_file(const char* path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return;
    nodes.clear();
    r_node node;
    while (true)
    {
        size_t bytes_read = fread(&node.parent, 1, 4, f);
        if (bytes_read < 4)
            break;
        for (int k = 0; k < 4; k++)
            fread(&node.children[k], 1, 4, f);
        for (int k = 0; k < 4; k++)
            fread(&node.v[k], 1, 4, f);
        nodes.push_back(new r_node(node));
    }
    fclose(f);
}

void r_quadtree::save_to_file(const char* path)
{
    FILE *f = fopen(path, "w");
    for (int i = 0; i < nodes.size(); i++)
    {
        r_node* node = nodes.at(i);
        fwrite(&node->parent, 1, 4, f);
        for (int k = 0; k < 4; k++)
            fwrite(&node->children[k], 1, 4, f);
        for (int k = 0; k < 4; k++)
            fwrite(&node->v[k], 1, 4, f);
    }
    fclose(f);
}

void r_quadtree::dump()
{
    fprintf(stderr, "\n");
    for (int i = 0; i < nodes.size(); i++)
    {
        fprintf(stderr, "#%4d p: %4d c: (%4d %4d %4d %4d), v: (%1.2f, %1.2f, %1.2f, %1.2f)\n",
                i, nodes.at(i)->parent,
                nodes.at(i)->children[0],
                nodes.at(i)->children[1],
                nodes.at(i)->children[2],
                nodes.at(i)->children[3],
                nodes.at(i)->v[0],
                nodes.at(i)->v[1],
                nodes.at(i)->v[2],
                nodes.at(i)->v[3]
        );
    }
}

unsigned char lm_float_to_byte(float f)
{
    f = 1.0 - exp(-f);
    if (f < 0.0) f = 0.0;
    if (f > 1.0) f = 1.0;
    return (unsigned char)round(f * 255.0);
}

void r_quadtree::save_to_texture_recurse(int mag, unsigned char* buffer, uint32_t p, int level, int x, int y)
{
    r_node* node = nodes.at(p);
    int width = (1 << (max_level - (level - min_level))) * mag;
    int dim = 1 << max_level;
    int vl = (int)(lm_float_to_byte(nodes.at(p)->v[0])) << 8;
    int vld = ((int)lm_float_to_byte(nodes.at(p)->v[2]) - lm_float_to_byte(nodes.at(p)->v[0])) * 256 / width;
    int vr = (int)(lm_float_to_byte(nodes.at(p)->v[1])) << 8;
    int vrd = ((int)lm_float_to_byte(nodes.at(p)->v[3]) - lm_float_to_byte(nodes.at(p)->v[1])) * 256 / width;
    for (int py = 0; py < width; py++)
    {
        int v = vl;
        int vd = ((int)vr - vl) * 256 / width / 256;
        for (int px = 0; px < width; px++)
        {
            buffer[(py + y) * dim * mag + px + x] = v >> 8;
            v += vd;
        }
        vl += vld; vr += vrd;
    }
    for (int cy = 0; cy < 2; cy++)
    {
        for (int cx = 0; cx < 2; cx++)
        {
            int c = cy * 2 + cx;
            if (nodes.at(p)->children[c])
                save_to_texture_recurse(mag, buffer, nodes.at(p)->children[c],
                                        level + 1,
                                        x + cx * width / 2,
                                        y + cy * width / 2);
        }
    }
//     unsigned char border_color = 0;
//     for (int i = 2; i < width - 2; i++)
//     {
//         buffer[(y + i) * dim * mag + x] = border_color;
//         buffer[(y + i) * dim * mag + x + width] = border_color;
//         buffer[y * dim * mag + x + i] = border_color;
//         buffer[(y + width) * dim * mag + x + i] = border_color;
//     }
}

void r_quadtree::save_to_texture(const char* path)
{
    FILE* f = fopen(path, "w");
    int mag = 1;
    int dim = (1 << max_level) * mag;
    fprintf(f, "P2 %d %d %d\n", dim, dim, 255);
    unsigned char* buffer = new unsigned char[dim * dim];
    memset(buffer, 0, dim * dim);
    save_to_texture_recurse(mag, buffer, 0, min_level, 0, 0);
    for (int i = 0; i < dim * dim; i++)
        fprintf(f, "%d ", buffer[i]);
    delete [] buffer;
    fclose(f);
}
