#pragma once

#include "math.h"

namespace noise {
extern bool initialized;

double fade(double t);
double lerp(double t, double a, double b);
double grad(int hash, double x, double y, double z);

extern int p[512];
extern int permutation[];

void initNoise();
double noise(double x, double y, double z);
}  // namespace noise