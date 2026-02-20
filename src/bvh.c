#include <math.h>
#include "bvh.h"
#include <stdlib.h>

static inline double minDouble (double a, double b) {
    return (a < b ? a : b);
}

static inline double maxDouble (double a, double b) {
    return (a > b ? a : b);
}

static BVHObject createBVHObject (void * data, GeometryType type, int index) {
    BVHObject newObject;
    newObject.type = type;        
    newObject.index = index;

    if (type == TRIANGLE) {
        Triangle triangle = *((Triangle *) data);

        newObject.bounds.min.x = minDouble(triangle.p1.x, minDouble(triangle.p2.x, triangle.p3.x));
        newObject.bounds.min.y = minDouble(triangle.p1.y, minDouble(triangle.p2.y, triangle.p3.y));
        newObject.bounds.min.z = minDouble(triangle.p1.z, minDouble(triangle.p2.z, triangle.p3.z));

        newObject.bounds.max.x = maxDouble(triangle.p1.x, maxDouble(triangle.p2.x, triangle.p3.x));
        newObject.bounds.max.y = maxDouble(triangle.p1.y, maxDouble(triangle.p2.y, triangle.p3.y));
        newObject.bounds.max.z = maxDouble(triangle.p1.z, maxDouble(triangle.p2.z, triangle.p3.z));

        newObject.bounds.min.x -= RAY_EPSILON; newObject.bounds.min.y -= RAY_EPSILON; newObject.bounds.min.z -= RAY_EPSILON;
        newObject.bounds.max.x += RAY_EPSILON; newObject.bounds.max.y += RAY_EPSILON; newObject.bounds.max.z += RAY_EPSILON;

        newObject.centroid.x = (triangle.p1.x + triangle.p2.x + triangle.p3.x)/ 3.0;
        newObject.centroid.y = (triangle.p1.y + triangle.p2.y + triangle.p3.y)/ 3.0;
        newObject.centroid.z = (triangle.p1.z + triangle.p2.z + triangle.p3.z)/ 3.0;

    } else if (type == SPHERE) {
        Sphere sphere = *((Sphere * ) data);

        newObject.bounds.min.x = sphere.center.x - sphere.radius;
        newObject.bounds.min.y = sphere.center.y - sphere.radius;
        newObject.bounds.min.z = sphere.center.z - sphere.radius;
        newObject.bounds.max.x = sphere.center.x + sphere.radius;
        newObject.bounds.max.y = sphere.center.y + sphere.radius;
        newObject.bounds.max.z = sphere.center.z + sphere.radius;

        newObject.centroid = sphere.center;
    }

    return newObject;
}

static int compareX (const void *a, const void *b ) {
    const BVHObject * objA = (const BVHObject *) a;
    const BVHObject * objB = (const BVHObject *) b;

    if (objA->centroid.x > objB->centroid.x) return 1;
    else if (objA->centroid.x < objB->centroid.x) return -1;
    return 0;
}

static int compareY (const void *a, const void *b ) {
    const BVHObject * objA = (const BVHObject *) a;
    const BVHObject * objB = (const BVHObject *) b;

    if (objA->centroid.y > objB->centroid.y) return 1;
    else if (objA->centroid.y < objB->centroid.y) return -1;
    return 0;
}

static int compareZ (const void *a, const void *b ) {
    const BVHObject * objA = (const BVHObject *) a;
    const BVHObject * objB = (const BVHObject *) b;

    if (objA->centroid.z > objB->centroid.z) return 1;
    else if (objA->centroid.z < objB->centroid.z) return -1;
    return 0;
}

static BVHNode * createBVHNode (BVHObject * bvhArray, int start, int end) {
    int count = end - start ;
    if (count <= 0) return NULL;

    BVHNode * newNode = malloc(sizeof(BVHNode));

    if (count == 1) {
        newNode->index = bvhArray[start].index;
        newNode->type = bvhArray [start].type;
        newNode->bounds = bvhArray[start].bounds;
        newNode->left = NULL;
        newNode->right = NULL;
    } else {
        BoundingBox centroidVolume = {.min = {INFINITY, INFINITY, INFINITY}, .max = {-INFINITY, -INFINITY, -INFINITY}};
        BoundingBox boundingVolume = {.min = {INFINITY, INFINITY, INFINITY}, .max = {-INFINITY, -INFINITY, -INFINITY}};
        for (int i = start; i < end; ++ i ) {
            Point p = bvhArray[i].centroid;
            BoundingBox bounds = bvhArray[i].bounds;
            
            centroidVolume.min.x = minDouble (centroidVolume.min.x, p.x);
            centroidVolume.min.y = minDouble (centroidVolume.min.y, p.y);
            centroidVolume.min.z = minDouble (centroidVolume.min.z, p.z);

            centroidVolume.max.x = maxDouble (centroidVolume.max.x, p.x);
            centroidVolume.max.y = maxDouble (centroidVolume.max.y, p.y);
            centroidVolume.max.z = maxDouble (centroidVolume.max.z, p.z);
            
            boundingVolume.min.x = minDouble (boundingVolume.min.x, bounds.min.x);
            boundingVolume.min.y = minDouble (boundingVolume.min.y, bounds.min.y);
            boundingVolume.min.z = minDouble (boundingVolume.min.z, bounds.min.z);

            boundingVolume.max.x = maxDouble (boundingVolume.max.x, bounds.max.x);
            boundingVolume.max.y = maxDouble (boundingVolume.max.y, bounds.max.y);
            boundingVolume.max.z = maxDouble (boundingVolume.max.z, bounds.max.z);
        }

        newNode->index = -1;
        newNode->type = -1;
        newNode->bounds = boundingVolume;

        double x = centroidVolume.max.x - centroidVolume.min.x;
        double y = centroidVolume.max.y - centroidVolume.min.y;
        double z = centroidVolume.max.z - centroidVolume.min.z; 

        if (x > y && x > z) {
            qsort(bvhArray + start, count, sizeof(BVHObject), compareX);
        } else if (y > x && y > z) {
            qsort(bvhArray + start, count, sizeof(BVHObject), compareY);
        } else {
            qsort(bvhArray + start, count, sizeof(BVHObject), compareZ);
        }

        int mid = start + count / 2;
        newNode->left = createBVHNode(bvhArray, start, mid);
        newNode->right = createBVHNode(bvhArray, mid, end);
    }
    

    return newNode;
}


void createBVH (Scene * scene) {
    int totalNumberOfObjects = scene->numTriangles + scene->numSpheres;
    BVHObject * bvhArray = malloc(totalNumberOfObjects * sizeof(BVHObject));

    int index = 0;
    for (int i = 0; i < scene->numTriangles; ++ i) {
        bvhArray [index++] = createBVHObject (&(scene->triangles[i]), TRIANGLE, i);
    }

    for (int i = 0; i < scene->numSpheres; ++ i) {
        bvhArray [index++] = createBVHObject (&(scene->spheres[i]), SPHERE, i);
    }

    scene->root = createBVHNode(bvhArray, 0, totalNumberOfObjects);

    free (bvhArray);
}