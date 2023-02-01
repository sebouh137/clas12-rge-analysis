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

// --- Define macro constants here. ----------------------------------------- //
// acc_corr.txt produced by acc_corr.
const char *INPUT_FILENAME  = "../data/acc_corr.txt";
// Root file where we'll write the plots.
const char *OUTPUT_FILENAME = "";
const std::map<int, const char *> PLOT_NAMES {
    // {0, "Q2"}, {1, "#nu"}, {2, "z_{h}"}, {3, "Pt2"}, {4, "#phi_{PQ}"}
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};

// --- Macro code begins here ----------------------------------------------- //
int main() {
    printf("Running... ");
    fflush(stdout);

    // Copy INPUT_FILENAME to char*.
    char input_filename[128];
    for (int i = 0; i <= strlen(INPUT_FILENAME); ++i) {
        if (i == strlen(INPUT_FILENAME)) input_filename[i] = '\0';
        else input_filename[i] = INPUT_FILENAME[i];
    }

    // Read acceptance correction from file.
    long int b_sizes[5]; // NOTE. All good!
    double **binnings;   // NOTE. All good!
    long int pids_size;
    long int nbins;
    long int *pids;
    double **acc_corr;

    printf("\n");
    read_acc_corr_file(input_filename, b_sizes, &binnings, &pids_size, &nbins,
            &pids, &acc_corr);

    for (int bi = 0; bi < 5; ++bi) {
        printf("binning[%02d] (%ld): [", bi, b_sizes[bi]);
        for (int bii = 0; bii < b_sizes[bi]; ++bii)
            printf("%5.2lf, ", binnings[bi][bii]);
        printf("\b\b]\n");
    }
    printf("pids_size = %ld\n", pids_size);
    printf("nbins     = %ld\n", nbins);
    printf("pids[%ld] = [", pids_size);
    for (int pi = 0; pi < pids_size; ++pi) {
        printf("%ld ", pids[pi]);
    }
    printf("\b\b]\n\n");

    // Clean up.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(acc_corr[pi]);
    free(acc_corr);

    return 0;
}
