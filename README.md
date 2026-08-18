# raytracer-cpp

A small ray tracer I built from scratch in C++ while following along with
ssloy's TinyRayTracer and experimenting with the concepts on my own.

## Final Render

![Final Render](renders/final_render.jpg)

## What it does

I started with basic ray-sphere intersection and gradually added more features
as I went:

- **Lighting** — Diffuse and specular (Phong) lighting for the objects.
- **Shadows** — Shadow rays to check whether a light source is blocked.
- **Reflections** — Recursive rays for reflective surfaces.
- **Refraction** — Transparent materials and glass-like objects.
- **Ground Plane** — Added a plane that can receive shadows and reflections.
- **Triangles** — Added triangle intersections using the Möller-Trumbore algorithm.
- **Environment Mapping** — Added an equirectangular image as the environment,
  so the background also shows up in reflections and refractions.
- **Custom Scene** — Put everything together with different materials,
  objects and lighting to create the final scene.

## Built With

- C++
- STL
- stb_image
- Git & GitHub

## Running it

```bash
g++ src/main.cpp -o raytracer
./raytracer