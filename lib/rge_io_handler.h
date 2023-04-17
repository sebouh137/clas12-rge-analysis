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

#ifndef RGE_IOHANDLER
#define RGE_IOHANDLER

// --+ preamble +---------------------------------------------------------------
// C.
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// rge-analysis.
#include "rge_err_handler.h"
#include "rge_file_handler.h"

// --+ internal +---------------------------------------------------------------
/** Check if character c is a number. Returns 1 if it is, 0 if it isn't. */
static int is_number(char c);

/** Check if string s is a number. Returns 1 if it is, 0 if it isn't. */
static int is_number(char *s);

/* Run strtol from n to t, returning appropriate error codes. */
static int run_strtol(long int *n, char *t);

// --+ library +----------------------------------------------------------------
/**
 * Grab a string from an argument.
 *
 * @param arg : optarg variable from getopt.
 * @param str : string to which optarg will be copied.
 * @return    : error code, which is always 0 (no error).
 */
int rge_grab_string(char *arg, char **str);

/**
 * Grab multiple arguments and fill an array with their values.
 *
 * @param argc    : size of list of arguments given to program.
 * @param argv    : list of arguments given to program.
 * @param opt_idx : optind variable from getopt.
 * @param size    : pointer to the size of the array that will be filled.
 * @param arr     : array that will be filled.
 * @return        : error code, which is always 0 (no error).
 */
int rge_grab_multiarg(
        int argc, char **argv, int *opt_idx, long unsigned int *size,
        double **arr
);

/* Run strtol on arg to get number of entries. */
int rge_process_nentries(long int *nentries, char *arg);

/* Run strtol on arg to get number of FMT layers required. */
int rge_process_fmtnlayers(long int *nlayers, char *arg);

/** Catch a y (yes) or a n (no) from stdin. */
bool rge_catch_yn();

/** Catch a long value from stdin. */
long rge_catch_long();

/** Catch a double value from stdin. */
double rge_catch_double();

/**
 * Catch a string from stdin and find its location in an array of strings. If
 *     string is not in list, ask the user again.
 *
 * @param arr  : array of strings.
 * @param size : size of arr.
 * @return     : location of string in stdin inside arr.
 */
int rge_catch_string(const char *arr[], int size);

#endif
