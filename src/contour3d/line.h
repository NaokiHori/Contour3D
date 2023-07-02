#if !defined(CONTOUR3D_LINE_H)
#define CONTOUR3D_LINE_H

extern int contour3d_process_line_obj (
    const camera_t * const camera,
    const screen_t * const screen,
    const contour3d_line_obj_t * const line_obj,
    pixel_t * const canvas
);

#endif // CONTOUR3D_LINE_H
