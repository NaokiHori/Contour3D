#include <math.h>
#include "contour3d.h"
#include "./vector.h"

contour3d_vector_t contour3d_vector_add (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
) {
  const contour3d_vector_t v2 = {
    .x = + v0.x + v1.x,
    .y = + v0.y + v1.y,
    .z = + v0.z + v1.z,
  };
  return v2;
}

contour3d_vector_t contour3d_vector_sub (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
) {
  const contour3d_vector_t v2 = {
    .x = + v0.x - v1.x,
    .y = + v0.y - v1.y,
    .z = + v0.z - v1.z,
  };
  return v2;
}

contour3d_vector_t contour3d_vector_mul (
    const double w,
    const contour3d_vector_t v0
) {
  const contour3d_vector_t v1 = {
    .x = w * v0.x,
    .y = w * v0.y,
    .z = w * v0.z,
  };
  return v1;
}

double contour3d_vector_inner_product (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
) {
  return
    + v0.x * v1.x
    + v0.y * v1.y
    + v0.z * v1.z;
}

contour3d_vector_t contour3d_vector_outer_product (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
) {
  const double x = + v0.y * v1.z - v0.z * v1.y;
  const double y = + v0.z * v1.x - v0.x * v1.z;
  const double z = + v0.x * v1.y - v0.y * v1.x;
  const contour3d_vector_t v2 = {
    .x = x,
    .y = y,
    .z = z,
  };
  return v2;
}

contour3d_vector_t contour3d_vector_normalise (
    const contour3d_vector_t v0
) {
  const double norminv = 1. / sqrt(contour3d_vector_inner_product(v0, v0));
  const contour3d_vector_t v1 = {
    .x = v0.x * norminv,
    .y = v0.y * norminv,
    .z = v0.z * norminv,
  };
  return v1;
}

