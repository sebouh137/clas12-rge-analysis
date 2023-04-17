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
#include <libgen.h>

// ROOT.
#include <TFile.h>
#include <TNtuple.h>
#include <TROOT.h>

// rge-analysis.
#include "../lib/bank_containers.h"
#include "../lib/constants.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_hipo_bank.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_particle.h"
#include "../lib/rge_progress.h"

static const char *USAGE_MESSAGE =
"Usage: make_ntuples [-hDf:n:w:d:] infile\n"
" * -h         : show this message and exit.\n"
" * -D         : activate debug mode.\n"
" * -f fmtlyrs : define how many FMT layers should the track have hit.\n"
"                Options are 0 (tracked only by DC), 2, and 3. If set to\n"
"                something other than 0 and there is no FMT::Tracks bank in\n"
"                the input file, the program will crash. Default is 0.\n"
" * -n nevents : number of events.\n"
" * -w workdir : location where output root files are to be stored. Default\n"
"                is root_io.\n"
" * -d datadir : location where sampling fraction files are. Default is data.\n"
" * infile     : input ROOT file. Expected file format: <text>run_no.root`.\n\n"
"    Generate ntuples relevant to SIDIS analysis based on the reconstructed\n"
"    variables from CLAS12 data.\n";

/** Detector IDs from CLAS12 reconstruction. */
static const uint FTOF_ID    = 12;
static const uint HTCC_ID    = 15;
static const uint LTCC_ID    = 16;

/** FTOF layer IDs from CLAS12 reconstruction. */
static const uint FTOF1A_LYR =  1;
static const uint FTOF1B_LYR =  2;
static const uint FTOF2_LYR  =  3;

/**
 * Find and return the most precise time of flight (TOF). Both the Forward Time
 *     Of Flight (FTOF) detectors and the Electronic Calorimeter (EC) can
 *     measure TOF, but they have different precisions. So, in order to get the
 *     most accurate measurement possible, this function returns the TOF
 *     measured by the most accurate detector for a given particle.
 *
 * In order of decreasing precision, the list of detectors are:
 *     FTOF1B > FTOF1A > FTOF2 > PCAL > ECIN > ECOU.
 *
 * @param scintillator: instance of the Scintillator class.
 * @param calorimeter:  instance of the Calorimeter class.
 * @param pindex:       particle index of the particle we're studying.
 * @return:             the most accurate TOF available in the scintillator and
 *                      calorimeter banks.
 */
static double get_tof(
        Scintillator scintillator, Calorimeter calorimeter, uint pindex
) {
    int    most_precise_lyr = 0;
    double tof              = INFINITY;
    for (uint i = 0; i < scintillator.pindex->size(); ++i) {
        // Filter out incorrect pindex and hits not from FTOF.
        if (
                static_cast<uint>(scintillator.pindex->at(i)) != pindex ||
                scintillator.detector->at(i) != FTOF_ID
        ) {
            continue;
        }

        // Check FTOF 1B (most precise FTOF layer).
        if (scintillator.layer->at(i) == FTOF1B_LYR) {
            most_precise_lyr = FTOF1B_LYR;
            tof = scintillator.time->at(i);
            break; // Things won't get better than this.
        }

        // Check FTOF 1A.
        else if (scintillator.layer->at(i) == FTOF1A_LYR) {
            if (most_precise_lyr == FTOF1A_LYR) continue;
            most_precise_lyr = FTOF1A_LYR;
            tof = scintillator.time->at(i);
        }

        // Check FTOF 2.
        else if (scintillator.layer->at(i) == FTOF2_LYR) {
            // We already have a similar or better hit.
            if (most_precise_lyr != 0) continue;
            most_precise_lyr = FTOF2_LYR;
            tof = scintillator.time->at(i);
        }
    }
    if (most_precise_lyr != 0) return tof;

    // If no hits from FTOF were found, try to find TOF from calorimeters.
    for (uint i = 0; i < calorimeter.pindex->size(); ++i) {
        // Filter out incorrect pindex.
        if (static_cast<uint>(calorimeter.pindex->at(i)) != pindex) {
            continue;
        }

        // Check PCAL (Calorimeter with the most precise TOF).
        if (calorimeter.layer->at(i) == PCAL_LYR) {
            most_precise_lyr = 10 + PCAL_LYR;
            tof = calorimeter.time->at(i);
            break; // Things won't get better than this.
        }

        // Check ECIN.
        else if (calorimeter.layer->at(i) == ECIN_LYR) {
            if (most_precise_lyr == 10 + ECIN_LYR) continue;
            most_precise_lyr = 10 + ECIN_LYR;
            tof = calorimeter.time->at(i);
        }

        // Check ECOU.
        else if (calorimeter.layer->at(i) == ECOU_LYR) {
            if (most_precise_lyr != 0) continue;
            most_precise_lyr = 10 + ECOU_LYR;
            tof = calorimeter.time->at(i);
        }
    }

    return tof;
}

/**
 * Get deposited energy for particle with pindex from PCAL, ECIN, and ECOU.
 *
 * @param calorimeter: instance of the Calorimeter class.
 * @param pindex:      particle index of the particle we're studying
 * @param energy_PCAL: pointer to double to which we'll write the PCAL energy.
 * @param energy_ECIN: pointer to double to which we'll write the ECIN energy.
 * @param energy_ECOU: pointer to double to which we'll write the ECOU energy.
 * @return             error code. 0 if successful, 1 otherwise. The function
 *                     only returns 1 if there's an invalid layer in the
 *                     Calorimeter instance, suggesting corruption or a change
 *                     in the REC::Calorimeter bank.
 */
static int get_deposited_energy(
        Calorimeter calorimeter, uint pindex, double *energy_PCAL,
        double *energy_ECIN, double *energy_ECOU
) {
    *energy_PCAL = 0;
    *energy_ECIN = 0;
    *energy_ECOU = 0;

    for (uint i = 0; i < calorimeter.pindex->size(); ++i) {
        if (static_cast<uint>(calorimeter.pindex->at(i)) != pindex) {
            continue;
        }
        int lyr = static_cast<int>(calorimeter.layer->at(i));

        if      (lyr == PCAL_LYR) *energy_PCAL += calorimeter.energy->at(i);
        else if (lyr == ECIN_LYR) *energy_ECIN += calorimeter.energy->at(i);
        else if (lyr == ECOU_LYR) *energy_ECOU += calorimeter.energy->at(i);
        else {
            rge_errno = RGEERR_INVALIDCALLAYER;
            return 1;
        }
    }

    return 0;
}

/**
 * Count number of photoelectrons deposited on HTCC and LTCC detectors.
 *
 * @param cherenkov: rge_hipobank struct with Cherenkov's data.
 * @param pindex:    particle index of the particle we're studying.
 * @param nphe_HTCC: pointer to int where we'll write the number of
 *                   photoelectrons deposited on HTCC.
 * @param nphe_LTCC: pointer to int where we'll write the number of
 *                   photoelectrons deposited on LTCC.
 * @return           error code. 0 if successful, 1 otherwise. The function only
 *                   returns 1 if there's an invalid detector ID in the
 *                   Cherenkov instance, suggesting data corruption or a change
 *                   in the REC::Cherenkov bank.
 */
static int count_photoelectrons(
        rge_hipobank *cherenkov, uint pindex, int *nphe_HTCC, int *nphe_LTCC
) {
    *nphe_HTCC = 0;
    *nphe_LTCC = 0;

    for (uint i = 0; i < rge_get_size(cherenkov, "pindex"); ++i) {
        if (
                static_cast<uint>(rge_get_entry(cherenkov, "pindex", i))
                != pindex
        ) {
            continue;
        }

        int detector = static_cast<int>(rge_get_entry(cherenkov, "detector",i));
        int nphe     = static_cast<int>(rge_get_entry(cherenkov, "nphe",    i));
        if      (detector == HTCC_ID) *nphe_HTCC += nphe;
        else if (detector == LTCC_ID) *nphe_LTCC += nphe;
        else {
            rge_errno = RGEERR_INVALIDCHERENKOVID;
            return 1;
        }
    }

    return 0;
}

/** run() function of the program. Check USAGE_MESSAGE for details. */
static int run(
        char *filename_in, char *work_dir, char *data_dir, bool debug,
        lint fmt_nlayers, lint n_events, int run_no, double energy_beam
) {
    // Get sampling fraction.
    char sampling_fraction_file[PATH_MAX];
    sprintf(sampling_fraction_file, "%s/sf_params_%06d.txt", data_dir, run_no);
    double sampling_fraction_params[NSECTORS][SF_NPARAMS][2];
    if (rge_get_sf_params(sampling_fraction_file, sampling_fraction_params)) {
        return 1;
    }

    // Access input file.
    TFile *file_in  = TFile::Open(filename_in, "READ");
    if (!file_in || file_in->IsZombie()) {
        rge_errno = RGEERR_BADINPUTFILE;
        return 1;
    }

    // If fmt_nlayers != 0, check that FMT::Tracks bank exists.
    if (fmt_nlayers != 0 && file_in->GetListOfKeys()->Contains(RGE_FMTTRACKS)) {
        rge_errno = RGEERR_NOFMTBANK;
        return 1;
    }

    // Return to top directory (weird root stuff).
    gROOT->cd();

    // Generate lists of variables.
    TString vars_string("");
    for (int var_i = 0; var_i < VAR_LIST_SIZE; ++var_i) {
        vars_string.Append(Form("%s", S_VAR_LIST[var_i]));
        if (var_i != VAR_LIST_SIZE-1) vars_string.Append(":");
    }

    // Create TTree and TNTuples.
    TTree *tree_in = file_in->Get<TTree>(RGE_TREENAMEDATA);
    if (tree_in == NULL) {
        rge_errno = RGEERR_BADROOTFILE;
        return 1;
    }
    TNtuple *tree_out;
    tree_out = new TNtuple(RGE_TREENAMEDATA, RGE_TREENAMEDATA, vars_string);

    // Change n_events to number of entries if it is equal to -1 or invalid.
    if (n_events == -1 || n_events > tree_in->GetEntries()) {
        n_events = tree_in->GetEntries();
    }

    // Associate banks to TTree.
    rge_hipobank bpart = rge_hipobank_init(RGE_RECPARTICLE,  tree_in);
    rge_hipobank btrk  = rge_hipobank_init(RGE_RECTRACK,     tree_in);
    Calorimeter  bank_cal   (tree_in);
    rge_hipobank bchkv = rge_hipobank_init(RGE_RECCHERENKOV, tree_in);
    Scintillator bank_sci   (tree_in);
    FMT_Tracks   bank_trk_fmt;
    if (fmt_nlayers != 0) bank_trk_fmt.link_tree(tree_in);

    // Iterate through input file. Each TTree entry is one event.
    printf("Processing %ld events from %s.\n", n_events, filename_in);

    // Prepare fancy progress bar.
    rge_pbar_set_nentries(n_events);

    // Particle counters.
    int trigger_counter  = 0;
    int particle_counter = 0;

    // Loop through events in input file.
    for (lint event = 0; event < n_events; ++event) {
        // Print fancy progress bar.
        if (!debug) {
            rge_pbar_update(event);
        }

        // Get entries from input file.
        rge_get_entries(&bpart, tree_in, event);
        rge_get_entries(&btrk,  tree_in, event);
        bank_cal   .get_entries(tree_in, event);
        rge_get_entries(&bchkv, tree_in, event);
        bank_sci   .get_entries(tree_in, event);
        if (fmt_nlayers != 0) bank_trk_fmt.get_entries(tree_in, event);

        // Filter events without the necessary banks.
        if (
                rge_get_size(&bpart, "vz")      == 0 ||
                rge_get_size(&btrk,  "pindex")  == 0
        ) {
            continue;
        }

        // Check existence of trigger electron
        rge_particle part_trigger;
        bool trigger_exist  = false;
        uint trigger_pos    = UINT_MAX;
        uint trigger_pindex = UINT_MAX;
        double trigger_tof  = -1.;
        for (uint pos = 0; pos < rge_get_size(&btrk, "index"); ++pos) {
            uint pindex = static_cast<uint>(rge_get_entry(&btrk,"pindex",pos));

            // Get reconstructed particle from DC and from FMT.
            part_trigger = rge_particle_init(
                    &bpart, &btrk, &bank_trk_fmt, pos, fmt_nlayers
            );

            // Skip particle if it doesn't fit requirements.
            if (!part_trigger.is_valid) continue;

            // Get energy deposited in calorimeters.
            double energy_PCAL, energy_ECIN, energy_ECOU;
            if (get_deposited_energy(
                    bank_cal, pindex, &energy_PCAL, &energy_ECIN, &energy_ECOU
            )) return 1;

            // Get number of photoelectrons from Cherenkov counters.
            int nphe_HTCC, nphe_LTCC;
            if (count_photoelectrons(&bchkv, pindex, &nphe_HTCC, &nphe_LTCC))
                return 1;

            // Get time of flight from scintillators or calorimeters.
            double tof = get_tof(bank_sci, bank_cal, pindex);

            // Get miscellaneous data.
            int status  = rge_get_entry(&bpart, "status", pindex);
            double chi2 = rge_get_entry(&btrk,  "chi2",   pos);
            double ndf  = rge_get_entry(&btrk,  "NDF",    pos);

            // Assign PID.
            if (rge_set_pid(
                    &part_trigger, rge_get_entry(&bpart, "pid", pindex),
                    status, energy_PCAL+energy_ECIN+energy_ECOU, energy_PCAL,
                    nphe_HTCC, nphe_LTCC,
                    sampling_fraction_params[static_cast<uint>(
                            rge_get_entry(&btrk,"sector",pos)
                    )]
            )) return 1;

            // Skip particle if its not the trigger electron.
            if (!part_trigger.is_trigger_electron) continue;

            // Fill TNtuple with trigger electron information.
            Float_t arr[VAR_LIST_SIZE];
            if (rge_fill_ntuples_arr(
                    arr, part_trigger, part_trigger, run_no, event, status,
                    energy_beam, chi2, ndf, energy_PCAL, energy_ECIN,
                    energy_ECOU, tof, tof, nphe_LTCC, nphe_HTCC
            )) return 1;

            tree_out->Fill(arr);

            // Fill out trigger electron data and end loop.
            trigger_exist  = true;
            trigger_pindex = pindex;
            trigger_pos    = pos;
            trigger_tof    = tof;
            break;
        }

        // Skip events without a trigger electron.
        if (!trigger_exist) continue;
        ++trigger_counter;

        // Processing particles.
        for (uint pos = 0; pos < rge_get_size(&btrk, "index"); ++pos) {
            // Currently pindex is always equal to pos, but this is not a given
            //     in the future of the reconstruction software development.
            uint pindex = static_cast<uint>(rge_get_entry(&btrk,"pindex",pos));

            // Avoid double-counting the trigger electron.
            if (trigger_pindex == pindex && trigger_pos == pos) {
                continue;
            }

            // Get reconstructed particle from DC and from FMT.
            rge_particle part = rge_particle_init(
                &bpart, &btrk, &bank_trk_fmt, pos, fmt_nlayers
            );

            // Skip particle if it doesn't fit requirements.
            if (!part.is_valid) continue;

            // Get energy deposited in calorimeters.
            double energy_PCAL, energy_ECIN, energy_ECOU;
            if (get_deposited_energy(
                    bank_cal, pindex, &energy_PCAL, &energy_ECIN, &energy_ECOU
            )) return 1;

            // Get Cherenkov counters data.
            int nphe_HTCC, nphe_LTCC;
            if (count_photoelectrons(&bchkv, pindex, &nphe_HTCC, &nphe_LTCC))
                return 1;

            // Get time-of-flight (tof).
            double tof = get_tof(bank_sci, bank_cal, pindex);

            // Get miscellaneous data.
            int status  = rge_get_entry(&bpart, "status", pindex);
            double chi2 = rge_get_entry(&btrk,  "chi2",   pos);
            double ndf  = rge_get_entry(&btrk,  "NDF",    pos);

            // Assign PID.
            if (rge_set_pid(
                    &part, rge_get_entry(&bpart, "pid", pindex), status,
                    energy_PCAL + energy_ECIN + energy_ECOU, energy_PCAL,
                    nphe_HTCC, nphe_LTCC,
                    sampling_fraction_params[static_cast<uint>(
                            rge_get_entry(&btrk,"sector",pos)
                    )]
            )) return 1;

            // Fill TNtuples.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            Float_t arr[VAR_LIST_SIZE];
            if (rge_fill_ntuples_arr(
                    arr, part, part_trigger, run_no, event, status, energy_beam,
                    chi2, ndf, energy_PCAL, energy_ECIN, energy_ECOU, tof,
                    trigger_tof, nphe_LTCC, nphe_HTCC
            )) return 1;

            tree_out->Fill(arr);

            ++particle_counter;
        }
    }

    // Print number of particles found to detect errors early.
    printf("Triggers found:  %d\n", trigger_counter);
    printf("Particles found: %d\n", trigger_counter + particle_counter);

    // Create output file.
    char filename_out[PATH_MAX];
    if (fmt_nlayers == 0) {
        sprintf(filename_out, "%s/ntuples_dc_%06d.root", work_dir, run_no);
    }
    else {
        sprintf(
                filename_out, "%s/ntuples_fmt%1ld_%06d.root", work_dir,
                fmt_nlayers, run_no
        );
    }
    TFile *file_out = TFile::Open(filename_out, "RECREATE");

    // Write to output file.
    file_out->cd();
    tree_out->Write();

    // Clean up after ourselves.
    file_in ->Close();
    file_out->Close();

    rge_errno = RGEERR_NOERR;
    return 0;
}

/** Handle arguments for make_ntuples using optarg. */
static int handle_args(
        int argc, char **argv, char **filename_in, char **work_dir,
        char **data_dir, bool *debug, lint *fmt_nlayers, lint *n_events,
        int *run_no, double *energy_beam
) {
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hDf:n:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                rge_errno = RGEERR_USAGE;
                return 1;
            case 'D':
                *debug = true;
                break;
            case 'f':
                if (rge_process_fmtnlayers(fmt_nlayers, optarg)) return 1;
                break;
            case 'n':
                if (rge_process_nentries(n_events, optarg)) return 1;
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
                *filename_in = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*filename_in, optarg);
                break;
            default:
                rge_errno = RGEERR_BADOPTARGS;
                return 1;
        }
    }

    // Define workdir if undefined.
    // NOTE. We copy argv[0] because sprintf() writes over it.
    char tmpfilename[PATH_MAX];
    sprintf(tmpfilename, "%s", argv[0]);
    if (*work_dir == NULL) {
        *work_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*data_dir, "%s/../data", dirname(tmpfilename));
    }

    // Check positional argument.
    if (*filename_in == NULL) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }
    if (rge_handle_root_filename(*filename_in, run_no, energy_beam)) return 1;

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *filename_in  = NULL;
    char *work_dir     = NULL;
    char *data_dir     = NULL;
    bool debug         = false;
    lint fmt_nlayers   = 0;
    lint n_events      = -1;
    int run_no         = -1;
    double energy_beam = -1;

    int err = handle_args(
            argc, argv, &filename_in, &work_dir, &data_dir, &debug,
            &fmt_nlayers, &n_events, &run_no, &energy_beam
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED && err == 0) {
        run(
                filename_in, work_dir, data_dir, debug, fmt_nlayers, n_events,
                run_no, energy_beam
        );
    }

    // Free up memory.
    if (filename_in != NULL) free(filename_in);
    if (work_dir    != NULL) free(work_dir);
    if (data_dir    != NULL) free(data_dir);

    // Return errcode.
    return rge_print_usage(USAGE_MESSAGE);
}
