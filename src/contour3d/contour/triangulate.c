#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "../internal.h"
#define CONTOUR3D_CONTOUR_INTERNAL
#include "internal.h"

static const size_t n_a = 255;

// return index of lattice edge
//   which connects two lattice vertices
static const size_t edge_table[8][8] = {
  {n_a,   0,   4,  16,   8,  14,  12,  18},
  {  0, n_a, n_a,   5, n_a,   9, n_a,  13},
  {  4, n_a, n_a,   1, n_a, n_a,  10,  15},
  { 16,   5,   1, n_a, n_a, n_a, n_a,  11},
  {  8, n_a, n_a, n_a, n_a,   2,   6,  17},
  { 14,   9, n_a, n_a,   2, n_a, n_a,   7},
  { 12, n_a,  10, n_a,   6, n_a, n_a,   3},
  { 18,  13,  15,  11,  17,   7,   3, n_a},
};

typedef struct {
  contour3d_vector_t position;
  double value;
  size_t cube_index;
} vertex_t;

static int interpolate(
    const double vt,
    const vertex_t * vertex0,
    const vertex_t * vertex1,
    contour3d_vector_t * pt
){
  const contour3d_vector_t * p0 = &vertex0->position;
  const contour3d_vector_t * p1 = &vertex1->position;
  const double v0 = vertex0->value;
  const double v1 = vertex1->value;
  const double small = 1.e-8;
  if(fabs(v1 - v0) < small){
    // two points are so close,
    //   just use the original value
    (*pt)[0] = (*p0)[0];
    (*pt)[1] = (*p0)[1];
    (*pt)[2] = (*p0)[2];
  }else{
    // simple linear interpolation
    const double factor = (vt - v0) / (v1 - v0);
    (*pt)[0] = (*p0)[0] + factor * ((*p1)[0] - (*p0)[0]);
    (*pt)[1] = (*p0)[1] + factor * ((*p1)[1] - (*p0)[1]);
    (*pt)[2] = (*p0)[2] + factor * ((*p1)[2] - (*p0)[2]);
  }
  return 0;
}

static int compute_face_normal(
    triangle_t * triangle
){
  const contour3d_vector_t * vertex0 = &triangle->vertices[0];
  const contour3d_vector_t * vertex1 = &triangle->vertices[1];
  const contour3d_vector_t * vertex2 = &triangle->vertices[2];
  contour3d_vector_t * face_normal = &triangle->face_normal;
  // find face normal using the cross product
  const contour3d_vector_t v01 = {
    + (*vertex1)[0] - (*vertex0)[0],
    + (*vertex1)[1] - (*vertex0)[1],
    + (*vertex1)[2] - (*vertex0)[2],
  };
  const contour3d_vector_t v02 = {
    + (*vertex2)[0] - (*vertex0)[0],
    + (*vertex2)[1] - (*vertex0)[1],
    + (*vertex2)[2] - (*vertex0)[2],
  };
  // outer product of two vectors
  //   and normalise it (norm: area)
  (*face_normal)[0] = + v01[1] * v02[2] - v01[2] * v02[1];
  (*face_normal)[1] = + v01[2] * v02[0] - v01[0] * v02[2];
  (*face_normal)[2] = + v01[0] * v02[1] - v01[1] * v02[0];
  const double areainv = 1. / sqrt(
      + pow((*face_normal)[0], 2.)
      + pow((*face_normal)[1], 2.)
      + pow((*face_normal)[2], 2.)
  );
  (*face_normal)[0] *= areainv;
  (*face_normal)[1] *= areainv;
  (*face_normal)[2] *= areainv;
  triangle->area = 1. / areainv;
  return 0;
}

static int kernel(
    const bool reverse,
    const double threshold,
    const size_t * tail_indices,
    const size_t * head_indices,
    const vertex_t vertices[4],
    triangle_t * triangle
){
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
  for(/* for each tetrahedron edge */ size_t index = 0; index < 3; index++){
    const size_t tail = tail_indices[order[index]];
    const size_t head = head_indices[order[index]];
    // find intersection
    interpolate(
        threshold,
        vertices + tail,
        vertices + head,
        triangle->vertices + index
    );
    // "on which cube edge is this vertex sitting?" is important
    //   to smoothen the vertex normal in the later stage
    const size_t tail_id = vertices[tail].cube_index;
    const size_t head_id = vertices[head].cube_index;
    const size_t cube_index = edge_table[tail_id][head_id];
    triangle->cube_indices[index] = cube_index;
  }
  compute_face_normal(triangle);
  return 0;
}

static int triangulate_tetrahedron(
    const double grids[3][2],
    int (* const coordinate_converter)(const contour3d_vector_t * orthogonal, contour3d_vector_t * cartesian),
    const double * array,
    const double threshold,
    const size_t tetrahedron_index,
    size_t * num_triangles,
    triangle_t triangles[2]
){
  // four vertices of the tetrahedron are chosen from this table
  // cube_index:
  // lower floor | upper floor
  //    2---3    |   6---7
  //    |   |    |   |   |
  //    0---1    |   4---5
  const size_t cube_indices[6][4] = {
    {0, 7, 3, 1},
    {0, 7, 1, 5},
    {0, 7, 5, 4},
    {0, 7, 4, 6},
    {0, 7, 6, 2},
    {0, 7, 2, 3},
  };
  // pick-up four vertices to construct a tetrahedra
  //   from the eight vertices of the given lattice
  vertex_t tetrahedron[4] = {0};
  for(size_t n = 0; n < 4; n++){
    const size_t cube_index = cube_indices[tetrahedron_index][n];
    // indices in xyz, see above schematic
    const size_t i = (0 == cube_index || 2 == cube_index || 4 == cube_index || 6 == cube_index) ? 0 : 1;
    const size_t j = (0 == cube_index || 1 == cube_index || 4 == cube_index || 5 == cube_index) ? 0 : 1;
    const size_t k = (0 == cube_index || 1 == cube_index || 2 == cube_index || 3 == cube_index) ? 0 : 1;
    // convert from a general to the cartesian coordinate systems
    const contour3d_vector_t cartesian = {
      grids[0][i],
      grids[1][j],
      grids[2][k],
    };
    coordinate_converter(
        &cartesian,
        &tetrahedron[n].position
    );
    tetrahedron[n].cube_index = cube_index;
    tetrahedron[n].value = array[k * 4 + j * 2 + i];
  }
  // find one triangle / two triangles inside the given tetrahedron
  // prepare 4-bit mask
  uint_fast8_t mask = 0
    | (tetrahedron[0].value > threshold) << 0
    | (tetrahedron[1].value > threshold) << 1
    | (tetrahedron[2].value > threshold) << 2
    | (tetrahedron[3].value > threshold) << 3;
  // normal goes to the opposite direction
  //   when the top bit is 1
  // 1: 0001 <-> 14: 1110
  // 2: 0010 <-> 13: 1101
  // 3: 0011 <-> 12: 1100
  // 4: 0100 <-> 11: 1011
  // 5: 0101 <-> 10: 1010
  // 6: 0110 <->  9: 1001
  // 7: 0111 <->  8: 1000
  const bool reverse = mask >= (1 << 3);
  if(reverse){
    mask = 0xF - mask;
  }
  if(0 == mask){
    // no triangle
    return 0;
  }
  // check triangles inside a tetrahedron
  //  2-1
  //  |/
  //  0
  // and the top corner is indexed as 3
  // NOTE: make sure normal vector goes from bit-0-zone to bit-1-zone
  //   following the right-hand rule
  if(/* 0001 */ 1 == mask){
    kernel(
        reverse, threshold,
        (size_t []){0, 0, 0, }, (size_t []){1, 3, 2, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0010 */ 2 == mask){
    kernel(
        reverse, threshold,
        (size_t []){1, 1, 1, }, (size_t []){0, 2, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0011 */ 3 == mask){
    kernel(
        reverse, threshold,
        (size_t []){0, 1, 0, }, (size_t []){2, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []){0, 1, 1, }, (size_t []){2, 2, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0100 */ 4 == mask){
    kernel(
        reverse, threshold,
        (size_t []){2, 2, 2, }, (size_t []){0, 3, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0101 */ 5 == mask){
    kernel(
        reverse, threshold,
        (size_t []){2, 0, 2, }, (size_t []){1, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []){2, 0, 0, }, (size_t []){1, 1, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0110 */ 6 == mask){
    kernel(
        reverse, threshold,
        (size_t []){0, 2, 1, }, (size_t []){2, 3, 3, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
    kernel(
        reverse, threshold,
        (size_t []){0, 1, 0, }, (size_t []){2, 3, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else if(/* 0111 */ 7 == mask){
    kernel(
        reverse, threshold,
        (size_t []){3, 3, 3, }, (size_t []){0, 2, 1, },
        tetrahedron,
        triangles + (*num_triangles)++
    );
  }else{
    fprintf(stderr, "(%s:%d): mask = %hhu: should not reach here\n", __FILE__, __LINE__, mask);
    fflush(stderr);
    return 1;
  }
  return 0;
}

static inline size_t ravel(
    const size_t glsizes[2],
    const size_t i,
    const size_t j,
    const size_t k
){
  return (k * glsizes[1] + j) * glsizes[0] + i;
}

int contour3d_contour_triangulate_slice (
    const size_t glsizes[2],
    double * const grids[3],
    int (* const coordinate_converter)(const contour3d_vector_t * orthogonal, contour3d_vector_t * cartesian),
    const double * array,
    const double threshold,
    lattice_t * lattices
) {
  const size_t imax = glsizes[0] - 1;
  const size_t jmax = glsizes[1] - 1;
  for(/* each y */ size_t j = 0; j < jmax; j++){
    for(/* each x */ size_t i = 0; i < imax; i++){
      lattice_t * lattice = lattices + j * imax + i;
      // pack information
      const double localgrids[3][2] = {
        {grids[0][i], grids[0][i + 1]},
        {grids[1][j], grids[1][j + 1]},
        {grids[2][0], grids[2][    1]},
      };
      const double scalars[8] = {
        array[ravel(glsizes, i, j    , 0)], array[ravel(glsizes, i + 1, j    , 0)],
        array[ravel(glsizes, i, j + 1, 0)], array[ravel(glsizes, i + 1, j + 1, 0)],
        array[ravel(glsizes, i, j    , 1)], array[ravel(glsizes, i + 1, j    , 1)],
        array[ravel(glsizes, i, j + 1, 1)], array[ravel(glsizes, i + 1, j + 1, 1)],
      };
      // there are six tetrahedra in one lattice
      // for each tetrahedron, execute tessellation
      size_t * num_triangles = &lattice->num_triangles;
      triangle_t * triangles = lattice->triangles;
      // just init number of triangles for simplicity,
      //   i.e., "triangles" may store the previous information
      *num_triangles = 0;
      for(/* each tetrahedron */ size_t n = 0; n < 6; n++){
        if(0 != triangulate_tetrahedron(
              localgrids,
              coordinate_converter,
              scalars,
              threshold,
              n,
              num_triangles,
              triangles
        )) return 1;
      }
    }
  }
  return 0;
}

