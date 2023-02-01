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
// Set this to 1 to get some debug information.
const int DEBUG = 1;
// Set to the PID to plot acceptance correction from.
const int PID = -211;
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
    long int bs[5];
    double **binnings;
    long int pids_size;
    long int nbins;
    long int *pids;
    double **acc_corr;

    read_acc_corr_file(input_filename, bs, &binnings, &pids_size, &nbins,
            &pids, &acc_corr);

    // Get place of PID in *pids.
    int pid_pos = -1;
    for (int pi = 0; pi < pids_size; ++pi) if (PID == pids[pi]) pid_pos = pi;
    if (pid_pos == -1) {
        printf("PID %d not found in %s! Exiting...\n", PID, input_filename);
        return 1;
    }

    printf("pid_pos = %d\n", pid_pos);

    // Print read acceptance correction data for debugging purposes.
    if (DEBUG) {
        for (int bi = 0; bi < 5; ++bi) {
            printf("binning[%d] (%02ld): [", bi, bs[bi]);
            for (int bii = 0; bii < bs[bi]; ++bii)
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
    }

    // TODO. Plot.
    int q2_bi = 0;
    long int bin_size = bs[q2_bi];
    // Define x[n] and y[n].
    double x_pos[bin_size - 1];
    double x_length[bin_size - 1];
    double y[bin_size - 1];
    for (int bii = 0; bii < bin_size - 1; ++bii) {
        x_pos[bii]    = (binnings[q2_bi][bii+1] + binnings[q2_bi][bii])/2;
        x_length[bii] = (binnings[q2_bi][bii+1] - binnings[q2_bi][bii])/2;
        y[bii] = 0;
    }

    // Fill y.
    for (int q2_i = 0; q2_i < bs[0]-1; ++q2_i) {
        for (int nu_i = 0; nu_i < bs[1]-1; ++nu_i) {
            for (int zh_i = 0; zh_i < bs[2]-1; ++zh_i) {
                for (int pt2_i = 0; pt2_i < bs[3]-1; ++pt2_i) {
                    for (int phiPQ_i = 0; phiPQ_i < bs[4]-1; ++phiPQ_i) {
                        y[q2_i] += acc_corr[pid_pos][
                            q2_i   * ((bs[1]-1)*(bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                            nu_i   * ((bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                            zh_i   * ((bs[3]-1)*(bs[4]-1)) +
                            pt2_i  * ((bs[4]-1)) +
                            phiPQ_i
                        ];
                    }
                }
            }
        }
    }

    for (int i = 0; i < 2; ++i) printf("y[%d] = %8.5f\n", i, y[i]);
    // for (int bi = 0; bi < 5; ++bi) {}

    // Clean up after ourselves.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(acc_corr[pi]);
    free(acc_corr);

    return 0;
}
