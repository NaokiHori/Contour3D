#include "sdecomp.h"
#include "contour3d.h"
#include "../struct.h"
#include "../memory.h"
#include "../logger.h"
#include "./internal.h"

// three slices are used to compute vertex normals in the middle slice
#define N_SLICES 3

int contour3d_process_contour_obj (
    const sdecomp_info_t * const sdecomp_info,
    const camera_t * const camera,
    const contour3d_vector_t * const light,
    const screen_t * const screen,
    const contour3d_contour_obj_t * const contour_obj,
    pixel_t * const canvas
) {
  // create an extended array for edge treatment
  //   and obtain its local size
  size_t mysizes_ext[CONTOUR3D_NDIMS] = {0};
  size_t offsets_ext[CONTOUR3D_NDIMS] = {0};
  double * array_ext = NULL;
  if (0 != contour3d_contour_extend_domain(
        sdecomp_info,
        contour_obj,
        mysizes_ext,
        offsets_ext,
        &array_ext
  )) {
    logger_error("failed to extend domain");
    return 1;
  }
  // prepare working place to store three slices
  // NOTE: vertices of a lattice coincide with the surrounding scalars,
  //         yielding smaller size by 1
  const size_t slice_sizes[] = {mysizes_ext[0] - 1, mysizes_ext[1] - 1};
  lattice_t * slices[N_SLICES] = {
    contour3d_memory_alloc(slice_sizes[0] * slice_sizes[1], sizeof(lattice_t)),
    contour3d_memory_alloc(slice_sizes[0] * slice_sizes[1], sizeof(lattice_t)),
    contour3d_memory_alloc(slice_sizes[0] * slice_sizes[1], sizeof(lattice_t)),
  };
  for (/* each slice */ size_t n = 0; n < N_SLICES; n++) {
    if (NULL == slices[n]) {
      logger_error("failed to allocate slice %zu", n);
      return 1;
    }
  }
  for (/* each z */ size_t k = 0; k < mysizes_ext[2] - 1; k++) {
    // extract triangles from a slice at k
    if (0 != contour3d_contour_triangulate_slice(
          (size_t [2]) {
            mysizes_ext[0],
            mysizes_ext[1],
          },
          (double * const [N_SLICES]) {
            contour_obj->grids[0] + offsets_ext[0],
            contour_obj->grids[1] + offsets_ext[1],
            contour_obj->grids[2] + offsets_ext[2] + k,
          },
          contour_obj->converter,
          array_ext + k * mysizes_ext[0] * mysizes_ext[1],
          contour_obj->threshold,
          slices[k % N_SLICES]
    )) {
      logger_error("failed to triangulate a slice at k = %zu", k);
      return 1;
    }
    // render only when three slices are available
    if (k < 2) {
      continue;
    }
    // render info at k - 1
    // compute the vertex normals of the triangles in the middle slice
    //   by using three slices
    if (0 != contour3d_contour_compute_vertex_normals(
          (size_t [2]) {
            mysizes_ext[0],
            mysizes_ext[1],
          },
          (lattice_t * [N_SLICES]) {
            slices[(k - 2) % N_SLICES],
            slices[(k - 1) % N_SLICES],
            slices[(k    ) % N_SLICES],
          }
    )) {
      logger_error("failed to find vertex normals at k = %zu", k - 1);
      return 1;
    }
    // NOTE: edge lattices (i, j = 0, mysize_ext - 1) are clipped
    //   since neighbouring lattices are necessary to average
    for (/* each y */ size_t j = 1; j < slice_sizes[1] - 1; j++) {
      for (/* each x */ size_t i = 1; i < slice_sizes[0] - 1; i++) {
        const lattice_t * const lattice = slices[(k - 1) % N_SLICES] + j * slice_sizes[0] + i;
        const size_t num_triangles = lattice->num_triangles;
        const triangle_t * const triangles = lattice->triangles;
        for (/* each triangle */ size_t index_triangle = 0; index_triangle < num_triangles; index_triangle++) {
          const triangle_t * const triangle = triangles + index_triangle;
          if (0 != contour3d_contour_render_triangle(
                camera,
                light,
                screen,
                &contour_obj->color,
                triangle,
                canvas
          )) {
            logger_error("failed to render triangle at k = %zu", k - 1);
            return 1;
          }
        }
      }
    }
  }
  for (/* each slice */ size_t n = 0; n < N_SLICES; n++) {
    contour3d_memory_free(slices[n]);
  }
  contour3d_memory_free(array_ext);
  return 0;
}

