#include "camera.h"
#include <stdlib.h>

Camera * createCamera (int imageWidth, int imageHeight) {
    Camera * newCamera = malloc(sizeof(Camera));
    newCamera->imageHeight = imageHeight;
    newCamera->imageWidth = imageWidth;
    newCamera->position = (Point){0, 0, 0};
    newCamera->up = (Vector){0, 1, 0};
    newCamera->right = (Vector){1, 0, 0};
    newCamera->forward = (Vector){0, 0, -1};

    newCamera->FOV = 90;
    double degreesToRadiansConversionFactor = M_PI / 180;
    newCamera->halfTanFOV = tan(newCamera->FOV * 0.5 * degreesToRadiansConversionFactor);

    return newCamera;
}

void frameScene (Scene * scene, Camera * cam) {
    Point bbMin = scene->boundingBoxMin;
    Point bbMax = scene->boundingBoxMax;
    Vector extent = getVector (bbMin, bbMax);
    Point center = movePoint (bbMin, scaleVector (extent, 0.5));

    double halfWidth = extent.x * 0.5;
    double halfHeight = extent.y * 0.5;
    double maxHalf = fmax (halfWidth, halfHeight);

    double fovDegrees = 39.0;
    double tanHalfFov = tan (fovDegrees * M_PI / 360.0);
    double distance = maxHalf / tanHalfFov;

    cam->position = (Point){center.x, center.y, bbMax.z + distance};
    cam->forward = normalizeVector (getVector (cam->position, center));
    cam->right = normalizeVector (crossProduct (cam->forward, (Vector){0, 1, 0}));
    cam->up = crossProduct (cam->right, cam->forward);
    cam->halfTanFOV = tanHalfFov;
    cam->FOV = fovDegrees;
}

void freeCamera(Camera * cam) {
    free (cam);
}

Ray getCameraRay (Camera * cam, double px, double py) {
    double aspectRatio = (double)cam->imageWidth/cam->imageHeight;

    double normalizedX = (2.0 * (px + 0.5)/cam->imageWidth) - 1.0;
    double normalizedY = -(2.0 * (py + 0.5)/cam->imageHeight) + 1.0;

    double x = normalizedX * aspectRatio * cam->halfTanFOV;
    double y = normalizedY * cam->halfTanFOV;

    Vector direction = addVector (cam->forward , (addVector(scaleVector (cam->right, x), scaleVector (cam->up, y))));

    return (Ray) {cam->position, normalizeVector(direction)};
}