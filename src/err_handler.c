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

// Map linking every error number with an explanation script for the user.
// NOTE. std::map isn't smart enough to detect if two keys have the same
//       value, so be *very careful* when adding rge_errnos. An invisibile error
//       arises if two keys have the same value!
const std::map<unsigned int, const char *> ERRMAP = {
    // Basic functionalities.
    {ERR_NOERR, ""}, // Handled before accessing this map.
    {ERR_USAGE, ""}, // Handled before accessing this map.
    {ERR_UNDEFINED,
            "rge_errno = ERR_UNDEFINED. Something is wrong."},

    // Argument errors.
    {ERR_BADOPTARGS,
            "Bad usage of optional arguments."},
    {ERR_INVALIDENTRIES,
            "Number of entries is invalid. Input a valid number after -n"},
    {ERR_NENTRIESLARGE,
            "Number of entries is too large. Input a number smaller than "
            "LONG_MAX."},
    {ERR_NENTRIESNEGATIVE,
            "Number of entries should be greater than 0."},
    {ERR_NOEDGE,
            "Edges for the five binning variables should be specified."},
    {ERR_BADEDGES,
            "All edges should have *at least* two values -- a minimum and a "
            "maximum."},
    {ERR_INVALIDFMTNLAYERS,
            "Number of FMT layers is invalid. fmt_nlayers should be at least "
            "FMTMINLAYERS and at most FMTNLAYERS."},
    {ERR_INVALIDACCEPTANCEOPT,
            "Option -A is only valid if an acceptance correction file is "
            "specified using -a."},

    // File errors.
    {ERR_NOINPUTFILE,
            "Input file doesn't exist."},
    {ERR_NOSAMPFRACFILE,
            // NOTE. A smoother behaviour here would be that the program calls
            //       extract_sf itself.
            "No sampling fraction file is available for this run number."},
    {ERR_NOACCCORRFILE,
            "Failed to access acceptance correction file."},
    {ERR_NOGENFILE,
            "A generated ntuples file is required to obtain acceptance "
            "correction."},
    {ERR_NOSIMFILE,
            "A simulation ntuples file is required to obtain acceptance "
            "correction."},
    {ERR_NODOTFILENAME,
            "Couldn't find a `.` in filename. Provide a valid file."},
    {ERR_BADFILENAMEFORMAT,
            "Couldn't extract run number from filename. Follow filename "
            "conventions specified in usage()."},
    {ERR_INVALIDROOTFILE,
            "Root filename should finish with the `.root` extension."},
    {ERR_INVALIDHIPOFILE,
            "Hipo filename should finish with the `.hipo` extension."},
    {ERR_BADINPUTFILE,
            "Failed to open input file."},
    {ERR_BADGENFILE,
            "Failed to open generated ntuples file."},
    {ERR_BADSIMFILE,
            "Failed to open simulated ntuples file."},
    {ERR_BADROOTFILE,
            "Couldn't extract tree/ntuple with TREENAMEDATA from root file."},
    {ERR_WRONGGENFILE,
            "Generated ntuples file is not a valid root file."},
    {ERR_WRONGSIMFILE,
            "Simulation ntuples file is not a valid root file."},
    {ERR_OUTFILEEXISTS,
            "Output file already exists."},
    {ERR_OUTPUTROOTFAILED,
            "Failed to create output root file."},
    {ERR_OUTPUTTEXTFAILED,
            "Failed to create output text file."},

    // Detector errors.
    {ERR_INVALIDCALLAYER,
            "Invalid layer in the calorimeter bank. Check bank integrity."},
    {ERR_INVALIDCALSECTOR,
            "Invalid sector in the calorimeter bank. Check bank integrity."},
    {ERR_INVALIDCHERENKOVID,
            "Invalid detector ID in the cherenkov bank. Check bank integrity."},
    {ERR_NOFMTBANK,
            "FMT::Tracks bank not found in input. No FMT analysis is available "
            "for this input file."},

    // Program errors.
    {ERR_UNIMPLEMENTEDBEAMENERGY,
            "No beam energy available in constants for run number. Add it from "
            "RCDB."},
    {ERR_2DACCEPTANCEPLOT,
            "2D acceptance correction plots haven't been implemented yet."},
    {ERR_WRONGACCVARS,
            "Erroneous variables in the ACC_VX arr. Check constants."},

    // Miscellaneous.
    {ERR_ANGLEOUTOFRANGE,
            "Invalid angle value. By convention, all angles should be between "
            "-180 (-pi) and 180 (pi)."},
    {ERR_NOACCDATA,
            "There's no acceptance correction data for the selected PID. Run "
            "acc_corr and define a binning scheme to use this feature."}
};

// Error number. Initially defined to ERR_UNDEFINED to check if the program
//     ends abruptly without setting an error number.
// NOTE. To check for undefined errors, all run() functions in the code should
//       have a line with `rge_errno = ERR_NOERR;` before returning 0.
unsigned int rge_errno = ERR_UNDEFINED;

/**
 * Entry point to all error handling. Decides how to react to errno.
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
        fprintf(stderr, "\n%s\n", ERRMAP.at(rge_errno));
        return 1;
    }
    else {
        // Error number not implemented.
        fprintf(stderr, "rge_errno %d not implemented.\n\n", rge_errno);
        return 2;
    }
}
