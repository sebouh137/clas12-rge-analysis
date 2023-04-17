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

#include "../lib/rge_math_utils.h"

// --+ library +----------------------------------------------------------------
int rge_to_rad(double src, double *dest) {
    // Check angle validity.
    if (-180 > src || src > 180) {
        rge_errno = RGEERR_ANGLEOUTOFRANGE;
        return 1;
    }

    // Transform.
    *dest = src * (M_PI / 180.0);
    return 0;
}

double rge_calc_magnitude(double x, double y) {
    return sqrt(x*x + y*y);
}

double rge_calc_magnitude(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

double rge_calc_angle(
        double x1, double y1, double z1, double x2, double y2, double z2
) {
    return acos((x1*x2 + y1*y2 + z1*z2)/(rge_calc_magnitude(x1,y1,z1) *
            rge_calc_magnitude(x2,y2,z2)));
}

void rge_rotate_y(double *x, double *z, double th) {
    double x_prev = *x;
    double z_prev = *z;
    *x =  x_prev*cos(th) + z_prev*sin(th);
    *z = -x_prev*sin(th) + z_prev*cos(th);
}

void rge_rotate_z(double *x, double *y, double th) {
    double x_prev = *x;
    double y_prev = *y;
    *x = x_prev*cos(th) - y_prev*sin(th);
    *y = x_prev*sin(th) + y_prev*cos(th);
}
