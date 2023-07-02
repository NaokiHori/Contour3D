#if !defined(CONTOUR3D_PROJECT_H)
#define CONTOUR3D_PROJECT_H

#include "contour3d.h"
#include "./struct.h"

extern int contour3d_project(
    const camera_t * camera,
    const screen_t * screen,
    const contour3d_vector_t * p0,
    contour3d_vector_t * p1
);

#endif // CONTOUR3D_PROJECT_H
