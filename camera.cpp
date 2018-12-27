#include "camera.h"
#include <math.h>

r_camera::r_camera(int _width, int _height, float _fov, 
                   const r_vec3d& _from,
                   const r_vec3d& _at,
                   const r_vec3d& _up
        )
    : width(_width), height(_height), fov(_fov),
        from(_from), at(_at), up(_up)
{
    shading_pass = false;
    w2 = (float)width * 0.5;
    h2 = (float)height * 0.5;
    w_2 = 1.0 / w2;
    h_2 = 1.0 / h2;
    dz = 1.0;
    dy = tan(fov);
    dx = dy / ((float)height / width);
    up.normalize();
    dir = r_vec3d(at);
    dir.subtract(from);
    dir.normalize();
    right = r_vec3d(dir);
    right.cross(up);
    right.normalize();
    right.multiply(dx);
    up = r_vec3d(right);
    up.cross(dir);
    up.normalize();
    up.multiply(dy);
}

void r_camera::mkray(r_vec3d* v, float x, float y)
{
    r_vec3d vx(right);
    vx.multiply((x - w2) * w_2);
    r_vec3d vy(up);
    vy.multiply((h2 - y) * h_2);
    *v = r_vec3d(dir);
    v->add(vx);
    v->add(vy);
    v->normalize();
}

float r_camera::trace(const r_vec3d& from, const r_vec3d& dir, int recursions_left)
{
    float nearest_t, nearest_color;
    float t, color;
    r_obj* nearest = NULL;
    for (std::vector<r_obj*>::iterator i = scene->objects.begin(); i != scene->objects.end(); i++)
    {
        if ((*i)->intersect(&t, from, dir))
        {
            if ((!nearest) || (t < nearest_t))
            {
                nearest = *i;
                nearest_t = t;
            }
        }
    }
    if (nearest) {
        r_vec3d p(dir);
        p.multiply(nearest_t);
        p.add(from);
        if (shading_pass)
            return 0;
        else
            return nearest->shade(from, dir, p, this, recursions_left);
    }
    if (shading_pass)
        return 1.0;
    else
        return dir.y * 1.5 + 0.2;
}

float r_camera::get_color(float sx, float sy, int level, int max_level)
{
    if (level < max_level)
    {
        float step = 1.0 / (1 << level);
        float step2 = step * 0.5;
        return (get_color(sx, sy, level + 1, max_level) +
                get_color(sx + step2, sy, level + 1, max_level) +
                get_color(sx, sy + step2, level + 1, max_level) + 
                get_color(sx + step2, sy + step2, level + 1, max_level)) * 0.25;
    }
    else
    {
        r_vec3d v;
        mkray(&v, sx, sy);
        return trace(from, v, 1);
    }
}

void r_camera::render(FILE* f, int aa_level, float* _sum)
{
    if (f) fprintf(f, "P2 %d %d %d\n", width, height, 255);
    float sum = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float c = get_color(x, y, 0, aa_level);
            if (!shading_pass)
            {
                c = 1.0 - exp(-c * 1);
                // add some screen space noise
                c += (((float)rand() / RAND_MAX) - 0.5) * 0.03;
            }
            if (shading_pass)
            {
                float r = ((float)x - w2) * ((float)x - w2) +
                          ((float)y - h2) * ((float)y - h2);
                c *= (1 - r / (width * height / 4));
            }
            // clamp color value
            if (c < 0.0) c = 0.0; else if (c > 1.0) c = 1.0;
            sum += c;
            unsigned char color = round(c * 255.0);
            if (f) fprintf(f, "%d ", color);
        }
    }
    if (_sum)
        *_sum = sum / (width * height);
}

