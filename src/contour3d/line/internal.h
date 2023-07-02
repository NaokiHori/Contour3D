#if !defined(CONTOUR3D_LINE_INTERNAL_H)
#define CONTOUR3D_LINE_INTERNAL_H

// simple include guard
#if !defined(CONTOUR3D_LINE_INTERNAL)
#error "do not include this file externally"
#endif

extern int contour3d_process_line_obj(
    const camera_t * camera,
    const screen_t * screen,
    const contour3d_line_obj_t * line_obj,
    pixel_t * canvas
);

#endif // CONTOUR3D_LINE_INTERNAL_H
