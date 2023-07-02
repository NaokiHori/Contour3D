##########
Contour 3D
##########

|CI|_ |License|_ |LastCommit|_

.. |CI| image:: https://github.com/NaokiHori/Contour3D/actions/workflows/ci.yml/badge.svg
.. _CI: https://github.com/NaokiHori/Contour3D/actions/workflows/ci.yml

.. |License| image:: https://img.shields.io/github/license/NaokiHori/Contour3D
.. _License: https://opensource.org/license/MIT

.. |LastCommit| image:: https://img.shields.io/github/last-commit/NaokiHori/Contour3D/main
.. _LastCommit: https://github.com/NaokiHori/Contour3D/commits/main

.. image:: https://github.com/NaokiHori/Contour3D/blob/main/.github/thumbnail.jpg
   :target: https://youtu.be/CbF5Bs9Jf30
   :width: 800

.. contents::
   :depth: 1

********
Overview
********

This library performs 3D contouring by extracting iso-surfaces from three-dimensional arrays and outputting the result as a portable pixmap (``ppm``) image.

The objective is to visualise three-dimensional flow fields simulated on massively parallelised CFD solvers (available at `here <https://github.com/NaokiHori/SimpleNSSolver>`_) without saving the full three-dimensional arrays.
By embedding this library into the existing flow solver, one can easily create movies without worrying about storage limitations.

This library does not have any additional dependencies on X, Qt, VTK, OpenGL, etc., making the initial cost minimal.

******
Caveat
******

In most cases, I recommend using other software options that offer more comprehensive features since the feature set of this library is limited.
This is because the original motivation was to visualise and understand the flow fields intuitively as a movie, without the support of a graphical library.

**********
Dependency
**********

* `C compiler <https://gcc.gnu.org>`_
* `GNU Make <https://www.gnu.org/software/make/>`_
* `MPI <https://www.open-mpi.org>`_
* `Simple Decomp Library <https://github.com/NaokiHori/SimpleDecomp>`_ (source files are included)

***********
Quick start
***********

#. Prepare workplace

   .. code-block:: console

      $ mkdir -p /path/to/your/directory
      $ cd       /path/to/your/directory

#. Get source

   For example:

   .. code-block:: console

      $ git clone --recurse-submodules https://github.com/NaokiHori/Contour3D
      $ cd Contour3D

#. Build

   .. code-block:: console

      $ make clean
      $ make all

#. Execute

   .. code-block:: console

      $ mpirun -n 2 --oversubscribe ./a.out

   This may take a few seconds.
   Change the number of processes depending on your machine spec.
#. Check output

   Find ``output.ppm``, which is the result of the 3D contouring:

   .. image:: https://github.com/NaokiHori/Contour3D/blob/artifact/output.jpg
      :width: 600

******
Detail
******

See ``src/main.c`` to investigate how the contours, the camera, the light, and the screen are configured.

Basically this library does the following things:

#. Array extension

   Edges of the given three-dimensional array are communicated among all processes to avoid gaps.

#. Tessellation

   From the extended three-dimensional array, each process extracts triangular elements and their surface normals using the marching-tetrahedra algorithm.

#. Smoothing

   Vertex normals are computed by averaging the surface normals of the neighbouring triangles to obtain a smoother result.

#. Rendering

   Decide the color of each element based on the direction of the light and the local normal vector interpolated on each barycentric coordinate.

#. Reduction

   Among all processes, find the nearest triangular element to the screen and output the result to an image.

The steps 1-4 are repeated if multiple arrays and/or thresholds are given.

See ``src/contour3d/main.c`` to check the overall procedures.

*********
Reference
*********

* `Ray Tracing in One Weekend <https://raytracing.github.io/books/RayTracingInOneWeekend.html>`_

* `Marching tetrahedra <https://en.wikipedia.org/wiki/Marching_tetrahedra>`_

* `Polygonising a scalar field <http://paulbourke.net/geometry/polygonise/>`_

* `Scratchapixel 3.0 <https://www.scratchapixel.com/index.html>`_

* `Tessellation (computer graphics) <https://en.wikipedia.org/wiki/Tessellation_(computer_graphics)>`_

* `Barycentric coordinate system <https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Applications_2>`_

***************
Acknowledgement
***************

I would like to thank `Prof. Roberto Verzicco <http://people.uniroma2.it/roberto.verzicco/>`_ for a stimulating lecture in the JMBC course *Multiphase Flow and Phase Transitions*.

