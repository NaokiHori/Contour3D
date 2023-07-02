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

int contour3d_process_line_obj(
    const camera_t * camera,
    const screen_t * screen,
    const contour3d_line_obj_t * line_obj,
    pixel_t * canvas
){
  const size_t nitems = line_obj->nitems;
  if(nitems < 2){
    printf("give more than two points to draw a line (or lines)\n");
    return 1;
  }
  double * const * grids = line_obj->grids;
  for(/* each pair of dots */ size_t index = 0; index < nitems - 1; index++){
    // project points (p0, p1) to screen (q0, q1)
    const vector_t p0 = {
      grids[0][index    ],
      grids[1][index    ],
      grids[2][index    ],
    };
    const vector_t p1 = {
      grids[0][index + 1],
      grids[1][index + 1],
      grids[2][index + 1],
    };
    vector_t q0 = {0};
    vector_t q1 = {0};
    if(0 != contour3d_project(
          camera,
          screen,
          &p0,
          &q0
    )){
      continue;
    }
    if(0 != contour3d_project(
          camera,
          screen,
          &p1,
          &q1
    )){
      continue;
    }
    // draw dots
    const size_t width  = screen->width;
    const size_t height = screen->height;
    // convert coordinate
    q0[0] = (0.5 + q0[0]) * width ;
    q0[1] = (0.5 + q0[1]) * height;
    q1[0] = (0.5 + q1[0]) * width ;
    q1[1] = (0.5 + q1[1]) * height;
    // prepare bounding box
    const int_fast32_t xmin = fmin(q0[0], q1[0]) - 1;
    const int_fast32_t xmax = fmax(q0[0], q1[0]) + 1;
    const int_fast32_t ymin = fmin(q0[1], q1[1]) - 1;
    const int_fast32_t ymax = fmax(q0[1], q1[1]) + 1;
    // avoid out-of-bounds access
    const size_t imin = intmin( width - 1, intmax(0, xmin));
    const size_t imax = intmin( width - 1, intmax(0, xmax));
    const size_t jmin = intmin(height - 1, intmax(0, ymin));
    const size_t jmax = intmin(height - 1, intmax(0, ymax));
    // make the dot white if the distance between
    //   the pixel and a line is below the width
    // line: ax + by + c = 0
    const double a = q1[1] - q0[1];
    const double b = q0[0] - q1[0];
    const double c =
      + (q1[0] - q0[0]) * q0[1]
      - (q1[1] - q0[1]) * q0[0];
    for(size_t j = jmin; j <= jmax; j++){
      for(size_t i = imin; i <= imax; i++){
        const size_t index = j * width + i;
        // check distance from the line segment
        const double threshold = pow(0.5 * line_obj->width, 2.);
        if(pow(a * i + b * j + c, 2.) / (a * a + b * b) > threshold){
          continue;
        }
        // find the interpolated depth
        // parametric
        const double param = -1. * (
            + (q1[0] - q0[0]) * (q0[0] - i)
            + (q1[1] - q0[1]) * (q0[1] - j)
        ) / (
          + pow(q1[0] - q0[0], 2.)
          + pow(q1[1] - q0[1], 2.)
        );
        const double dist0 = 1. / (param / q1[2] + (1. - param) / q0[2]);
        // check z-buffer
        pixel_t * pixel = canvas + index;
        double * dist1 = &pixel->depth;
        // by default depth is negative
        if(dist0 > *dist1){
          // draw if this dot comes the nearest
          pixel->color[0] = line_obj->color[0];
          pixel->color[1] = line_obj->color[1];
          pixel->color[2] = line_obj->color[2];
          // update nearest distance
          *dist1 = dist0;
        }
      }
    }
  }
  return 0;
}

