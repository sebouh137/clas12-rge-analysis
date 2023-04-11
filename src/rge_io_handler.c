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

#include "../lib/rge_io_handler.h"

// TODO. Separate this file into io_handler, file_handler, and progress_bar.

/* Run strtol, returning appropriate error codes. */
int run_strtol(long int *n, char *t) {
    char *eptr;
    errno = 0;
    *n = strtol(t, &eptr, 10);
    if (errno == EINVAL) return 1;
    if (errno == ERANGE) return 2;
    return 0;
}

/* Run strtol on arg to get number of FMT layers required. */
int process_fmtnlayers(long int *nlayers, char *arg) {
    int err = run_strtol(nlayers, arg);
    if (err == 1 || (
            *nlayers != 0 &&
            (FMTMINLAYERS > *nlayers || *nlayers > FMTNLAYERS)
    )) {
        rge_errno = RGEERR_INVALIDFMTNLAYERS;
        return 1;
    }
    return 0;
}

/* Run strtol on arg to get number of entries. */
int process_nentries(long int *nentries, char *arg) {
    int err = run_strtol(nentries, arg);
    if (err == 1) {
        rge_errno = RGEERR_INVALIDENTRIES;
        return 1;
    }
    if (err == 2) {
        rge_errno = RGEERR_NENTRIESLARGE;
        return 1;
    }
    if (*nentries <= 0) {
        rge_errno = RGEERR_NENTRIESNEGATIVE;
        return 1;
    }

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
 * @param argc:    size of list of arguments given to program.
 * @param argv:    list of arguments given to program.
 * @param opt_idx: optind variable from getopt.
 * @param size:    pointer to the size of the array that we'll fill.
 * @param arr:     array that we'll fill.
 * @return:        error code, which is always 0 (no error).
 */
int grab_multiarg(
        int argc, char **argv, int *opt_idx, long unsigned int *size,
        double **arr
) {
    int idx   = *opt_idx - 1;
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
    (*arr) = static_cast<double *>(malloc((*size) * sizeof(**arr)));

    // Fill binning.
    int i = 0;
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (is_number(next)) (*arr)[i++] = atof(next);
        else break;
    }

    // Continue with getopts.
    *opt_idx = idx - 1;
    return 0;
}

/**
 * Grab a string from an optarg.
 *
 * @param getopt_arg: optarg variable from getopt.
 * @param str:        string to which optarg will be copied.
 * @return:           error code, which is always 0 (no error).
 */
int grab_str(char *getopt_arg, char **str) {
    *str = static_cast<char *>(malloc(strlen(getopt_arg) + 1));
    strcpy(*str, getopt_arg);
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
