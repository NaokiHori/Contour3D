#include <math.h>
#include <float.h>
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "internal.h"
#define CONTOUR3D_CONTOUR_INTERNAL
#include "contour/internal.h"
#define CONTOUR3D_LINE_INTERNAL
#include "line/internal.h"

// from a given scalar field, extract triangle elements
//   using the marching-tetrahedra algorithm,
//   and output the result to an image
int contour3d_execute(
    const sdecomp_info_t * sdecomp_info,
    const double camera_position[3],
    const double camera_look_at[3],
    const double light_direction[3],
    const size_t screen_sizes[2],
    const double screen_center[3],
    const double screen_local[2][3],
    const uint8_t bg_color[3],
    const size_t num_contours,
    const contour3d_contour_obj_t contour3d_contour_objs[],
    const size_t num_lines,
    const contour3d_line_obj_t contour3d_line_objs[],
    const char fname[]
){
  // prepare structures
  const camera_t camera = {
    .position[0] = camera_position[0],
    .position[1] = camera_position[1],
    .position[2] = camera_position[2],
    .look_at[0]  = camera_look_at[0],
    .look_at[1]  = camera_look_at[1],
    .look_at[2]  = camera_look_at[2],
  };
  vector_t light = {0};
  {
    const double norminv = 1. / sqrt(
        + pow(light_direction[0], 2.)
        + pow(light_direction[1], 2.)
        + pow(light_direction[2], 2.)
    );
    light[0] = light_direction[0] * norminv;
    light[1] = light_direction[1] * norminv;
    light[2] = light_direction[2] * norminv;
  }
  screen_t screen = {0};
  {
    // center position
    screen.center[0]  = screen_center[0];
    screen.center[1]  = screen_center[1];
    screen.center[2]  = screen_center[2];
    // number of pixels
    screen.width      = screen_sizes[0];
    screen.height     = screen_sizes[1];
    // screen local vector
    screen.local_x[0] = screen_local[0][0];
    screen.local_x[1] = screen_local[0][1];
    screen.local_x[2] = screen_local[0][2];
    screen.local_y[0] = screen_local[1][0];
    screen.local_y[1] = screen_local[1][1];
    screen.local_y[2] = screen_local[1][2];
    // inversed and squared lengths of them
    // (i.e. inversed self inner products)
    screen.ipinv_local_x = 1. / (
        + pow(screen.local_x[0], 2.)
        + pow(screen.local_x[1], 2.)
        + pow(screen.local_x[2], 2.)
    );
    screen.ipinv_local_y = 1. / (
        + pow(screen.local_y[0], 2.)
        + pow(screen.local_y[1], 2.)
        + pow(screen.local_y[2], 2.)
    );
    // normalised screen normal vector
    {
      screen.normal[0] = + screen.local_x[1] * screen.local_y[2]
                         - screen.local_x[2] * screen.local_y[1];
      screen.normal[1] = + screen.local_x[2] * screen.local_y[0]
                         - screen.local_x[0] * screen.local_y[2];
      screen.normal[2] = + screen.local_x[0] * screen.local_y[1]
                         - screen.local_x[1] * screen.local_y[0];
      const double norminv = 1. / sqrt(
          + pow(screen.normal[0], 2.)
          + pow(screen.normal[1], 2.)
          + pow(screen.normal[2], 2.)
      );
      screen.normal[0] *= norminv;
      screen.normal[1] *= norminv;
      screen.normal[2] *= norminv;
    }
    // identify screen location based on
    //   the normal vector n_i
    //   and the screen center i_c,
    //   i.e. n_i i_c = d
    screen.intercept =
      + screen.normal[0] * screen.center[0]
      + screen.normal[1] * screen.center[1]
      + screen.normal[2] * screen.center[2];
  };
  // prepare canvas: pixels (to store colors) and z buffer
  const size_t width  = screen.width;
  const size_t height = screen.height;
  pixel_t * canvas  = contour3d_calloc(width * height, sizeof(pixel_t));
  if(NULL == canvas){
    goto abort;
  }
  for(/* each pixel */ size_t index = 0; index < width * height; index++){
    pixel_t * pixel = canvas + index;
    uint8_t * color = pixel->color;
    double * depth = &pixel->depth;
    // fill canvas with the default background color
    color[0] = bg_color[0];
    color[1] = bg_color[1];
    color[2] = bg_color[2];
    // assign negative infinity to the minimum distance field
    *depth = -1. * DBL_MAX;
  }
  // process contour objects
  for(/* each contour object */ size_t index = 0; index < num_contours; index++){
    if(0 != contour3d_process_contour_obj(
          sdecomp_info,
          &camera,
          &light,
          &screen,
          contour3d_contour_objs + index,
          canvas
    )) goto abort;
  }
  // process line objects
  for(/* each line object */ size_t index = 0; index < num_lines; index++){
    if(0 != contour3d_process_line_obj(
          &camera,
          &screen,
          contour3d_line_objs + index,
          canvas
    )) goto abort;
  }
  if(0 != contour3d_output_image(
        sdecomp_info,
        fname,
        width,
        height,
        canvas
  )) goto abort;
  contour3d_free(canvas);
  return 0;
abort:
  // error detected, deallocate all internal memory
  //   and report error
  contour3d_free_all();
  return 1;
}

