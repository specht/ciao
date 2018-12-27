#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include "camera.h"
#include "obj.h"
#include "quadtree.h"
#include "scene.h"
#include "vec3d.h"

r_scene scene;
    
struct r_floor_disc: r_obj {
    float r, r2; 
    r_vec3d anchor, n, u, v;
    
    r_floor_disc(r_scene* scene, const r_vec3d& _p, const r_vec3d& _n, float _radius)
        : r_obj(scene)
        , anchor(_p)
        , n(_n)
        , r(_radius)
        , r2(_radius * _radius)
    {
        n.normalize();
        shading = new r_quadtree(this, -r, r, -r, r, 0.5, 5);
        float lmax = 0.0;
        for (int i = 0; i < 3; i++)
        {
            r_vec3d t = r_vec3d(i == 0 ? 1 : 0, i == 1 ? 1 : 0, i == 2 ? 1 : 0);
            t.cross(n);
            float l = t.dot(t);
            if (l > lmax)
            {
                lmax = l;
                u = t;
            }
        }
        u.normalize();
        v = u;
        v.cross(n);
    }
    
    virtual ~r_floor_disc()
    {
    }
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir)
    {
        r_vec3d f(from);
        f.subtract(anchor);
        float d = dir.dot(n);
        if (fabs(d) > EPSILON)
        {
            *t = -n.dot(f) / d;
            if (*t > EPSILON)
            {
                r_vec3d x(dir);
                x.multiply(*t);
                x.add(f);
                if (x.x * x.x + x.z * x.z < r2)
                {
                    return true;
                }
            }
        }
        return false;
    }
    
    virtual float shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left)
    {
        r_vec3d temp(p);
        temp.subtract(anchor);
        float l = shading->query(u.dot(temp), v.dot(temp));
//         float l = 1.0;
        r_vec3d x(p);
        x.subtract(anchor);
        float uc = u.dot(x) * 0.5;
        float vc = v.dot(x) * 0.5;
        bool fu = (uc - floor(uc)) < 0.5;
        bool fv = (vc - floor(vc)) < 0.5;
        return (fu ^ fv) ? 0.9 * l: 0.5 * l;
//         return 0.9 * l;
    }

    virtual float calculate_occlusion(float x, float y, r_scene* scene)
    {
        r_vec3d tu(u);
        r_vec3d tv(v);
        tu.multiply(x);
        tv.multiply(y);
        
        r_vec3d s_from(anchor);
        s_from.add(tu);
        s_from.add(tv);
        r_vec3d at(s_from);
        at.add(n);
        // TODO experiment with size and antialiasing of this image
        r_camera lc(LIGHT_FRAME_SIZE, LIGHT_FRAME_SIZE, 0.4 * M_PI,
            s_from, at, u);
        lc.scene = scene;
        lc.shading_pass = true;

        FILE* f = NULL;
//         if ((float)rand() / RAND_MAX < 0.001)
//         {
//             char filename[256];
//             sprintf(filename, "%s_lc_%1.5f_%1.5f.pgm", lm_path, x, y);
//             f = fopen(filename, "w");
//         }
        float sum = 0.0;
        lc.render(f, LIGHT_FRAME_ANTIALIASING, 0, &sum);
        if (f) fclose(f);
//         fprintf(stderr, "%1.4f\n", sum);
        return sum;
    }
};

struct r_sphere: r_obj {
    r_vec3d c;
    float r2;
    float b, r;
    
    r_sphere(r_scene* scene, const r_vec3d& _center, float _radius, float _b = 0.5, float _r = 0.5)
        : r_obj(scene), c(_center), r2(_radius * _radius), b(_b), r(_r)
    {
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
    
    virtual float shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left)
    {
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
            reflected.normalize();
            r_vec3d t(reflected);
            t.multiply(EPSILON * 2);
            t.add(p);
            return b + r * ((r_camera*)camera)->trace(t, reflected, recursions_left - 1);
        }
        else
            return b;
    }
    
    virtual float calculate_occlusion(float x, float y, r_scene* scene)
    {
        return 1;
    }
};

int main(int argc, char** argv)
{
    scene.objects.push_back(new r_floor_disc(&scene, r_vec3d(0, 0, 0), r_vec3d(0, 1, 0), 8));
//     scene.objects.push_back(new r_floor_disc(&scene, r_vec3d(0, 0.4, 0), r_vec3d(-0.1, 1, 0), 1));
//     scene.objects.push_back(new r_floor_disc(&scene, r_vec3d(-1, 0.7, 0), r_vec3d(-0.2, 1, 0), 1));
//     scene.objects.push_back(new r_floor_disc(&scene, r_vec3d(0, 0, 0), r_vec3d(-1, -0.8, 0), 2));
    scene.objects.push_back(new r_sphere(&scene, r_vec3d(0, 2.01, 0), 2, 0.3, 0.7));
    for (int i = 0; i < 16; i++)
    {
        float x = cos(M_PI * i / 8) * 2;
        float y = sin(M_PI * i / 8) * 2;
        scene.objects.push_back(new r_sphere(&scene, r_vec3d(x, 0.21, y), 0.2, 0.3, 0.7));
    }

    r_camera camera(512 * 2, 288 * 2, 20.0 / 180.0 * M_PI,
        r_vec3d(-3, 1.3, 5),
        r_vec3d(0, 1, 0),
        r_vec3d(0.2, 1, 0)
    );
    camera.scene = &scene;
    
    FILE *f = fopen(argv[1], "w");
    camera.render(f, 3, 3);
    fclose(f);
    
    return 0;
}
