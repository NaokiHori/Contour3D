#include <stdio.h>
#include <float.h> // DBL_MAX
#include "contour3d.h"
#include "./struct.h"
#include "./vector.h"
#include "./memory.h"
#include "./output.h"
#include "./logger.h"
#include "./line.h"
#include "./contour/internal.h"

// assign user input to a struct camera_t
static camera_t init_camera (
    const contour3d_vector_t * const position,
    const contour3d_vector_t * const look_at
) {
  const camera_t camera = {
    .position = *position,
    .look_at  = *look_at,
  };
  return camera;
}

// normalise the given light direction
static contour3d_vector_t init_light (
    const contour3d_vector_t * const direction
) {
  return contour3d_vector_normalise(*direction);
}

// assign
// - center of screen
// - number of pixels
// - horizontal and vertical vectors
// - screen normal
static screen_t init_screen (
    const size_t sizes[2],
    const contour3d_vector_t * const center,
    const contour3d_vector_t local[2]
) {
  // screen local vectors, horizontal and vertical
  const contour3d_vector_t local_x = local[0];
  const contour3d_vector_t local_y = local[1];
  // normalised screen normal vector,
  //   computed by the outer product of two local vectors
  const contour3d_vector_t normal = contour3d_vector_normalise(
      contour3d_vector_outer_product(local_x, local_y)
  );
  // assign all
  const screen_t screen = {
    // center position
    .center = *center,
    // number of pixels
    .width  = sizes[0],
    .height = sizes[1],
    // horizontal and vertical vectors (in Cartesian coordinate)
    .local_x = local_x,
    .local_y = local_y,
    // screen normal vector
    .normal = normal,
  };
  return screen;
}

// from a given scalar field (or fields), extract triangle elements
//   using the marching-tetrahedra algorithm,
//   and output the result to an image
int contour3d_execute (
    const sdecomp_info_t * const sdecomp_info,
    const contour3d_vector_t * const camera_position,
    const contour3d_vector_t * const camera_look_at,
    const contour3d_vector_t * const light_direction,
    const size_t screen_sizes[2],
    const contour3d_vector_t * const screen_center,
    const contour3d_vector_t screen_local[2],
    const contour3d_color_t * const bg_color,
    const size_t num_contours,
    const contour3d_contour_obj_t * const contour3d_contour_objs,
    const size_t num_lines,
    const contour3d_line_obj_t * const contour3d_line_objs,
    const char fname[]
) {
  // prepare structures
  const camera_t camera = init_camera(camera_position, camera_look_at);
  const contour3d_vector_t light = init_light(light_direction);
  const screen_t screen = init_screen(screen_sizes, screen_center, screen_local);
  // prepare canvas: pixels (to store colors) and z buffer
  const size_t width  = screen.width;
  const size_t height = screen.height;
  pixel_t * const canvas = contour3d_memory_alloc(width * height, sizeof(pixel_t));
  if (NULL == canvas) {
    logger_error("canvas allocation failed");
    goto abort;
  }
  for (/* each pixel */ size_t n = 0; n < width * height; n++) {
    pixel_t * const pixel = canvas + n;
    contour3d_color_t * const color = &pixel->color;
    double * const depth = &pixel->depth;
    // fill canvas with the default background color
    *color = *bg_color;
    // assign negative infinity as the minimum distance
    *depth = -1. * DBL_MAX;
  }
  // process contour objects
  for (/* each contour object */ size_t n = 0; n < num_contours; n++) {
    if (0 != contour3d_process_contour_obj(
          sdecomp_info,
          &camera,
          &light,
          &screen,
          contour3d_contour_objs + n,
          canvas
    )) {
      logger_error("contour processing failed");
      goto abort;
    }
  }
  // draw lines
  for (/* each line object */ size_t n = 0; n < num_lines; n++) {
    if (0 != contour3d_process_line_obj(
          &camera,
          &screen,
          contour3d_line_objs + n,
          canvas
    )) {
      logger_error("line processing failed");
      goto abort;
    }
  }
  if (0 != contour3d_output_image(
        sdecomp_info,
        fname,
        width,
        height,
        canvas
  )) {
    logger_error("image output failed");
    goto abort;
  }
  contour3d_memory_free(canvas);
  return 0;
abort:
  // error detected, deallocate all internal memory
  contour3d_memory_free_all();
  return 1;
}

