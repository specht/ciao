#include "quadtree.h"
#include <stdio.h>
#include <math.h>

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
    fprintf(stderr, "sizeof r_node: %d\n", sizeof(r_node));
    float w = x1 - x0;
    float h = y1 - y0;
    float dim = (w > h) ? w : h;
    xs = x0;
    ys = y0;
    scale1 = 1.0 / scale;
    extra_levels = _extra_levels;
    min_level = 0;
    while (min_level < 32 && (1 << min_level) < dim * scale1)
        min_level += 1;
    max_level = min_level + extra_levels;
//         fprintf(stderr, "levels: %d - %d\n", min_level, min_level + extra_levels);
    r_node* root = new r_node();
    for (int y = 0; y < 2; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            int offset = y * 2 + x;
            float fx = xs + (x << min_level);
            float fy = ys + (y << min_level);
            fprintf(stderr, "init #%d: %1.2f, %1.2f\n", offset, fx, fy);
            float v = obj->calculate_occlusion(fx, fy, obj->scene);
            int c = round(v * 255.0);
            if (c < 0) c = 0;
            if (c > 255) c = 255;
            root->v[offset] = c;
        }
    }
    nodes.push_back(root);
}

void r_quadtree::insert(float _x, float _y)
{
    int x = (int)(floor((_x - xs) * scale1 * (1 << extra_levels)));
    int y = (int)(floor((_y - ys) * scale1 * (1 << extra_levels)));
    int level = 0;
    int level1 = max_level;
    unsigned int node_index = 0;
    r_node* node = nodes.at(node_index);
    float step = (1 << min_level);
    float px = xs;
    float py = ys;
//     fprintf(stderr, "inserting at %d, %d, step = %1.4f\n", x, y, step);
    while (true) 
    {
        int ix = x >> level1;
        int iy = y >> level1;
//         fprintf(stderr, "  (at %d, %d @ level %d)\n", ix, iy, level);
        if ((((ix << level1) != x) || ((iy << level1) != y)) && level < max_level)
        {
            int cx = (x >> (level1 - 1)) & 1;
            int cy = (y >> (level1 - 1)) & 1;
            unsigned char offset = cy * 2 + cx;
            px += cx * step;
            py += cy * step;
            step *= 0.5;
            if (node->children[offset])
            {
                // child already exists
//                 fprintf(stderr, ".");
                node = nodes.at(node->children[offset]);
            }
            else
            {
                fprintf(stderr, "+");
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
                            float v = obj->calculate_occlusion(px + cx * step, py + cy * step, obj->scene);
                            int c = round(v * 255.0);
                            if (c < 0) c = 0;
                            if (c > 255) c = 255;
                            node->v[co] = c;
                        }
                    }
                }
                int min = node->v[0];
                int max = node->v[0];
                for (int i = 1; i < 4; i++)
                {
                    if (node->v[i] < min) min = node->v[i];
                    if (node->v[i] > max) max = node->v[i];
                }
                if ((level >= min_level) && ((max - min) < 8))
                    break;
            }
//             fprintf(stderr, "  %d %d\n", cx, cy);
            level += 1;
            level1 -= 1;
        }
        else
            break;
    }
}

float r_quadtree::query(float _x, float _y, bool insert_value)
{
//     if (!insert_value)
//     {
//         float v = obj->calculate_occlusion(_x, _y, obj->scene);
//         return v;
//     }
    if (insert_value)
        insert(_x, _y);
    
    int x = (int)(floor((_x - xs) * scale1 * (1 << extra_levels)));
    int y = (int)(floor((_y - ys) * scale1 * (1 << extra_levels)));
    int level = 0;
    int level1 = max_level;
    unsigned int node_index = 0;
    r_node* node = nodes.at(node_index);
    float step = 1 << min_level;
    float px = xs;
    float py = ys;
    while (true) 
    {
//         int ix = x >> level1;
//         int iy = y >> level1;
//         if ((((ix << level1) != x) || ((iy << level1) != y)) && level < max_level)
//         {
            int cx = (x >> (level1 - 1)) & 1;
            int cy = (y >> (level1 - 1)) & 1;
            unsigned char offset = cy * 2 + cx;
            px += cx * step;
            py += cy * step;
            step *= 0.5;
            if (node->children[offset])
            {
                node = nodes.at(node->children[offset]);
            }
            else
                break;
            level += 1;
            level1 -= 1;
//         }
//         else
//             break;
    }
    return (node->v[0] + node->v[1] + node->v[2] + node->v[3]) * 0.25 / 255.0;
}

void r_quadtree::load_from_file(const char* path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return;
    nodes.clear();
    r_node node;
    while (!feof(f))
    {
        fread(&node.parent, 4, 1, f);
        for (int k = 0; k < 4; k++)
            fread(&node.children[k], 4, 1, f);
        for (int k = 0; k < 4; k++)
            fread(&node.v[k], 1, 1, f);
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
        fwrite(&node->parent, 4, 1, f);
        for (int k = 0; k < 4; k++)
            fwrite(&node->children[k], 4, 1, f);
        for (int k = 0; k < 4; k++)
            fwrite(&node->v[k], 1, 1, f);
    }
    fclose(f);
}

void r_quadtree::dump()
{
    return;
    fprintf(stderr, "\n");
    for (int i = 0; i < nodes.size(); i++)
    {
        fprintf(stderr, "#%4d p: %4d c: (%4d %4d %4d %4d), v: (%3d, %3d, %3d, %3d)\n",
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

void r_quadtree::save_to_texture(const char* path)
{
    FILE* f = fopen(path, "w");
    fprintf(f, "P2 %d %d %d\n", 32, 32, 255);
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 32; x++)
        {
            float v = query(xs + x, ys + y, false);
            int c = round(v * 255.0);
            if (c < 0) c = 0;
            if (c > 255) c = 255;
            fprintf(f, "%d ", c);
        }
    }
    fclose(f);
}
