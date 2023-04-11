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

#ifndef RGE_FILENAMEHANDLER
#define RGE_FILENAMEHANDLER

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rge_err_handler.h"

// --+ internal +---------------------------------------------------------------
// Beam energy of different runs. TODO. This should be taken from RCDB.
#define BE11983 10.3894 //  50 nA.
#define BE12016 10.3894 // 250 nA.
#define BE12439  2.1864 //  15 nA.
#define BE12933 10.4057 // 250 nA.

/**
 * Get run number from a filename, assuming the filename is in format
 *     <text><run_no>.<extension>. Write run number to *run_no, and return an
 *     error code.
 *
 * @param filename : filename to be processed.
 * @param run_no   : pointer to the int where the run number is written.
 * @return         : error code:
 *                     * 0: everything went fine.
 *                     * 1: couldn't find dot position in filename.
 *                     * 2: strtoul failed -- couldn't find run number.
 */
static int get_run_no(char *filename, int *run_no);

/**
 * Match a run number to a beam energy. The beam energy for different runs is
 *     available in the constants.
 *
 * @param run_no      : run number for which the beam energy is to be found.
 * @param beam_energy : pointer to the double where the beam energy will be
 *                      saved.
 * @return            : an error code.
 */
static int get_beam_energy(int run_no, double *beam_energy);

/**
 * Check if a hipo filename is valid.
 *
 * @param filename : filename to be processed.
 * @return         : error code.
 */
static int check_hipo_filename(char *filename);

// --+ library +----------------------------------------------------------------
/**
 * Check if a root filename is valid.
 *
 * @param filename : filename to be processed.
 * @return         : 0 if file is valid, 1 otherwise.
 */
int rge_check_root_filename(char *filename);

/**
 * Handle a root filename, checking its validity, file existence, and grabbing
 *     the run number and beam energy from it.
 *
 * @param filename    : filename to be processed.
 * @param run_no      : pointer to the int where the run number will be written.
 * @param beam_energy : pointer to the double where the beam energy will be
 *                      written.
 * @return            : error code.
 */
int rge_handle_root_filename(char *filename, int *run_no, double *beam_energy);

/**
 * Run rge_handle_root_filename(char *, int *, double *) without writing
 *     beam_energy.
 */
int rge_handle_root_filename(char *filename, int *run_no);

/**
 * Handle a hipo filename, checking its validity, file existence, and grabbing
 *     the run number from it.
 *
 * @param filename : filename to be processed.
 * @param run_no   : pointer to the int where the run number will be written.
 * @return         : error code.
 */
int rge_handle_hipo_filename(char *filename, int *run_no);

#endif
