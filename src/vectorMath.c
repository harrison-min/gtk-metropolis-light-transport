#include "vectorMath.h"

Vector addVector (Vector a, Vector b) {
    return (Vector){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vector subtractVector (Vector a, Vector b) {
    return (Vector){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vector multiplyVector (Vector a, Vector b) {
    return (Vector){a.x * b.x, a.y * b.y, a.z * b.z};
}

Vector scaleVector (Vector a, double scale) {
    return (Vector){a.x * scale, a.y * scale, a.z * scale};
}

Vector negateVector (Vector a) {
    return (Vector){-a.x, -a.y, -a.z};
}

double dotProduct (Vector a, Vector b) {
    return (double)(a.x * b.x + a.y * b.y + a.z * b.z);
}

Vector crossProduct (Vector a, Vector b) {
    return (Vector){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

double vectorLengthSquared (Vector a) {
    return a.x * a.x + a.y * a.y + a.z * a.z;
}

double vectorLength (Vector a) {
    return sqrt(vectorLengthSquared(a));
}

double maxComponent (Vector a) {
    return fmax(a.x, fmax(a.y, a.z));
}

double luminance (Vector a) {
    return 0.2126 * a.x + 0.7152 * a.y + 0.0722 * a.z;
}

Vector normalizeVector (Vector a) {
    double len = vectorLength (a);
    if (len < 1e-10) return (Vector){0, 0, 0}; // essentially a length == 0 check 
    return (Vector){a.x / len, a.y / len, a.z / len};
}

Vector reflectVector (Vector incoming, Vector normal) {
    return subtractVector(incoming, scaleVector(normal, 2.0 * dotProduct(incoming, normal)));
}

Vector getVector (Point a, Point b) {
    return (Vector){b.x - a.x, b.y - a.y, b.z - a.z};
}

double getDistance (Point a, Point b) {
    return vectorLength (getVector(a, b));
}

Point movePoint (Point a, Vector v) {
    return (Point){a.x + v.x, a.y + v.y, a.z + v.z};
}