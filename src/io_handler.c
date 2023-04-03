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

#include "../lib/io_handler.h"

// TODO. Separate this file into io_handler, file_handler, and progress_bar.

/**
 * Get run number from a filename, assuming the filename is in format
 *     <text><run_no>.<extension>. Write run number to *run_no, and return an
 *     error code.
 *
 * @param filename: filename to be processed.
 * @param run_no:   pointer to the int where the run number is written.
 * @return          error code:
 *                    * 0: everything went fine.
 *                    * 1: couldn't find dot position in filename.
 *                    * 2: strtoul failed -- couldn't find run number.
 */
int get_run_no(char *filename, int *run_no) {
    char run_no_str[7];

    // Find position of dot in string.
    char *dot_pos = strrchr(filename, '.');
    if (!dot_pos) {
        rge_errno = ERR_NODOTFILENAME;
        return 1; // Couldn't find dot.
    }

    // Copy final 6 chars from string -- should be the run number!
    strncpy(run_no_str, dot_pos - 6, 6);
    run_no_str[6] = '\0';

    // Run strtoul, giving an error upon failure.
    char *eptr;
    errno = 0;
    *run_no = strtoul(run_no_str, &eptr, 10);
    if (errno == EINVAL || errno == ERANGE) {
        rge_errno = ERR_BADFILENAMEFORMAT;
        return 1; // Value not supported.
    }

    return 0;
}

/**
 * Match a run number to a beam energy. The beam energy for different runs is
 *     available in the constants.
 *
 * @param run_no:      run number for which the beam energy is to be found.
 * @param beam_energy: pointer to the double where the beam energy will be
 *                     saved.
 * @return             an error code.
 */
int get_beam_energy(int run_no, double *beam_energy) {
    // Input file is from gemc, beam energy is encoded in run number.
    if (run_no / 1000 == 999) {
        *beam_energy = static_cast<double>(run_no % 1000) / 10;
        return 0;
    }

    // TODO. This should be taken directly from RCDB.
    switch (run_no) {
        case  11983:
            *beam_energy = BE11983;
            break;
        case  12016:
            *beam_energy = BE12016;
            break;
        case  12439:
            *beam_energy = BE12439;
            break;
        case  12933:
            *beam_energy = BE12933;
            break;
        default:
            rge_errno = ERR_UNIMPLEMENTEDBEAMENERGY;
            return 1;
    }

    return 0;
}

/**
 * Check if a root filename is valid.
 *
 * @param filename: filename to be processed.
 * @return          0 if successful, 1 otherwise.
 */
int check_root_filename(char *filename) {
    // Check that filename extension is correct.
    if (!strstr(filename, ".root")) {
        rge_errno = ERR_INVALIDROOTFILE;
        return 1;
    }

    // Check if file exists.
    if (access(filename, F_OK)) {
        rge_errno = ERR_NOINPUTFILE;
        return 1;
    }

    // All good.
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
 * @return             error code.
 */
int handle_root_filename(char *filename, int *run_no, double *beam_energy) {
    if (check_root_filename(filename))         return 1;
    if (get_run_no(filename, run_no))          return 1;
    if (get_beam_energy(*run_no, beam_energy)) return 1;

    return 0;
}

/** Run handle_root_filename() without writing beam_energy. */
int handle_root_filename(char *filename, int *run_no) {
    double dump = 0.;
    int err = handle_root_filename(filename, run_no, &dump);

    // We don't care about missing beam energy here.
    if (rge_errno == ERR_UNIMPLEMENTEDBEAMENERGY) {
        rge_errno = ERR_NOERR;
        err = 0;
    }

    return err;
}

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
        rge_errno = ERR_INVALIDFMTNLAYERS;
        return 1;
    }
    return 0;
}

/* Run strtol on arg to get number of entries. */
int process_nentries(long int *nentries, char *arg) {
    int err = run_strtol(nentries, arg);
    if (err == 1) {
        rge_errno = ERR_INVALIDENTRIES;
        return 1;
    }
    if (err == 2) {
        rge_errno = ERR_NENTRIESLARGE;
        return 1;
    }
    if (*nentries <= 0) {
        rge_errno = ERR_NENTRIESNEGATIVE;
        return 1;
    }

    return 0;
}

/**
 * Check if a hipo filename is valid.
 *
 * @param filename: filename to be processed.
 * @return          error code.
 */
int check_hipo_filename(char *filename) {
    if (!strstr(filename, ".hipo")) {
        rge_errno = ERR_INVALIDHIPOFILE;
        return 1;
    }
    if (access(filename, F_OK)) {
        rge_errno = ERR_NOINPUTFILE;
        return 1;
    }
    return 0;
}

/**
 * Handle a hipo filename, checking its validity, file existence, and grabbing
 *     the run number from it.
 *
 * @param filename: filename to be processed.
 * @param run_no:   pointer to the int where the run number will be written.
 * @return          error code.
 */
int handle_hipo_filename(char *filename, int *run_no) {
    if (check_hipo_filename(filename)) return 1;
    if (get_run_no(filename, run_no))  return 1;
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
 * Update a progress bar counting the number of events processed. Length of the
 *     bar is defined by the `length` variable defined at the first line of the
 *     function.
 *
 * @param nevn:        total number of events.
 * @param evn:         number of the event being processed.
 * @param evnsplitter: internal state variable used across executions, should be
 *                     set to 0 for first iteration.
 * @param divcntr:     internal state variable used across executions, should be
 *                     set to 0 for first iteration.
 * @return:            0 if no change was made, 1 otherwise, and 2 if bar is
 *                     full.
 */
int update_progress_bar(int nevn, int evn, int *evnsplitter, int *divcntr) {
    double length = 50.; // Length of the progress bar.

    // Only update if necessary.
    if (evn == nevn-1) {
        printf("\n");
        return 2;
    }
    if (*evnsplitter == -1 || evn < *evnsplitter) return 0;

    // Clear line if a previous bar has been printed.
    if (evn != 0) printf("\33[2K\r");

    // Print progress bar.
    printf("[");
    for (int i = 0; i <= length; ++i) {
        if (i <= (length/100.) * (*divcntr)) printf("=");
        else                                 printf(" ");
    }
    printf("] %2d%%", *divcntr);
    fflush(stdout);

    // Update divcntr and evnsplitter.
    (*divcntr)++;
    if (*divcntr <= 100) {
        *evnsplitter = (nevn/100) * (*divcntr);
        return 1;
    }

    *evnsplitter = -1;
    printf("\n");
    return 2;
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
int grab_multiarg(int argc, char **argv, int *opt_idx, long unsigned int *size,
        double **arr) {
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
