#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include "ray.h"

#define MAX_BOUNCES 20 

int tracePath (Ray ray, HitRecord * path, int totalBounces, Scene * scene);
Vector calculatePathColor (HitRecord * path, int numHits, Scene * scene);

#endif