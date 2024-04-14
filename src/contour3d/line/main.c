#include <stdio.h>
#include <math.h>
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "../internal.h"
#define CONTOUR3D_LINE_INTERNAL
#include "internal.h"

static inline int_fast32_t intmin(const int_fast32_t v0, const int_fast32_t v1){
  return v0 < v1 ? v0 : v1;
}

static inline int_fast32_t intmax(const int_fast32_t v0, const int_fast32_t v1){
  return v0 > v1 ? v0 : v1;
}

// draw a line between point "p0" and point "p1",
//   whose components are both in orthogonal coordinate system
static int draw_a_line_between_two_points (
    const camera_t * camera,
    const screen_t * screen,
    int (* converter) (
      const contour3d_vector_t * orthogonal,
      contour3d_vector_t * cartesian
    ),
    const uint8_t color[3],
    const double line_width,
    const contour3d_vector_t * p0_orthogonal,
    const contour3d_vector_t * p1_orthogonal,
    pixel_t * canvas
) {
  // transfer points to Cartesian coordinate system
  contour3d_vector_t p0_cartesian = {0., 0., 0.};
  contour3d_vector_t p1_cartesian = {0., 0., 0.};
  converter(p0_orthogonal, &p0_cartesian);
  converter(p1_orthogonal, &p1_cartesian);
  // project points in Cartesian coordinat to screen Coordinate
  contour3d_vector_t p0_screen = {0., 0., 0.};
  contour3d_vector_t p1_screen = {0., 0., 0.};
  if(0 != contour3d_project(
        camera,
        screen,
        &p0_cartesian,
        &p0_screen
  )){
    return 1;
  }
  if(0 != contour3d_project(
        camera,
        screen,
        &p1_cartesian,
        &p1_screen
  )){
    return 1;
  }
  // draw dots
  const size_t width  = screen->width;
  const size_t height = screen->height;
  // convert coordinate
  p0_screen[0] = (0.5 + p0_screen[0]) * width ;
  p0_screen[1] = (0.5 + p0_screen[1]) * height;
  p1_screen[0] = (0.5 + p1_screen[0]) * width ;
  p1_screen[1] = (0.5 + p1_screen[1]) * height;
  // prepare bounding box
  const int_fast32_t xmin = fmin(p0_screen[0], p1_screen[0]) - 1;
  const int_fast32_t xmax = fmax(p0_screen[0], p1_screen[0]) + 1;
  const int_fast32_t ymin = fmin(p0_screen[1], p1_screen[1]) - 1;
  const int_fast32_t ymax = fmax(p0_screen[1], p1_screen[1]) + 1;
  // avoid out-of-bounds access
  const size_t imin = intmin( width - 1, intmax(0, xmin));
  const size_t imax = intmin( width - 1, intmax(0, xmax));
  const size_t jmin = intmin(height - 1, intmax(0, ymin));
  const size_t jmax = intmin(height - 1, intmax(0, ymax));
  // make the dot white if the distance between
  //   the pixel and a line is below the width
  // line: ax + by + c = 0
  const double a = p1_screen[1] - p0_screen[1];
  const double b = p0_screen[0] - p1_screen[0];
  const double c =
    + (p1_screen[0] - p0_screen[0]) * p0_screen[1]
    - (p1_screen[1] - p0_screen[1]) * p0_screen[0];
  for(size_t j = jmin; j <= jmax; j++){
    for(size_t i = imin; i <= imax; i++){
      const size_t index = j * width + i;
      // check distance from the line segment
      const double threshold = pow(0.5 * line_width, 2.);
      if(pow(a * i + b * j + c, 2.) / (a * a + b * b) > threshold){
        continue;
      }
      // find the interpolated depth
      // parametric
      const double param = -1. * (
          + (p1_screen[0] - p0_screen[0]) * (p0_screen[0] - i)
          + (p1_screen[1] - p0_screen[1]) * (p0_screen[1] - j)
      ) / (
        + pow(p1_screen[0] - p0_screen[0], 2.)
        + pow(p1_screen[1] - p0_screen[1], 2.)
      );
      const double dist0 = 1. / (param / p1_screen[2] + (1. - param) / p0_screen[2]);
      // check z-buffer
      pixel_t * pixel = canvas + index;
      double * dist1 = &pixel->depth;
      // by default depth is negative
      if(dist0 > *dist1){
        // draw if this dot comes the nearest
        pixel->color[0] = color[0];
        pixel->color[1] = color[1];
        pixel->color[2] = color[2];
        // update nearest distance
        *dist1 = dist0;
      }
    }
  }
  return 0;
}

int contour3d_process_line_obj(
    const camera_t * camera,
    const screen_t * screen,
    const contour3d_line_obj_t * line_obj,
    pixel_t * canvas
){
  const double * xedges = line_obj->line_configs[0].edges;
  const double * yedges = line_obj->line_configs[1].edges;
  const double * zedges = line_obj->line_configs[2].edges;
  const size_t xnitems = line_obj->line_configs[0].nitems;
  const size_t ynitems = line_obj->line_configs[1].nitems;
  const size_t znitems = line_obj->line_configs[2].nitems;
  if (xnitems < 2 || ynitems < 2 || znitems < 2) {
    printf("give more than two points to draw a line\n");
    return 1;
  }
  const double dx = (xedges[1] - xedges[0]) / (xnitems - 1);
  const double dy = (yedges[1] - yedges[0]) / (ynitems - 1);
  const double dz = (zedges[1] - zedges[0]) / (znitems - 1);
  // draw 12 lines in total (moving in one line, while other directions are fixed)
  for (size_t line_index = 0; line_index < 12; line_index++) {
    const size_t vary_in = line_index < 4 ? 0 : line_index < 8 ? 1 : 2;
    const size_t nitems = 0 == vary_in ? xnitems : 1 == vary_in ? ynitems : znitems;
    for (size_t point_index = 0; point_index < nitems - 1; point_index++) {
      double xs = 0.;
      double xe = 0.;
      double ys = 0.;
      double ye = 0.;
      double zs = 0.;
      double ze = 0.;
      if (line_index < 4) {
        const size_t n = line_index;
        xs = xedges[0] + dx * (point_index + 0), xe = xedges[0] + dx * (point_index + 1);
        ys = yedges[n % 2], ye = yedges[n % 2];
        zs = zedges[n / 2], ze = zedges[n / 2];
      } else if (line_index < 8) {
        const size_t n = line_index - 4;
        xs = xedges[n / 2], xe = xedges[n / 2];
        ys = yedges[0] + dy * (point_index + 0), ye = yedges[0] + dy * (point_index + 1);
        zs = zedges[n % 2], ze = zedges[n % 2];
      } else {
        const size_t n = line_index - 8;
        xs = xedges[n % 2], xe = xedges[n % 2];
        ys = yedges[n / 2], ye = yedges[n / 2];
        zs = zedges[0] + dz * (point_index + 0), ze = zedges[0] + dz * (point_index + 1);
      }
      const contour3d_vector_t s = {xs, ys, zs};
      const contour3d_vector_t e = {xe, ye, ze};
      if (0 != draw_a_line_between_two_points(
            camera,
            screen,
            line_obj->converter,
            line_obj->color,
            line_obj->width,
            &s,
            &e,
            canvas
      )) {
        return 1;
      }
    }
  }
  return 0;
}

