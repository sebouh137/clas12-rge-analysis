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
    if (access(filename, F_OK) != 0) return 1;
    FILE *t_in = fopen(filename, "r");

    for (int si = 0; si < NSECTORS; ++si) {
        for (int ppi = 0; ppi < 2; ++ppi) {
            for (int pi = 0; pi < SF_NPARAMS; ++pi) {
                fscanf(t_in, "%lf ", &sf[si][pi][ppi]);
            }
        }
    }

    fclose(t_in);
    return 0;
}

/**
 * Read binning data from text file and fill binning sizes array, bin_edges
 *     array, and an array of PID list sizes.
 *
 * @param f_in:       file from which we extract the data.
 * @param bin_nedges: array of size 5 with binning sizes.
 * @param bin_edges:  2-dimensional array containing all 5 bin_edges.
 * @param pids_size:  number of PIDs in list of PIDs.
 * @return:           success code (0).
 */
int get_bin_edges(FILE *f_in, long int *bin_nedges, double **bin_edges,
        long int *pids_size)
{
    // Get binning sizes.
    for (int bi = 0; bi < 5; ++bi) fscanf(f_in, "%ld ", &(bin_nedges[bi]));

    // Get bin_edges.
    for (int bi = 0; bi < 5; ++bi) {
        bin_edges[bi] =
                (double *) malloc(bin_nedges[bi] * sizeof(*bin_edges[bi]));
        for (int bii = 0; bii < bin_nedges[bi]; ++bii)
            fscanf(f_in, "%lf ", &(bin_edges[bi][bii]));
    }

    // Get # of pids.
    fscanf(f_in, "%ld", pids_size);

    return 0;
}

/**
 * Read acceptance correction data from text file and fill PIDs list and
 *     accceptance correction array. This function follows from get_bin_edges().
 *
 * @param f_in:      file from which we extract the data.
 * @param pids_size: number of PIDs to process, as found in get_bin_edges().
 * @param nbins:     total number of bins.
 * @param pids:      array of PIDs to process.
 * @param n_thrown:  2-dimensional array containing the number of thrown events
 *                   for each bin, for each PID.
 * @param n_simul:   2-dimensional array containing the number of simul events
 *                   for each bin, for each PID.
 * @return:          success code (0).
 */
int get_acc_corr(FILE *f_in, long int pids_size, long int nbins, long int *pids,
        int **n_thrown, int **n_simul)
{
    // Get PIDs.
    for (int pi = 0; pi < pids_size; ++pi) fscanf(f_in, "%ld ", &(pids[pi]));

    // Get acceptance correction.
    for (int pi = 0; pi < pids_size; ++pi) {
        // Get number of thrown events.
        n_thrown[pi] = (int *) malloc(nbins * sizeof(*n_thrown[pi]));
        for (int bii = 0; bii < nbins; ++bii)
            fscanf(f_in, "%d ", &(n_thrown[pi][bii]));

        // Get number of simulated events.
        n_simul[pi]  = (int *) malloc(nbins * sizeof(*n_simul[pi]));
        for (int bii = 0; bii < nbins; ++bii)
            fscanf(f_in, "%d ", &(n_simul[pi][bii]));
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
int read_acc_corr_file(char *acc_filename, long int bin_nedges[5],
        double ***bin_edges, long int *pids_size, long int *nbins,
        long int **pids, int ***n_thrown, int ***n_simul)
{
    // Access file.
    if (access(acc_filename, F_OK) != 0) return 1;
    FILE *acc_file = fopen(acc_filename, "r");

    // Get bin_nedges, bin_edges, and pids_size.
    *bin_edges = (double **) malloc(5 * sizeof(**bin_edges));
    get_bin_edges(acc_file, bin_nedges, *bin_edges, pids_size);

    // Compute total number of bins.
    *nbins = 1;
    for (int bi = 0; bi < 5; ++bi) *nbins *= (bin_nedges[bi] - 1);

    // Malloc list of pids and first dimension of pids and events.
    *pids = (long int *) malloc(*pids_size * sizeof(**pids));
    *n_thrown = (int **) malloc(*pids_size * sizeof(**n_thrown));
    *n_simul  = (int **) malloc(*pids_size * sizeof(**n_simul));

    // Get pids and acc_corr from acceptance correction file.
    get_acc_corr(acc_file, *pids_size, *nbins, *pids, *n_thrown, *n_simul);

    // Clean up.
    fclose(acc_file);

    return 0;
}
