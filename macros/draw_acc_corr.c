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
const int NPLOTS = 5;
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
    int **n_thrown;
    int **n_simul;
    read_acc_corr_file(input_filename, bs, &binnings, &pids_size, &nbins,
            &pids, &n_thrown, &n_simul);

    // Get place of PID in *pids.
    int pid_pos = -1;
    for (int pi = 0; pi < pids_size; ++pi) if (PID == pids[pi]) pid_pos = pi;
    if (pid_pos == -1) {
        printf("PID %d not found in %s! Exiting...\n", PID, input_filename);
        return 1;
    }

    printf("pid_pos = %d\n", pid_pos);

    if (DEBUG) {
        // Print read acceptance correction data.
        for (int bi = 0; bi < NPLOTS; ++bi) {
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
    for (int var_idx = 0; var_idx < NPLOTS; ++var_idx) {
        // var_idx represents the variable we're processing, in the order
        //     defined in PLOT_NAMES[].
        long int bin_size = bs[var_idx];

        // Define x and y.
        double x_pos[bin_size - 1];
        double x_length[bin_size - 1];
        int y_thrown[bin_size - 1];
        int y_simul[bin_size - 1];
        for (int bii = 0; bii < bin_size - 1; ++bii) {
            x_pos[bii]    = (binnings[var_idx][bii+1]+binnings[var_idx][bii])/2;
            x_length[bii] = (binnings[var_idx][bii+1]-binnings[var_idx][bii])/2;
            y_thrown[bii] = 0;
            y_simul[bii]  = 0;
        }

        // Fill y. NOTE. This is a very sub-optimal and ugly approach, but it
        //     gets the job done -- and this is just a macro anyway.
        for (int i0 = 0; i0 < bs[0]-1; ++i0) {
            for (int i1 = 0; i1 < bs[1]-1; ++i1) {
                for (int i2 = 0; i2 < bs[2]-1; ++i2) {
                    for (int i3 = 0; i3 < bs[3]-1; ++i3) {
                        for (int i4 = 0; i4 < bs[4]-1; ++i4) {
                            // Find 1D bin position from 5 indices.
                            int bin_pos =
                                i0 * ((bs[1]-1)*(bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                                i1 * ((bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                                i2 * ((bs[3]-1)*(bs[4]-1)) +
                                i3 * ((bs[4]-1)) +
                                i4;

                            // Find which ID should be updated.
                            int sel_idx;
                            switch(var_idx) {
                                case 0: sel_idx = i0; break;
                                case 1: sel_idx = i1; break;
                                case 2: sel_idx = i2; break;
                                case 3: sel_idx = i3; break;
                                case 4: sel_idx = i4; break;
                            }

                            // Increase appropiate counters.
                            y_thrown[sel_idx] += n_thrown[pid_pos][bin_pos];
                            y_simul [sel_idx] += n_simul [pid_pos][bin_pos];
                        }
                    }
                }
            }
        }

        if (DEBUG) {
            // Print counting results.
            printf("%6s thrown  simul\n", PLOT_NAMES.at(var_idx));
            for (int i = 0; i < bs[var_idx]-1; ++i)
            printf("       %08d %08d\n", y_thrown[i], y_simul[i]);
            printf("\n");
        }
    }

    // Clean up after ourselves.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) {
        free(n_thrown[pi]);
        free(n_simul[pi]);
    }
    free(n_thrown);
    free(n_simul);

    return 0;
}
