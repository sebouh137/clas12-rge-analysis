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

#include "../lib/file_handler.h"

/**
 * Get sampling fraction parameters from file.
 *
 * @param filename: filename to be processed.
 * @param sf[][][]: 3-dimensional array where sampling fraction data is to be
 *                  written. Check the README.md for more information.
 * @return          error code:
 *                    * 0: everything went fine.
 *                    * 1: no file with filename was found.
 */
int get_sf_params(char *filename, double sf[NSECTORS][SF_NPARAMS][2]) {
    if (access(filename, F_OK) != 0) {
        rge_errno = ERR_NOSAMPFRACFILE;
        return 1;
    }
    FILE *file_in = fopen(filename, "r");

    for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
        for (int edge_i = 0; edge_i < 2; ++edge_i) {
            for (int param_i = 0; param_i < SF_NPARAMS; ++param_i) {
                fscanf(file_in, "%lf ", &sf[sector_i][param_i][edge_i]);
            }
        }
    }

    fclose(file_in);
    return 0;
}

/**
 * Read binning data from text file and fill binning sizes array, bin_edges
 *     array, and an array of PID list sizes.
 *
 * @param file_in:    file from which we extract the data.
 * @param bin_nedges: array of size 5 with binning sizes.
 * @param bin_edges:  2-dimensional array containing all 5 bin_edges.
 * @param pids_size:  number of PIDs in list of PIDs.
 * @return:           success code (0).
 */
int get_bin_edges(
        FILE *file_in, long unsigned int *bin_nedges, double **bin_edges,
        long unsigned int *pids_size
) {
    // Get binning sizes.
    for (int bin_i = 0; bin_i < 5; ++bin_i) {
        fscanf(file_in, "%lu ", &(bin_nedges[bin_i]));
    }

    // Get bin edges.
    for (int bin_i = 0; bin_i < 5; ++bin_i) {
        bin_edges[bin_i] = static_cast<double *>(
                malloc(bin_nedges[bin_i] * sizeof(*bin_edges[bin_i]))
        );
        for (long unsigned int edge_i = 0; edge_i < bin_nedges[bin_i]; ++edge_i)
            fscanf(file_in, "%lf ", &(bin_edges[bin_i][edge_i]));
    }

    // Get # of pids.
    fscanf(file_in, "%lu", pids_size);

    return 0;
}

/**
 * Read acceptance correction data from text file and fill PIDs list and
 *     accceptance correction array. This function follows from get_bin_edges().
 *
 * @param file_in:   file from which we extract the data.
 * @param pids_size: number of PIDs to process, as found in get_bin_edges().
 * @param nbins:     total number of bins.
 * @param pids:      array of PIDs to process.
 * @param n_thrown:  2-dimensional array containing the number of thrown events
 *                   for each bin, for each PID.
 * @param n_simul:   2-dimensional array containing the number of simul events
 *                   for each bin, for each PID.
 * @return:          success code (0).
 */
int get_acc_corr(
        FILE *file_in, long unsigned int pids_size, long unsigned int nbins,
        long int *pids, int **n_thrown, int **n_simul
) {
    // Get PIDs.
    for (long unsigned int pid_i = 0; pid_i < pids_size; ++pid_i) {
        fscanf(file_in, "%ld ", &(pids[pid_i]));
    }

    // Get acceptance correction.
    for (long unsigned int pid_i = 0; pid_i < pids_size; ++pid_i) {
        // Get number of thrown events.
        n_thrown[pid_i] = static_cast<int *>(
                malloc(nbins * sizeof(*n_thrown[pid_i]))
        );
        for (long unsigned int bin_i = 0; bin_i < nbins; ++bin_i) {
            fscanf(file_in, "%d ", &(n_thrown[pid_i][bin_i]));
        }

        // Get number of simulated events.
        n_simul[pid_i] = static_cast<int *>(
                malloc(nbins * sizeof(*n_simul[pid_i]))
        );
        for (long unsigned int bin_i = 0; bin_i < nbins; ++bin_i) {
            fscanf(file_in, "%d ", &(n_simul[pid_i][bin_i]));
        }
    }

    return 0;
}

/**
 * Read acc_corr.txt file to get the acceptance correction for each bin for each
 *     PID. Acceptance correction for a particular PID is stored in an array
 *     instead of a 5D array to avoid writing it as *******acc_corr, in addition
 *     to simplicity in access. To access the array, think of it as a 5D array
 *     in the shape acc_corr[Q2 bin][nu bin][zh bin][PT2 bin][phiPQ bin],
 *     multiplying each entry by the appropiate size.
 *
 * @param acc_filename: char array with the filename containing the acceptance
 *                      correction to be processed.
 * @param bin_nedges:   array that we'll fill with the size of each of the 5
 *                      bin_edges.
 * @param bin_edges:    pointer to a 2-dimensional array that we'll fill with
 *                      the bin walls for each binning.
 * @param pids_size:    int where we'll write the number of PIDs in the file.
 * @param nbins:        int where we'll write the total number of bins.
 * @param pids:         pointer to an array where we'll write the list of PIDs
 *                      in the acceptance correction file.
 * @param n_thrown:     pointer to a 2-dimensional array where we'll write the
 *                      number of thrown events in each bin for each PID.
 * @param n_simul:      pointer to a 2-dimensional array where we'll write the
 *                      number of simulated events in each bin for each PID.
 * @return: errcode:
 *            * 0: Function performed correctly.
 *            * 1: Failed to access acceptance correction file.
 */
int read_acc_corr_file(
        char *acc_filename, long unsigned int bin_nedges[5],
        double ***bin_edges, long unsigned int *pids_size,
        long unsigned int *nbins, long int **pids, int ***n_thrown,
        int ***n_simul)
{
    // Access file.
    if (access(acc_filename, F_OK) != 0) {
        rge_errno = ERR_NOACCCORRFILE;
        return 1;
    }
    FILE *acc_file = fopen(acc_filename, "r");

    // Get bin_nedges, bin_edges, and pids_size.
    *bin_edges = static_cast<double **>(malloc(5 * sizeof(**bin_edges)));
    get_bin_edges(acc_file, bin_nedges, *bin_edges, pids_size);

    // Compute total number of bins.
    *nbins = 1;
    for (int bi = 0; bi < 5; ++bi) *nbins *= (bin_nedges[bi] - 1);

    // Malloc list of pids and first dimension of pids and events.
    *pids = static_cast<long int *>(malloc(*pids_size * sizeof(**pids)));
    *n_thrown = static_cast<int **>(malloc(*pids_size * sizeof(**n_thrown)));
    *n_simul  = static_cast<int **>(malloc(*pids_size * sizeof(**n_simul)));

    // Get pids and acc_corr from acceptance correction file.
    get_acc_corr(acc_file, *pids_size, *nbins, *pids, *n_thrown, *n_simul);

    // Clean up.
    fclose(acc_file);

    return 0;
}
