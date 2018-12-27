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
    r_floor_disc(r_scene* scene)
        : r_obj(scene)
    {
    }
    
    virtual ~r_floor_disc()
    {
    }
    
    virtual bool intersect(float* t, const r_vec3d& from, const r_vec3d& dir)
    {
        if (fabs(dir.y) > 0.0001)
        {
            *t = -(from.y / dir.y);
            if (*t > 0.0)
            {
                r_vec3d x(dir);
                x.multiply(*t);
                x.add(from);
                if (x.x * x.x + x.z * x.z < 64)
                {
                    return true;
                }
            }
        }
        return false;
    }
    
    virtual float shade(const r_vec3d& from, const r_vec3d& dir, const r_vec3d& p, void* camera, int recursions_left)
    {
        float l = shading->query(p.x, p.z);
        r_vec3d x(p);
        x.multiply(0.5);
        bool fx = (x.x - floor(x.x)) < 0.5;
        bool fz = (x.z - floor(x.z)) < 0.5;
        return (fx ^ fz) ? 0.9 * l: 0.5 * l;
    }

    virtual float calculate_occlusion(float x, float y, r_scene* scene)
    {
        r_vec3d s_from(x, 0.0001, y);
        r_vec3d at(s_from);
        at.add(r_vec3d(0, 1, 0));
        r_camera lc(64, 64, 0.4 * M_PI,
            s_from, at, r_vec3d(1, 0, 0));
        lc.scene = scene;
        lc.shading_pass = true;

        FILE* f = NULL;
        if ((float)rand() / RAND_MAX < 0.001)
        {
            char filename[256];
            sprintf(filename, "lc_%1.5f_%1.5f.pgm", x, y);
            f = fopen(filename, "w");
        }
        float sum = 0.0;
        lc.render(f, 0, &sum);
        if (f) fclose(f);
//         sum = 1.0 - exp(-sum * 5);
        sum = pow(sum * 2.56, 2);
//         fprintf(stderr, "occlusion: %1.4f\n", sum);
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
            *t = -p2 - sqrt(dd);
            if (*t > 0.0)
                return true;
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
            r_vec3d t(reflected);
            t.multiply(0.000001);
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
    scene.objects.push_back(new r_floor_disc(&scene));
    scene.objects.back()->shading = new r_quadtree(scene.objects.back(), -8, 8, -8, 8, 0.5, 2);
    scene.objects.at(0)->shading->load_from_file("ao_plane.raw");
    scene.objects.push_back(new r_sphere(&scene, r_vec3d(0, 2.01, 0), 2, 0.5, 0.5));

    r_camera camera(512, 288, 20.0 / 180.0 * M_PI,
//     r_camera camera(2, 2, 20.0 / 180.0 * M_PI,
        r_vec3d(-3, 1.3, 5),
        r_vec3d(0, 1, 0),
        r_vec3d(0.2, 1, 0)
    );
//     r_camera camera(2, 2, 20.0 / 180.0 * M_PI,
//         r_vec3d(0, 15, 0),
//         r_vec3d(0, 0, 0),
//         r_vec3d(1, 0, 0.2)
//     );
    camera.scene = &scene;
    
    FILE *f = fopen(argv[1], "w");

//     scene.objects.at(0)->shading->query(-9, -9);
    camera.render(f, 3);
    fclose(f);
    
    scene.objects.at(0)->shading->dump();

    scene.objects.at(0)->shading->save_to_file("ao_plane.raw");
    scene.objects.at(0)->shading->save_to_texture("ao_plane.pbm");
    
    return 0;
}
