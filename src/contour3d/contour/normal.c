#include <math.h>
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "../internal.h"
#define CONTOUR3D_CONTOUR_INTERNAL
#include "internal.h"

static inline int average(
    const lattice_t * lattice,
    const size_t cube_index,
    vector_t * normal_sum
){
  // check three vertices of the triangles inside the given lattice
  // if one of the vertices matches with the "cube_index",
  // this triangle shares the same edge and thus I need the face normal
  // to compute the vertex normal
  const size_t num_triangles = lattice->num_triangles;
  const triangle_t * triangles = lattice->triangles;
  for(/* each triangle */ size_t n = 0; n < num_triangles; n++){
    const triangle_t * triangle = triangles + n;
    const size_t * cube_indices = triangle->cube_indices;
    // check three vertices
    if(
           cube_index == cube_indices[0]
        || cube_index == cube_indices[1]
        || cube_index == cube_indices[2]
    ){
      // NOTE: sum of the weight is not stored,
      //   since the resulting vector will be
      //   normalised later anyhow
      // NOTE: simple average for now to be robust
      //   harmonic average might be more consistent
      const double weight = triangle->area;
      (*normal_sum)[0] += weight * triangle->face_normal[0];
      (*normal_sum)[1] += weight * triangle->face_normal[1];
      (*normal_sum)[2] += weight * triangle->face_normal[2];
    }
  }
  return 0;
}

int contour3d_contour_compute_vertex_normals(
    const size_t glsizes[2],
    lattice_t * slices[3]
){
  const size_t imax = glsizes[0] - 1;
  const size_t jmax = glsizes[1] - 1;
  // NOTE: edge lattices (i = 0, imax, j = 0, jmax) are discarded
  //   since smoothening needs neighbouring cells
  for(/* each y */ size_t j = 1; j < jmax - 1; j++){
    for(/* each x */ size_t i = 1; i < imax - 1; i++){
      const size_t index = j * imax + i;
      lattice_t * lattice = slices[1] + index;
      const size_t num_triangles = lattice->num_triangles;
      triangle_t * triangles = lattice->triangles;
      for(/* each triangle */ size_t n = 0; n < num_triangles; n++){
        triangle_t * triangle = triangles + n;
        for(/* each vertex */ size_t m = 0; m < 3; m++){
          // this vertex sits on this cube edge:
          const size_t cube_index = triangle->cube_indices[m];
          // this is to be updated
          vector_t * vertex_normal = &triangle->vertex_normals[m];
          (*vertex_normal)[0] = 0.;
          (*vertex_normal)[1] = 0.;
          (*vertex_normal)[2] = 0.;
          // visit neighbouring lattices which share the same cube edge
          //   and average all face-normals of the triangles
          //  0 -  3 : x edges
          //  4 -  7 : y edges
          //  8 - 11 : z edges
          // 12 - 13 : diagonal x face
          // 14 - 15 : diagonal y face
          // 16 - 17 : diagonal z face
          // 18      : cube diagonal
          if( 0 == cube_index){
            average(slices[0] + (j - 1) * imax + (i    ),  3, vertex_normal);
            average(slices[0] + (j    ) * imax + (i    ),  2, vertex_normal);
            average(slices[1] + (j - 1) * imax + (i    ),  1, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  0, vertex_normal);
          }else if( 1 == cube_index){
            average(slices[0] + (j    ) * imax + (i    ),  3, vertex_normal);
            average(slices[0] + (j + 1) * imax + (i    ),  2, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  1, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i    ),  0, vertex_normal);
          }else if( 2 == cube_index){
            average(slices[1] + (j - 1) * imax + (i    ),  3, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  2, vertex_normal);
            average(slices[2] + (j - 1) * imax + (i    ),  1, vertex_normal);
            average(slices[2] + (j    ) * imax + (i    ),  0, vertex_normal);
          }else if( 3 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ),  3, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i    ),  2, vertex_normal);
            average(slices[2] + (j    ) * imax + (i    ),  1, vertex_normal);
            average(slices[2] + (j + 1) * imax + (i    ),  0, vertex_normal);
          }else if( 4 == cube_index){
            average(slices[0] + (j    ) * imax + (i - 1),  7, vertex_normal);
            average(slices[0] + (j    ) * imax + (i    ),  6, vertex_normal);
            average(slices[1] + (j    ) * imax + (i - 1),  5, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  4, vertex_normal);
          }else if( 5 == cube_index){
            average(slices[0] + (j    ) * imax + (i    ),  7, vertex_normal);
            average(slices[0] + (j    ) * imax + (i + 1),  6, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  5, vertex_normal);
            average(slices[1] + (j    ) * imax + (i + 1),  4, vertex_normal);
          }else if( 6 == cube_index){
            average(slices[1] + (j    ) * imax + (i - 1),  7, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  6, vertex_normal);
            average(slices[2] + (j    ) * imax + (i - 1),  5, vertex_normal);
            average(slices[2] + (j    ) * imax + (i    ),  4, vertex_normal);
          }else if( 7 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ),  7, vertex_normal);
            average(slices[1] + (j    ) * imax + (i + 1),  6, vertex_normal);
            average(slices[2] + (j    ) * imax + (i    ),  5, vertex_normal);
            average(slices[2] + (j    ) * imax + (i + 1),  4, vertex_normal);
          }else if( 8 == cube_index){
            average(slices[1] + (j - 1) * imax + (i - 1), 11, vertex_normal);
            average(slices[1] + (j - 1) * imax + (i    ), 10, vertex_normal);
            average(slices[1] + (j    ) * imax + (i - 1),  9, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  8, vertex_normal);
          }else if( 9 == cube_index){
            average(slices[1] + (j - 1) * imax + (i    ), 11, vertex_normal);
            average(slices[1] + (j - 1) * imax + (i + 1), 10, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ),  9, vertex_normal);
            average(slices[1] + (j    ) * imax + (i + 1),  8, vertex_normal);
          }else if(10 == cube_index){
            average(slices[1] + (j    ) * imax + (i - 1), 11, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ), 10, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i - 1),  9, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i    ),  8, vertex_normal);
          }else if(11 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ), 11, vertex_normal);
            average(slices[1] + (j    ) * imax + (i + 1), 10, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i    ),  9, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i + 1),  8, vertex_normal);
          }else if(12 == cube_index){
            average(slices[1] + (j    ) * imax + (i - 1), 13, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ), 12, vertex_normal);
          }else if(13 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ), 13, vertex_normal);
            average(slices[1] + (j    ) * imax + (i + 1), 12, vertex_normal);
          }else if(14 == cube_index){
            average(slices[1] + (j - 1) * imax + (i    ), 15, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ), 14, vertex_normal);
          }else if(15 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ), 15, vertex_normal);
            average(slices[1] + (j + 1) * imax + (i    ), 14, vertex_normal);
          }else if(16 == cube_index){
            average(slices[0] + (j    ) * imax + (i    ), 17, vertex_normal);
            average(slices[1] + (j    ) * imax + (i    ), 16, vertex_normal);
          }else if(17 == cube_index){
            average(slices[1] + (j    ) * imax + (i    ), 17, vertex_normal);
            average(slices[2] + (j    ) * imax + (i    ), 16, vertex_normal);
          }else{
            average(slices[1] + (j    ) * imax + (i    ), 18, vertex_normal);
          }
          // normalise
          const double norminv = 1. / sqrt(
              + pow((*vertex_normal)[0], 2.)
              + pow((*vertex_normal)[1], 2.)
              + pow((*vertex_normal)[2], 2.)
          );
          (*vertex_normal)[0] *= norminv;
          (*vertex_normal)[1] *= norminv;
          (*vertex_normal)[2] *= norminv;
        }
      }
    }
  }
  return 0;
}

