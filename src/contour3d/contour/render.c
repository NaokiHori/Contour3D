#include <math.h>
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "../internal.h"
#define CONTOUR3D_CONTOUR_INTERNAL
#include "internal.h"

static inline double dblmin3(const double v0, const double v1, const double v2){
  return fmin(v0, fmin(v1, v2));
}

static inline double dblmax3(const double v0, const double v1, const double v2){
  return fmax(v0, fmax(v1, v2));
}

static inline int_fast32_t intmin(const int_fast32_t v0, const int_fast32_t v1){
  return v0 < v1 ? v0 : v1;
}

static inline int_fast32_t intmax(const int_fast32_t v0, const int_fast32_t v1){
  return v0 > v1 ? v0 : v1;
}

static double edge_function(const vector_t * v0, const vector_t * v1, const vector_t * v2){
  return
    + ((*v2)[0] - (*v0)[0]) * ((*v1)[1] - (*v0)[1])
    - ((*v2)[1] - (*v0)[1]) * ((*v1)[0] - (*v0)[0]);
}

int contour3d_contour_render_triangle(
    const camera_t * camera,
    const vector_t * light,
    const screen_t * screen,
    const uint8_t fg_color[3],
    const triangle_t * triangle,
    pixel_t * canvas
){
  const size_t width  = screen->width;
  const size_t height = screen->height;
  // consider a triangle "p"rojected onto the screen
  // if non-zero value is returned at least for one vertex,
  //   I assume tihs triangle is out-of-range
  triangle_t projected = {0};
  for(size_t dim = 0; dim < 3; dim++){
    if(0 != contour3d_project(
          camera,
          screen,
          triangle->vertices + dim,
          projected.vertices + dim
    )) return 0;
  }
  // three vertices of the projected triangle on the screen
  // the vector components are on the screen coordinate (2D),
  //   while the z element is used to store the depth,
  //   which will be used to compare / update the z-buffer
  const vector_t * v0 = projected.vertices + 0;
  const vector_t * v1 = projected.vertices + 1;
  const vector_t * v2 = projected.vertices + 2;
  // prepare bounding box
  const int_fast32_t xmin = (0.5 + dblmin3((*v0)[0], (*v1)[0], (*v2)[0])) * width  - 1;
  const int_fast32_t xmax = (0.5 + dblmax3((*v0)[0], (*v1)[0], (*v2)[0])) * width  + 1;
  const int_fast32_t ymin = (0.5 + dblmin3((*v0)[1], (*v1)[1], (*v2)[1])) * height - 1;
  const int_fast32_t ymax = (0.5 + dblmax3((*v0)[1], (*v1)[1], (*v2)[1])) * height + 1;
  // avoid out-of-bounds access
  const size_t imin = intmin( width - 1, intmax(0, xmin));
  const size_t imax = intmin( width - 1, intmax(0, xmax));
  const size_t jmin = intmin(height - 1, intmax(0, ymin));
  const size_t jmax = intmin(height - 1, intmax(0, ymax));
  // in-out check
  for(size_t j = jmin; j <= jmax; j++){
    for(size_t i = imin; i <= imax; i++){
      // target point inside the bounding box (on screen coordinate[-0.5:+0.5])
      const vector_t v3 = {
        1. * (i + 0.5) / width  - 0.5,
        1. * (j + 0.5) / height - 0.5,
        0., // not used
      };
      // compute weights on the barycentric coordinate
      //   using the edge function normalised by the area
      const double ainv = 1. / edge_function(v0, v1, v2);
      // early return if this pixel is out of the triangle
      // give small negative number instead of 0 to avoid hole
      const double small = -1.e-8;
      const double w0 = ainv * edge_function(v1, v2, &v3);
      if(w0 < small) continue;
      const double w1 = ainv * edge_function(v2, v0, &v3);
      if(w1 < small) continue;
      const double w2 = ainv * edge_function(v0, v1, &v3);
      if(w2 < small) continue;
      // now this pixel is inside the triangle and thus this is of my interest
      pixel_t * pixel = canvas + j * width + i;
      double * dist1 = &pixel->depth;
      // compute depth by using harmonic average in the barycentric coordinate
      const double dist0 = 1. / (
          + w0 / (*v0)[2]
          + w1 / (*v1)[2]
          + w2 / (*v2)[2]
      );
      if(dist0 < *dist1){
        // go on to the next triangle
        //   since this one does not come the nearest
        continue;
      }
      // draw this facet since it comes the nearest
      // update the nearest distance for later elements
      *dist1 = dist0;
      // adjust facet color (make it darker) depending on
      //   the angle between the normal vector and the light
      // first obtain the local face normal
      //   by averaging three vertex normals on the barycentric coordinate
      const vector_t * vertex_normals = triangle->vertex_normals;
      vector_t face_normal = {
        + w0 * vertex_normals[0][0] + w1 * vertex_normals[1][0] + w2 * vertex_normals[2][0],
        + w0 * vertex_normals[0][1] + w1 * vertex_normals[1][1] + w2 * vertex_normals[2][1],
        + w0 * vertex_normals[0][2] + w1 * vertex_normals[1][2] + w2 * vertex_normals[2][2],
      };
      const double norminv = 1. / sqrt(
        + pow(face_normal[0], 2.)
        + pow(face_normal[1], 2.)
        + pow(face_normal[2], 2.)
        + 1.e-8 // avoid zero division, just in case
      );
      face_normal[0] *= norminv;
      face_normal[1] *= norminv;
      face_normal[2] *= norminv;
      // now adjust color using the computed local surface normal
      // here I do not care front / back
      // NOTE: inner product yields [-1:+1],
      //   which is adjusted to enforce [0:1]
      double factor = fabs(
          + face_normal[0] * (*light)[0]
          + face_normal[1] * (*light)[1]
          + face_normal[2] * (*light)[2]
      );
      // avoid too-dark color
      if(0.25 > factor){
        factor = 0.25;
      }
      // now factor is in [0.25, 1.0], decide the final colour
      pixel->color[0] = (uint8_t)(factor * fg_color[0]);
      pixel->color[1] = (uint8_t)(factor * fg_color[1]);
      pixel->color[2] = (uint8_t)(factor * fg_color[2]);
    }
  }
  return 0;
}

