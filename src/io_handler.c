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

/**
 * Get run number from a filename, assuming the filename is in format
 *     <text><run_no>.<extension>. If the filename is badly formatted or if atoi
 *     fails, the function returns 0.
 *
 * @param filename: filename to be processed.
 * @param run_no:   pointer to the int where the run number is written.
 * @return          an error code:
 *                    * 0: everything went fine.
 *                    * 1: couldn't find dot position in filename.
 *                    * 2: atoi failed -- couldn't find run number.
 */
int get_run_no(char *filename, int *run_no) {
    char run_no_str[7];
    char *dot_pos = strrchr(filename, '.');
    if (!dot_pos) return 1; // Couldn't find dot.
    strncpy(run_no_str, dot_pos - 6, 6);
    run_no_str[6] = '\0';
    *run_no = atoi(run_no_str); // If atoi fails, it returns 0.
    if (*run_no == 0) return 2;

    return 0;
}

/**
 * Match a run number to a beam energy. The beam energy for different runs is
 *     available in the constants.
 *
 * @param run_no:      run number for which the beam energy is to be found.
 * @param beam_energy: pointer to the double where the beam energy will be
 *                     saved.
 * @return             an error code:
 *                       * 0: everything went fine.
 *                       * 1: beam energy for run number is unavailable.
 */
int get_beam_energy(int run_no, double *beam_energy) {
    // NOTE. This should be taken directly from RCDB.
    switch (run_no) {
        case  11983: *beam_energy = BE11983;  break;
        case  12016: *beam_energy = BE12016;  break;
        case  12439: *beam_energy = BE12439;  break;
        case  12933: *beam_energy = BE12933;  break;
        case 999106: *beam_energy = BE999106; break;
        case 999110: *beam_energy = BE999110; break;
        case 999120: *beam_energy = BE999120; break;
        default:     return 1;
    }

    return 0;
}

/**
 * Check if a root filename is valid.
 *
 * @param filename: filename to be processed.
 * @return          an error code:
 *                    * 1: filename extension isn't root.
 *                    * 2: no file with filename was found.
 */
int check_root_filename(char *filename) {
    if (!strstr(filename, ".root")) return 1;
    if (access(filename, F_OK))     return 2;
    return 0;
}

/**
 * Handle a root filename, checking its validity, file existence, and grabbing
 *     the run number and beam energy from it.
 *
 * @param filename:    filename to be processed.
 * @param run_no:      pointer to the int where the run number will be written.
 * @param beam_energy: pointer to the double where the beam energy will be
 *                     written.
 * @return             an error code:
 *                       * 0: everything went fine.
 *                       * 1: filename extension isn't root.
 *                       * 2: no file with filename was found.
 *                       * 3: couldn't find dot position in filename.
 *                       * 4: atoi failed -- couldn't find run number.
 *                       * 5: beam energy for run number is unavailable.
 */
int handle_root_filename(char *filename, int *run_no, double *beam_energy) {
    int check = check_root_filename(filename);
    if (check) return check;

    check = get_run_no(filename, run_no);
    if (check) return check + 2;

    check = get_beam_energy(*run_no, beam_energy);
    if (check) return check + 4;

    return 0;
}

/** Run handle_root_filename() without writing beam_energy. */
int handle_root_filename(char *filename, int *run_no) {
    double dump = 0.;
    return handle_root_filename(filename, run_no, &dump);
}

/**
 * Check if a hipo filename is valid.
 *
 * @param filename: filename to be processed.
 * @return          an error code:
 *                    * 1: filename extension isn't hipo.
 *                    * 2: no file with filename was found.
 */
int check_hipo_filename(char *filename) {
    if (!strstr(filename, ".hipo")) return 1;
    if (access(filename, F_OK))     return 2;
    return 0;
}

/**
 * Handle a hipo filename, checking its validity, file existence, and grabbing
 *     the run number from it.
 *
 * @param filename: filename to be processed.
 * @param run_no:   pointer to the int where the run number will be written.
 * @return          an error code:
 *                    * 0: everything went fine.
 *                    * 1: filename extension isn't hipo.
 *                    * 2: no file with filename was found.
 *                    * 3: couldn't find dot position in filename.
 *                    * 4: atoi failed -- couldn't find run number.
 */
int handle_hipo_filename(char *filename, int *run_no) {
    int check = check_hipo_filename(filename);
    if (check) return check;

    check = get_run_no(filename, run_no);
    if (check) return check + 2;

    return 0;
}

/** Check if string s is a number. Returns 1 if it is, 0 if it isn't. */
int is_number(char *s) {
    if (is_number(s[0]) || (s[0] == '-' && is_number(s[1]))) return 1;
    return 0;
}

/** Check if character c is a number. Returns 1 if it is, 0 if it isn't. */
int is_number(char c) {
    if (c >= '0' && c <= '9') return 1;
    return 0;
}

/**
 * Grab multiple arguments and fill an array with their values.
 *
 * @param argc:   size of list of arguments given to program.
 * @param argv:   list of arguments given to program.
 * @param optind: optind variable from getopt.
 * @param size:   pointer to the size of the array that we'll fill.
 * @param arr:    array that we'll fill.
 * @return:       error code, which is always 0 (no error).
 */
int grab_multiarg(int argc, char **argv, int *optind, int *size, double **arr) {
    int idx   = *optind - 1;
    int start = idx;
    *size     = 0;
    char *next;

    // Get size.
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (is_number(next)) ++(*size);
        else break;
    }

    // Restart counter and initialize binning.
    idx = start;
    (*arr) = (double *) malloc((*size) * sizeof(**arr));

    // Fill binning.
    int i = 0;
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (is_number(next)) (*arr)[i++] = atof(next);
        else break;
    }

    // Continue with getopts.
    *optind = idx - 1;
    return 0;
}

/**
 * Grab a string from an optarg.
 *
 * @param optarg: optarg variable from getopt.
 * @param str:    string to which optarg will be copied.
 * @return:       error code, which is always 0 (no error).
 */
int grab_str(char *optarg, char **str) {
    *str = (char *) malloc(strlen(optarg) + 1);
    strcpy(*str, optarg);
    return 0;
}

/** Catch a y (yes) or a n (no) from stdin. */
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

/**
 * Catch a string from stdin and find its location in an array of strings. If
 *     string is not in list, ask the user again.
 *
 * @param arr:  array of strings.
 * @param size: size of arr.
 * @return:     location of string in stdin inside arr.
 */
int catch_string(const char *arr[], int size) {
    double x;
    while (true) {
        char str[32];
        printf(">>> ");
        scanf("%31s", str);

        for (int i = 0; i < size; ++i) if (!strcmp(str, arr[i])) x = i;
        if (x != -1) break;
    }

    return x;
}

/** Catch a long value from stdin. */
long catch_long() {
    long r;
    while (true) {
        char str[32];
        char *endptr;
        printf(">>> ");
        scanf("%31s", str);
        r = strtol(str, &endptr, 10);

        if (endptr != str) break;
    }

    return r;
}

/** Catch a double value from stdin. */
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
