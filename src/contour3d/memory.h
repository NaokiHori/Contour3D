#if !defined(CONTOUR3D_MEMORY_H)
#define CONTOUR3D_MEMORY_H

extern void * contour3d_memory_alloc (
    const size_t nitems,
    const size_t size
);

extern int contour3d_memory_free (
    void * ptr
);

extern int contour3d_memory_free_all (
    void
);

#endif // CONTOUR3D_MEMORY_H
