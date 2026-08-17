#pragma once
#include <cmath>

struct Vec3f {
	float x, y, z;

	Vec3f() : x(0), y(0), z(0) {}
	Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}

	float& operator[](int i)
	{
		if (i == 0) return x;
		if (i == 1) return y;
		return z;
	}

	float operator*(const Vec3f &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3f operator-(const Vec3f &v) const
	{
		return Vec3f(x - v.x, y - v.y, z - v.z);
	}
	Vec3f operator+(const Vec3f &v) const
	{
		return Vec3f(x + v.x, y + v.y, z + v.z);
	}

	Vec3f operator*(float f) const
	{
		return Vec3f(x * f, y * f, z * f);
	}

	float norm() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Vec3f& normalize()
	{
		float n = norm();
		x /= n;
		y /= n;
		z /= n;

		return *this;
	}
};

Vec3f reflect(const Vec3f &I, const Vec3f &N)
{
	return I - N * 2.f * (I * N);
}

struct Ray
{
	Vec3f origin;
	Vec3f direction;
};


struct Material
{
	Vec3f diffuse_color;
	float specular_exponent;
	float reflectivity;
	float refractive_index;
	float transparency;
};

struct Sphere
{
	Vec3f center;
	float radius;
	Vec3f diffuse_color;
	Material material;

	bool ray_intersect(const Ray &ray, float &t0) const
	{
		Vec3f L = center - ray.origin;

		float tca = L * ray.direction;
		float d2 = L * L - tca * tca;

		if (d2 > radius * radius)
		{
			return false;
		}

		float thc = sqrtf(radius * radius - d2);
		t0 = tca - thc;
		float t1 = tca + thc;

		if (t0 < 0)
		{
			t0 = t1;
		}
		if (t0 < 0)
		{
			return false;
		}
		return true;
	}
};

struct Light
{
	Vec3f position;
	float intensity;
};
