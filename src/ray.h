#ifndef RAY_H
#define RAY_H

#include "geometry.h"
#include <stdbool.h>
#include "constants.h"

typedef struct {
    Point origin;
    Vector vector;
} Ray;

typedef struct {
    double distance;
    Point intersection;
    Vector normal;
    int materialId;
} HitRecord;

bool getTriangleHit (Triangle triangle, Ray ray, double minDist, double maxDist, HitRecord * record);
bool getSphereHit (Sphere sphere, Ray ray, double minDist, double maxDist, HitRecord * record);
bool getSceneHitBVH (Scene * scene, Ray ray, HitRecord * record);
bool getSceneHit (Scene * scene, Ray ray, HitRecord * record);

#endif
