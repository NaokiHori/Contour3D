#if !defined(CONTOUR3D_CONTOUR_INTERNAL_H)
#define CONTOUR3D_CONTOUR_INTERNAL_H

// simple include guard
#if !defined(CONTOUR3D_CONTOUR_INTERNAL)
#error "do not include this file externally"
#endif

extern int contour3d_process_contour_obj(
    const sdecomp_info_t * sdecomp_info,
    const camera_t * camera,
    const contour3d_vector_t * light,
    const screen_t * screen,
    const contour3d_contour_obj_t * contour_obj,
    pixel_t * canvas
);

extern int contour3d_contour_extend_domain(
    const sdecomp_info_t * sdecomp_info,
    const contour3d_contour_obj_t * contour_obj,
    size_t mysizes_ext[3],
    size_t offsets_ext[3],
    double ** array_ext
);

extern int contour3d_contour_triangulate_slice(
    const size_t glsizes[2],
    double * const grids[3],
    int (* const coordinate_converter)(const contour3d_vector_t * orthogonal, contour3d_vector_t * cartesian),
    const double * array,
    const double threshold,
    lattice_t * lattices
);

extern int contour3d_contour_compute_vertex_normals(
    const size_t glsizes[2],
    lattice_t * slices[3]
);

extern int contour3d_contour_render_triangle(
    const camera_t * camera,
    const contour3d_vector_t * light,
    const screen_t * screen,
    const uint8_t fg_color[3],
    const triangle_t * triangle,
    pixel_t * canvas
);

#endif // CONTOUR3D_CONTOUR_INTERNAL_H
