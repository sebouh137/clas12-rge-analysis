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

// Get run number from input filename.
int get_run_no(char *input_file, int *run_no_int) {
    // TODO. This is a very brute way to find the run number and **should** be
    //       changed.
    char run_no_str[7];
    char *dot_pos = strrchr(input_file, '.');
    if (!dot_pos) return 0;
    strncpy(run_no_str, dot_pos - 6, 6);
    run_no_str[6] = '\0';
    *run_no_int = atoi(run_no_str);

    return *run_no_int;
}

// Match run number to beam energy from constants.
int get_beam_energy(int run_no, double *beam_energy) {
    // NOTE. This should be a map in constants or taken directly from clas12mon.
    switch (run_no) {
        case 11983:  *beam_energy = BE11983;  break;
        case 12016:  *beam_energy = BE12016;  break;
        case 12439:  *beam_energy = BE12439;  break;
        case 999106: *beam_energy = BE999106; break;
        case 999110: *beam_energy = BE999110; break;
        case 999120: *beam_energy = BE999120; break;
        default:     return -1;
    }

    return 0;
}

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

// Get sizes of binning arrays from acceptance correction file.
int get_b_sizes(char *fname, long int *sizes) {
    if (access(fname, F_OK) != 0) return 1;
    FILE *f_in = fopen(fname, "r");
    double dump;

    for (int bi = 0; bi < 5; ++bi) {
        fscanf(f_in, "%ld ", &sizes[bi]);
        for (int bii = 0; bii < sizes[bi]; ++bii) fscanf(f_in, "%lf ", &dump);
    }

    fclose(f_in);
    return 0;
}

int get_binnings(char *fname, long int *sizes, double *b_Q2, double *b_nu,
        double *b_zh, double *b_Pt2, double *b_pPQ)
{
    FILE *f_in = fopen(fname, "r");
    long int dump;

    for (int bi = 0; bi < 5; ++bi) {
        fscanf(f_in, "%ld ", &dump);
        for (int bii = 0; bii < sizes[bi]; ++bii) {
            if (bi == 0) fscanf(f_in, "%lf ", &b_Q2[bii]);
            if (bi == 1) fscanf(f_in, "%lf ", &b_nu[bii]);
            if (bi == 2) fscanf(f_in, "%lf ", &b_zh[bii]);
            if (bi == 3) fscanf(f_in, "%lf ", &b_Pt2[bii]);
            if (bi == 4) fscanf(f_in, "%lf ", &b_pPQ[bii]);
        }
    }

    fclose(f_in);
    return 0;
}

// Get array size for acceptance correction from file.
// int get_ac_size(char *fname, std::vector<double> b_Q2,
//         std::vector<double> b_nu,  std::vector<double> b_zh,
//         std::vector<double> b_Pt2, std::vector<double> b_pPQ)
// {
//     if (access(fname, F_OK) != 0) return 1;
//     FILE *t_in = fopen(fname, "r");
//
//
// }

// // Get acceptance correction from file.
// int get_acc_correction(char *fname, )
