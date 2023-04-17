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

#include "../lib/rge_progress.h"

// --+ library +----------------------------------------------------------------
int rge_pbar_set_nentries(long int in_nentries) {
    rge_pbar_nentries = in_nentries;
    return 0;
}

int rge_pbar_reset() {
    rge_pbar_divcntr  = 0;
    rge_pbar_splitter = 0;
    return 0;
}

int rge_pbar_update(long int entry) {
    // Only update if necessary.
    if (entry == rge_pbar_nentries-1) {
        printf("\n");
        return 2;
    }
    if (rge_pbar_splitter == -1 || entry < rge_pbar_splitter) return 0;

    // Clear line if a previous bar has been printed.
    if (entry != 0) printf("\33[2K\r");

    // Print progress bar.
    printf("[");
    for (long int i = 0; i < RGE_PBARLENGTH; ++i) {
        if (i <= (RGE_PBARLENGTH/100.) * (rge_pbar_divcntr)) printf("=");
        else                                                 printf(" ");
    }
    printf("] %2ld%%", rge_pbar_divcntr);
    fflush(stdout);

    // Update rge_pbar_divcntr and rge_pbar_splitter.
    ++rge_pbar_divcntr;
    if (rge_pbar_divcntr <= 100) {
        rge_pbar_splitter = (rge_pbar_nentries/100) * rge_pbar_divcntr;
        return 1;
    }

    rge_pbar_splitter = -1;
    printf("\n");
    return 2;
}
