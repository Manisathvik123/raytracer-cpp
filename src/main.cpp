#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "geometry.h"

struct EnvironmentMap
{
    unsigned char *data;
    int width;
    int height;
};

Vec3f cast_ray(const Ray &ray, const std::vector<Sphere> &spheres, const Plane &ground, const Triangle &triangle, const std::vector<Light> &lights, const EnvironmentMap &env, int depth)
{
    const int max_depth = 4;

    if (depth > max_depth)
    {
        return Vec3f(0.2, 0.7, 0.8);
    }
    float closest_dist = std::numeric_limits<float>::max();
    const Sphere *hit_sphere = nullptr;
    const Plane *hit_plane = nullptr;
    const Triangle *hit_triangle = nullptr;

    for (const auto &sphere : spheres)
    {
        float dist_i;

        if (sphere.ray_intersect(ray, dist_i) && dist_i < closest_dist)
        {
            closest_dist = dist_i;
            hit_sphere = &sphere;
        }
    }

    float plane_dist;

    if (ground.ray_intersect(ray, plane_dist) && plane_dist < closest_dist)
    {
        closest_dist = plane_dist;
        hit_plane = &ground;
    }

    float triangle_dist;

    if (triangle.ray_intersect(ray, triangle_dist) && triangle_dist < closest_dist)
    {
        closest_dist = triangle_dist;
        hit_triangle = &triangle;
    }

    if (!hit_sphere && !hit_plane && !hit_triangle)
    {
        float u = atan2f(ray.direction.z, ray.direction.x) / (2.0f * M_PI) + 0.5f;
        float v = 0.5 - asinf(ray.direction.y) / M_PI;

        u = u - floorf(u);
        v = std::max(0.0f, std::min(1.0f, v));

        int px = static_cast<int>(u * (env.width - 1));
        int py = static_cast<int>(v * (env.height - 1));

        int idx = (py * env.width + px) * 3;

        Vec3f environment_color(env.data[idx] / 255.0f, env.data[idx + 1] / 255.0f, env.data[idx + 2] / 255.0f);

        return environment_color;
    }

    Vec3f hit = ray.origin + ray.direction * closest_dist;
    Vec3f N;

    if (hit_sphere)
    {
        N = (hit - hit_sphere->center).normalize();
    }
    else if (hit_plane)
    {
        N = ground.normal;
    }
    else
    {
        N = ((hit_triangle->v1 - hit_triangle->v0) ^ (hit_triangle->v2 - hit_triangle->v0)).normalize();
    }

    const Material *hit_material;

    if (hit_sphere)
    {
        hit_material = &hit_sphere->material;
    }
    else if (hit_plane)
    {
        hit_material = &ground.material;
    }
    else
    {
        hit_material = &hit_triangle->material;
    }

    float cosi = -(ray.direction * N);

    float ri_from = 1.0f;
    float ri_to = hit_material->refractive_index;

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

        refract_color = cast_ray(refract_ray, spheres, ground, triangle, lights, env, depth + 1);
    }

    Vec3f reflect_dir = reflect(ray.direction, N).normalize();

    Ray reflect_ray;
    reflect_ray.origin = hit + N * 1e-3f;
    reflect_ray.direction = reflect_dir;

    Vec3f reflect_color = cast_ray(reflect_ray, spheres, ground, triangle, lights, env, depth + 1);

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

        float ground_shadow_dist;

        if (ground.ray_intersect(shadow_ray, ground_shadow_dist) && ground_shadow_dist > 1e-3f && ground_shadow_dist < light_distance)
        {
            in_shadow = true;
        }

        float trianle_shadow_distance;

        if (triangle.ray_intersect(shadow_ray, triangle_dist) && trianle_shadow_distance > 1e-3f && triangle_dist < light_distance)
        {
            in_shadow = true;
        }


        if (!in_shadow)
        {
            diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * N);
            specular_light_intensity += powf(std::max(0.f, reflect(light_dir, N) * ray.direction), hit_material->specular_exponent) * light.intensity;
        }
    }

    Vec3f surface_color = hit_material->diffuse_color * diffuse_light_intensity + Vec3f(1., 1., 1.) * specular_light_intensity;

    float reflectivity = hit_material->reflectivity;
    float transparency = hit_material->transparency;

    Vec3f final_color = surface_color * (1.f - reflectivity - transparency) + reflect_color * reflectivity + refract_color * transparency;

    return final_color;
}

void render(const std::vector<Sphere> &spheres, const Plane &ground, const Triangle &triangle, const std::vector<Light> &lights, const EnvironmentMap &env)
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
            framebuffer[i + j * width] = cast_ray(ray, spheres, ground, triangle, lights, env, 0);
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

    int env_width;
    int env_height;
    int env_channels;

    unsigned char *env_data = stbi_load("image.jpg", &env_width, &env_height, &env_channels, 3);

    if (!env_data)
    {
        std::cerr << "Failed to load environment map\n";
        return 1;
    }

    std::cout << "Environment loaded: " << env_width << "x" << env_height << "\n";

    EnvironmentMap env;
    env.data = env_data;
    env.width = env_width;
    env.height = env_height;

    std::vector<Sphere> spheres;

    Material m1(Vec3f(0.4, 0.4, 0.3), 50.0f, 0.0f, 1.0f, 0.0f);

    Sphere sphere1;
    sphere1.center = Vec3f(-4.2f, -1.3f, -15.0f);
    sphere1.radius = 2.2f;
    sphere1.material = m1;
    spheres.push_back(sphere1);

    Material m2(Vec3f(0.3, 0.1, 0.1), 80.0f, 0.5f, 1.0f, 0.0f);

    Sphere sphere2;
    sphere2.center = Vec3f(-1.0f, -1.2f, -12.0f);
    sphere2.radius = 2.3f;
    sphere2.material = m2;
    spheres.push_back(sphere2);

    Material m3(Vec3f(0.7, 0.15, 0.1), 67.0f, 0.2f, 1.0f, 0.0f);
    
    Sphere sphere3;
    sphere3.center = Vec3f(2.5f, -0.8f, -18.0f);
    sphere3.radius = 2.7f;
    sphere3.material = m3;
    spheres.push_back(sphere3);

    Material m4(Vec3f(0.7, 0.85, 0.95), 69.0f, 0.2f, 1.5f, 0.8f);

    Sphere sphere4;
    sphere4.center = Vec3f(6.5f, 4.5f, -18.0f);
    sphere4.radius = 3.5f;
    sphere4.material = m4;
    spheres.push_back(sphere4);

    Plane ground;
    ground.point = Vec3f(0, -3.5f, 0);
    ground.normal = Vec3f(0, 1, 0);
    ground.material = Material(Vec3f(0.35f, 0.35f, 0.35f), 20.0f, 0.1f, 1.0f, 0.0f);

    Triangle triangle1(Vec3f(11.5f, 0.5f, -18.0f), Vec3f(15.5f, 0.5f, -18.0f), Vec3f(13.5f, 4.5f, -18.0f), Material(Vec3f(0.55f, 0.12f, 0.04f), 100.0f, 0.35f, 1.0f, 0.0f)
);

    std::vector<Light> lights;

    Light l1;
    l1.position = Vec3f(-10, 15, 10);
    l1.intensity = 1.5;
    lights.push_back(l1);

    render(spheres, ground, triangle1, lights, env);
    

    return 0;
}