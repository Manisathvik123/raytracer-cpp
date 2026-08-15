#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "geometry.h"

Vec3f cast_ray(const Ray &ray, const Sphere & sphere)
{
    float sphere_dist = std::numeric_limits<float>::max();

    if (!sphere.ray_intersect(ray, sphere_dist))
    {
        return Vec3f(0.2, 0.7, 0.8); // bg color
    }
    return Vec3f(0.4, 0.4, 0.3); // sphere color
}

void render(const Sphere & sphere)
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
            framebuffer[i + j * width] = cast_ray(ray, sphere);
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
    Sphere sphere;
    sphere.center = Vec3f(-3, 0, -16);
    sphere.radius = 2;
    render(sphere);
    

    return 0;
}