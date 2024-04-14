#if !defined(CONTOUR3D_INTERNAL_H)
#define CONTOUR3D_INTERNAL_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t
#include "sdecomp.h"
#include "contour3d.h"

// simple include guard
#if !defined(CONTOUR3D_INTERNAL)
#error "do not include this file externally"
#endif

// triangle element which is rendered
typedef struct {
  // vertex positions
  contour3d_vector_t vertices[3];
  // face normal (single vector per element)
  contour3d_vector_t face_normal;
  // vertex normals (defined at each vertex)
  contour3d_vector_t vertex_normals[3];
  // indices of the lattice edges where the corners are sitting
  size_t cube_indices[3];
  // area of triangle
  double area;
} triangle_t;

// cubic element which is organised by eight surrounding vertices
//   and contains six tetrahedra
typedef struct {
  size_t num_triangles;
  triangle_t triangles[12];
} lattice_t;

// pixel which has z-buffer and color information
typedef struct {
  double depth;
  uint8_t color[3];
} pixel_t;

// screen configuration
typedef struct {
  // center position
  contour3d_vector_t center;
  // number of pixels
  size_t width;
  size_t height;
  // horizontal and vertical vectors of the screen horizontal and vertical axes
  contour3d_vector_t local_x;
  contour3d_vector_t local_y;
  // the above vectors are NOT normalised and
  //   their l2 norms represent the lengths of the screen
  // inversed and squared values are stored for convenience
  double ipinv_local_x;
  double ipinv_local_y;
  // normal vector perpendicular to the screen
  contour3d_vector_t normal;
  // intercept ("d" of n_x x_c + n_y y_c + n_z z_c = d),
  //   where n_i is the screen normal and i_c is the screen center
  double intercept;
} screen_t;

typedef struct {
  // where the observer is sitting
  contour3d_vector_t position;
  // where the observer is looking at
  contour3d_vector_t look_at;
} camera_t;

extern void * contour3d_calloc(
    const size_t nitems,
    const size_t size
);

extern int contour3d_free(
    void * ptr
);

extern int contour3d_free_all(
    void
);

extern int contour3d_project(
    const camera_t * camera,
    const screen_t * screen,
    const contour3d_vector_t * p0,
    contour3d_vector_t * p1
);

extern int contour3d_output_image(
    const sdecomp_info_t * sdecomp_info,
    const char fname[],
    const size_t width,
    const size_t height,
    pixel_t * canvas
);

#endif // CONTOUR3D_INTERNAL_H
