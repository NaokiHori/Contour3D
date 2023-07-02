#if !defined(CONTOUR3D_OUTPUT_H)
#define CONTOUR3D_OUTPUT_H

#include <stddef.h>
#include "sdecomp.h"
#include "./struct.h"

extern int contour3d_output_image(
    const sdecomp_info_t * sdecomp_info,
    const char fname[],
    const size_t width,
    const size_t height,
    pixel_t * canvas
);

#endif // CONTOUR3D_OUTPUT_H
