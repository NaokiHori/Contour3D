#include <math.h>
#include "contour3d.h"
#include "../struct.h"
#include "../project.h"
#include "../vector.h"
#include "./internal.h"

static inline double dblmin3 (
    const double v0,
    const double v1,
    const double v2
) {
  return fmin(v0, fmin(v1, v2));
}

static inline double dblmax3 (
    const double v0,
    const double v1,
    const double v2
) {
  return fmax(v0, fmax(v1, v2));
}

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

static inline double edge_function (
    const contour3d_vector_t * restrict const v0,
    const contour3d_vector_t * restrict const v1,
    const contour3d_vector_t * restrict const v2
) {
  return
    + (v2->x - v0->x) * (v1->y - v0->y)
    - (v2->y - v0->y) * (v1->x - v0->x);
}

int contour3d_contour_render_triangle (
    const camera_t * const camera,
    const contour3d_vector_t * const light,
    const screen_t * const screen,
    const contour3d_color_t * const fg_color,
    const triangle_t * const triangle,
    pixel_t * const canvas
) {
  const size_t width  = screen->width;
  const size_t height = screen->height;
  // consider a triangle projected onto the screen
  // if at least one vertex returns non-zero value,
  //   I assume this triangle is out-of-range
  triangle_t projected = {0};
  for (size_t dim = 0; dim < 3; dim++) {
    if (0 != contour3d_project(
          camera,
          screen,
          triangle->vertices + dim,
          projected.vertices + dim
    )) {
      // this is an expected termination and thus return success
      return 0;
    }
  }
  // three vertices of the projected triangle on the screen
  // the vector components are on the screen coordinate (2D),
  //   while the z element is used to store the depth,
  //   which will be used to compare / update the z-buffer
  const contour3d_vector_t * restrict const v0 = projected.vertices + 0;
  const contour3d_vector_t * restrict const v1 = projected.vertices + 1;
  const contour3d_vector_t * restrict const v2 = projected.vertices + 2;
  // prepare bounding box
  const int_fast32_t xmin = (0.5 + dblmin3(v0->x, v1->x, v2->x)) * width  - 1;
  const int_fast32_t xmax = (0.5 + dblmax3(v0->x, v1->x, v2->x)) * width  + 1;
  const int_fast32_t ymin = (0.5 + dblmin3(v0->y, v1->y, v2->y)) * height - 1;
  const int_fast32_t ymax = (0.5 + dblmax3(v0->y, v1->y, v2->y)) * height + 1;
  // avoid out-of-bounds access
  const size_t imin = intmin( width - 1, intmax(0, xmin));
  const size_t imax = intmin( width - 1, intmax(0, xmax));
  const size_t jmin = intmin(height - 1, intmax(0, ymin));
  const size_t jmax = intmin(height - 1, intmax(0, ymax));
  // perform in-out check for each pixel inside the bounding box
  for (size_t j = jmin; j <= jmax; j++) {
    for (size_t i = imin; i <= imax; i++) {
      // target point inside the bounding box, on the screen coordinate[-0.5 : +0.5]
      const contour3d_vector_t v3 = {
        1. * (i + 0.5) / width  - 0.5,
        1. * (j + 0.5) / height - 0.5,
        0., // not used
      };
      // compute weights on the barycentric coordinate
      //   using the edge function normalised by the area
      // compute inverse signed area while taking care of zero divisions
      const double area = edge_function(v0, v1, v2);
      const double area_inv = 1. / (
          area < 0. ? fmin(area, - 1.e-16)
                    : fmax(area, + 1.e-16)
      );
      const double w0 = area_inv * edge_function(v1, v2, &v3);
      const double w1 = area_inv * edge_function(v2, v0, &v3);
      const double w2 = area_inv * edge_function(v0, v1, &v3);
      // early return if this pixel is out of the triangle
      // give small negative number instead of 0 to avoid hole
      const double small = - 1.e-8;
      if (w0 < small || w1 < small || w2 < small) {
        continue;
      }
      // we know this pixel is inside the triangle now
      pixel_t * const pixel = canvas + j * width + i;
      double * const dist1 = &pixel->depth;
      // compute depth by using harmonic average in the barycentric coordinate
      const double dist0 = 1. / (
          + w0 / v0->z
          + w1 / v1->z
          + w2 / v2->z
      );
      if (dist0 < *dist1) {
        // this triangle does not come to the nearest in this pixel
        // go on to the next pixel
        continue;
      }
      // we found this facet comes the nearest
      // update the nearest distance for later elements
      *dist1 = dist0;
      // adjust facet color (make it darker) depending on
      //   the angle between the normal vector and the light
      // first obtain the local face normal
      //   by averaging three vertex normals on the barycentric coordinate
      const contour3d_vector_t * restrict const vertex_normals = triangle->vertex_normals;
      const contour3d_vector_t face_normal = contour3d_vector_normalise(
          contour3d_vector_add(
            contour3d_vector_mul(w0, vertex_normals[0]),
            contour3d_vector_add(
              contour3d_vector_mul(w1, vertex_normals[1]),
              contour3d_vector_mul(w2, vertex_normals[2])
            )
          )
      );
      // now adjust color using the computed local surface normal
      // here I do not care front / back
      // NOTE: inner product yields [-1:+1],
      //   which is adjusted to enforce [0:1]
      const double factor = fmax(
          // give a lower bound to avoid too-dark colors
          0.15,
          fabs(contour3d_vector_inner_product(face_normal, *light))
      );
      // decide the final colour
      pixel->color.r = (uint8_t)(factor * fg_color->r);
      pixel->color.g = (uint8_t)(factor * fg_color->g);
      pixel->color.b = (uint8_t)(factor * fg_color->b);
    }
  }
  return 0;
}

