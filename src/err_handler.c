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

#include "../lib/err_handler.h"

int rge_errno = 0;

/**
 * Entry point to all error handling. Decides which handler should act. Returns
 *     0 if no error was found, 1 if a user error was fou
 *
 * @return:
 *    * 0 : no error was found.
 *    * 1 : a user error was found.
 *    * 2 : a programmer error was found.
 */
int handle_err() {
    if (rge_errno == ERR_NOERR) return 0; // No error.
    if (rge_errno == ERR_USAGE) return 1; // Print usage().

    // Store error type to tell program what to print.
    int error_type = -1;

    // Find range of error.
    if (   0 <= rge_errno && rge_errno <  200) error_type = general_err();
    if ( 200 <= rge_errno && rge_errno <  300) error_type = acc_corr_err();
    if (1000 <= rge_errno && rge_errno < 1100) error_type = io_handler_err();
    if (1300 <= rge_errno && rge_errno < 1400) error_type = utilities_err();

    if (error_type == 0) return 0; // All good.
    if (error_type == 1) return 1; // Print usage().
    if (error_type == 2) return 2; // Programmer error.

    // Uninplemented rge_errno error.
    fprintf(stderr,
            "Non-valid error_type! rge_errno = %d is not supported.\n",
            rge_errno
    );
    return 2;
}

int general_err() {
    // Specify program throwing error.
    fprintf(stderr, "general error:\n  ");

    // Out of range. Something was programmed wrong.
    if (0 > rge_errno || rge_errno >= 200) {
        fprintf(stderr,
                "Invalid error code -- value should be between 0 and 200."
        );
        return 2;
    }

    switch (rge_errno) {
        case ERR_INVALIDROOTFILE:
            fprintf(stderr,
                    "  Input file is missing `.root` extension."
            );
            break;
        default:
            fprintf(stderr,
                "rge_errno not found! Please correct this error before "
                "making a pull request."
            );
            return 2;
    }

    return 1;
}

int acc_corr_err() {
    // Specify program throwing error.
    fprintf(stderr, "acc_corr error:\n  ");

    // Out of range. Something was programmed wrong.
    if (200 > rge_errno || rge_errno >= 300) {
        fprintf(stderr,
                "Invalid error code -- value should be between 200 and 300."
        );
        return 2;
    }

    // Check acc_corr error.
    switch (rge_errno) {
        case ERR_ACCCORR_NOEDGE:
            fprintf(stderr,
                    "Edges for the five binning variables should be "
                    "specified."
            );
            break;
        case ERR_ACCCORR_BADEDGES:
            fprintf(stderr,
                    "All edges should have *at least* two values -- a minimum"
                    " and a maximum."
            );
            break;
        case ERR_ACCCORR_NOGENFILE:
            fprintf(stderr,
                    "A generated ntuples file is required to obtain acceptance "
                    "correction."
            );
            break;
        case ERR_ACCCORR_NOSIMFILE:
            fprintf(stderr,
                    "A simulation ntuples file is required to obtain acceptance"
                    " correction."
            );
            break;
        case ERR_ACCCORR_WRONGGENFILE:
            fprintf(stderr,
                    "Generated ntuples file is not a valid root file."
            );
            break;
        case ERR_ACCCORR_BADGENFILE:
            fprintf(stderr,
                    "Generated ntuples file is badly formatted."
            );
            break;
        case ERR_ACCCORR_WRONGSIMFILE:
            fprintf(stderr,
                    "Simulation ntuples file is not a valid root file."
            );
            break;
        case ERR_ACCCORR_OUTFILEEXISTS:
            fprintf(stderr,
                    "Output file already exists."
            );
            break;
        default:
            fprintf(stderr,
                    "rge_errno not found! Please correct this error before "
                    "making a pull request."
            );
            return 2;
    }

    return 1;
}

int io_handler_err() {
    // Specify program throwing error.
    fprintf(stderr, "io_handler error:\n  ");

    // Out of range. Something was programmed wrong.
    if (1000 > rge_errno || rge_errno >= 1100) {
        fprintf(stderr,
                "Invalid error code -- value should be between 1000 and 1100."
        );
        return 2;
    }

    // Check io_handler error.
    switch (rge_errno) {
        case ERR_IOHANDLER_INVALIDROOTFILE:
            fprintf(stderr,
                    "Root filename should finish with the `.root` extension."
            );
            break;
        case ERR_IOHANDLER_NOINPUTFILE:
            fprintf(stderr,
                    "Input root file doesn't exist."
            );
            break;
        default:
            fprintf(stderr,
                    "rge_errno not found! Please correct this error before "
                    "making a pull request."
            );
            return 2;
    }

    return 1;
}

int utilities_err() {
    // Specify program throwing error.
    fprintf(stderr, "utilities error:\n  ");

    // Out of range. Something was programmed wrong.
    if (1300 > rge_errno || rge_errno >= 1400) {
        fprintf(stderr,
                "Invalid error code -- value should be betwenn 1300 and 1400."
        );
        return 2;
    }

    // Check utilities error.
    switch (rge_errno) {
        case ERR_UTILITIES_ANGLEOUTOFRANGE:
            fprintf(stderr,
                    "Invalid angle value. By convention, all angles should be "
                    "between -180 (-pi) and 180 (pi)."
            );
            break;
        default:
            fprintf(stderr,
                "rge_errno not found! Please correct this error before "
                "making a pull request."
            );
            return 2;
    }

    return 1;
}
