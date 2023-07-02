#if !defined(CONTOUR3D_STRUCT_H)
#define CONTOUR3D_STRUCT_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t
#include "sdecomp.h"
#include "contour3d.h"

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
  contour3d_color_t color;
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

#endif // CONTOUR3D_STRUCT_H
