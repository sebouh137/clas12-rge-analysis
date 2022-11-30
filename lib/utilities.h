// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
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

#ifndef UTILS
#define UTILS

#include <TH2F.h>
#include "constants.h"

double to_deg(double radians);
double calc_magnitude(double x, double y);
double calc_magnitude(double x, double y, double z);
double calc_angle(double x1, double y1, double z1, double x2, double y2,
        double z2);
void rotate_y(double *x, double *z, double th);
void rotate_z(double *x, double *y, double th);
int insert_TH1F(std::map<const char *, TH1 *> *map, const char *k,
        const char *n, const char *xn, int bins, double min, double max);
int insert_TH2F(std::map<const char *, TH1 *> *map, const char *k,
        const char *n, const char *nx, const char *ny, int xbins, double xmin,
        double xmax, int ybins, double ymin, double ymax);

#endif
