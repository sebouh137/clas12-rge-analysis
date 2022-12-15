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

// Get sampling fraction parameters from file.
int get_sf_params(char *fname, double sf[NSECTORS][SF_NPARAMS][2]) {
    if (access(fname, F_OK) != 0) return 1;
    FILE *t_in = fopen(fname, "r");

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

// Get sizes of binnings, initialize, and fill them from file.
int get_binnings(FILE *f_in, long int *b_sizes, double **binnings,
        long int *pids_size)
{
    // Get binning sizes.
    for (int bi = 0; bi < 5; ++bi) fscanf(f_in, "%ld ", &(b_sizes[bi]));

    // Get binnings.
    for (int bi = 0; bi < 5; ++bi) {
        binnings[bi] = (double *) malloc(b_sizes[bi] * sizeof(*binnings[bi]));
        for (int bii = 0; bii < b_sizes[bi]; ++bii)
            fscanf(f_in, "%lf ", &(binnings[bi][bii]));
    }

    // Get # of pids.
    fscanf(f_in, "%ld", pids_size);

    return 0;
}

// Get pids and acceptance correction from file.
int get_acc_corr(FILE *f_in, long int pids_size, long int tsize, long int *pids,
        double **acc_corr)
{
    // Get PIDs.
    for (int pi = 0; pi < pids_size; ++pi)
        fscanf(f_in, "%ld ", &(pids[pi]));

    // Get acceptance correction.
    for (int pi = 0; pi < pids_size; ++pi) {
        acc_corr[pi] = (double *) malloc(tsize * sizeof(*acc_corr[pi]));
        for (int bii = 0; bii < tsize; ++ bii)
            fscanf(f_in, "%lf ", &(acc_corr[pi][bii]));
    }

    return 0;
}
