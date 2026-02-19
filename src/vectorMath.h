#ifndef VECTOR_MATH_H
#define VECTOR_MATH_H

#include <math.h>

#include "constants.h"


typedef struct {
    double x, y, z;
} Vector;

typedef struct {
    double x, y, z;
} Point;

Vector addVector (Vector a, Vector b);
Vector subtractVector (Vector a, Vector b);
Vector multiplyVector (Vector a, Vector b);
Vector scaleVector (Vector a, double scale);
Vector negateVector (Vector a);
double dotProduct (Vector a, Vector b);
Vector crossProduct (Vector a, Vector b);
double vectorLengthSquared (Vector a);
double vectorLength (Vector a);
double maxComponent (Vector a);
double luminance (Vector a);
Vector normalizeVector (Vector a);
Vector reflectVector (Vector incoming, Vector normal);

Vector getVector (Point a, Point b);
double getDistance (Point a, Point b);
Point movePoint (Point a, Vector v);

#endif