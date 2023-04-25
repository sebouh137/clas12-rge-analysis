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

// C.
#include "libgen.h"
#include "limits.h"

// rge-analysis.
#include "../lib/rge_err_handler.h"
#include "../lib/rge_extract_sf.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_io_handler.h"

static const char *USAGE_MESSAGE =
"Usage: extract_sf [-hn:w:d:] infile\n"
" * -h         : show this message and exit.\n"
" * -n nevents : number of events\n"
" * -w workdir : location where output root files are to be stored. Default\n"
"                is root_io.\n"
" * -d datadir : location where sampling fraction files are stored. Default\n"
"                is data.\n"
" * infile     : input ROOT file. Expected file format: <text>run_no.root.\n\n"
"    Obtain the EC sampling fraction from an input file.\n";

/**
 * Handle arguments for make_ntuples using optarg. Error codes used are
 *     explained in the handle_err() function.
 */
static int handle_args(
        int argc, char **argv, char **in_filename, char **work_dir,
        char **data_dir, int *run_no, lint *nevn
) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                rge_errno = RGEERR_USAGE;
                return 1;
            case 'n':
                if (rge_process_nentries(nevn, optarg)) return 1;
                break;
            case 'w':
                *work_dir = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*work_dir, optarg);
                break;
            case 'd':
                *data_dir = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*data_dir, optarg);
                break;
            case 1:
                *in_filename = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*in_filename, optarg);
                break;
            default:
                rge_errno = RGEERR_BADOPTARGS;
                return 1;
        }
    }

    // Define workdir if undefined.
    char tmpfile[PATH_MAX];
    sprintf(tmpfile, "%s", argv[0]);
    if (*work_dir == NULL) {
        *work_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*data_dir, "%s/../data", dirname(tmpfile));
    }

    // Check positional argument.
    if (*in_filename == NULL) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }

    // Handle input filename.
    if (rge_handle_root_filename(*in_filename, run_no)) return 1;

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename = NULL;
    char *work_dir    = NULL;
    char *data_dir    = NULL;
    lint nevn         = -1;
    int run_no        = -1;

    int err = handle_args(
            argc, argv, &in_filename, &work_dir, &data_dir, &run_no, &nevn
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED && err == 0) {
        rge_extract_sf(in_filename, work_dir, data_dir, nevn, run_no);
    }

    // Free up memory.
    if (in_filename != NULL) free(in_filename);
    if (work_dir    != NULL) free(work_dir);
    if (data_dir    != NULL) free(data_dir);

    // Return errcode.
    return rge_print_usage(USAGE_MESSAGE);
}
