#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mpi.h>
#include "./struct.h"
#include "./memory.h"
#include "./logger.h"
#include "./output.h"

static void reduction_func (
    void * const void_invec,
    void * const void_inoutvec,
    int * const len,
    MPI_Datatype * const datatype
) {
  if (MPI_DATATYPE_NULL == *datatype) {
    // to suppress -Wunused-parameter warning
    return;
  }
  const pixel_t * const invec = void_invec;
  pixel_t * const inoutvec = void_inoutvec;
  for (int index = 0; index < *len; index++) {
    const pixel_t * const inpixel = invec + index;
    pixel_t * const inoutpixel = inoutvec + index;
    // take out the nearest pixel
    if (inoutpixel->depth < inpixel->depth) {
      *inoutpixel = *inpixel;
    }
  }
}

static int extract_nearest (
    const int myrank,
    const size_t width,
    const size_t height,
    pixel_t * const canvas
) {
  const size_t nitems = width * height;
  // create a datatype to store pixel_t
  MPI_Datatype color_type = MPI_DATATYPE_NULL;
  MPI_Datatype pixel_type = MPI_DATATYPE_NULL;
  // create a sub-type for contour3d_color_t first
  MPI_Type_create_struct(
      1,
      (int []) {3},
      (MPI_Aint []) {
        offsetof(contour3d_color_t, r),
        offsetof(contour3d_color_t, g),
        offsetof(contour3d_color_t, b),
      },
      (MPI_Datatype []) {
        MPI_UNSIGNED_CHAR,
        MPI_UNSIGNED_CHAR,
        MPI_UNSIGNED_CHAR,
      },
      &color_type
  );
  MPI_Type_commit(&color_type);
  // create a main type for pixel_t
  MPI_Type_create_struct(
      2,
      (int []) {1, 1},
      (MPI_Aint []) {
        offsetof(pixel_t, depth),
        offsetof(pixel_t, color),
      },
      (MPI_Datatype []) {
        MPI_DOUBLE,
        color_type,
      },
      &pixel_type
  );
  MPI_Type_commit(&pixel_type);
  // define original reduction operation to
  //   communicate the nearest pixel information
  MPI_Op op = MPI_OP_NULL;
  MPI_Op_create(reduction_func, 1, &op);
  // gather result to the main process
  const int root = 0;
  const void * const sendbuf = root == myrank ? MPI_IN_PLACE : canvas;
  void * const recvbuf = canvas;
  MPI_Reduce(sendbuf, recvbuf, nitems, pixel_type, op, root, MPI_COMM_WORLD);
  // clean-up
  MPI_Type_free(&pixel_type);
  MPI_Type_free(&color_type);
  MPI_Op_free(&op);
  return 0;
}

int contour3d_output_image (
    const sdecomp_info_t * const sdecomp_info,
    const char fname[],
    const size_t width,
    const size_t height,
    pixel_t * const canvas
) {
  int myrank = 0;
  sdecomp.get_comm_rank(sdecomp_info, &myrank);
  // communicate among all processes to obtain the nearest pixel color
  // the result is only held by the main process
  extract_nearest(myrank, width, height, canvas);
  if (0 != myrank) {
    return 0;
  }
  // pack all to a buffer (rgb for each pixel)
  const size_t nitems = width * height;
  const size_t size = 3 * sizeof(uint8_t);
  uint8_t * const buffer = contour3d_memory_alloc(nitems, size);
  if (NULL == buffer) {
    logger_error("failed to allocate image buffer");
    return 1;
  }
  for (size_t cnt = 0, j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      // flip in y
      const size_t index = (height - j - 1) * width + i;
      const pixel_t * const pixel = canvas + index;
      const contour3d_color_t * const color = &pixel->color;
      buffer[cnt++] = color->r;
      buffer[cnt++] = color->g;
      buffer[cnt++] = color->b;
    }
  }
  // dump a ppm file
  errno = 0;
  FILE * const fp = fopen(fname, "w");
  if (NULL == fp) {
    logger_error("%s: %s", fname, strerror(errno));
    return 1;
  }
  // header
  errno = 0;
  fprintf(fp, "P6\n%zu %zu\n255\n", width, height);
  if (0 != errno) {
    logger_error("%s: %s", fname, strerror(errno));
    fclose(fp);
    return 1;
  }
  // contents
  const size_t retval = fwrite(buffer, size, nitems, fp);
  if (nitems != retval) {
    logger_error("fwrite failed (%zu expected, %zu returned)\n", nitems, retval);
    fclose(fp);
    return 1;
  }
  fclose(fp);
  // clean-up
  contour3d_memory_free(buffer);
  return 0;
}

