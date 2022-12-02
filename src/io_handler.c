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

int check_root_filename(char *input_file) {
    if (!strstr(input_file, ".root"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_root_filename(char *input_file, int *run_no) {
    double dump = 0.;
    return handle_root_filename(input_file, run_no, &dump);
}

int handle_root_filename(char *input_file, int *run_no, double *beam_energy) {
    int chk = check_root_filename(input_file);
    if (chk) return chk;
    // Get run number and beam energy from filename.
    if (!get_run_no(input_file, run_no))       return 5;
    if (get_beam_energy(*run_no, beam_energy)) return 6;

    return 0;
}

int check_hipo_filename(char *input_file) {
    if (!strstr(input_file, ".hipo"))     return 3; // Check that file is valid.
    if (!(access(input_file, F_OK) == 0)) return 4; // Check that file exists.
    return 0;
}

int handle_hipo_filename(char *input_file, int *run_no) {
    int chk = check_hipo_filename(input_file);
    if (chk) return chk;

    // Get run number from filename.
    if (!get_run_no(input_file, run_no)) return 5;

    return 0;
}

// Grab multiple arguments and fill a vector of doubles with it.
// TODO. This doesn't accept negative numbers!
int grab_multiarg(int argc, char **argv, int *optind, int *size, double **b) {
    int idx   = *optind - 1;
    int start = idx;
    *size     = 0;
    char *next;

    // Get size.
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (next[0] >= '0' && next[0] <= '9') ++(*size);
        else                                  break;
    }

    // Restart counter and initialize binning.
    idx = start;
    (*b) = (double *) malloc((*size) * sizeof(**b));

    // Fill binning.
    int i = 0;
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (next[0] >= '0' && next[0] <= '9') (*b)[i++] = atof(next);
        else                                  break;
    }

    // Continue with getopts.
    *optind = idx - 1;
    return 0;
}

// Grab filename from optarg.
int grab_filename(char *optarg, char **file) {
    *file = (char *) malloc(strlen(optarg) + 1);
    strcpy(*file, optarg);
    return 0;
}

// Catch a y or n input.
bool catch_yn() {
    // TODO. Figure out how to catch no input so that this can be [Y/n].
    while (true) {
        char str[32];
        printf(">>> ");
        scanf("%31s", str);

        if (!strcmp(str, "y") || !strcmp(str, "Y")) return true;
        if (!strcmp(str, "n") || !strcmp(str, "N")) return false;
    }
}

// Catch a string within a list.
int catch_string(const char * list[], int size) {
    double x;
    while (true) {
        char str[32];
        printf(">>> ");
        scanf("%31s", str);

        for (int i = 0; i < size; ++i) if (!strcmp(str, list[i])) x = i;
        if (x != -1) break;
    }

    return x;
}

// Catch a long value from stdin.
long catch_long() {
    long r;
    while (true) {
        char str[32];
        char * endptr;
        printf(">>> ");
        scanf("%31s", str);
        r = strtol(str, &endptr, 10);

        if (endptr != str) break;
    }

    return r;
}

// Catch a double value from stdin.
double catch_double() {
    double r;
    while (true) {
        char str[32];
        char * endptr;
        printf(">>> ");
        scanf("%31s", str);
        r = strtod(str, &endptr);

        if (endptr != str) break;
    }

    return r;
}
