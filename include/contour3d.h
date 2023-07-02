#if !defined(CONTOUR3D_H)
#define CONTOUR3D_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t
#include "sdecomp.h"

#define CONTOUR3D_NDIMS 3

typedef struct {
  double x;
  double y;
  double z;
} contour3d_vector_t;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} contour3d_color_t;

typedef struct {
  // pencil on which the array is defined
  // see also: https://github.com/NaokiHori/SimpleDecomp
  sdecomp_pencil_t pencil;
  // number of grid points (XYZ)
  size_t glsizes[CONTOUR3D_NDIMS];
  // rectilinear grids in each XYZ
  double * grids[CONTOUR3D_NDIMS];
  // pointer to a function which maps the given orthogonal coordinate to the Cartesian coordinate
  contour3d_vector_t (* converter) (
      const contour3d_vector_t orthogonal
  );
  // where the iso-surface is formed
  double threshold;
  // object color
  contour3d_color_t color;
  // three-dimensional array, from which a contour is generated
  double * array;
} contour3d_contour_obj_t;

typedef struct {
  // start and end, on the user-defined orthogonal coordinate system
  contour3d_vector_t edges[2];
  // number of intermediate points (the more the smoother)
  size_t nitems;
  // pointer to a function which maps
  //   the given orthogonal coordinate to the Cartesian coordinate
  contour3d_vector_t (* converter) (
      const contour3d_vector_t orthogonal
  );
  // line color
  contour3d_color_t color;
  // line width
  double width;
} contour3d_line_obj_t;

extern int contour3d_execute (
    // information about the pencil domain decomposition
    const sdecomp_info_t * const sdecomp_info,
    // camera coordinate
    const contour3d_vector_t * const camera_position,
    // focal point
    const contour3d_vector_t * const camera_look_at,
    // direction of light
    const contour3d_vector_t * const light_direction,
    // resolution (number of pixels) in x and y
    const size_t screen_sizes[2],
    // screen center coordinate
    const contour3d_vector_t * const screen_center,
    // screen vector (local x and y axes, xyz components)
    const contour3d_vector_t screen_local[2],
    // background image color
    const contour3d_color_t * const bg_color,
    // number of iso-surfaces to be drawn
    const size_t num_contours,
    // details of the iso-surfaces, which should have "num_contours" members
    // see above, the definition of "contour3d_contour_obj_t"
    const contour3d_contour_obj_t * const contour3d_contour_objs,
    // number of lines to be drawn
    const size_t num_lines,
    // details of the lines (12 domain edges)
    const contour3d_line_obj_t * const contour3d_line_objs,
    // output image file name
    const char fname[]
);

#endif // CONTOUR3D_H
