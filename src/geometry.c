#include "geometry.h"
#include <stdlib.h>
#include <string.h>

Triangle createTriangle (Point p1, Point p2, Point p3, int materialId) {
    Triangle newTriangle;
    newTriangle.p1 = p1;
    newTriangle.p2 = p2;
    newTriangle.p3 = p3;
    newTriangle.edge1 = getVector (p1, p2);
    newTriangle.edge2 = getVector (p1, p3);
    newTriangle.edge3 = getVector (p2, p3);
    newTriangle.normal = normalizeVector (crossProduct (newTriangle.edge1, newTriangle.edge2));
    newTriangle.materialId = materialId;
    return newTriangle;
}

Sphere createSphere (Point center, double radius, int materialId) {
    Sphere newSphere;
    newSphere.center = center;
    newSphere.radius = radius;
    newSphere.materialId = materialId;
    return newSphere;
}

Material createMaterial (Vector color, Vector emission, MaterialType type, double indexOfRefraction) {
    Material newMaterial;
    newMaterial.color = color;
    newMaterial.emission = emission;
    newMaterial.type = type;
    newMaterial.indexOfRefraction = indexOfRefraction;
    return newMaterial;
}

void addTriangle (Scene * scene, Triangle triangle) {
    if (scene->numTriangles == scene->trianglesCapacity) {
        scene->trianglesCapacity = scene->trianglesCapacity * 2;
        Triangle * temp = realloc (scene->triangles, scene->trianglesCapacity * sizeof(*(scene->triangles)));
        if (temp == NULL) {
            return;
        }
        scene->triangles = temp;
    }
    scene->triangles[scene->numTriangles] = triangle;
    scene->numTriangles ++;
}

void addSphere (Scene * scene, Sphere sphere) {
    if (scene->numSpheres == scene->spheresCapacity) {
        scene->spheresCapacity = scene->spheresCapacity * 2;
        Sphere * temp = realloc (scene->spheres, scene->spheresCapacity * sizeof(*(scene->spheres)));
        if (temp == NULL) {
            //likely due to being out of memory
            return;
        }
        scene->spheres = temp;
    }
    scene->spheres[scene->numSpheres] = sphere;
    scene->numSpheres ++;
}

void addMaterial (Scene * scene, Material material) {
    if (scene->numMaterials == scene->materialsCapacity) {
        scene->materialsCapacity = scene->materialsCapacity * 2;
        Material * temp = realloc (scene->materials, scene->materialsCapacity * sizeof(*(scene->materials)));
        if (temp == NULL) {
            //likely due to being out of memory
            return;
        }
        scene->materials = temp;
    }
    scene->materials[scene->numMaterials] = material;
    scene->numMaterials ++;
}

Scene * initScene () {
    Scene * newScene = calloc (1, sizeof(Scene));

    int initialCapacity = 100;
    newScene->spheresCapacity = initialCapacity;
    newScene->trianglesCapacity = initialCapacity;
    newScene->materialsCapacity = initialCapacity;

    newScene->triangles = malloc (sizeof(Triangle) * newScene->trianglesCapacity);
    newScene->spheres = malloc (sizeof(Sphere) * newScene->spheresCapacity);
    newScene->materials = malloc (sizeof(Material) * newScene->materialsCapacity);

    return newScene;
}

void freeScene (Scene * scene) {
    if (!scene) return;
    free (scene->spheres);
    free (scene->triangles);
    free (scene->materials);
    free (scene);
}

void detectLight (Scene * scene) {
    scene->hasLight = false;

    int lightMaterial = -1;
    for (int i = 0; i < scene->numMaterials; ++ i) {
        if (maxComponent (scene->materials[i].emission) > 0) {
            lightMaterial = i;
            break;
        }
    }
    if (lightMaterial < 0) return;
    scene->lightMaterialId = lightMaterial;

    int lightTriangles[8];
    int numLightTriangles = 0;
    for (int i = 0; i < scene->numTriangles && numLightTriangles < 8; ++ i) {
        if (scene->triangles[i].materialId == lightMaterial) {
            lightTriangles[numLightTriangles ++] = i;
        }
    }

    if (numLightTriangles == 2) {
        Triangle * t0 = &scene->triangles[lightTriangles[0]];
        Triangle * t1 = &scene->triangles[lightTriangles[1]];

        Point t0Verts[3] = {t0->p1, t0->p2, t0->p3};
        Point t1Verts[3] = {t1->p1, t1->p2, t1->p3};
        Point uniqueVertex = t1Verts[0];
        int found = 0;

        for (int i = 0; i < 3 && !found; ++ i) {
            int inFirstTriangle = 0;
            for (int j = 0; j < 3; ++ j) {
                Vector diff = getVector (t1Verts[i], t0Verts[j]);
                if (vectorLengthSquared (diff) < 1e-8) {
                    inFirstTriangle = 1;
                    break;
                }
            }
            if (!inFirstTriangle) {
                uniqueVertex = t1Verts[i];
                found = 1;
            }
        }

        if (found) {
            scene->lightEdge1 = getVector (t0->p1, t0->p2);
            scene->lightEdge2 = getVector (t0->p1, uniqueVertex);

            Point lightCorner = t0->p1;
            Vector halfE1 = scaleVector(scene->lightEdge1, 0.5);
            Vector halfE2 = scaleVector(scene->lightEdge2, 0.5);
            scene->lightVertex = movePoint(lightCorner, addVector(halfE1, halfE2));

            scene->lightNormal = t0->normal;
            scene->lightArea = vectorLength (crossProduct (scene->lightEdge1, scene->lightEdge2));
            scene->hasLight = true;
        }
    } else if (numLightTriangles == 1) {
        Triangle * t = &scene->triangles[lightTriangles[0]];
        scene->lightEdge1 = getVector (t->p1, t->p2);
        scene->lightEdge2 = getVector (t->p1, t->p3);
        scene->lightVertex = t->p1;
        scene->lightNormal = t->normal;
        scene->lightArea = 0.5 * vectorLength (crossProduct (scene->lightEdge1, scene->lightEdge2));
        scene->hasLight = true;
    }
}

static BVHObject createBVHObject (void * data, GeometryType type, int index) {
    BVHObject newObject;
    newObject.type = type;        
    newObject.index = index;

    if (type == TRIANGLE) {
        Triangle triangle = *((Triangle *) data);

        double temp = triangle.p1.x < triangle.p2.x ? triangle.p1.x : triangle.p2.x;
        newObject.bounds.min.x = temp < triangle.p3.x ? temp : triangle.p3.x;
        temp = triangle.p1.y < triangle.p2.y ? triangle.p1.y : triangle.p2.y;
        newObject.bounds.min.y = temp < triangle.p3.y ? temp : triangle.p3.y;
        temp = triangle.p1.z < triangle.p2.z ? triangle.p1.z : triangle.p2.z;
        newObject.bounds.min.z = temp < triangle.p3.z ? temp : triangle.p3.z;

        temp = triangle.p1.x > triangle.p2.x ? triangle.p1.x : triangle.p2.x;
        newObject.bounds.max.x = temp > triangle.p3.x ? temp : triangle.p3.x;
        temp = triangle.p1.y > triangle.p2.y ? triangle.p1.y : triangle.p2.y;
        newObject.bounds.max.y = temp > triangle.p3.y ? temp : triangle.p3.y;
        temp = triangle.p1.z > triangle.p2.z ? triangle.p1.z : triangle.p2.z;
        newObject.bounds.max.z = temp > triangle.p3.z ? temp : triangle.p3.z;

        double rayEpsilon = 1e-3; //using this instead of including ray.h to avoid circular dependencies
        newObject.bounds.min.x -= rayEpsilon; newObject.bounds.min.y -= rayEpsilon; newObject.bounds.min.z -= rayEpsilon;
        newObject.bounds.max.x += rayEpsilon; newObject.bounds.max.y += rayEpsilon; newObject.bounds.max.z += rayEpsilon;

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

    

    free (bvhArray);
}