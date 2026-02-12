#include "ray.h"
#include <float.h>

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

bool getSphereHit (Sphere sphere, Ray ray, double minDist, double maxDist, HitRecord * record) {
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
