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

#include "../lib/rge_filename_handler.h"

// --+ internal +---------------------------------------------------------------
int get_run_no(char *filename, int *run_no) {
    char run_no_str[7];

    // Find position of dot in string.
    char *dot_pos = strrchr(filename, '.');
    if (!dot_pos) {
        rge_errno = RGEERR_NODOTFILENAME;
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
        rge_errno = RGEERR_BADFILENAMEFORMAT;
        return 1; // Value not supported.
    }

    return 0;
}

int get_beam_energy(int run_no, double *beam_energy) {
    // Input file is from gemc, beam energy is encoded in run number.
    if (run_no / 1000 == 999) {
        *beam_energy = static_cast<double>(run_no % 1000) / 10;
        return 0;
    }

    switch (run_no) {
        case 11983:
            *beam_energy = RGE_BE11983;
            break;
        case 12016:
            *beam_energy = RGE_BE12016;
            break;
        case 12439:
            *beam_energy = RGE_BE12439;
            break;
        case 12933:
            *beam_energy = RGE_BE12933;
            break;
        default:
            *beam_energy = RGE_BE;
            break;
    }

    return 0;
}

int check_hipo_filename(char *filename) {
    if (!strstr(filename, ".hipo")) {
        rge_errno = RGEERR_INVALIDHIPOFILE;
        return 1;
    }
    if (access(filename, F_OK)) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }
    return 0;
}

// --+ library +----------------------------------------------------------------
int rge_check_root_filename(char *filename) {
    // Check that filename extension is correct.
    if (!strstr(filename, ".root")) {
        rge_errno = RGEERR_INVALIDROOTFILE;
        return 1;
    }

    // Check if file exists.
    if (access(filename, F_OK)) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }

    // All good.
    return 0;
}

int rge_handle_root_filename(char *filename, int *run_no, double *beam_energy) {
    if (rge_check_root_filename(filename))     return 1;
    if (get_run_no(filename, run_no))          return 1;
    if (get_beam_energy(*run_no, beam_energy)) return 1;

    return 0;
}

int rge_handle_root_filename(char *filename, int *run_no) {
    double dump = 0.;
    int err = rge_handle_root_filename(filename, run_no, &dump);

    // We don't care about missing beam energy here.
    if (rge_errno == RGEERR_UNIMPLEMENTEDBEAMENERGY) {
        rge_errno = RGEERR_NOERR;
        err = 0;
    }

    return err;
}

int rge_handle_hipo_filename(char *filename, int *run_no) {
    if (check_hipo_filename(filename)) return 1;
    if (get_run_no(filename, run_no))  return 1;
    return 0;
}
