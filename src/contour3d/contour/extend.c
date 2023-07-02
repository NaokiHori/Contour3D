#include <stdbool.h>
#include <mpi.h>
#include "sdecomp.h"
#include "contour3d.h"
#include "../struct.h"
#include "../memory.h"
#include "../logger.h"
#include "./internal.h"

static int communicate_in_x (
    const sdecomp_info_t * const sdecomp_info,
    const sdecomp_pencil_t pencil,
    const size_t n_add,
    const size_t mysizes_tmp[CONTOUR3D_NDIMS],
    double * const array_tmp
) {
  MPI_Comm comm_cart = MPI_COMM_NULL;
  sdecomp.get_comm_cart(sdecomp_info, &comm_cart);
  // check negative / positive neighbour ranks
  int neighbours[2] = {MPI_PROC_NULL, MPI_PROC_NULL};
  sdecomp.get_neighbours(sdecomp_info, pencil, SDECOMP_XDIR, neighbours);
  // define datatype
  MPI_Datatype dtype = MPI_DATATYPE_NULL;
  MPI_Type_create_hvector(
      mysizes_tmp[1] * mysizes_tmp[2],
      n_add,
      sizeof(double) * mysizes_tmp[0],
      MPI_DOUBLE,
      &dtype
  );
  MPI_Type_commit(&dtype);
  // send to positive, receive from negative
  {
    const size_t soffset = mysizes_tmp[0] - 2 * n_add;
    const size_t roffset =                  0 * n_add;
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[1], 0,
        array_tmp + roffset, 1, dtype, neighbours[0], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // send to negative, receive from positive
  {
    const size_t soffset =                  1 * n_add;
    const size_t roffset = mysizes_tmp[0] - 1 * n_add;
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[0], 0,
        array_tmp + roffset, 1, dtype, neighbours[1], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // clean-up used datatype
  MPI_Type_free(&dtype);
  return 0;
}

static int communicate_in_y (
    const sdecomp_info_t * const sdecomp_info,
    const sdecomp_pencil_t pencil,
    const size_t n_add,
    const size_t mysizes_tmp[CONTOUR3D_NDIMS],
    double * const array_tmp
) {
  MPI_Comm comm_cart = MPI_COMM_NULL;
  sdecomp.get_comm_cart(sdecomp_info, &comm_cart);
  // check negative / positive neighbour ranks
  int neighbours[2] = {MPI_PROC_NULL, MPI_PROC_NULL};
  sdecomp.get_neighbours(sdecomp_info, pencil, SDECOMP_YDIR, neighbours);
  // define datatype
  MPI_Datatype dtype = MPI_DATATYPE_NULL;
  MPI_Type_create_hvector(
      mysizes_tmp[2],
      mysizes_tmp[0] * n_add,
      sizeof(double) * mysizes_tmp[0] * mysizes_tmp[1],
      MPI_DOUBLE,
      &dtype
  );
  MPI_Type_commit(&dtype);
  // send to positive, receive from negative
  {
    const size_t soffset = (mysizes_tmp[1] - 2 * n_add) * mysizes_tmp[0];
    const size_t roffset = (                 0 * n_add) * mysizes_tmp[0];
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[1], 0,
        array_tmp + roffset, 1, dtype, neighbours[0], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // send to negative, receive from positive
  {
    const size_t soffset = (                 1 * n_add) * mysizes_tmp[0];
    const size_t roffset = (mysizes_tmp[1] - 1 * n_add) * mysizes_tmp[0];
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[0], 0,
        array_tmp + roffset, 1, dtype, neighbours[1], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // clean-up used datatype
  MPI_Type_free(&dtype);
  return 0;
}

static int communicate_in_z (
    const sdecomp_info_t * const sdecomp_info,
    const sdecomp_pencil_t pencil,
    const size_t n_add,
    const size_t mysizes_tmp[CONTOUR3D_NDIMS],
    double * const array_tmp
) {
  MPI_Comm comm_cart = MPI_COMM_NULL;
  sdecomp.get_comm_cart(sdecomp_info, &comm_cart);
  // check negative / positive neighbour ranks
  int neighbours[2] = {MPI_PROC_NULL, MPI_PROC_NULL};
  sdecomp.get_neighbours(sdecomp_info, pencil, SDECOMP_ZDIR, neighbours);
  // define datatype
  MPI_Datatype dtype = MPI_DATATYPE_NULL;
  MPI_Type_contiguous(
      mysizes_tmp[0] * mysizes_tmp[1] * n_add,
      MPI_DOUBLE,
      &dtype
  );
  MPI_Type_commit(&dtype);
  // send to positive, receive from negative
  {
    const size_t soffset = (mysizes_tmp[2] - 2 * n_add) * mysizes_tmp[1] * mysizes_tmp[0];
    const size_t roffset = (                 0 * n_add) * mysizes_tmp[1] * mysizes_tmp[0];
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[1], 0,
        array_tmp + roffset, 1, dtype, neighbours[0], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // send to negative, receive from positive
  {
    const size_t soffset = (                 1 * n_add) * mysizes_tmp[1] * mysizes_tmp[0];
    const size_t roffset = (mysizes_tmp[2] - 1 * n_add) * mysizes_tmp[1] * mysizes_tmp[0];
    MPI_Sendrecv(
        array_tmp + soffset, 1, dtype, neighbours[0], 0,
        array_tmp + roffset, 1, dtype, neighbours[1], 0,
        comm_cart, MPI_STATUS_IGNORE
    );
  }
  // clean-up used datatype
  MPI_Type_free(&dtype);
  return 0;
}

// extend the given three-dimensional domain to avoid gaps between processes
int contour3d_contour_extend_domain (
    const sdecomp_info_t * const sdecomp_info,
    const contour3d_contour_obj_t * const contour_obj,
    size_t mysizes_ext[CONTOUR3D_NDIMS],
    size_t offsets_ext[CONTOUR3D_NDIMS],
    double ** const array_ext
) {
  // number of local grid points and offsets of the original array
  size_t mysizes[CONTOUR3D_NDIMS] = {0};
  size_t offsets[CONTOUR3D_NDIMS] = {0};
  for (sdecomp_dir_t dir = 0; dir < CONTOUR3D_NDIMS; dir++) {
    if (0 != sdecomp.get_pencil_mysize(
          sdecomp_info,
          contour_obj->pencil,
          dir,
          contour_obj->glsizes[dir],
          mysizes + dir
    )) {
      logger_error("sdecomp.get_pencil_mysize failed");
      return 1;
    }
    if (0 != sdecomp.get_pencil_offset(
          sdecomp_info,
          contour_obj->pencil,
          dir,
          contour_obj->glsizes[dir],
          offsets + dir
    )) {
      logger_error("sdecomp.get_pencil_offset failed");
      return 1;
    }
  }
  const size_t n_add = 2;
  // first consider a temporary array, where all directions are extended by 2 * n_add grids
  size_t mysizes_tmp[CONTOUR3D_NDIMS] = {0};
  size_t offsets_tmp[CONTOUR3D_NDIMS] = {0};
  for (sdecomp_dir_t dir = 0; dir < CONTOUR3D_NDIMS; dir++) {
    mysizes_tmp[dir] = mysizes[dir] + 2 * n_add;
    offsets_tmp[dir] = offsets[dir];
  }
  // allocate the extended array and pack the original array
  double * const array_tmp = contour3d_memory_alloc(
      mysizes_tmp[0] * mysizes_tmp[1] * mysizes_tmp[2],
      sizeof(double)
  );
  if (NULL == array_tmp) {
    logger_error("failed to allocate temporary array");
    return 1;
  }
  const double * const array = contour_obj->array;
  for (size_t k = 0; k < mysizes[2]; k++) {
    for (size_t j = 0; j < mysizes[1]; j++) {
      for (size_t i = 0; i < mysizes[0]; i++) {
        const size_t index     = ((k +     0) *     mysizes[1] + (j +     0)) *     mysizes[0] + (i +     0);
        const size_t index_tmp = ((k + n_add) * mysizes_tmp[1] + (j + n_add)) * mysizes_tmp[0] + (i + n_add);
        array_tmp[index_tmp] = array[index];
      }
    }
  }
  // exchange edge values
  // NOTE: straightforward but verbose implementation
  communicate_in_x(sdecomp_info, contour_obj->pencil, n_add, mysizes_tmp, array_tmp);
  communicate_in_y(sdecomp_info, contour_obj->pencil, n_add, mysizes_tmp, array_tmp);
  communicate_in_z(sdecomp_info, contour_obj->pencil, n_add, mysizes_tmp, array_tmp);
  // now allocate another array, where the edge cells
  //   of the edge processes are excluded to avoid out-of-bounds access
  int nprocss[CONTOUR3D_NDIMS] = {0};
  int myranks[CONTOUR3D_NDIMS] = {0};
  for (sdecomp_dir_t dir = 0; dir < CONTOUR3D_NDIMS; dir++) {
    sdecomp.get_nprocs(sdecomp_info, contour_obj->pencil, dir, nprocss + dir);
    sdecomp.get_myrank(sdecomp_info, contour_obj->pencil, dir, myranks + dir);
  }
  // flags to tell whether clip the edge or not,
  //   for each dimension, for each direction (negative and positive directions)
  bool clip[CONTOUR3D_NDIMS][2] = {
    {false, false},
    {false, false},
    {false, false},
  };
  for (sdecomp_dir_t dir = 0; dir < CONTOUR3D_NDIMS; dir++) {
    // if I am the most-negative/positive process,
    //   the negative/positive 2 cells are to be clipped respectively
    if (0 == myranks[dir]) {
      clip[dir][0] = true;
    }
    if (nprocss[dir] - 1 == myranks[dir]) {
      clip[dir][1] = true;
    }
  }
  for (sdecomp_dir_t dir = 0; dir < CONTOUR3D_NDIMS; dir++) {
    // anyway assume the same configuration
    mysizes_ext[dir] = mysizes_tmp[dir];
    offsets_ext[dir] = offsets_tmp[dir];
    // reduce if necessary
    if (clip[dir][0]) {
      mysizes_ext[dir] -= n_add;
    } else {
      offsets_ext[dir] -= n_add;
    }
    if (clip[dir][1]) {
      mysizes_ext[dir] -= n_add;
    }
  }
  // allocate the resulting extended array and pack the temporal array
  *array_ext = contour3d_memory_alloc(
      mysizes_ext[0] * mysizes_ext[1] * mysizes_ext[2],
      sizeof(double)
  );
  if (NULL == *array_ext) {
    logger_error("failed to allocate extended array");
    return 1;
  }
  // mimimum indices are n_add or 0, depending on the negative-clipping flags
  const size_t imin = clip[0][0] ? n_add : 0;
  const size_t jmin = clip[1][0] ? n_add : 0;
  const size_t kmin = clip[2][0] ? n_add : 0;
  // maximum (+1) indices are simply determined by the sizes
  const size_t imax = imin + mysizes_ext[0];
  const size_t jmax = jmin + mysizes_ext[1];
  const size_t kmax = kmin + mysizes_ext[2];
  // copy from array_tmp to array_ext
  for (size_t index_ext = 0, k = kmin; k < kmax; k++) {
    for (size_t j = jmin; j < jmax; j++) {
      for (size_t i = imin; i < imax; i++) {
        const size_t index_tmp = (k * mysizes_tmp[1] + j) * mysizes_tmp[0] + i;
        (*array_ext)[index_ext++] = array_tmp[index_tmp];
      }
    }
  }
  // clean-up
  contour3d_memory_free(array_tmp);
  return 0;
}

