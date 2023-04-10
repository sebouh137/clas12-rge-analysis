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

#ifndef PID_UTILS
#define PID_UTILS

#include <float.h>
#include <map>
#include "../lib/err_handler.h"

/** Data associated to a particular PID. */
typedef struct {
    int charge;
    double mass;
    const char *name;
} pid_constants;

// --+ internal +---------------------------------------------------------------
/**
 * Initialize one pid_constants struct with the given input data.
 *
 * @param c : PID charge.
 * @param m : PID mass.
 * @param n : PID name.
 * @return  : pid_constants instance with input data.
 */
static pid_constants pid_constants_init(int c, double m, const char *n);

/** Return 0 if PID_MAP contains pid, 1 otherwise. */
static int pid_invalid(int pid);

// --+ library +----------------------------------------------------------------
/**
 * Get charge of particle associated to PID. If PID is not found in PID_MAP,
 *     sets rge_errno to ERR_PIDNOTFOUND and returns 1.
 *
 * @param pid    : pid value of the charge to look for.
 * @param charge : pointer to integer where to write charge.
 * @return       : error code.
 */
int rge_get_charge(int pid, int *charge);

/**
 * Get mass of particle associated to pid. If PID is not found in PID_MAP, sets
 *     rge_errno to ERR_PIDNOTFOUND and returns 1.
 *
 * @param pid  : pid value of the mass to look for.
 * @param mass : pointer to double where to write mass.
 * @return     : error code.
 */
int rge_get_mass(int pid, double *mass);

/**
 * Get number of PIDs in PID_MAP that match the given charge.
 *
 * @param charge : charge value of the PIDs to look for.
 * @param size   : pointer to int where to write size of pidlist of charge.
 * @param return : error code.
 */
int rge_get_pidlist_size_by_charge(int charge, unsigned int *size);

/**
 * Fill an array of PIDs that match the given charge from PID_MAP.
 *
 * @param charge  : charge value of the PIDs to look for.
 * @param pidlist : array of PIDs where to write list of PIDs.
 * @return        : error code.
 */
int rge_get_pidlist_by_charge(int charge, int pidlist[]);

/** Print all PIDs in PID_MAP and their corresponding names to stdout. */
int rge_print_pid_names();

#endif
