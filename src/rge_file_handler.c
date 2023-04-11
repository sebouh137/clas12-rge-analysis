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

#include "../lib/rge_file_handler.h"

// --+ internal +---------------------------------------------------------------
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

// --+ library +----------------------------------------------------------------
int rge_get_sf_params(char *filename, double sf[NSECTORS][SF_NPARAMS][2]) {
    if (access(filename, F_OK) != 0) {
        rge_errno = RGEERR_NOSAMPFRACFILE;
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

int rge_read_acc_corr_file(
        char *acc_filename, long unsigned int bin_nedges[5],
        double ***bin_edges, long unsigned int *pids_size,
        long unsigned int *nbins, long int **pids, int ***n_thrown,
        int ***n_simul
) {
    // Access file.
    if (access(acc_filename, F_OK) != 0) {
        rge_errno = RGEERR_NOACCCORRFILE;
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
