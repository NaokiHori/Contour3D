#include <stdio.h>
#include <math.h>
#include "contour3d.h"
#include "./struct.h"
#include "./vector.h"
#include "./logger.h"
#include "./project.h"
#include "./line.h"

static inline int_fast32_t intmin (
    const int_fast32_t v0,
    const int_fast32_t v1
) {
  return v0 < v1 ? v0 : v1;
}

static inline int_fast32_t intmax (
    const int_fast32_t v0,
    const int_fast32_t v1
) {
  return v1 < v0 ? v0 : v1;
}

// draw a line between point "p0" and point "p1",
//   whose components are both in orthogonal coordinate system
static int draw_line_between_two_points (
    const camera_t * const camera,
    const screen_t * const screen,
    contour3d_vector_t (* const converter) (
      const contour3d_vector_t orthogonal
    ),
    const contour3d_color_t * const color,
    const double line_width,
    const contour3d_vector_t * restrict const p0_orthogonal,
    const contour3d_vector_t * restrict const p1_orthogonal,
    pixel_t * const canvas
) {
  // transform points to Cartesian coordinate system
  const contour3d_vector_t p0_cartesian = converter(*p0_orthogonal);
  const contour3d_vector_t p1_cartesian = converter(*p1_orthogonal);
  // project points in Cartesian coordinate to screen coordinate [-0.5 : +0.5]
  contour3d_vector_t p0_screen = {.x = 0., .y = 0., .z = 0.};
  contour3d_vector_t p1_screen = {.x = 0., .y = 0., .z = 0.};
  // early-return when one of the points are out of screen
  if (0 != contour3d_project(
        camera,
        screen,
        &p0_cartesian,
        &p0_screen
  )) {
    return 1;
  }
  if (0 != contour3d_project(
        camera,
        screen,
        &p1_cartesian,
        &p1_screen
  )) {
    return 1;
  }
  // draw dots
  const size_t width  = screen->width;
  const size_t height = screen->height;
  // convert to pixel indices, i.e., [-0.5 : +0.5] -> [0 : number of pixels - 1]
  p0_screen.x = (0.5 + p0_screen.x) * width ;
  p0_screen.y = (0.5 + p0_screen.y) * height;
  p1_screen.x = (0.5 + p1_screen.x) * width ;
  p1_screen.y = (0.5 + p1_screen.y) * height;
  // prepare a bounding box
  const int_fast32_t xmin = fmin(p0_screen.x, p1_screen.x) - 1;
  const int_fast32_t xmax = fmax(p0_screen.x, p1_screen.x) + 1;
  const int_fast32_t ymin = fmin(p0_screen.y, p1_screen.y) - 1;
  const int_fast32_t ymax = fmax(p0_screen.y, p1_screen.y) + 1;
  // avoid out-of-bounds access
  const size_t imin = intmin( width - 1, intmax(0, xmin));
  const size_t imax = intmin( width - 1, intmax(0, xmax));
  const size_t jmin = intmin(height - 1, intmax(0, ymin));
  const size_t jmax = intmin(height - 1, intmax(0, ymax));
  // change the color of a dot if the distance between
  //   the pixel and a line is below the width
  // line: ax + by + c = 0
  const double a = + p1_screen.y - p0_screen.y;
  const double b = - p1_screen.x + p0_screen.x;
  const double c =
    + (p1_screen.x - p0_screen.x) * p0_screen.y
    - (p1_screen.y - p0_screen.y) * p0_screen.x;
  for (size_t j = jmin; j <= jmax; j++) {
    for (size_t i = imin; i <= imax; i++) {
      // check distance from the line segment
      const double threshold = pow(0.5 * line_width, 2.);
      const double distance = pow(a * i + b * j + c, 2.) / (a * a + b * b);
      if (threshold < distance) {
        continue;
      }
      // find the interpolated depth
      // parametric
      const double param = -1. * (
          + (p1_screen.x - p0_screen.x) * (p0_screen.x - i)
          + (p1_screen.y - p0_screen.y) * (p0_screen.y - j)
      ) / (
        + pow(p1_screen.x - p0_screen.x, 2.)
        + pow(p1_screen.y - p0_screen.y, 2.)
      );
      const double dist0 = 1. / (param / p1_screen.z + (1. - param) / p0_screen.z);
      // check z-buffer
      pixel_t * const pixel = canvas + j * width + i;
      double * const dist1 = &pixel->depth;
      // by default depth is negative and thus we pick-up larger one
      if (dist0 < *dist1) {
        continue;
      }
      // draw this dot as it comes to the nearest
      pixel->color = *color;
      // update nearest distance as well
      *dist1 = dist0;
    }
  }
  return 0;
}

int contour3d_process_line_obj (
    const camera_t * const camera,
    const screen_t * const screen,
    const contour3d_line_obj_t * const line_obj,
    pixel_t * const canvas
) {
  // aliases for convenience
  contour3d_vector_t (* const add) (
      const contour3d_vector_t v0,
      const contour3d_vector_t v1
  ) = contour3d_vector_add;
  contour3d_vector_t (* const sub) (
      const contour3d_vector_t v0,
      const contour3d_vector_t v1
  ) = contour3d_vector_sub;
  contour3d_vector_t (* const mul) (
      const double w,
      const contour3d_vector_t v0
  ) = contour3d_vector_mul;
  contour3d_vector_t (* const converter) (
      const contour3d_vector_t orthogonal
  ) = line_obj->converter;
  const size_t npoints = line_obj->nitems;
  if (npoints < 2) {
    logger_error("give more than two points to draw a line, received %zu", npoints);
    return 1;
  }
  const contour3d_color_t color = line_obj->color;
  const double width = line_obj->width;
  const contour3d_vector_t * const restrict heads = line_obj->edges + 0;
  const contour3d_vector_t * const restrict tails = line_obj->edges + 1;
  // inter-point distances
  const contour3d_vector_t deltas = mul(1. / (npoints - 1), sub(*tails, *heads));
  // draw lines between two neighbouring points
  for (size_t n = 0; n < npoints - 1; n++) {
    const contour3d_vector_t s = add(*heads, mul(n + 0, deltas));
    const contour3d_vector_t e = add(*heads, mul(n + 1, deltas));
    draw_line_between_two_points(
        camera,
        screen,
        converter,
        &color,
        width,
        &s,
        &e,
        canvas
    );
  }
  return 0;
}

