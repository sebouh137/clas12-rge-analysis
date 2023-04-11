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

#include "../lib/rge_err_handler.h"

// --+ internal +---------------------------------------------------------------
/**
 * Map linking every error number with an explanation script for the user.
 *
 * NOTE. std::map isn't smart enough to detect if two keys have the same
 *       value, so be *very careful* when adding rge_errnos. An invisibile error
 *       arises if two keys have the same value.
 */
static const std::map<unsigned int, const char *> ERRMAP = {
    // Basic functionalities.
    {RGEERR_NOERR, ""}, // Handled before accessing this map.
    {RGEERR_USAGE, ""}, // Handled before accessing this map.
    {RGEERR_UNDEFINED,
            "rge_errno = RGEERR_UNDEFINED. Something is wrong."},

    // Argument errors.
    {RGEERR_BADOPTARGS,
            "Bad usage of optional arguments."},
    {RGEERR_INVALIDENTRIES,
            "Number of entries is invalid. Input a valid number after -n"},
    {RGEERR_NENTRIESLARGE,
            "Number of entries is too large. Input a number smaller than "
            "LONG_MAX."},
    {RGEERR_NENTRIESNEGATIVE,
            "Number of entries should be greater than 0."},
    {RGEERR_NOEDGE,
            "Edges for the five binning variables should be specified."},
    {RGEERR_BADEDGES,
            "All edges should have *at least* two values -- a minimum and a "
            "maximum."},
    {RGEERR_INVALIDFMTNLAYERS,
            "Number of FMT layers is invalid. fmt_nlayers should be at least "
            "FMTMINLAYERS and at most FMTNLAYERS."},
    {RGEERR_INVALIDACCEPTANCEOPT,
            "Option -A is only valid if an acceptance correction file is "
            "specified using -a."},

    // File errors.
    {RGEERR_NOINPUTFILE,
            "Input file doesn't exist."},
    {RGEERR_NOSAMPFRACFILE,
            // NOTE. A smoother behaviour here would be that the program calls
            //       extract_sf itself.
            "No sampling fraction file is available for this run number."},
    {RGEERR_NOACCCORRFILE,
            "Failed to access acceptance correction file."},
    {RGEERR_NOGENFILE,
            "A generated ntuples file is required to obtain acceptance "
            "correction."},
    {RGEERR_NOSIMFILE,
            "A simulation ntuples file is required to obtain acceptance "
            "correction."},
    {RGEERR_NODOTFILENAME,
            "Couldn't find a `.` in filename. Provide a valid file."},
    {RGEERR_BADFILENAMEFORMAT,
            "Couldn't extract run number from filename. Follow filename "
            "conventions specified in usage()."},
    {RGEERR_INVALIDROOTFILE,
            "Root filename should finish with the `.root` extension."},
    {RGEERR_INVALIDHIPOFILE,
            "Hipo filename should finish with the `.hipo` extension."},
    {RGEERR_BADINPUTFILE,
            "Failed to open input file."},
    {RGEERR_BADGENFILE,
            "Failed to open generated ntuples file."},
    {RGEERR_BADSIMFILE,
            "Failed to open simulated ntuples file."},
    {RGEERR_BADROOTFILE,
            "Couldn't extract tree/ntuple with TREENAMEDATA from root file."},
    {RGEERR_WRONGGENFILE,
            "Generated ntuples file is not a valid root file."},
    {RGEERR_WRONGSIMFILE,
            "Simulation ntuples file is not a valid root file."},
    {RGEERR_OUTFILEEXISTS,
            "Output file already exists."},
    {RGEERR_OUTPUTROOTFAILED,
            "Failed to create output root file."},
    {RGEERR_OUTPUTTEXTFAILED,
            "Failed to create output text file."},

    // Detector errors.
    {RGEERR_INVALIDCALLAYER,
            "Invalid layer in the calorimeter bank. Check bank integrity."},
    {RGEERR_INVALIDCALSECTOR,
            "Invalid sector in the calorimeter bank. Check bank integrity."},
    {RGEERR_INVALIDCHERENKOVID,
            "Invalid detector ID in the cherenkov bank. Check bank integrity."},
    {RGEERR_NOFMTBANK,
            "FMT::Tracks bank not found in input. No FMT analysis is available "
            "for this input file."},

    // Program errors.
    {RGEERR_UNIMPLEMENTEDBEAMENERGY,
            "No beam energy available in constants for run number. Add it from "
            "RCDB."},
    {RGEERR_2DACCEPTANCEPLOT,
            "2D acceptance correction plots haven't been implemented yet."},
    {RGEERR_WRONGACCVARS,
            "Erroneous variables in the ACC_VX arr. Check constants."},

    // Particle errors.
    {RGEERR_PIDNOTFOUND,
            "Program looked for an unavailable PID. Check that all requested "
            "PIDs are in PID_MAP in pid_utils file."},
    {RGEERR_UNSUPPORTEDPID,
            "Program tried to identify a particle with an unsupported PID. "
            "Check that all hypotheses are implemented in match_pid function in"
            " rge_particle."},

    // Miscellaneous.
    {RGEERR_ANGLEOUTOFRANGE,
            "Invalid angle value. By convention, all angles should be between "
            "-180 (-pi) and 180 (pi)."},
    {RGEERR_NOACCDATA,
            "There's no acceptance correction data for the selected PID. Run "
            "acc_corr and define a binning scheme to use this feature."}
};

int handle_err() {
    if (rge_errno == RGEERR_NOERR) return 0; // No error.
    if (rge_errno == RGEERR_USAGE) return 1; // Print usage().

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

// --+ library +----------------------------------------------------------------
unsigned int rge_errno = RGEERR_UNDEFINED;

int rge_print_usage(const char *msg) {
    int err = handle_err();
    if (err == 1) fprintf(stderr, "\n%s\n", msg);
    return err;
}
