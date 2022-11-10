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

#ifndef IO_HANDLER
#define IO_HANDLER

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "file_handler.h"

int make_ntuples_handle_args(int argc, char ** argv, bool * debug, int * nevents,
                             char ** input_file, int * run_no, double * beam_energy);
int extractsf_handle_args(int argc, char ** argv, bool * use_fmt, int * nevents,
                          char ** input_file, int * run_no);
int hipo2root_handle_args(int argc, char ** argv, char ** input_file, int * run_no);

int check_root_filename(char * input_file);
int handle_root_filename(char * input_file, int * run_no);
int handle_root_filename(char * input_file, int * run_no, double * beam_energy);

int check_hipo_filename(char * input_file);
int handle_hipo_filename(char * input_file, int * run_no);

#endif
