#include "./struct.h"
#include "./vector.h"
#include "./project.h"

// project a point p0 described on the Cartesian domain
//   to the screen
// the resulting point p1 is described on the screen coordinate system,
//   and the third element holds the depth information
int contour3d_project (
    const camera_t * const camera,
    const screen_t * const screen,
    const contour3d_vector_t * restrict const p0,
    contour3d_vector_t * restrict const p1
) {
  // aliases for convenience
  contour3d_vector_t (* const sub) (
      const contour3d_vector_t v0,
      const contour3d_vector_t v1
  ) = contour3d_vector_sub;
  double (* const ip) (
      const contour3d_vector_t v0,
      const contour3d_vector_t v1
  ) = contour3d_vector_inner_product;
  // vector from the camera to the given point
  // note that an arbitrary point on this line
  //   can be represented as "camera + t * ray",
  //   where "t" is a parameter
  const contour3d_vector_t ray = sub(*p0, camera->position);
  // determine parameter t where ray intersects with the screen
  // a function describing the screen surface reads
  //   nx x + ny y + nz z = d
  // "t" is uniquely determined by coupling the surface and the line:
  //   normal (camera + t ray) = d
  const double nc = ip(screen->normal, camera->position);
  const double nr = ip(screen->normal, ray);
  const double d  = ip(screen->normal, screen->center);
  // avoid zero division
  const double small = 1.e-8;
  if (- small < nr && nr < small) {
    return 1;
  }
  const double t = (d - nc) / nr;
  // out-of-range
  if (t <= 0. || 1. <= t) {
    return 1;
  }
  // find a vector from the screen center to the intersection on the screen
  const contour3d_vector_t delta = {
    + camera->position.x + t * ray.x - screen->center.x,
    + camera->position.y + t * ray.y - screen->center.y,
    + camera->position.z + t * ray.z - screen->center.z,
  };
  // the above vector components are still in the global Cartesian coordinate,
  //   which is transformed to the screen coordinate
  // this is done by computing the inner product with
  //   the horizontal and the vertical vectors respectively
  //   which are followed by appropriate normalisations
  // these vectors are NOT normalised and
  //   their l2 norms represent the lengths of the screen,
  //   which are used to normalise
  // NOTE: two screen bases are assumed to be orthogonal
  p1->x = ip(delta, screen->local_x) / ip(screen->local_x, screen->local_x);
  p1->y = ip(delta, screen->local_y) / ip(screen->local_y, screen->local_y);
  // z is used to store the depth, which is usually negative
  p1->z = nr;
  return 0;
}

