#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "sdecomp.h"
#include "contour3d.h"

static const double pi = 3.1415926535897932;

static int rodrigues(const double n[3], const double t, double b[3]){
  // Rodrigues' rotation formula:
  //   rotate a vector "b" by the angle "t" (in radian) around a vector "n"
  // since the length of the rotation vector may not be unity,
  //   I normalise it first
  const double norminv = 1. / sqrt(
      + pow(n[0], 2.)
      + pow(n[1], 2.)
      + pow(n[2], 2.)
  );
  const double ntmp[3] = {
    n[0] * norminv,
    n[1] * norminv,
    n[2] * norminv,
  };
  // prepare 3x3 rotation matrix
  const double cost = cos(t);
  const double sint = sin(t);
  const double a00 = ntmp[0] * ntmp[0] * (1. - cost) +           cost;
  const double a01 = ntmp[0] * ntmp[1] * (1. - cost) - ntmp[2] * sint;
  const double a02 = ntmp[0] * ntmp[2] * (1. - cost) + ntmp[1] * sint;
  const double a10 = ntmp[1] * ntmp[0] * (1. - cost) + ntmp[2] * sint;
  const double a11 = ntmp[1] * ntmp[1] * (1. - cost) +           cost;
  const double a12 = ntmp[1] * ntmp[2] * (1. - cost) - ntmp[0] * sint;
  const double a20 = ntmp[2] * ntmp[0] * (1. - cost) - ntmp[1] * sint;
  const double a21 = ntmp[2] * ntmp[1] * (1. - cost) + ntmp[0] * sint;
  const double a22 = ntmp[2] * ntmp[2] * (1. - cost) +           cost;
  const double btmp[3] = {
    + a00 * b[0] + a01 * b[1] + a02 * b[2],
    + a10 * b[0] + a11 * b[1] + a12 * b[2],
    + a20 * b[0] + a21 * b[1] + a22 * b[2],
  };
  b[0] = btmp[0];
  b[1] = btmp[1];
  b[2] = btmp[2];
  return 0;
}

static int configure_screen(double screen_center[3], double screen_local[2][3]){
  // check orthogonality of the screen basis vectors
  {
    const double dot =
      + screen_local[0][0] * screen_local[1][0]
      + screen_local[0][1] * screen_local[1][1]
      + screen_local[0][2] * screen_local[1][2];
    if(fabs(dot) > 1.e-8){
      printf("screen horizontal and vertical vectors are not orthogonal (inner product: % .1e)\n", dot);
      return 1;
    }
  }
  // translate in the z direction
  screen_center[2] += 7.5;
  // rotate around the z axis
  const double angle0 = + pi / 6.;
  rodrigues((double [3]){0., 0., 1.}, angle0, screen_center);
  rodrigues((double [3]){0., 0., 1.}, angle0, screen_local[0]);
  rodrigues((double [3]){0., 0., 1.}, angle0, screen_local[1]);
  // rotate around another vector
  const double angle1 = + 5. * pi / 12.;
  rodrigues((double [3]){cos(angle0), sin(angle0), 0.}, angle1, screen_center);
  rodrigues((double [3]){cos(angle0), sin(angle0), 0.}, angle1, screen_local[0]);
  rodrigues((double [3]){cos(angle0), sin(angle0), 0.}, angle1, screen_local[1]);
  return 0;
}

static int init_field(const sdecomp_info_t * sdecomp_info, const sdecomp_pencil_t pencil, size_t glsizes[3], double * grids[3], double ** array){
  // create a sample grid and array to be rendered
  // NOTE: the detail of this function is not important,
  //   since this is just to create a "nice" 3d array
  // set resolution
  glsizes[0] = 128;
  glsizes[1] = 128;
  glsizes[2] = 128;
  // domain lengths
  const double lengths[3] = {1., 1., 1., };
  // rectilinear grids in three directions
  grids[0] = calloc(glsizes[0], sizeof(double));
  grids[1] = calloc(glsizes[1], sizeof(double));
  grids[2] = calloc(glsizes[2], sizeof(double));
  // x: Chebyshev-Gauss, [-0.5 : +0.5]
  for(size_t n = 0; n < glsizes[0]; n++){
    grids[0][n] = -0.5 * lengths[0] + lengths[0] * 0.5 * (1. + cos(pi - pi * (n + 0.5) / glsizes[0]));
  }
  // y: uniform, [-0.5 : +0.5]
  for(size_t n = 0; n < glsizes[1]; n++){
    grids[1][n] = -0.5 * lengths[1] + lengths[1] / glsizes[1] * 0.5 * (2 * n + 1);
  }
  // z: uniform, [-0.5 : +0.5]
  for(size_t n = 0; n < glsizes[2]; n++){
    grids[2][n] = -0.5 * lengths[2] + lengths[2] / glsizes[2] * 0.5 * (2 * n + 1);
  }
  // prepare a sample 3D array, which is decomposed into several processes
  // number of local grid points and offsets
  size_t mysizes[3] = {0};
  size_t offsets[3] = {0};
  for(size_t dim = 0; dim < 3; dim++){
    if(0 != sdecomp.get_pencil_mysize(sdecomp_info, pencil, dim, glsizes[dim], mysizes + dim)) return 1;
    if(0 != sdecomp.get_pencil_offset(sdecomp_info, pencil, dim, glsizes[dim], offsets + dim)) return 1;
  }
  // allocate partial array which this process is responsible for
  const size_t nitems = mysizes[0] * mysizes[1] * mysizes[2];
  *array = calloc(nitems, sizeof(double));
  // several sinusoidal functions are superposed
  // consider wavenumbers from 1 to 4 in all directions
  for(size_t kz = 1; kz < 5; kz += 1){
    for(size_t ky = 1; ky < 5; ky += 1){
      for(size_t kx = 1; kx < 5; kx += 1){
        // generate random phases in all directions
        const double xphase = 2. * pi * rand() / RAND_MAX;
        const double yphase = 2. * pi * rand() / RAND_MAX;
        const double zphase = 2. * pi * rand() / RAND_MAX;
        // for each cartesian point
        for(size_t k = 0; k < mysizes[2]; k++){
          const double z = grids[2][k + offsets[2]];
          for(size_t j = 0; j < mysizes[1]; j++){
            const double y = grids[1][j + offsets[1]];
            for(size_t i = 0; i < mysizes[0]; i++){
              const double x = grids[0][i + offsets[0]];
              const size_t index = (k * mysizes[1] + j) * mysizes[0] + i;
              (*array)[index] += 1.
                * cos(2. * pi / lengths[0] * kx * x + xphase)
                * cos(2. * pi / lengths[1] * ky * y + yphase)
                * cos(2. * pi / lengths[2] * kz * z + zphase);
            }
          }
        }
      }
    }
  }
  // normalise to make [-1:+1]
  double min = + 1. * DBL_MAX;
  double max = - 1. * DBL_MAX;
  for(size_t index = 0; index < mysizes[0] * mysizes[1] * mysizes[2]; index++){
    double value = (*array)[index];
    min = fmin(min, value);
    max = fmax(max, value);
  }
  MPI_Comm comm_cart = MPI_COMM_NULL;
  sdecomp.get_comm_cart(sdecomp_info, &comm_cart);
  MPI_Allreduce(MPI_IN_PLACE, &min, 1, MPI_DOUBLE, MPI_MIN, comm_cart);
  MPI_Allreduce(MPI_IN_PLACE, &max, 1, MPI_DOUBLE, MPI_MAX, comm_cart);
  for(size_t index = 0; index < mysizes[0] * mysizes[1] * mysizes[2]; index++){
    double * value = (*array) + index;
    *value = (*value - min) / (max - min) * 2. - 1.;
  }
  return 0;
}

int main(void){
  // initialise MPI and 3D domain decomposition
  MPI_Init(NULL, NULL);
  sdecomp_info_t * sdecomp_info = NULL;
  if(0 != sdecomp.construct(
        MPI_COMM_WORLD,
        3,
        (size_t [3]){0, 0, 0},
        (bool [3]){true, true, true},
        &sdecomp_info
  )) return 1;
  // screen size (physical length and resolution: number of pixels)
  // NOTE: aspect ratio should be consistent
  const double screen_lengths[2] = {1.6, 1.2};
  const size_t screen_sizes[2] = {1440, 1080};
  // focal point
  const double camera_look_at[3] = {0., 0., 0.};
  // screen position and direction of local basis vectors
  // anyway put at the focal point on the xy plane
  double screen_center[3] = {
    camera_look_at[0],
    camera_look_at[1],
    camera_look_at[2],
  };
  double screen_local[2][3] = {
    /* local x basis */ {screen_lengths[0],                0.,  0.},
    /* local y basis */ {               0., screen_lengths[1],  0.},
  };
  // move screen and adjust the position and rotation manually and nicely
  if(0 != configure_screen(screen_center, screen_local)) return 1;
  // put camera on the line connecting the screen center and the focal point
  // 0: focal, 1: screen center, and thus the factor should be larger than 1
  const double factor = 5.;
  const double camera_position[3] = {
    camera_look_at[0] + (screen_center[0] - camera_look_at[0]) * factor,
    camera_look_at[1] + (screen_center[1] - camera_look_at[1]) * factor,
    camera_look_at[2] + (screen_center[2] - camera_look_at[2]) * factor,
  };
  // light
  const double light_direction[3] = {0., 2., -1.};
  // color of the image background, black is recommended
  // see also: src/contour3d/render.c, where the colour is further adjusted
  const uint8_t bg_color[3] = {0x00, 0x00, 0x00};
  // initialise sample data to be visualised
  // number of total grid points (assigned later)
  size_t glsizes[3] = {0, 0, 0};
  // rectilinear grids in three directions (assigned later)
  // define array on x1pencil
  const sdecomp_pencil_t pencil = SDECOMP_X1PENCIL;
  // see also: convert_coordinate
  double * grids[3] = {NULL};
  // 3d array
  double * array = NULL;
  init_field(sdecomp_info, pencil, glsizes, grids, &array);
  // configure contour objects
  // example: 2 contours having two different thresholds and colours
  // although the same array defined on the same grid is used,
  //   they (both array and grids) can be different
  // see also: include/contour3d.h
  const size_t num_contours = 2;
  contour3d_contour_obj_t contour_objs[2] = {0};
  // 0th contour
  contour_objs[0].pencil     = pencil;
  contour_objs[0].glsizes[0] = glsizes[0];
  contour_objs[0].glsizes[1] = glsizes[1];
  contour_objs[0].glsizes[2] = glsizes[2];
  contour_objs[0].grids[0]   = grids[0];
  contour_objs[0].grids[1]   = grids[1];
  contour_objs[0].grids[2]   = grids[2];
  contour_objs[0].converter  = NULL;
  contour_objs[0].threshold  = -0.25;
  contour_objs[0].color[0]   = 0x00;
  contour_objs[0].color[1]   = 0xFF;
  contour_objs[0].color[2]   = 0xFF;
  contour_objs[0].array      = array;
  // 1st contour
  contour_objs[1].pencil     = pencil;
  contour_objs[1].glsizes[0] = glsizes[0];
  contour_objs[1].glsizes[1] = glsizes[1];
  contour_objs[1].glsizes[2] = glsizes[2];
  contour_objs[1].grids[0]   = grids[0];
  contour_objs[1].grids[1]   = grids[1];
  contour_objs[1].grids[2]   = grids[2];
  contour_objs[1].converter  = NULL;
  contour_objs[1].threshold  = +0.25;
  contour_objs[1].color[0]   = 0xFF;
  contour_objs[1].color[1]   = 0xFF;
  contour_objs[1].color[2]   = 0x00;
  contour_objs[1].array      = array;
  // lines, which are used to draw domain edges
  // in this example draw edges of the external cube
  // see also: include/contour3d.h
  const size_t num_lines = 12;
  contour3d_line_obj_t line_objs[12] = {0};
  for(size_t cnt = 0, j = 0; j < 8; j++){
    const double x1 = 0 == j % 2               ? -0.5 : +0.5;
    const double y1 = 2 == j % 4 || 3 == j % 4 ? -0.5 : +0.5;
    const double z1 = j < 4                    ? -0.5 : +0.5;
    for(size_t i = j + 1; i < 8; i++){
      const double x0 = 0 == i % 2               ? -0.5 : +0.5;
      const double y0 = 2 == i % 4 || 3 == i % 4 ? -0.5 : +0.5;
      const double z0 = i < 4                    ? -0.5 : +0.5;
      size_t difcnt = 0;
      if(x0 != x1) difcnt++;
      if(y0 != y1) difcnt++;
      if(z0 != z1) difcnt++;
      if(1 != difcnt) continue;
      line_objs[cnt].nitems   = 2;
      line_objs[cnt].grids[0] = calloc(2, sizeof(double));
      line_objs[cnt].grids[1] = calloc(2, sizeof(double));
      line_objs[cnt].grids[2] = calloc(2, sizeof(double));
      line_objs[cnt].grids[0][0] = x0;
      line_objs[cnt].grids[0][1] = x1;
      line_objs[cnt].grids[1][0] = y0;
      line_objs[cnt].grids[1][1] = y1;
      line_objs[cnt].grids[2][0] = z0;
      line_objs[cnt].grids[2][1] = z1;
      line_objs[cnt].color[0] = 0xFF;
      line_objs[cnt].color[1] = 0xFF;
      line_objs[cnt].color[2] = 0xFF;
      line_objs[cnt].width    = 2.;
      cnt++;
    }
  }
  // draw 3d contour and output an image
  // see also: include/contour3d.h
  if(0 != contour3d_execute(
        sdecomp_info,
        camera_position,
        camera_look_at,
        light_direction,
        screen_sizes,
        screen_center,
        screen_local,
        bg_color,
        num_contours,
        contour_objs,
        num_lines,
        line_objs,
        "output.ppm"
  )){
    printf("contour3d failed\n");
  }
  // clean-up
  free(grids[0]);
  free(grids[1]);
  free(grids[2]);
  free(array);
  for(size_t index = 0; index < num_lines; index++){
    free(line_objs[index].grids[0]);
    free(line_objs[index].grids[1]);
    free(line_objs[index].grids[2]);
  }
  sdecomp.destruct(sdecomp_info);
  MPI_Finalize();
  return 0;
}

