#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include "ray.h"
#include "rand.h"

#define MAX_BOUNCES 20

int tracePath (Ray ray, HitRecord * path, int totalBounces, Scene * scene, Seed * seed);
Vector calculatePathColor (HitRecord * path, int numHits, Scene * scene, Seed * seed);

#endif