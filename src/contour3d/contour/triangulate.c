#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "contour3d.h"
#include "../struct.h"
#include "../vector.h"
#include "../logger.h"
#include "./internal.h"

// number of tetrahedra in a lattice
#define N_TETRAHEDRA 6
// number of vertices of a tetrahedron
#define N_VERTICES 4

// cube_index:
//   lower floor | upper floor
//      2---3    |   6---7
//      |   |    |   |   |
//      0---1    |   4---5
//   consider six tetrahedra, which share 0-7 diagonal line
// table which maps two cube indices (denoting lattice vertices)
//   to a lattice edge
static const size_t edge_table[8][8] = {
  {255,   0,   4,  16,   8,  14,  12,  18},
  {  0, 255, 255,   5, 255,   9, 255,  13},
  {  4, 255, 255,   1, 255, 255,  10,  15},
  { 16,   5,   1, 255, 255, 255, 255,  11},
  {  8, 255, 255, 255, 255,   2,   6,  17},
  { 14,   9, 255, 255,   2, 255, 255,   7},
  { 12, 255,  10, 255,   6, 255, 255,   3},
  { 18,  13,  15,  11,  17,   7,   3, 255},
};

typedef struct {
  contour3d_vector_t position;
  double value;
  size_t cube_index;
} vertex_t;

static int interpolate (
    const double vt,
    const vertex_t * restrict const vertex0,
    const vertex_t * restrict const vertex1,
    contour3d_vector_t * const pt
) {
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
  const contour3d_vector_t * const restrict p0 = &vertex0->position;
  const contour3d_vector_t * const restrict p1 = &vertex1->position;
  const double v0 = vertex0->value;
  const double v1 = vertex1->value;
  const double small = 1.e-8;
  if (fabs(v1 - v0) < small) {
    // two points are so close
    // just use the original value
    *pt = *p0;
  } else {
    // simple linear interpolation
    const double factor = (vt - v0) / (v1 - v0);
    *pt = add(*p0, mul(factor, sub(*p1, *p0)));
  }
  return 0;
}

static int compute_face_normal (
    triangle_t * const triangle
) {
  const contour3d_vector_t * restrict const v0 = &triangle->vertices[0];
  const contour3d_vector_t * restrict const v1 = &triangle->vertices[1];
  const contour3d_vector_t * restrict const v2 = &triangle->vertices[2];
  contour3d_vector_t * restrict const face_normal = &triangle->face_normal;
  double * const area = &triangle->area;
  // compute outer product of two vectors to find face normal vector
  *face_normal = contour3d_vector_outer_product(
      contour3d_vector_sub(*v1, *v0),
      contour3d_vector_sub(*v2, *v0)
  );
  // use the norm of this vector to compute the triangle area
  *area = sqrt(contour3d_vector_inner_product(*face_normal, *face_normal));
  // normalise it to obtain the normal vector
  *face_normal = contour3d_vector_mul(1. / *area, *face_normal);
  return 0;
}

static int kernel (
    const bool reverse,
    const double threshold,
    const size_t * const tail_indices,
    const size_t * const head_indices,
    const vertex_t vertices[N_VERTICES],
    triangle_t * const triangle
) {
  // visit three tetrahedron edges and find the intersections
  // for each tetrahedron edge (or equivalently a pair of tetrahedron vertices),
  //   linearly interpolate scalar values to find intersection (= triangle vertex)
  // the visiting order can vary depending on the "reverse" flag,
  //   which alters the direction of the surface normal
  const size_t order[] = {
    reverse ? 2 : 0,
    reverse ? 1 : 1,
    reverse ? 0 : 2,
  };
  for (/* for each tetrahedron edge where a triangle vertex is sitting */ size_t n = 0; n < 3; n++) {
    const size_t tail = tail_indices[order[n]];
    const size_t head = head_indices[order[n]];
    // find intersection
    interpolate(
        threshold,
        vertices + tail,
        vertices + head,
        triangle->vertices + n
    );
    // find a cube edge on which this vertex is sitting,
    //   which is used to smoothen the vertex normal in the later stage
    const size_t tail_id = vertices[tail].cube_index;
    const size_t head_id = vertices[head].cube_index;
    triangle->cube_indices[n] = edge_table[tail_id][head_id];
  }
  compute_face_normal(triangle);
  return 0;
}

static int triangulate_tetrahedron (
    const double grids[3][2],
    contour3d_vector_t (* const coordinate_converter) (
      const contour3d_vector_t orthogonal
    ),
    const double * const array,
    const double threshold,
    const size_t tetrahedron_index,
    size_t * const num_triangles,
    triangle_t triangles[2]
) {
  // four vertices of the tetrahedron are chosen from this table
  const size_t cube_indices[N_TETRAHEDRA][N_VERTICES] = {
    {0, 7, 3, 1},
    {0, 7, 1, 5},
    {0, 7, 5, 4},
    {0, 7, 4, 6},
    {0, 7, 6, 2},
    {0, 7, 2, 3},
  };
  // pick-up four vertices to construct a tetrahedra
  //   from the eight vertices of the given lattice
  vertex_t tetrahedron[N_VERTICES] = {0};
  for (size_t n = 0; n < N_VERTICES; n++) {
    vertex_t * const vertex = tetrahedron + n;
    const size_t cube_index = cube_indices[tetrahedron_index][n];
    // indices in xyz, see above schematic
    const size_t i = cube_index % 2 == 0 ? 0 : 1;
    const size_t j = cube_index % 4 <  2 ? 0 : 1;
    const size_t k = cube_index     <  4 ? 0 : 1;
    const double value = array[k * 4 + j * 2 + i];
    // convert from a general to the cartesian coordinate systems
    const contour3d_vector_t cartesian = {
      grids[0][i],
      grids[1][j],
      grids[2][k],
    };
    vertex->position = coordinate_converter(cartesian);
    vertex->cube_index = cube_index;
    vertex->value = value;
  }
  // find one triangle / two triangles inside the given tetrahedron
  // prepare 4-bit mask
  uint_fast8_t mask = 0;
  for (size_t n = 0; n < N_VERTICES; n++) {
    mask |= (threshold < tetrahedron[n].value) << n;
  }
  // normal goes to the opposite direction
  //   when the top bit is 1
  // 1: 0001 <-> 14: 1110
  // 2: 0010 <-> 13: 1101
  // 3: 0011 <-> 12: 1100
  // 4: 0100 <-> 11: 1011
  // 5: 0101 <-> 10: 1010
  // 6: 0110 <->  9: 1001
  // 7: 0111 <->  8: 1000
  const bool reverse = (1 << 3) <= mask;
  if (reverse) {
    mask = 0xF - mask;
  }
  if (0 == mask) {
    // no triangle
    return 0;
  }
  // check triangles inside a tetrahedron
  //   2-1
  //   |/
  //   0
  // and the top corner is indexed as 3
  // NOTE: make sure normal vector goes from bit-0-zone to bit-1-zone
  //   following the right-hand rule
  if (/* 0001 */ 1 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {0, 0, 0, }, (size_t []) {1, 3, 2, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0010 */ 2 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {1, 1, 1, }, (size_t []) {0, 2, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0011 */ 3 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {0, 1, 0, }, (size_t []) {2, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []) {0, 1, 1, }, (size_t []) {2, 2, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0100 */ 4 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {2, 2, 2, }, (size_t []) {0, 3, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0101 */ 5 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {2, 0, 2, }, (size_t []) {1, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []) {2, 0, 0, }, (size_t []) {1, 1, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0110 */ 6 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {0, 2, 1, }, (size_t []) {2, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []) {0, 1, 0, }, (size_t []) {2, 3, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else if (/* 0111 */ 7 == mask) {
    kernel(
        reverse, threshold,
        (size_t []) {3, 3, 3, }, (size_t []) {0, 2, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  } else {
    logger_error("mask = %hhu: should not reach here\n", mask);
    return 1;
  }
  return 0;
}

static inline size_t ravel (
    const size_t glsizes[2],
    const size_t i,
    const size_t j,
    const size_t k
) {
  return (k * glsizes[1] + j) * glsizes[0] + i;
}

int contour3d_contour_triangulate_slice (
    const size_t glsizes[2],
    double * const grids[3],
    contour3d_vector_t (* const coordinate_converter) (
      const contour3d_vector_t orthogonal
    ),
    const double * const array,
    const double threshold,
    lattice_t * const lattices
) {
  const size_t imax = glsizes[0] - 1;
  const size_t jmax = glsizes[1] - 1;
  for (/* each y */ size_t j = 0; j < jmax; j++) {
    for (/* each x */ size_t i = 0; i < imax; i++) {
      lattice_t * const lattice = lattices + j * imax + i;
      // pack information
      const double localgrids[CONTOUR3D_NDIMS][2] = {
        {grids[0][i], grids[0][i + 1]},
        {grids[1][j], grids[1][j + 1]},
        {grids[2][0], grids[2][    1]},
      };
      const double scalars[8] = {
        array[ravel(glsizes, i    , j    , 0)],
        array[ravel(glsizes, i + 1, j    , 0)],
        array[ravel(glsizes, i    , j + 1, 0)],
        array[ravel(glsizes, i + 1, j + 1, 0)],
        array[ravel(glsizes, i    , j    , 1)],
        array[ravel(glsizes, i + 1, j    , 1)],
        array[ravel(glsizes, i    , j + 1, 1)],
        array[ravel(glsizes, i + 1, j + 1, 1)],
      };
      // there are six tetrahedra in one lattice
      // tessellate each tetrahedron
      size_t * const num_triangles = &lattice->num_triangles;
      triangle_t * const triangles = lattice->triangles;
      // reset number of triangles,
      //   as it may store the previous value
      *num_triangles = 0;
      for (/* each tetrahedron */ size_t n = 0; n < N_TETRAHEDRA; n++) {
        if (0 != triangulate_tetrahedron(
              localgrids,
              coordinate_converter,
              scalars,
              threshold,
              n,
              num_triangles,
              triangles
        )) {
          logger_error("failed to triangulate a tetrahedron");
          return 1;
        }
      }
    }
  }
  return 0;
}

