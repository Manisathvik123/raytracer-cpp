#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "geometry.h"

Vec3f cast_ray(const Ray &ray, const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
{
    float spheres_dist = std::numeric_limits<float>::max();
    const Sphere *hit_sphere = nullptr;

    for (const auto &sphere : spheres)
    {
        float dist_i;

        if (sphere.ray_intersect(ray, dist_i) && dist_i < spheres_dist)
        {
            spheres_dist = dist_i;
            hit_sphere = &sphere;
        }
    }

    if (!hit_sphere)
    {
        return Vec3f(0.2, 0.7, 0.8); // bg
    }

    Vec3f hit = ray.origin + ray.direction * spheres_dist;
    Vec3f N = (hit - hit_sphere->center).normalize();

    float diffuse_light_intensity = 0;
    float specular_light_intensity = 0;

    for (const auto &light : lights)
    {
        Vec3f light_dir = (light.position - hit).normalize();
        diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);
        specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N) * ray.direction), hit_sphere->material.specular_exponent) * light.intensity;
    }

    return hit_sphere->material.diffuse_color * diffuse_light_intensity + Vec3f(1., 1., 1.) * specular_light_intensity;
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
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
            framebuffer[i + j * width] = cast_ray(ray, spheres, lights);
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

    Material m1;
    m1.diffuse_color = Vec3f(0.4, 0.4, 0.3);
    m1.specular_exponent = 50.0;

    Sphere sphere1;
    sphere1.center = Vec3f(-3, 0, -16);
    sphere1.radius = 2;
    sphere1.material = m1;
    spheres.push_back(sphere1);

    Material m2;
    m2.diffuse_color = Vec3f(0.3, 0.1, 0.1);
    m2.specular_exponent = 80.0;

    Sphere sphere2;
    sphere2.center = Vec3f(-1.0, -1.5, -12);
    sphere2.radius = 2;
    sphere2.material = m2;
    spheres.push_back(sphere2);

    Material m3;
    m3.diffuse_color = Vec3f(0.7, 0.4, 0.3);
    m3.specular_exponent = 67.0;
    
    Sphere sphere3;
    sphere3.center = Vec3f(-1.5, -0.5, -18);
    sphere3.radius = 3;
    sphere3.material = m3;
    spheres.push_back(sphere3);

    Material m4;
    m4.diffuse_color = Vec3f(0.3, 0.7, 0.1);
    m4.specular_exponent = 69.0;

    Sphere sphere4;
    sphere4.center = Vec3f(7, 5, -18);
    sphere4.radius = 4;
    sphere4.material = m4;
    spheres.push_back(sphere4);

    std::vector<Light> lights;

    Light l1;
    l1.position = Vec3f(-20, 20, 20);
    l1.intensity = 1.5;
    lights.push_back(l1);

    render(spheres, lights);
    

    return 0;
}