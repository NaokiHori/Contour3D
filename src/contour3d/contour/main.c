#include "sdecomp.h"
#include "contour3d.h"
#define CONTOUR3D_INTERNAL
#include "../internal.h"
#define CONTOUR3D_CONTOUR_INTERNAL
#include "internal.h"

int contour3d_process_contour_obj(
    const sdecomp_info_t * sdecomp_info,
    const camera_t * camera,
    const contour3d_vector_t * light,
    const screen_t * screen,
    const contour3d_contour_obj_t * contour_obj,
    pixel_t * canvas
){
  // create an extended array for edge treatment
  //   and obtain its local size
  size_t mysizes_ext[3] = {0};
  size_t offsets_ext[3] = {0};
  double * array_ext = NULL;
  if(0 != contour3d_contour_extend_domain(
        sdecomp_info,
        contour_obj,
        mysizes_ext,
        offsets_ext,
        &array_ext
  )) return 1;
  // prepare working place to store three slices
  lattice_t * slices[3] = {
    contour3d_calloc((mysizes_ext[0] - 1) * (mysizes_ext[1] - 1), sizeof(lattice_t)),
    contour3d_calloc((mysizes_ext[0] - 1) * (mysizes_ext[1] - 1), sizeof(lattice_t)),
    contour3d_calloc((mysizes_ext[0] - 1) * (mysizes_ext[1] - 1), sizeof(lattice_t)),
  };
  for(/* each slice */ size_t nslice = 0; nslice < 3; nslice++){
    if(NULL == slices[nslice]) return 1;
  }
  for(/* each z */ size_t k = 1; k < mysizes_ext[2] - 2; k++){
    // lower slice
    if(1 == k){
      if(0 != contour3d_contour_triangulate_slice(
            (size_t [2]){mysizes_ext[0], mysizes_ext[1]},
            (double * const [3]){
              contour_obj->grids[0] + offsets_ext[0],
              contour_obj->grids[1] + offsets_ext[1],
              contour_obj->grids[2] + offsets_ext[2] + k - 1,
            },
            contour_obj->converter,
            array_ext + (k - 1) * mysizes_ext[0] * mysizes_ext[1],
            contour_obj->threshold,
            slices[0]
      )) return 1;
    }else{
      // copy slice which has already been computed
      lattice_t * tmp = slices[0];
      slices[0] = slices[1];
      slices[1] = tmp;
    }
    // middle slice
    if(1 == k){
      if(0 != contour3d_contour_triangulate_slice(
            (size_t [2]){mysizes_ext[0], mysizes_ext[1]},
            (double * const [3]){
              contour_obj->grids[0] + offsets_ext[0],
              contour_obj->grids[1] + offsets_ext[1],
              contour_obj->grids[2] + offsets_ext[2] + k + 0,
            },
            contour_obj->converter,
            array_ext + (k + 0) * mysizes_ext[0] * mysizes_ext[1],
            contour_obj->threshold,
            slices[1]
      )) return 1;
    }else{
      // copy slice which has already been computed
      lattice_t * tmp = slices[1];
      slices[1] = slices[2];
      slices[2] = tmp;
    }
    // upper slice
    if(0 != contour3d_contour_triangulate_slice(
          (size_t [2]){mysizes_ext[0], mysizes_ext[1]},
          (double * const [3]){
            contour_obj->grids[0] + offsets_ext[0],
            contour_obj->grids[1] + offsets_ext[1],
            contour_obj->grids[2] + offsets_ext[2] + k + 1,
          },
          contour_obj->converter,
          array_ext + (k + 1) * mysizes_ext[0] * mysizes_ext[1],
          contour_obj->threshold,
          slices[2]
    )) return 1;
    // using the three slices, compute the vertex normals
    //   of the triangles in the middle slice
    if(0 != contour3d_contour_compute_vertex_normals(
          (size_t [2]){mysizes_ext[0], mysizes_ext[1]},
          slices
    )) return 1;
    // render the middle slice
    // NOTE: edge lattices (i, j = 0, mysize_ext - 1) are clipped
    //   since neighbouring lattices are necessary to average
    for(/* each y */ size_t j = 1; j < mysizes_ext[1] - 2; j++){
      for(/* each x */ size_t i = 1; i < mysizes_ext[0] - 2; i++){
        const lattice_t * lattice = slices[1] + j * (mysizes_ext[0] - 1) + i;
        const size_t num_triangles = lattice->num_triangles;
        const triangle_t * triangles = lattice->triangles;
        for(/* each triangle */ size_t n = 0; n < num_triangles; n++){
          const triangle_t * triangle = triangles + n;
          if(0 != contour3d_contour_render_triangle(
                camera,
                light,
                screen,
                contour_obj->color,
                triangle,
                canvas
          )) return 1;
        }
      }
    }
  }
  contour3d_free(slices[0]);
  contour3d_free(slices[1]);
  contour3d_free(slices[2]);
  contour3d_free(array_ext);
  return 0;
}

