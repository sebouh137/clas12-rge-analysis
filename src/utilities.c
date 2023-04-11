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

#include "../lib/utilities.h"

/**
 * Return position of value v inside a doubles array b of size s. If v is not
 *     inside b, return -1.
 */
int find_pos(double v, double *b, int size) {
    for (int i = 0; i < size; ++i) if (b[i] < v && v < b[i+1]) return i;
    return -1;
}

/** Transform an angle a from radians to degrees. */
int to_deg(double src, double *dest) {
    // Check angle validity.
    if (-M_PI > src || src > M_PI) {
        rge_errno = RGEERR_ANGLEOUTOFRANGE;
        return 1;
    }

    // Transform.
    *dest = src * (180.0 / M_PI);
    return 0;
}

/** Transform an angle a from degrees to radians. */
int to_rad(double src, double *dest) {
    // Check angle validity.
    if (-180 > src || src > 180) {
        rge_errno = RGEERR_ANGLEOUTOFRANGE;
        return 1;
    }

    // Transform.
    *dest = src * (M_PI / 180.0);
    return 0;
}

/** Compute a 2D vector's magntitude from its x and y components. */
double calc_magnitude(double x, double y) {
    return sqrt(x*x + y*y);
}

/** Compute a 3D vector's magnitude from its x, y, and z components. */
double calc_magnitude(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

/** Compute the angle between two vectors (x1,y1,z1) and (x2,y2,z2). */
double calc_angle(
        double x1, double y1, double z1, double x2, double y2, double z2
) {
    return acos((x1*x2 + y1*y2 + z1*z2)/(calc_magnitude(x1,y1,z1) *
            calc_magnitude(x2,y2,z2)));
}

/**
 * Rotate a 3D vector around the y axis by an angle theta.
 *
 * @param x:  x coordinate of the vector. This variable *will be updated*.
 * @param z:  z coordinate of the vector. This variable *will be updated*.
 * @param th: theta angle (in radians) for the rotation.
 */
void rotate_y(double *x, double *z, double th) {
    double x_prev = *x;
    double z_prev = *z;
    *x =  x_prev*cos(th) + z_prev*sin(th);
    *z = -x_prev*sin(th) + z_prev*cos(th);
}

/**
 * Rotate a 3D vector around the z axis by an angle theta.
 *
 * @param x:  x coordinate of the vector. This variable *will be updated*.
 * @param y:  y coordinate of the vector. This variable *will be updated*.
 * @param th: theta angle (in radians) for the rotation.
 */
void rotate_z(double *x, double *y, double th) {
    double x_prev = *x;
    double y_prev = *y;
    *x = x_prev*cos(th) - y_prev*sin(th);
    *y = x_prev*sin(th) + y_prev*cos(th);
}

/**
 * Insert a 1-dimensional histogram of floats into a map.
 *
 * @param map:  Map onto which we'll insert the histogram.
 * @param k:    Name of the particle (or set of particles) in the histogram.
 * @param n:    Name of the histogram to be inserted.
 * @param xn:   Name of the variable in the histogram's x axis.
 * @param bins: Number of bins in the histogram.
 * @param min:  Minimum value for the x axis of the histogram.
 * @param max:  Maximum value for the x axis of the histogram.
 * @return:      Success code (0).
 */
int insert_TH1F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *xn, int bins, double min, double max
) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH1F(Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min,
            max)));
    return 0;
}

/**
 * Insert a 2-dimensional histogram of floats into a map.
 *
 * @param map:   Map onto which we'll insert the histogram.
 * @param k:     Name of the particle (or set of particles) in the histogram.
 * @param n:     Name of the histogram to be inserted.
 * @param nx:    Name of the variable in the histogram's x axis.
 * @param ny:    Name of the variable in the histogram's y axis.
 * @param xbins: Number of bins in the x axis of the histogram.
 * @param ybins: Number of bins in the y axis of the histogram.
 * @param xmin:  Minimum value for the x axis of the histogram.
 * @param xmax:  Maximum value for the x axis of the histogram.
 * @param ymin:  Minimum value for the y axis of the histogram.
 * @param ymax:  Maximum value for the y axis of the histogram.
 * @return:      Success code (0).
 */
int insert_TH2F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *nx, const char *ny, int xbins, double xmin, double xmax,
        int ybins, double ymin, double ymax
) {
    map->insert(std::pair<const char *, TH1 *>
            (n, new TH2F(Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
                         xbins, xmin, xmax, ybins, ymin, ymax)));
    return 0;
}
