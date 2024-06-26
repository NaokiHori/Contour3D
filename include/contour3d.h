#if !defined(CONTOUR3D_H)
#define CONTOUR3D_H

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t
#include "sdecomp.h"

// (x, y, z)
typedef double contour3d_vector_t[3];

typedef struct {
  // pencil on which the array is defined
  // see also: https://github.com/NaokiHori/SimpleDecomp
  sdecomp_pencil_t pencil;
  // number of grid points (XYZ)
  size_t glsizes[3];
  // rectilinear grids in each XYZ
  double * grids[3];
  // pointer to a function which maps the given orthogonal coordinate to the Cartesian coordinate
  int (* converter)(const contour3d_vector_t * orthogonal, contour3d_vector_t * cartesian);
  // where the iso-surface is formed
  double threshold;
  // object color (rgb)
  uint8_t color[3];
  // three-dimensional array, from which a contour is generated
  double * array;
} contour3d_contour_obj_t;

typedef struct {
  // specify edges in each direction
  struct {
    // start and end, on the user-defined orthogonal coordinate system
    double edges[2];
    // number of intermediate points (the more the smoother)
    size_t nitems;
  } line_configs[3];
  // pointer to a function which maps the given orthogonal coordinate to the Cartesian coordinate
  int (* converter)(
      const contour3d_vector_t * orthogonal,
      contour3d_vector_t * cartesian
  );
  // line color (rgb)
  uint8_t color[3];
  // line width
  double width;
} contour3d_line_obj_t;

extern int contour3d_execute(
    // information about the pencil domain decomposition
    const sdecomp_info_t * sdecomp_info,
    // camera coordinate (xyz)
    const double camera_position[3],
    // focal point (xyz)
    const double camera_look_at[3],
    // direction of light (xyz)
    const double light_direction[3],
    // resolution (number of pixels) in x and y
    const size_t screen_sizes[2],
    // screen center coordinate (xyz)
    const double screen_center[3],
    // screen vector (local x and y axes, each has xyz)
    const double screen_local[2][3],
    // background image color (rgb)
    const uint8_t bg_color[3],
    // number of iso-surfaces to be drawn
    const size_t num_contours,
    // details of the iso-surfaces, which should have "num_contours" members
    // see above, the definition of "contour3d_contour_obj_t"
    const contour3d_contour_obj_t contour3d_contour_objs[],
    // details of the lines (12 domain edges)
    const contour3d_line_obj_t * contour3d_line_objs,
    // output image file name
    const char fname[]
);

#endif // CONTOUR3D_H
