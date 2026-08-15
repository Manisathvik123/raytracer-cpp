#pragma once

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
};
