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

#ifndef PID_CONSTANTS
#define PID_CONSTANTS

#include <float.h>
#include <map>

// TODO. Change filename.

/** Data associated to a particular PID. */
typedef struct {
    int charge;
    double mass;
    const char *name;
} pid_constants;

static pid_constants pid_constants_init(int c, double m, const char *);

extern const std::map<int, pid_constants> PID_MAP;

// TODO. Change names!
int get_charge(int pid);
double get_mass(int pid);
const char *get_name(int pid);

#endif
