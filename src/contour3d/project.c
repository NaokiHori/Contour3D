#define CONTOUR3D_INTERNAL
#include "internal.h"

// project a point p0 described on the Cartesian domain
//   to the screen
// the resulting point p1 is described on the screen coordinate system,
//   and the third element holds the depth information

int contour3d_project(
    const camera_t * camera,
    const screen_t * screen,
    const vector_t * p0,
    vector_t * p1
){
  // vector from the camera to the given point
  // note that an arbitrary point on this line
  //   can be represented as "camera + t * ray",
  //   where "t" is a parameter
  const vector_t ray = {
    + (*p0)[0] - camera->position[0],
    + (*p0)[1] - camera->position[1],
    + (*p0)[2] - camera->position[2],
  };
  // determine parameter t where ray intersects with the screen
  // a function describing the screen surface reads
  //   nx x + ny y + nz z = d,
  //   where "d" is "screen->intercept" computed beforehand
  // "t" is uniquely determined by coupling the surface function and the line equaion
  const double v0 =
    + screen->normal[0] * camera->position[0]
    + screen->normal[1] * camera->position[1]
    + screen->normal[2] * camera->position[2];
  const double v1 =
    + screen->normal[0] * ray[0]
    + screen->normal[1] * ray[1]
    + screen->normal[2] * ray[2];
  const double t = (screen->intercept - v0) / v1;
  // out-of-range
  if(0. >= t) return 1;
  if(1. <= t) return 1;
  // find a vector from the screen center to the intersection on the screen
  const vector_t delta = {
      + camera->position[0] + t * ray[0] - screen->center[0],
      + camera->position[1] + t * ray[1] - screen->center[1],
      + camera->position[2] + t * ray[2] - screen->center[2],
  };
  // the above vector components are still in Cartesian coordinate,
  //   which is to be transformed to the screen coordinate
  // this can be done by computing the inner product with
  //   the horizontal and the vertical vectors respectively,
  //   which are followed by appropriate normalisations
  // NOTE: two screen bases are assumed to be orthogonal
  (*p1)[0] = screen->ipinv_local_x * (
      + delta[0] * screen->local_x[0]
      + delta[1] * screen->local_x[1]
      + delta[2] * screen->local_x[2]
  );
  (*p1)[1] = screen->ipinv_local_y * (
      + delta[0] * screen->local_y[0]
      + delta[1] * screen->local_y[1]
      + delta[2] * screen->local_y[2]
  );
  // z is used to store "depth", which is usually negative
  (*p1)[2] =
    + ray[0] * screen->normal[0]
    + ray[1] * screen->normal[1]
    + ray[2] * screen->normal[2];
  return 0;
}

