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

#ifndef RGE_FILEHANDLER
#define RGE_FILEHANDLER

// --+ preamble +---------------------------------------------------------------
// C.
#include <unistd.h>

// rge-analysis.
#include "rge_constants.h"
#include "rge_err_handler.h"

// typedefs.
typedef unsigned int uint;
typedef long unsigned int luint;
typedef long int lint;

// --+ internal +---------------------------------------------------------------
/**
 * Read binning data from text file and fill binning sizes array, bin_edges
 *     array, and an array of PID list sizes.
 *
 * @param file_in    : file from which we extract the data.
 * @param bin_nedges : array of size 5 with binning sizes.
 * @param bin_edges  : 2-dimensional array containing all 5 bin_edges.
 * @param pids_size  : number of PIDs in list of PIDs.
 * @return           : success code (0).
 */
static int get_bin_edges(
        FILE *file_in, luint *bin_nedges, double **bin_edges, luint *pids_size
);

/**
 * Read acceptance correction data from text file and fill PIDs list and
 *     accceptance correction array. This function follows from get_bin_edges().
 *
 * @param file_in   : file from which we extract the data.
 * @param pids_size : number of PIDs to process, as found in get_bin_edges().
 * @param nbins     : total number of bins.
 * @param pids      : array of PIDs to process.
 * @param n_thrown  : 2-dimensional array containing the number of thrown events
 *                    for each bin, for each PID.
 * @param n_simul   : 2-dimensional array containing the number of simul events
 *                    for each bin, for each PID.
 * @return          : success code (0).
 */
static int get_acc_corr(
        FILE *file_in, luint pids_size, luint nbins, lint *pids, int **n_thrown,
        int **n_simul
);

// --+ library +----------------------------------------------------------------
/**
 * Get sampling fraction parameters from file. File contents must follow CCDB
 *     format, same to what is provided by extract_sf.
 *
 * @param filename : filename to be processed.
 * @param sf[][][] : 3-dimensional array where sampling fraction data is to be
 *                   written. Check the README.md for more information.
 * @return         : error code:
 *                     * 0: everything went fine.
 *                     * 1: no file with filename was found.
 */
int rge_get_sf_params(char *filename, double sf[NSECTORS][SF_NPARAMS][2]);

/**
 * Read acc_corr.txt file to get the acceptance correction for each bin for each
 *     PID. Acceptance correction for a particular PID is stored in an array
 *     instead of a 5D array to avoid writing it as *******acc_corr, in addition
 *     to simplicity in access. To access the array, think of it as a 5D array
 *     in the shape acc_corr[Q2 bin][nu bin][zh bin][PT2 bin][phiPQ bin],
 *     multiplying each entry by the appropiate size.
 *
 * @param acc_filename : char array with the filename containing the acceptance
 *                       correction to be processed.
 * @param bin_nedges   : array that we'll fill with the size of each of the 5
 *                       bin_edges.
 * @param bin_edges    : pointer to a 2-dimensional array that we'll fill with
 *                       the bin walls for each binning.
 * @param pids_size    : int where we'll write the number of PIDs in the file.
 * @param nbins        : int where we'll write the total number of bins.
 * @param pids         : pointer to an array where we'll write the list of PIDs
 *                       in the acceptance correction file.
 * @param n_thrown     : pointer to a 2-dimensional array where we'll write the
 *                       number of thrown events in each bin for each PID.
 * @param n_simul      : pointer to a 2-dimensional array where we'll write the
 *                       number of simulated events in each bin for each PID.
 * @return             : error code:
 *                         * 0: Function performed correctly.
 *                         * 1: Failed to access acceptance correction file.
 */
int rge_read_acc_corr_file(
        char *acc_filename, luint bin_nedges[5], double ***bin_edges,
        luint *pids_size, luint *nbins, lint **pids, int ***n_thrown,
        int ***n_simul
);

#endif
