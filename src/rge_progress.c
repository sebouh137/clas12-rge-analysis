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
int rge_pbar_set_nentries(lint in_nentries) {
    pbar_nentries = in_nentries;
    return 0;
}

int rge_pbar_reset() {
    pbar_divcntr  = 0;
    pbar_splitter = 0;
    return 0;
}

int rge_pbar_update(lint entry) {
    // Only update if necessary.
    if (entry == pbar_nentries-1) {
        printf("\n");
        return 2;
    }
    if (pbar_splitter == -1 || entry < pbar_splitter) return 0;

    // Clear line if a previous bar has been printed.
    if (entry != 0) printf("\33[2K\r");

    // Print progress bar.
    printf("[");
    for (lint i = 0; i < PBARLENGTH; ++i) {
        if (i <= (PBARLENGTH/100.) * (pbar_divcntr)) printf("=");
        else                                         printf(" ");
    }
    printf("] %2ld%%", pbar_divcntr);
    fflush(stdout);

    // Update pbar_divcntr and pbar_splitter.
    ++pbar_divcntr;
    if (pbar_divcntr <= 100) {
        pbar_splitter = (pbar_nentries/100) * pbar_divcntr;
        return 1;
    }

    pbar_splitter = -1;
    printf("\n");
    return 2;
}
