// CLAS12 RG-E Analyser.
// Copyright (C) 2022-2023 Bruno Benkel
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You can see a copy of the GNU Lesser Public License under the LICENSE file.

#ifndef RGE_MATHUTILS
#define RGE_MATHUTILS

#include "math.h"
#include "rge_err_handler.h"

/** Transform an angle a from degrees to radians. */
int rge_to_rad(double src, double *dest);

/** Compute a 2D vector's magntitude from its x and y components. */
double rge_calc_magnitude(double x, double y);

/** Compute a 3D vector's magnitude from its x, y, and z components. */
double rge_calc_magnitude(double x, double y, double z);

/** Compute the angle between two vectors (x1,y1,z1) and (x2,y2,z2). */
double rge_calc_angle(
        double x1, double y1, double z1, double x2, double y2, double z2
);

/**
 * Rotate a 3D vector around the y axis by an angle theta.
 *
 * @param x:  x coordinate of the vector. This variable *will be updated*.
 * @param z:  z coordinate of the vector. This variable *will be updated*.
 * @param th: theta angle (in radians) for the rotation.
 */
void rge_rotate_y(double *x, double *z, double th);

/**
 * Rotate a 3D vector around the z axis by an angle theta.
 *
 * @param x:  x coordinate of the vector. This variable *will be updated*.
 * @param y:  y coordinate of the vector. This variable *will be updated*.
 * @param th: theta angle (in radians) for the rotation.
 */
void rge_rotate_z(double *x, double *y, double th);

#endif
