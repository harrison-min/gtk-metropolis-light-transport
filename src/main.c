#include "pathTracer.h"
#include "display.h"
#include "geometry.h"
#include "camera.h"
#include "ray.h"
#include "sceneLoader.h"
#include <windows.h>
#include <stdio.h>

#define DEFAULT_OBJ "../test_scenes/cornell_box/CornellBox-Sphere.obj"
#define DEFAULT_MTL "../test_scenes/cornell_box/CornellBox-Sphere.mtl"

PixelMap * createPixelMap (int width, int height) {
    PixelMap * map = malloc(sizeof(PixelMap));
    map->width = width;
    map->height = height;
    map->size = width * height * sizeof(unsigned char) * 4;
    map->data = malloc (map->size);
    return map;
}

PixelMap * generateTestPixelMap (int width, int height) {
    Scene * scene = initScene();
    Camera * cam = createCamera(width, height);

    if (!loadScene (scene, DEFAULT_OBJ, DEFAULT_MTL)) {
        fprintf (stderr, "Failed to load scene: %s\n", DEFAULT_OBJ);
        freeScene (scene);
        freeCamera (cam);
        return NULL;
    }

    fprintf (stderr, "Loaded: %d triangles, %d spheres, %d materials\n",
             scene->numTriangles, scene->numSpheres, scene->numMaterials);

    frameScene(scene, cam);
    PixelMap * newPixels = createPixelMap(width, height);

    for (int x = 0; x < width; ++ x) {
        for (int y = 0; y < height; ++ y) {
            HitRecord path [MAX_BOUNCES];
           
            int totalSamples = 5;

            Vector color = {0, 0, 0};

            for (int samples = 0; samples < totalSamples; ++ samples) {
                double jitterX = (double)x + ((double)rand() / RAND_MAX - 0.5);
                double jitterY = (double)y + ((double)rand() / RAND_MAX - 0.5);
                Ray cameraRay = getCameraRay(cam, jitterX, jitterY);
                int totalHits = tracePath(cameraRay, path, 0, scene);
                Vector tempColor = calculatePathColor(path, totalHits, scene);
                color = addVector(color, tempColor);
            }

            color = scaleVector (color, 1.0/totalSamples);

            int index = (x + y * width) * 4;
            newPixels->data[index + 0] = (unsigned char)(fmin(1.0, color.x) * 255.0); 
            newPixels->data[index + 1] = (unsigned char)(fmin(1.0, color.y) * 255.0); 
            newPixels->data[index + 2] = (unsigned char)(fmin(1.0, color.z) * 255.0); 
            newPixels->data[index + 3] = 255; 
            
        }
    }

    freeScene(scene);
    freeCamera(cam);

    return newPixels;
}

int main (int argc, char ** argv) {
    int width = 500, height = 500;    

    if (argc == 3) {
        width = strtol(argv[1], NULL, 10);
        height = strtol(argv[2], NULL, 10);
    } 

    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    LARGE_INTEGER end;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    PixelMap * map = generateTestPixelMap(width, height);
    QueryPerformanceCounter(&end);

    if (!map) {
        fprintf (stderr, "Failed to generate pixel map.\n");
        return 1;
    }

    double timeSpent = (double)(end.QuadPart - start.QuadPart)/frequency.QuadPart;

    fprintf(stderr,"Rendered %d x %d pixels in %f seconds.\n", width, height, timeSpent);

    GtkDisplay * display = createDisplay(width, height);
    setPixelMap(display, map);
    runDisplay(display, 0, NULL);
    cleanDisplay(display);
}