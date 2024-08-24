#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "sdecomp.h"
#include "contour3d.h"

static const double pi = 3.1415926535897932;

// map from orthogonal coordinate components
//   to the Cartesian coordinate components
static contour3d_vector_t converter (
    const contour3d_vector_t orthogonal
) {
  const contour3d_vector_t cartesian = {
    .x = orthogonal.x,
    .y = orthogonal.y,
    .z = orthogonal.z,
  };
  return cartesian;
}

static int init_field (
    const sdecomp_info_t * const sdecomp_info,
    const sdecomp_pencil_t pencil,
    size_t glsizes[3],
    double * grids[3],
    double ** const array
) {
  // create a sample grid and array to be rendered
  // NOTE: the detail of this function is not important,
  //   since this is just to create a "nice" 3d array
  // set resolution
  glsizes[0] = 128;
  glsizes[1] = 128;
  glsizes[2] = 128;
  // domain lengths
  const double lengths[3] = {1., 1., 1.};
  // orthogonal grids in three directions
  grids[0] = calloc(glsizes[0], sizeof(double));
  grids[1] = calloc(glsizes[1], sizeof(double));
  grids[2] = calloc(glsizes[2], sizeof(double));
  // x: Chebyshev-Gauss, [-0.5 : +0.5]
  for (size_t n = 0; n < glsizes[0]; n++) {
    grids[0][n] = -0.5 * lengths[0] + lengths[0] * 0.5 * (1. + cos(pi - pi * (n + 0.5) / glsizes[0]));
  }
  // y: uniform, [-0.5 : +0.5]
  for (size_t n = 0; n < glsizes[1]; n++) {
    grids[1][n] = -0.5 * lengths[1] + lengths[1] / glsizes[1] * 0.5 * (2 * n + 1);
  }
  // z: uniform, [-0.5 : +0.5]
  for (size_t n = 0; n < glsizes[2]; n++) {
    grids[2][n] = -0.5 * lengths[2] + lengths[2] / glsizes[2] * 0.5 * (2 * n + 1);
  }
  // prepare a sample 3D array, which is decomposed into several processes
  // number of local grid points and offsets
  size_t mysizes[3] = {0};
  size_t offsets[3] = {0};
  for (size_t dim = 0; dim < 3; dim++) {
    if (0 != sdecomp.get_pencil_mysize(sdecomp_info, pencil, dim, glsizes[dim], mysizes + dim)) return 1;
    if (0 != sdecomp.get_pencil_offset(sdecomp_info, pencil, dim, glsizes[dim], offsets + dim)) return 1;
  }
  // allocate partial array which this process is responsible for
  const size_t nitems = mysizes[0] * mysizes[1] * mysizes[2];
  *array = calloc(nitems, sizeof(double));
  // several sinusoidal functions are superposed
  // consider wavenumbers from 1 to 4 in all directions
  for (size_t kz = 1; kz < 5; kz += 1) {
    for (size_t ky = 1; ky < 5; ky += 1) {
      for (size_t kx = 1; kx < 5; kx += 1) {
        // generate random phases in all directions
        //   except x, where Dirichlet condition (=0) is imposed
        const double xphase = 0.;
        const double yphase = 2. * pi * rand() / RAND_MAX;
        const double zphase = 2. * pi * rand() / RAND_MAX;
        // for each cartesian point
        for (size_t k = 0; k < mysizes[2]; k++) {
          const double z = grids[2][k + offsets[2]];
          for (size_t j = 0; j < mysizes[1]; j++) {
            const double y = grids[1][j + offsets[1]];
            for (size_t i = 0; i < mysizes[0]; i++) {
              const double x = grids[0][i + offsets[0]];
              double * const elem = *array + (k * mysizes[1] + j) * mysizes[0] + i;
              *elem += 1.
                * sin(2. * pi / lengths[0] * kx * x + xphase)
                * sin(2. * pi / lengths[1] * ky * y + yphase)
                * sin(2. * pi / lengths[2] * kz * z + zphase);
            }
          }
        }
      }
    }
  }
  // normalise to make [-1:+1]
  double min = + 1. * DBL_MAX;
  double max = - 1. * DBL_MAX;
  for (size_t index = 0; index < mysizes[0] * mysizes[1] * mysizes[2]; index++) {
    const double * const value = *array + index;
    min = fmin(min, *value);
    max = fmax(max, *value);
  }
  MPI_Comm comm_cart = MPI_COMM_NULL;
  sdecomp.get_comm_cart(sdecomp_info, &comm_cart);
  MPI_Allreduce(MPI_IN_PLACE, &min, 1, MPI_DOUBLE, MPI_MIN, comm_cart);
  MPI_Allreduce(MPI_IN_PLACE, &max, 1, MPI_DOUBLE, MPI_MAX, comm_cart);
  for (size_t index = 0; index < mysizes[0] * mysizes[1] * mysizes[2]; index++) {
    double * const value = *array + index;
    *value = (*value - min) / (max - min) * 2. - 1.;
  }
  return 0;
}

int main (
    void
) {
  // initialise MPI and 3D domain decomposition
  MPI_Init(NULL, NULL);
  sdecomp_info_t * sdecomp_info = NULL;
  if (0 != sdecomp.construct(
        MPI_COMM_WORLD,
        3,
        (size_t [3]) {0, 0, 0},
        (bool [3]) {true, true, true},
        &sdecomp_info
  )) return 1;
  // camera and screen configurations
  const contour3d_vector_t camera_position = {
    .x = +8.660254037844386e-1,
    .y = -1.500000000000000e+0,
    .z = +1.000000000000000e+0,
  };
  const contour3d_vector_t camera_look_at = {
    .x = +0.000000000000000e+0,
    .y = +0.000000000000000e+0,
    .z = +0.000000000000000e+0,
  };
  const size_t screen_sizes[2] = {
    1440,
    1080,
  };
  const contour3d_vector_t screen_center = {
    .x = +4.330127018922194e-1,
    .y = -7.500000000000003e-1,
    .z = +5.000000000000003e-1,
  };
  const contour3d_vector_t screen_local[2] = {
    {
      .x = +1.154700538379251e+0,
      .y = +6.666666666666665e-1,
      .z = +0.000000000000000e+0,
    },
    {
      .x = -2.500000000000000e-1,
      .y = +4.330127018922195e-1,
      .z = +8.660254037844386e-1,
    },
  };
  // light
  const contour3d_vector_t light_direction = {
    .x = 0.,
    .y = 2.,
    .z = -1.,
  };
  // color of the image background, black is recommended
  // see also: src/contour3d/render.c, where the colour is further adjusted
  const contour3d_color_t bg_color = {0x00, 0x00, 0x00};
  // initialise sample data to be visualised
  // number of total grid points (assigned later)
  size_t glsizes[3] = {0, 0, 0};
  // orthogonal grids in three directions (assigned later)
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
  const contour3d_contour_obj_t contour_objs[] = {
    // 0th contour
    {
      .pencil     = pencil,
      .glsizes[0] = glsizes[0],
      .glsizes[1] = glsizes[1],
      .glsizes[2] = glsizes[2],
      .grids[0]   = grids[0],
      .grids[1]   = grids[1],
      .grids[2]   = grids[2],
      .converter  = converter,
      .threshold  = -0.25,
      .color.r    = 0x00,
      .color.g    = 0xFF,
      .color.b    = 0xFF,
      .array      = array,
    },
    // 1st contour
    {
      .pencil     = pencil,
      .glsizes[0] = glsizes[0],
      .glsizes[1] = glsizes[1],
      .glsizes[2] = glsizes[2],
      .grids[0]   = grids[0],
      .grids[1]   = grids[1],
      .grids[2]   = grids[2],
      .converter  = converter,
      .threshold  = +0.25,
      .color.r    = 0xFF,
      .color.g    = 0xFF,
      .color.b    = 0x00,
      .array      = array,
    },
  };
  const size_t num_contours = sizeof(contour_objs) / sizeof(contour_objs[0]);
  // lines, which are used to draw domain edges
  // in this example draw edges of the external cube
  // see also: include/contour3d.h
  const contour3d_color_t line_color = {.r = 0xFF, .g = 0xFF, .b = 0xFF};
  const double line_width = 4.;
  const contour3d_line_obj_t line_objs[] = {
    // lines in y direction
    {
      .edges = {
        {.x = -0.5, .y = -0.5, .z = -0.5},
        {.x = -0.5, .y = +0.5, .z = -0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = +0.5, .y = -0.5, .z = -0.5},
        {.x = +0.5, .y = +0.5, .z = -0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = -0.5, .y = -0.5, .z = +0.5},
        {.x = -0.5, .y = +0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = +0.5, .y = -0.5, .z = +0.5},
        {.x = +0.5, .y = +0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    // lines in z direction
    {
      .edges = {
        {.x = -0.5, .y = -0.5, .z = -0.5},
        {.x = -0.5, .y = -0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = +0.5, .y = -0.5, .z = -0.5},
        {.x = +0.5, .y = -0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = -0.5, .y = +0.5, .z = -0.5},
        {.x = -0.5, .y = +0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
    {
      .edges = {
        {.x = +0.5, .y = +0.5, .z = -0.5},
        {.x = +0.5, .y = +0.5, .z = +0.5},
      },
      .nitems = glsizes[0],
      .converter = converter,
      .color = line_color,
      .width = line_width,
    },
  };
  const size_t num_lines = sizeof(line_objs) / sizeof(line_objs[0]);
  // draw 3d contour and output an image
  // see also: include/contour3d.h
  if (0 != contour3d_execute(
        sdecomp_info,
        &camera_position,
        &camera_look_at,
        &light_direction,
        screen_sizes,
        &screen_center,
        screen_local,
        &bg_color,
        num_contours,
        contour_objs,
        num_lines,
        line_objs,
        "output.ppm"
  )) {
    fprintf(stderr, "contour3d failed\n");
  }
  // clean-up
  free(grids[0]);
  free(grids[1]);
  free(grids[2]);
  free(array);
  sdecomp.destruct(sdecomp_info);
  MPI_Finalize();
  return 0;
}

