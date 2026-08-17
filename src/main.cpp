#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "geometry.h"

Vec3f cast_ray(const Ray &ray, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, int depth)
{
    const int max_depth = 4;

    if (depth > max_depth)
    {
        return Vec3f(0.2, 0.7, 0.8);
    }
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

    float cosi = -(ray.direction * N);

    float ri_from = 1.0f;
    float ri_to = hit_sphere->material.refractive_index;

    if (cosi < 0)
    {
        cosi = -cosi;
        std::swap(ri_from, ri_to);
        N = N * -1.f;
    }

    float ri_ratio = ri_from / ri_to;

    float sin2_t = ri_ratio * ri_ratio * (1 - cosi * cosi);
    Vec3f refract_color(0, 0, 0);
    if (sin2_t <= 1.0f)
    {
        float cost = sqrtf(1.0f - sin2_t);

        Vec3f refract_dir = ray.direction * ri_ratio + N * (ri_ratio * cosi - cost);

        Ray refract_ray;
        refract_ray.origin = hit - N * 1e-3f;
        refract_ray.direction = refract_dir;

        refract_color = cast_ray(refract_ray, spheres, lights, depth + 1);
    }

    Vec3f reflect_dir = reflect(ray.direction, N).normalize();

    Ray reflect_ray;
    reflect_ray.origin = hit + N * 1e-3f;
    reflect_ray.direction = reflect_dir;

    Vec3f reflect_color = cast_ray(reflect_ray, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0;
    float specular_light_intensity = 0;

    for (const auto &light : lights)
    {
        Vec3f light_vec = light.position - hit;
        Vec3f light_dir = light_vec.normalize();
        float light_distance = light_vec.norm();

        Ray shadow_ray;
        shadow_ray.origin = hit + N * 1e-3f;
        shadow_ray.direction = light_dir;
        
        bool in_shadow = false;

        for (const auto &sphere : spheres)
        {
            float shadow_dist;

            if (sphere.ray_intersect(shadow_ray, shadow_dist) && shadow_dist < light_distance)
            {
                in_shadow = true;
                break;
            }
        }
        if (!in_shadow)
        {
            diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);
            specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N) * ray.direction), hit_sphere->material.specular_exponent) * light.intensity;
        }
    }

    Vec3f surface_color = hit_sphere->material.diffuse_color * diffuse_light_intensity + Vec3f(1., 1., 1.) * specular_light_intensity;

    float reflectivity = hit_sphere->material.reflectivity;
    float transparency = hit_sphere->material.transparency;

    Vec3f final_color = surface_color * (1.f - reflectivity - transparency) + reflect_color * reflectivity + refract_color * transparency;

    return final_color;
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
            framebuffer[i + j * width] = cast_ray(ray, spheres, lights, 0);
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
    m1.reflectivity = 0.0;
    m1.refractive_index = 1.0;
    m1.transparency = 0.0f;

    Sphere sphere1;
    sphere1.center = Vec3f(-3, 0, -16);
    sphere1.radius = 2;
    sphere1.material = m1;
    spheres.push_back(sphere1);

    Material m2;
    m2.diffuse_color = Vec3f(0.3, 0.1, 0.1);
    m2.specular_exponent = 80.0;
    m2.reflectivity = 0.2;
    m2.refractive_index = 1.0;
    m2.transparency = 0.0f;

    Sphere sphere2;
    sphere2.center = Vec3f(-1.0, -1.5, -12);
    sphere2.radius = 2;
    sphere2.material = m2;
    spheres.push_back(sphere2);

    Material m3;
    m3.diffuse_color = Vec3f(0.7, 0.4, 0.3);
    m3.specular_exponent = 67.0;
    m3.reflectivity = 0.2;
    m3.refractive_index = 1.5;
    m3.transparency = 0.8f;
    
    Sphere sphere3;
    sphere3.center = Vec3f(-1.5, -0.5, -18);
    sphere3.radius = 3;
    sphere3.material = m3;
    spheres.push_back(sphere3);

    Material m4;
    m4.diffuse_color = Vec3f(0.3, 0.7, 0.1);
    m4.specular_exponent = 69.0;
    m4.reflectivity = 0.8;
    m4.refractive_index = 1.0;
    m4.transparency = 0.0f;

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