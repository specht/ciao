#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include "camera.h"
#include "color.h"
#include "hdr_image.h"
#include "obj.h"
#include "quadtree.h"
#include "scene.h"
#include "vec3d.h"
#include "render.h"

r_scene scene;
r_camera camera;
hdr_image* diffuse;
    
struct r_floor_disc: r_obj {
    float r, r2, color; 
    r_vec3d anchor, normal, u_base, v_base;
    
    r_floor_disc(r_scene* scene, const r_vec3d& _p, const r_vec3d& _n, float _radius, float _color)
        : r_obj(scene)
        , anchor(_p)
        , normal(_n)
        , r(_radius)
        , r2(_radius * _radius)
        , color(_color)
    {
        normal.normalize();
        float lmax = 0.0;
        for (int i = 0; i < 3; i++)
        {
            r_vec3d t = r_vec3d(i == 0 ? 1 : 0, i == 1 ? 1 : 0, i == 2 ? 1 : 0);
            t.cross(normal);
            float l = t.dot(t);
            if (l > lmax)
            {
                lmax = l;
                u_base = t;
            }
        }
        u_base.normalize();
        v_base = u_base;
        v_base.cross(normal);
        shading = new r_quadtree(this, -r, r, -r, r, 0.125, 0);
    }
    
    virtual ~r_floor_disc()
    {
    }
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir)
    {
        r_vec3d f(from);
        f.subtract(anchor);
        float d = dir.dot(normal);
        if (fabs(d) > EPSILON)
        {
            *t = -normal.dot(f) / d;
            if (*t > EPSILON)
            {
                r_vec3d x(dir);
                x.multiply(*t);
                x.add(f);
                float uc = u_base.dot(x);
                float vc = v_base.dot(x);
                if (uc * uc + vc * vc < r2)
                {
                    return true;
                }
            }
        }
        return false;
    }
    
    virtual void calculate_uv(const r_vec3d& p, float* u, float *v)
    {
        r_vec3d temp(p);
        temp.subtract(anchor);
        *u = u_base.dot(temp);
        *v = v_base.dot(temp);
    };
    
    virtual void calculate_p_from_uv(float u, float v, r_vec3d* p)
    {
        r_vec3d result(anchor);
        r_vec3d tu(u_base);
        tu.multiply(u);
        r_vec3d tv(v_base);
        tv.multiply(v);
        result.add(tu);
        result.add(tv);
        *p = result;
    }
    
    virtual void calculate_n(const r_vec3d& p, r_vec3d* n)
    {
        *n = normal;
    }
    
    virtual void calculate_tangent(const r_vec3d& p, r_vec3d* tangent)
    {
        *tangent = u_base;
    }
    
    virtual void shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left, r_color* result)
    {
        r_vec3d temp(p);
        temp.subtract(anchor);
        float l = 1.0;
        if (shading)
        {
            float ref = shading->query(r * 0.95, 0.0);
            l = shading->query(u_base.dot(temp), v_base.dot(temp)) / ref;
        }
        if (scene->backdrop)
        {
            float u = (atan2(dir.x, dir.z) + M_PI) / (2 * M_PI); // -pi to pi
            float v = acos(dir.y) / M_PI; // 0 to pi
            scene->backdrop->sample(u, v, result);
            result->multiply(l);
        }
    }
};

struct r_sphere: r_obj {
    r_vec3d c;
    float r2;
    float radius;
    float b, r;
    
    r_sphere(r_scene* scene, const r_vec3d& _center, float _radius, float _b = 0.5, float _r = 0.5)
        : r_obj(scene), c(_center), r2(_radius * _radius), radius(_radius), b(_b), r(_r)
    {
//         shading = new r_quadtree(this, 0.0, 1.0, 0, 1.0, 1.0 / 8, 5);
    }
    
    virtual ~r_sphere()
    {
    }
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir)
    {
        r_vec3d g(from);
        g.subtract(c);
        float d = 1.0 / dir.dot(dir);
        float p2 = dir.dot(g) * d;
        float q = g.dot(g) * d - r2;
        float dd = p2 * p2 - q;
        if (dd >= 0)
        {
            dd = sqrt(dd);
            *t = -p2 - dd;
            if (*t > EPSILON)
                return true;
            else
            {
                *t = -p2 + dd;
                if (*t > EPSILON)
                    return true;
            }
        }
        return false;
    }
    
    virtual void shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left, r_color* result)
    {
        float u, v;
        calculate_uv(p, &u, &v);

        float l = shading ? 10.0 * shading->query(u, v) : 1.0 - recursions_left * 0.3;
//         u /= M_PI;
//         v /= M_PI;
//         u *= 4;
//         v *= 4;
//         bool fu = (u - floor(u)) < 0.5;
//         bool fv = (v - floor(v)) < 0.5;
//         float base = (fu ^ fv) ? b : b * 0.3;
        float base = b;
//         r_vec3d t(p);
//         t.subtract(c);
//         t.normalize();
//         float base = diffuse->sample(u * 4, v * 4);

        if (((r_camera*)camera)->shading_pass)
        {
            *result = r_color(0, 0, 0);
            return;
        }
        if (recursions_left > 0)
        {
            r_vec3d n(p);
            n.subtract(c);
            n.normalize();
            r_vec3d reflected(n);
            reflected.multiply(2);
            reflected.multiply(dir.dot(n));
            reflected.subtract(dir);
            reflected.multiply(-1);
//             reflected.x += (((float)rand() / RAND_MAX) - 0.5) * 0.1;
//             reflected.y += (((float)rand() / RAND_MAX) - 0.5) * 0.1;
//             reflected.z += (((float)rand() / RAND_MAX) - 0.5) * 0.1;
            reflected.normalize();
            ((r_camera*)camera)->trace(p, reflected, result, recursions_left - 1, this);
        }
//         else
//             return l * base;
    }
    
    virtual void calculate_uv(const r_vec3d& p, float* u, float *v)
    {
        r_vec3d temp(p);
        temp.subtract(c);
        *u = (atan2(temp.x, temp.z) + M_PI) / (2 * M_PI);
        *v = acos(temp.y / radius) / M_PI;
    }
    
    virtual void calculate_p_from_uv(float u, float v, r_vec3d* p)
    {
        u = (u * 2.0 - 1.0) * M_PI;
        v = v * M_PI;
        *p = r_vec3d(radius * sin(v) * sin(u),
                     radius * cos(v),
                     radius * sin(v) * cos(u)
                     );
        p->add(c);
    }
    
    virtual void calculate_n(const r_vec3d& p, r_vec3d* n)
    {
        r_vec3d result(p);
        result.subtract(c);
        result.normalize();
        *n = result;
    }
    
    virtual void calculate_tangent(const r_vec3d& p, r_vec3d* tangent)
    {
        r_vec3d result;
        calculate_n(p, &result);
        r_vec3d temp(result);
        temp.cross(r_vec3d(1, 0, 0));
        *tangent = temp;
        temp = result;
        temp.cross(r_vec3d(0, 1, 0));
        if (temp.dot(temp) > tangent->dot(*tangent))
            *tangent = temp;
    }
};

int main(int argc, char** argv)
{
//       hdr_image* backdrop = new hdr_image("boiler_room_4k.hdr");
//     diffuse = new hdr_image("brown_planks_03_diff_4k.hdr", 300);
//     diffuse = new hdr_image("white_plaster_02_diff_4k.hdr", 300);
     hdr_image* backdrop = new hdr_image("small_hangar_01_8k.hdr");
//      hdr_image* backdrop = new hdr_image("flower_road_8k.hdr");
//      hdr_image* backdrop = new hdr_image("whipple_creek_regional_park_01_8k.hdr");
//      hdr_image* backdrop = new hdr_image("lythwood_field_8k.hdr");
//      hdr_image* backdrop = new hdr_image("schadowplatz_8k.hdr");
     
     
//      exit(1);
//     hdr_image* backdrop = new hdr_image("venice_sunset_8k.hdr");
//     exit(0);
//    hdr_image* backdrop = new hdr_image("paul_lobe_haus_8k.hdr");
//     hdr_image* backdrop = new hdr_image("pool_8k.hdr");
//     hdr_image* backdrop = new hdr_image("zhengyang_gate_8k.hdr");
    scene.add_backdrop(backdrop);
    
    scene.objects.push_back(new r_floor_disc(&scene, r_vec3d(0, 0, 0), r_vec3d(0, 1, 0), 8, 0.9));
    for (int k = 0; k < 32; k++)
    {
        if (k != 27)
            continue;
        int i = (k + 2) % 32;
        float r2 = 0.5 + pow((float)i / 32.0, 1.6) * 2.0;
        float x = cos(M_PI * k / 8) * r2;
        float y = sin(M_PI * k / 8) * r2;
        float r = 0.05 + 0.2 * pow((float)i / 32, 2.0);
        scene.objects.push_back(new r_sphere(&scene, r_vec3d(x, r + 0.0001, y), r, 0.0, 1.0));
    }
    camera = r_camera(1366 / 3, 768 / 3, 20.0 / 180.0 * M_PI,
        r_vec3d(-1, 0.3, -3),
        r_vec3d(0.0, 0.2, 0),
        r_vec3d(0, 1, 0)
    );
    scene.camera = &camera;
    camera.scene = &scene;
    
    camera.scale = 3;
    camera.aa_level = 2;
    camera.render_to_window = true;
   
    camera.render();
    
    delete diffuse;
    delete backdrop;
    return 0;
}
