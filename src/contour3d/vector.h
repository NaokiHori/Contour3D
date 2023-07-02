#if !defined(CONTOUR3D_VECTOR_H)
#define CONTOUR3D_VECTOR_H

extern contour3d_vector_t contour3d_vector_add (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
);

extern contour3d_vector_t contour3d_vector_sub (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
);

extern contour3d_vector_t contour3d_vector_mul (
    const double w,
    const contour3d_vector_t v0
);

extern double contour3d_vector_inner_product (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
);

extern contour3d_vector_t contour3d_vector_outer_product (
    const contour3d_vector_t v0,
    const contour3d_vector_t v1
);

extern contour3d_vector_t contour3d_vector_normalise (
    const contour3d_vector_t v0
);

#endif // CONTOUR3D_VECTOR_H
