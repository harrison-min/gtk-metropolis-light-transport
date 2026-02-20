#include "ray.h"
#include "bvh.h"
#include <float.h>
#include <stdio.h>

bool getTriangleHit (Triangle triangle, Ray ray, double minDist, double maxDist, HitRecord * record) {
    //Moller Trumbore intersection algorithm
    Vector rayCrossE2 = crossProduct (ray.vector, triangle.edge2);
    double det = dotProduct (triangle.edge1, rayCrossE2);

    if (det > -DBL_EPSILON && det < DBL_EPSILON) {
        return false;
    }

    double inverseDet = 1.0 / det;

    Vector s = getVector (triangle.p1, ray.origin);
    double u = dotProduct (s, rayCrossE2) * inverseDet;

    if ((u < 0 && fabs (u) > DBL_EPSILON) || (u > 1 && fabs (u - 1) > DBL_EPSILON)) {
        return false;
    }

    Vector sCrossE1 = crossProduct (s, triangle.edge1);
    double v = inverseDet * dotProduct (ray.vector, sCrossE1);

    if ((v < 0 && fabs (v) > DBL_EPSILON) || ((u + v) > 1 && fabs (u + v - 1) > DBL_EPSILON)) {
        return false;
    }

    double distance = inverseDet * dotProduct (triangle.edge2, sCrossE1);

    if (distance < minDist || distance > maxDist) {
        return false;
    }

    record->distance = distance;
    record->intersection = movePoint (ray.origin, scaleVector (ray.vector, distance));
    record->normal = triangle.normal;
    record->materialId = triangle.materialId;
    return true;
}

bool getSphereHit (Sphere sphere, Ray ray, double minDist, double maxDist, HitRecord * record){
    Vector originToCenter = getVector (sphere.center, ray.origin);
    double a = dotProduct (ray.vector, ray.vector);
    double halfB = dotProduct (originToCenter, ray.vector);
    double c = dotProduct (originToCenter, originToCenter) - sphere.radius * sphere.radius;
    double discriminant = halfB * halfB - a * c;

    if (discriminant < 0.0) {
        return false;
    }

    double sqrtDisc = sqrt (discriminant);
    double distance = (-halfB - sqrtDisc) / a;

    if (distance < minDist || distance > maxDist) {
        distance = (-halfB + sqrtDisc) / a;
        if (distance < minDist || distance > maxDist) {
            return false;
        }
    }

    record->distance = distance;
    record->intersection = movePoint (ray.origin, scaleVector (ray.vector, distance));
    record->normal = scaleVector (getVector (sphere.center, record->intersection), 1.0 / sphere.radius);
    record->materialId = sphere.materialId;
    return true;
}

static bool boundingBoxHit (BoundingBox * box, Ray ray) {
    //slab method
    double close = -INFINITY, far = INFINITY;
    double tempTLow, tempTHigh;

    double invX = 1.0/ ray.vector.x;
    double invY = 1.0/ ray.vector.y;
    double invZ = 1.0/ ray.vector.z;

    tempTLow = (box->min.x - ray.origin.x) * invX;
    tempTHigh = (box->max.x - ray.origin.x) * invX;
    close = fmax(close, fmin(tempTLow, tempTHigh));
    far = fmin(far, fmax(tempTLow, tempTHigh));
    
    tempTLow = (box->min.y - ray.origin.y) * invY;
    tempTHigh = (box->max.y - ray.origin.y) * invY;
    close = fmax(close, fmin(tempTLow, tempTHigh));
    far = fmin(far, fmax(tempTLow, tempTHigh));

    tempTLow = (box->min.z - ray.origin.z) * invZ;
    tempTHigh = (box->max.z - ray.origin.z) * invZ;
    close = fmax(close, fmin(tempTLow, tempTHigh));
    far = fmin(far, fmax(tempTLow, tempTHigh));

    if (close > far || far < RAY_EPSILON) return false;
    return true;
}

static bool getBVHHit (Scene * scene, BVHNode * currentNode, Ray ray, double minDist, double maxDist, HitRecord * record) {
    if (currentNode == NULL) return false;
    if (!boundingBoxHit (&(currentNode->bounds), ray)) return false;

    if (currentNode->left || currentNode->right) {
        bool leftResult = getBVHHit(scene, currentNode->left, ray, minDist, maxDist, record);
        
        if (leftResult) maxDist = record->distance;

        bool rightResult = getBVHHit(scene, currentNode->right, ray, minDist, maxDist, record);

        return leftResult || rightResult;
    }

    if (currentNode->type == TRIANGLE) {
        return getTriangleHit (scene->triangles[currentNode->index], ray, minDist, maxDist, record);
    } else if (currentNode->type == SPHERE) {
        return getSphereHit (scene->spheres[currentNode->index], ray, minDist, maxDist, record);
    }

    return false;

}

bool getSceneHitBVH (Scene * scene, Ray ray, HitRecord * record) {
    double maxDistance = 1e20;
    
    return getBVHHit(scene, scene->root, ray, RAY_EPSILON, maxDistance, record);
}

bool getSceneHit (Scene * scene, Ray ray, HitRecord * record) {
    double closest = 1e20;
    bool hit = false;
    HitRecord temp;

    for (int i = 0; i < scene->numSpheres; ++ i) {
        if (getSphereHit (scene->spheres[i], ray, RAY_EPSILON, closest, &temp)) {
            closest = temp.distance;
            *record = temp;
            hit = true;
        }
    }

    for (int i = 0; i < scene->numTriangles; ++ i) {
        if (getTriangleHit (scene->triangles[i], ray, RAY_EPSILON, closest, &temp)) {
            closest = temp.distance;
            *record = temp;
            hit = true;
        }
    }

    return hit;
}