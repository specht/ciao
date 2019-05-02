#include "camera.h"
#include <math.h>

r_camera::r_camera()
    : r_renderer(1, 1)
{
}

r_camera::r_camera(int _width, int _height, float _fov, 
                   const r_vec3d& _from,
                   const r_vec3d& _at,
                   const r_vec3d& _up
        )
    : r_renderer(_width, _height), fov(_fov), // TODO: why are width & height not correctly initialized?
      from(_from), at(_at), up(_up),
      ignore_object(0)
{
    shading_pass = false;
    shading_backdrop_pass = NULL;
    use_shading_backdrop = NULL;
    width = _width;
    height = _height;
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

void r_camera::trace(const r_vec3d& from, const r_vec3d& dir, rgb* color, int recursions_left, r_obj* additional_ignore)
{
    if (shading_backdrop_pass)
    {
        float u = (atan2(dir.x, dir.z) + M_PI) / (2 * M_PI); // -pi to pi
        float v = acos(dir.y) / M_PI; // 0 to pi
        scene->backdrop->sample(u, v, color);
        return;
    }
    float nearest_t, nearest_color;
    float t;
    r_obj* nearest = NULL;
    for (std::vector<r_obj*>::iterator i = scene->objects.begin(); i != scene->objects.end(); i++)
    {
        // ignore self object in shading pass
        if ((*i == ignore_object) || (*i == additional_ignore))
            continue;
        if ((*i)->intersect(&t, from, dir))
        {
            if ((!nearest) || (t < nearest_t))
            {
                nearest = *i;
                nearest_t = t;
            }
            // if we're in a shading pass, we can exit early
            if (shading_pass)
                break;
        }
    }

//     if (shading_pass)
//         return !nearest;
// TODO: uncomment
//     if (shading_pass)
//     {
//         if (!nearest)
//         {
// //                 fprintf(stderr, ".");
//             if (scene->backdrop)
//             {
//                 float u = (atan2(dir.x, dir.z) + M_PI) / (2 * M_PI); // -pi to pi
//                 float v = acos(dir.y) / M_PI; // 0 to pi
//                 return scene->backdrop->sample(u, v);
//             }
//             return 0.1;
//         }
//         else
//             return nearest->light_emitted;
//     }
    
    if (shading_pass)
    {
        if (nearest)
            *color = rgb(0, 0, 0);
        else
            *color = rgb(1, 1, 1);
//         {
//             float u = (atan2(dir.x, dir.z) + M_PI) / (2 * M_PI); // -pi to pi
//             float v = acos(dir.y) / M_PI; // 0 to pi
//             scene->backdrop->sample(u, v, color);
//             float l = color->r * 0.299 + color->g * 0.587 + color->b * 0.114;
//             accumulated += l;
//             l = 1.0 - exp(-l);
//             color->r = l;
//             color->g = l;
//             color->b = l;
//         }
        return;
    }
    if (nearest) {
        r_vec3d p(dir);
        p.multiply(nearest_t);
        p.add(from);
        nearest->shade(from, dir, p, this, recursions_left, color);
        return;
    }
    if (scene->backdrop)
    {
        float u = (atan2(dir.x, dir.z) + M_PI) / (2 * M_PI); // -pi to pi
        float v = acos(dir.y) / M_PI; // 0 to pi
        scene->backdrop->sample(u, v, color);
        return;
    }
    color->c[0] = dir.y * 1.5 + 0.2;
    color->c[1] = dir.y * 1.5 + 0.2;
    color->c[2] = dir.y * 1.5 + 0.2;
}

// float r_camera::get_color(float sx, float sy, int level, int max_level, int recursions_left)
// {
//     if (level < max_level)
//     {
//         float step = 1.0 / (1 << level);
//         float step2 = step * 0.5;
//         float c0 = get_color(sx, sy, level + 1, max_level, recursions_left);
//         float c1 = get_color(sx + step2, sy, level + 1, max_level, recursions_left);
//         float c2 = get_color(sx, sy + step2, level + 1, max_level, recursions_left);
//         float c3 = get_color(sx + step2, sy + step2, level + 1, max_level, recursions_left);
//         return (c0 + c1 + c2 + c3) * 0.25;
//     }
//     else
//     {
//         r_vec3d v;
//         mkray(&v, sx, sy);
//         float f = trace(from, v, recursions_left);
//         if (!shading_pass)
//             f = 1.0 - exp(-f * 1);
// 
//         return f;
//     }
// }

// void r_camera::render(FILE* f, int aa_level, int recursions_left, float* _sum)
// {
//     if (shading_pass && !LIGHT_FRAME_MASK)
//     {
//         // create light frame mask if it doesn't already exist
//         LIGHT_FRAME_MASK = new unsigned char[LIGHT_FRAME_SIZE * LIGHT_FRAME_SIZE];
//         LIGHT_FRAME_SUM = 0;
//         for (int ly = 0; ly < LIGHT_FRAME_SIZE; ly++)
//         {
//             float fy = ((float)ly / (LIGHT_FRAME_SIZE - 1) - 0.5) * 2;
//             for (int lx = 0; lx < LIGHT_FRAME_SIZE; lx++)
//             {
//                 float fx = ((float)lx / (LIGHT_FRAME_SIZE - 1) - 0.5) * 2;
//                 float f = 1.0 - (fx * fx + fy * fy);
//                 if (f < 0.0) f = 0.0;
//                 if (f > 1.0) f = 1.0;
//                 unsigned char c = round(f * 255.0);
//                 LIGHT_FRAME_SUM += c;
//                 LIGHT_FRAME_MASK[ly * LIGHT_FRAME_SIZE + lx] = c;
//             }
//         }
//     }
//     if (f) fprintf(f, "P2 %d %d %d\n", width, height, 255);
//     float sum = 0;
//     for (int y = 0; y < height; y++)
//     {
//         uint32_t *target_pixel = 0;
//         for (int x = 0; x < width; x++)
//         {
//             // in shading pass, skip pixels that don't contribute
//             float c = 0;
//             if (!(shading_pass && (!LIGHT_FRAME_MASK[y * LIGHT_FRAME_SIZE + x])))
//             {
//                 c = get_color(x, y);
//                 if (shading_pass)
//                     c *= (float)LIGHT_FRAME_MASK[y * LIGHT_FRAME_SIZE + x] / 255.0;
//                 sum += c;
//                 if (shading_pass)
//                     c = 1.0 - exp(-c);
//                 // add some screen space noise
// //                 c += (((float)rand() / RAND_MAX) - 0.5) * 0.03;
//                 // clamp color value
//                 if (c < 0.0) c = 0.0; else if (c > 1.0) c = 1.0;
//             }
//             unsigned char color = round(c * 255.0);
//             if (f) fprintf(f, "%d ", color);
//         }
//     }
//     if (_sum)
//         *_sum = sum * 255.0 / LIGHT_FRAME_SUM;
// }

void r_camera::ignore(r_obj* _ignore_object)
{
    ignore_object = _ignore_object;
}

void r_camera::get_color(float x, float y, rgb* color)
{
    r_vec3d v;
    mkray(&v, x, y);
    int recursions_left = 3;
    trace(from, v, color, recursions_left);
    if (shading_backdrop_pass)
    {
        color->r *= pow(v.y, 2);
        color->g *= pow(v.y, 2);
        color->b *= pow(v.y, 2);
    }
    if (!shading_pass)
    {
        for (int i = 0; i < 3; i++)
            color->c[i] = 1.0 - exp(-color->c[i]);
    }
}
