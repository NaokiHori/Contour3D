#include <stdio.h>
#include <errno.h>
#include <mpi.h>
#define CONTOUR3D_INTERNAL
#include "internal.h"

static void reduction_func(void * invec, void * inoutvec, int * len, MPI_Datatype * datatype){
  if(MPI_DATATYPE_NULL == *datatype){
    // to suppress -Wunused-parameter warning
    return;
  }
  const pixel_t * invec_ = invec;
  pixel_t * inoutvec_ = inoutvec;
  for(int index = 0; index < *len; index++){
    // take out the nearest pixel
    if(invec_[index].depth > inoutvec_[index].depth){
      inoutvec_[index].depth    = invec_[index].depth;
      inoutvec_[index].color[0] = invec_[index].color[0];
      inoutvec_[index].color[1] = invec_[index].color[1];
      inoutvec_[index].color[2] = invec_[index].color[2];
    }
  }
}

static int extract_nearest(const int myrank, const size_t width, const size_t height, pixel_t * canvas){
  const size_t nitems = width * height;
  // create a datatype to store pixel_t
  MPI_Datatype type = MPI_DATATYPE_NULL;
  MPI_Type_create_struct(
      2,
      (int []){1, 3},
      (MPI_Aint []){
        offsetof(pixel_t, depth),
        offsetof(pixel_t, color),
      },
      (MPI_Datatype []){
        MPI_DOUBLE,
        MPI_UNSIGNED_CHAR,
      },
      &type
  );
  MPI_Type_commit(&type);
  // define original reduction operation to
  //   communicate the nearest pixel information
  MPI_Op op = MPI_OP_NULL;
  MPI_Op_create(reduction_func, 1, &op);
  // gather result to the main process
  const int root = 0;
  const void * sendbuf = root == myrank ? MPI_IN_PLACE : canvas;
  void * recvbuf = canvas;
  MPI_Reduce(sendbuf, recvbuf, nitems, type, op, root, MPI_COMM_WORLD);
  // clean-up
  MPI_Type_free(&type);
  MPI_Op_free(&op);
  return 0;
}

int contour3d_output_image(const sdecomp_info_t * sdecomp_info, const char fname[], const size_t width, const size_t height, pixel_t * canvas){
  int myrank = 0;
  sdecomp.get_comm_rank(sdecomp_info, &myrank);
  // communicate among all processes to obtain the nearest pixel color
  // the result is only held by the main process
  extract_nearest(myrank, width, height, canvas);
  if(0 != myrank) return 0;
  // pack all to a buffer (rgb for each pixel)
  const size_t nitems = width * height;
  const size_t size = 3 * sizeof(uint8_t);
  uint8_t * buffer = contour3d_calloc(nitems, size);
  if(NULL == buffer){
    return 1;
  }
  for(size_t cnt = 0, j = 0; j < height; j++){
    for(size_t i = 0; i < width; i++){
      // flip in y
      const size_t index = (height - j - 1) * width + i;
      const pixel_t * pixel = canvas + index;
      const uint8_t * color = pixel->color;
      buffer[cnt++] = color[0];
      buffer[cnt++] = color[1];
      buffer[cnt++] = color[2];
    }
  }
  // dump a ppm file
  errno = 0;
  FILE * fp = fopen(fname, "w");
  if(NULL == fp){
    perror(fname);
    return 1;
  }
  // header
  errno = 0;
  fprintf(fp, "P6\n%zu %zu\n255\n", width, height);
  if(0 != errno){
    perror(fname);
    fclose(fp);
    return 1;
  }
  // contents
  const size_t retval = fwrite(buffer, size, nitems, fp);
  if(nitems != retval){
    fprintf(stderr, "fwrite failed (%zu expected, %zu returned)\n", nitems, retval);
    fflush(stderr);
    fclose(fp);
    return 1;
  }
  fclose(fp);
  // clean-up
  contour3d_free(buffer);
  return 0;
}

