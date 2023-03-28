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

const std::map<unsigned int, const char *> ERRMAP = {
    //    0 -  199 general
    {ERR_NOERR, ""}, // Handled before accessing this map.
    {ERR_USAGE, ""}, // Handled before accessing this map.
    {ERR_INVALIDROOTFILE,
            "Input file is missing `.root` extension."},

    //  200 -  299 acc_corr
    {ERR_ACCCORR_NOEDGE,
            "Edges for the five binning variables should be specified."},
    {ERR_ACCCORR_BADEDGES,
            "All edges should have *at least* two values -- a minimum and a "
            "maximum."},
    {ERR_ACCCORR_NOGENFILE,
            "A generated ntuples file is required to obtain acceptance "
            "correction."},
    {ERR_ACCCORR_NOSIMFILE,
            "A simulation ntuples file is required to obtain acceptance "
            "correction."},
    {ERR_ACCCORR_WRONGGENFILE,
            "Generated ntuples file is not a valid root file."},
    {ERR_ACCCORR_BADGENFILE,
            "Generated ntuples file is badly formatted."},
    {ERR_ACCCORR_WRONGSIMFILE,
            "Simulation ntuples file is not a valid root file."},
    {ERR_ACCCORR_BADSIMFILE,
            "Simulated ntuples file is badly formatted."},
    {ERR_ACCCORR_OUTFILEEXISTS,
            "Output file already exists."},

    //  300 -  399 bank_containers
    //  400 -  499 constants
    //  500 -  599 draw_plots
    //  600 -  699 err_handler
    //  700 -  799 extract_sf
    //  800 -  899 file_handler
    //  900 -  999 hipo2root
    // 1000 - 1099 io_handler
    {ERR_IOHANDLER_INVALIDROOTFILE,
            "Root filename should finish with the `.root` extension."},
    {ERR_IOHANDLER_NOINPUTFILE,
            "Input root file doesn't exist."},

    // 1100 - 1199 make_ntuples
    // 1200 - 1299 particle
    // 1300 - 1399 utilities
    {ERR_UTILITIES_ANGLEOUTOFRANGE,
            "Invalid angle value. By convention, all angles should be between "
            "-180 (-pi) and 180 (pi)."}
};

unsigned int rge_errno = 0;

/**
 * Entry point to all error handling. Decides which handler should react.
 *
 * @return:
 *    * 0 : no error was found.
 *    * 1 : a user error was found.
 *    * 2 : a programmer error was found.
 */
int handle_err() {
    if (rge_errno == ERR_NOERR) return 0; // No error.
    if (rge_errno == ERR_USAGE) return 1; // Print usage().

    // Print error.
    if (ERRMAP.contains(rge_errno)) {
        fprintf(stderr, "%s\n\n", ERRMAP.at(rge_errno));
        return 1;
    }
    else {
        // Error number not implemented.
        fprintf(stderr, "rge_errno %d not implemented.\n\n", rge_errno);
        return 2;
    }
}
