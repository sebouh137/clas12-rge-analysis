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

#include "../lib/io_handler.h"

int extractsf_handle_args(int argc, char ** argv, bool * use_fmt, int * nevents,
                          char ** input_file, int * run_no) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-fn:")) != -1) {
        switch (opt) {
            case 'f': * use_fmt = true;         break;
            case 'n': * nevents = atoi(optarg); break;
            case  1 :{
                * input_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(* input_file, optarg);
                break;
            }
            default:  return 1;
        }
    }
    if (* nevents == 0) return 2;
    if (argc < 2) return 5;

    return handle_root_filename(* input_file, run_no);
}

int check_root_filename(char * input_file) {
    if (!strstr(input_file, ".root"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_root_filename(char * input_file, int * run_no) {
    double dump = 0.;
    return handle_root_filename(input_file, run_no, &dump);
}

int handle_root_filename(char * input_file, int * run_no, double * beam_energy) {
    int chk = check_root_filename(input_file);
    if (chk) return chk;
    // Get run number and beam energy from filename.
    if (!get_run_no(input_file, run_no))  return 5;
    if (get_beam_energy(* run_no, beam_energy))      return 6;

    return 0;
}

int check_hipo_filename(char * input_file) {
    if (!strstr(input_file, ".hipo"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_hipo_filename(char * input_file, int * run_no) {
    int chk = check_hipo_filename(input_file);
    if (chk) return chk;

    // Get run number from filename.
    if (!get_run_no(input_file, run_no)) return 5;

    return 0;
}
