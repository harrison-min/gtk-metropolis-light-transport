#include "pathTracer.h"
#include <stdlib.h>

int tracePath (Ray ray, HitRecord * path, int totalBounces, Scene * scene) {
    if (totalBounces >= MAX_BOUNCES) return totalBounces;
    
    HitRecord temp;
    if (!getSceneHit(scene, ray, &temp)) {
        return totalBounces;
    } else {
        path[totalBounces]  = temp;
    }
    Material mat = scene->materials[temp.materialId];

    Ray reflectedRay;
    reflectedRay.origin = movePoint(temp.intersection, scaleVector(temp.normal, 1e-4));
        
    if (mat.type == MATERIAL_DIFFUSE) {
        Vector randVec;
        
        randVec.x = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        randVec.y = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        randVec.z = ((double)rand() / RAND_MAX) * 2.0 - 1.0;

        reflectedRay.vector = normalizeVector(addVector(temp.normal, randVec));
    } else {
        reflectedRay.vector = subtractVector(ray.vector, scaleVector(temp.normal, 2 * dotProduct(temp.normal, ray.vector)));
        reflectedRay.vector = normalizeVector(reflectedRay.vector);
    }
    return tracePath (reflectedRay, path, totalBounces + 1, scene);
}

Vector calculatePathColor (HitRecord * path, int numHits, Scene * scene) {
    Vector color = {0, 0, 0};
    Vector throughput = {1, 1, 1};

    for (int i = 0; i < numHits; ++ i) {
        HitRecord * currentHit = &(path [i]);
        Material mat = scene->materials [currentHit->materialId];

        if (i == 0) {
            color = addVector(color, multiplyVector (throughput, mat.emission));
        }

        if (scene->hasLight) {
            Vector lightDirection = getVector (currentHit->intersection, scene->lightVertex);
            double distanceToLight = vectorLength(lightDirection);
            lightDirection = normalizeVector(lightDirection);

            Point origin = movePoint(currentHit->intersection, scaleVector(currentHit->normal, 1e-4));
            Ray directLightRay = {origin, lightDirection};

            HitRecord directLightHit;
            if (!getSceneHit(scene, directLightRay, &directLightHit) || directLightHit.distance > (distanceToLight - 1e-4)) {
                double cosTheta = dotProduct(currentHit->normal, lightDirection);
                if (cosTheta > 0) {
                    double falloff = 1.0/(pow(distanceToLight, 2.0) + 1);

                    Vector directLightContribution = scaleVector(scene->materials[scene->lightMaterialId].emission, cosTheta * falloff);
                    Vector reflectedLight = multiplyVector(directLightContribution, mat.color);
                    color = addVector(color, multiplyVector(throughput, reflectedLight));
                }
            }
        }

        throughput = multiplyVector (throughput, mat.color);
    }

    return color;
}