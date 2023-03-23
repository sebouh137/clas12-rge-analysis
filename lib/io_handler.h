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

#ifndef IO_HANDLER
#define IO_HANDLER

#include "file_handler.h"

int get_run_no(char *filename, int *run_no);
int get_beam_energy(int run_no, double *beam_energy);

int check_root_filename(char *filename);
int handle_root_filename(char *filename, int *run_no, double *beam_energy);
int handle_root_filename(char *filename, int *run_no);

int check_hipo_filename(char *filename);
int handle_hipo_filename(char *filename, int *run_no);

int is_number(char *s);
int is_number(char c);

int update_progress_bar(int nevn, int evn, int *evnsplitter, int *divcntr);

int grab_multiarg(int argc, char **argv, int *opt_idx, long unsigned int *size,
        double **b);
int grab_str(char *optarg, char **str);

bool catch_yn();
int catch_string(const char *arr[], int size);
double catch_double();
long catch_long();

#endif
