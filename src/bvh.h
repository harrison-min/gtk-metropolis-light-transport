#ifndef BVH_H
#define BVH_H

#include "geometry.h"
typedef enum {
    TRIANGLE,
    SPHERE
} GeometryType;

struct _BVHNode {
    BoundingBox bounds;
    BVHNode * left;
    BVHNode * right;
    GeometryType type;
    int index;
};

typedef struct {
    GeometryType type;
    BoundingBox bounds;
    Point centroid;
    int index;
} BVHObject;

void createBVH (Scene * scene);
#endif