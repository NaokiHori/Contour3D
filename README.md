# Contour 3D

[![CI](https://github.com/NaokiHori/Contour3D/actions/workflows/ci.yml/badge.svg)](https://github.com/NaokiHori/Contour3D/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/NaokiHori/Contour3D)](https://opensource.org/license/MIT)
[![LastCommit](https://img.shields.io/github/last-commit/NaokiHori/Contour3D/main)](https://github.com/NaokiHori/Contour3D/commits/main)

[![Thumbnail](https://github.com/NaokiHori/Contour3D/blob/main/thumbnail.jpg)](https://youtu.be/CbF5Bs9Jf30)

## Overview

This library performs 3D contouring by extracting iso-surfaces from three-dimensional arrays and outputting the result as a portable pixmap (`ppm`) image.

The objective is to visualize three-dimensional flow fields simulated on massively parallelized CFD solvers (available [here](https://github.com/NaokiHori/SimpleNSSolver)) without saving the full three-dimensional arrays.
By embedding this library into the existing flow solver, one can easily create movies without worrying about storage limitations.

This library does not have any additional dependencies on `X`, `Qt`, `VTK`, `OpenGL`, etc., making the initial cost minimal.

## Caveat

The motivation for this project is to visualize the (flow) fields quickly and intuitively without the support of graphical libraries.
For beautiful and comprehensive renderings, use other software options that offer more features.

## Dependencies

- [C compiler](https://gcc.gnu.org)
- [GNU Make](https://www.gnu.org/software/make/)
- [MPI](https://www.open-mpi.org)
- [Simple Decomp](https://github.com/NaokiHori/SimpleDecomp) (included as a submodule)

## Quick Start

1. Prepare the workspace

    ```sh
    mkdir -p /path/to/your/directory
    cd       /path/to/your/directory
    ```

2. Get the source

    ```sh
    git clone --recurse-submodules https://github.com/NaokiHori/Contour3D
    cd Contour3D
    ```

    Do not forget to fetch the submodule as well.

3. Build

    ```sh
    make clean
    make all
    ```

4. Execute

    ```sh
    mpirun -n 2 --oversubscribe ./a.out
    ```

    This may take a few seconds.
    Change the number of processes depending on your machine's specifications.

5. Check the output

    Find `output.ppm`, which is the result of the 3D contouring:

    <img src="https://github.com/NaokiHori/Contour3D/blob/artifact/output.jpg" alt="Sample Image" width="80%" />

## Method

See `src/main.c` to investigate how the contours, the camera, the light, and the screen are configured.

This library essentially performs the following steps:

1. Array extension

    The edges of the given three-dimensional array are communicated among all processes to avoid gaps.

2. Tessellation

    From the extended three-dimensional array, each process extracts triangular elements and their surface normals using the marching-tetrahedra algorithm.

3. Smoothing

    Vertex normals are computed by averaging the surface normals of the neighboring triangles to obtain a smoother result.

4. Rendering

    The color of each element is decided based on the direction of the light and the local normal vector interpolated on each barycentric coordinate.

5. Reduction

    Among all processes, the nearest triangular element to the screen is found, and the result is output to an image.

Steps 1-4 are repeated if multiple arrays and/or thresholds are given.

See `src/contour3d/main.c` to check the overall procedures.

## References

- [Ray Tracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html)
- [Marching tetrahedra](https://en.wikipedia.org/wiki/Marching_tetrahedra)
- [Polygonising a scalar field](http://paulbourke.net/geometry/polygonise/)
- [Scratchapixel](https://www.scratchapixel.com/index.html)
- [Tessellation (computer graphics)](https://en.wikipedia.org/wiki/Tessellation_(computer_graphics))
- [Barycentric coordinate system](https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Applications_2)

## Acknowledgement

I would like to thank [Prof. Roberto Verzicco](http://people.uniroma2.it/roberto.verzicco/) for a stimulating lecture in the JMBC course `Multiphase Flow and Phase Transitions`.

