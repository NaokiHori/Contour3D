#if !defined(CONTOUR3D_CONTOUR_INTERNAL_H)
#define CONTOUR3D_CONTOUR_INTERNAL_H

#include "sdecomp.h"
#include "contour3d.h"
#include "../struct.h"

extern int contour3d_process_contour_obj (
    const sdecomp_info_t * const sdecomp_info,
    const camera_t * const camera,
    const contour3d_vector_t * const light,
    const screen_t * const screen,
    const contour3d_contour_obj_t * const contour_obj,
    pixel_t * const canvas
);

extern int contour3d_contour_extend_domain (
    const sdecomp_info_t * const sdecomp_info,
    const contour3d_contour_obj_t * const contour_obj,
    size_t mysizes_ext[3],
    size_t offsets_ext[3],
    double ** array_ext
);

extern int contour3d_contour_triangulate_slice (
    const size_t glsizes[2],
    double * const grids[3],
    contour3d_vector_t (* const coordinate_converter) (
      const contour3d_vector_t orthogonal
    ),
    const double * const array,
    const double threshold,
    lattice_t * const lattices
);

extern int contour3d_contour_compute_vertex_normals (
    const size_t glsizes[2],
    lattice_t * const slices[3]
);

extern int contour3d_contour_render_triangle (
    const camera_t * const camera,
    const contour3d_vector_t * const light,
    const screen_t * const screen,
    const contour3d_color_t * const fg_color,
    const triangle_t * const triangle,
    pixel_t * const canvas
);

#endif // CONTOUR3D_CONTOUR_INTERNAL_H
