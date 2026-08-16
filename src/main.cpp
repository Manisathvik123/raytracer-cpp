#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "geometry.h"

Vec3f cast_ray(const Ray &ray, const std::vector<Sphere> &spheres)
{
    float spheres_dist = std::numeric_limits<float>::max();
    Vec3f hit_color = Vec3f(0.2, 0.7, 0.8);

    for (const auto &sphere : spheres)
    {
        float dist_i;

        if (sphere.ray_intersect(ray, dist_i) && dist_i < spheres_dist)
        {
            spheres_dist = dist_i;
            hit_color = sphere.diffuse_color;
        }
    }
    return hit_color;
}

void render(const std::vector<Sphere> &spheres)
{
    const int width = 1024;
    const int height = 768;

    const float fov = M_PI / 2.0f;
    std::vector<Vec3f> framebuffer(width * height);

    for (size_t j = 0; j < height; j++)
    {
        for (size_t i = 0; i < width; i++)
        {
            float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.) * width / (float)height;
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.);
            Vec3f dir = Vec3f(x, y, -1).normalize();
            Ray ray;
            ray.origin = Vec3f(0, 0, 0);
            ray.direction = dir;
            framebuffer[i + j * width] = cast_ray(ray, spheres);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";

    for (size_t i = 0; i < height * width; i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            ofs << static_cast<char>(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main()
{
    std::vector<Sphere> spheres;

    Sphere sphere1;
    sphere1.center = Vec3f(-3, 0, -16);
    sphere1.radius = 2;
    sphere1.diffuse_color = Vec3f(0.4, 0.4, 0.3);
    spheres.push_back(sphere1);

    Sphere sphere2;
    sphere2.center = Vec3f(-1.0, -1.5, -12);
    sphere2.radius = 2;
    sphere2.diffuse_color = Vec3f(0.3, 0.1, 0.1);
    spheres.push_back(sphere2);
    
    Sphere sphere3;
    sphere3.center = Vec3f(-1.5, -0.5, -18);
    sphere3.radius = 3;
    sphere3.diffuse_color = Vec3f(0.7, 0.4, 0.3);
    spheres.push_back(sphere3);

    Sphere sphere4;
    sphere4.center = Vec3f(7, 5, -18);
    sphere4.radius = 4;
    sphere4.diffuse_color = Vec3f(0.3, 0.7, 0.1);
    spheres.push_back(sphere4);

    render(spheres);
    

    return 0;
}