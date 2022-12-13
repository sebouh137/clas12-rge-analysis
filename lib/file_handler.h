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

#ifndef FILE_HANDLER
#define FILE_HANDLER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include "constants.h"

int get_run_no(char *input_file, int *run_no_int);
int get_beam_energy(int run_no, double *beam_energy);
int get_sf_params(char *fname, double sf[NSECTORS][SF_NPARAMS][2]);
int get_binnings(FILE *f_in, long int *b_sizes, double **binnings,
        long int *pids_size);
int get_acc_corr(FILE *f_in, long int pids_size, long int tsize, long int *pids,
        double **acc_corr);

#endif
