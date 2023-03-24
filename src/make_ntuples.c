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

#include <libgen.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TROOT.h>
#include "../lib/io_handler.h"
#include "../lib/particle.h"

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
static double get_tof(Scintillator scintillator, Calorimeter calorimeter,
        unsigned int pindex)
{
    int    most_precise_lyr = 0;
    double tof              = INFINITY;
    for (unsigned int i = 0; i < scintillator.pindex->size(); ++i) {
        // Filter out incorrect pindex and hits not from FTOF.
        if (
                static_cast<unsigned int>(
                        scintillator.pindex->at(i)
                ) != pindex ||
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
    for (unsigned int i = 0; i < calorimeter.pindex->size(); ++i) {
        // Filter out incorrect pindex.
        if (static_cast<unsigned int>(calorimeter.pindex->at(i)) != pindex) {
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
static int get_deposited_energy(Calorimeter calorimeter, unsigned int pindex,
        double *energy_PCAL, double *energy_ECIN, double *energy_ECOU)
{
    *energy_PCAL = 0;
    *energy_ECIN = 0;
    *energy_ECOU = 0;

    for (unsigned int i = 0; i < calorimeter.pindex->size(); ++i) {
        if (static_cast<unsigned int>(calorimeter.pindex->at(i)) != pindex) {
            continue;
        }
        int lyr = static_cast<int>(calorimeter.layer->at(i));

        if      (lyr == PCAL_LYR) *energy_PCAL += calorimeter.energy->at(i);
        else if (lyr == ECIN_LYR) *energy_ECIN += calorimeter.energy->at(i);
        else if (lyr == ECOU_LYR) *energy_ECOU += calorimeter.energy->at(i);
        else return 1;
    }

    return 0;
}

/**
 * Count number of photoelectrons deposited on HTCC and LTCC detectors.
 *
 * @param cherenkov: Instance of the Cherenkov class.
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
static int count_photoelectrons(Cherenkov cherenkov, unsigned int pindex,
        int *nphe_HTCC, int *nphe_LTCC)
{
    *nphe_HTCC = 0;
    *nphe_LTCC = 0;

    for (unsigned int i = 0; i < cherenkov.pindex->size(); ++i) {
        if (static_cast<unsigned int>(cherenkov.pindex->at(i)) != pindex) {
            continue;
        }

        int detector = cherenkov.detector->at(i);
        if      (detector == HTCC_ID) *nphe_HTCC += cherenkov.nphe->at(i);
        else if (detector == LTCC_ID) *nphe_LTCC += cherenkov.nphe->at(i);
        else return 1;
    }

    return 0;
}

/** run() function of the program. Check usage() for details. */
static int run(char *filename_in, char *work_dir, char *data_dir, bool debug,
        bool use_fmt, int n_events, int run_no, double energy_beam)
{
    // Get sampling fraction.
    char sampling_fraction_file[PATH_MAX];
    sprintf(sampling_fraction_file, "%s/sf_params_%06d.txt", data_dir, run_no);
    double smplng_frctn_prmtrs[NSECTORS][SF_NPARAMS][2];
    int errcode = get_sf_params(sampling_fraction_file, smplng_frctn_prmtrs);

    // Throw an error if sampling fraction parameters are not found.
    if (errcode) return 10;

    // Access input file.
    TFile *file_in  = TFile::Open(filename_in, "READ");
    if (!file_in || file_in->IsZombie()) return 11;

    // Return to top directory (weird root stuff).
    gROOT->cd();

    // Generate lists of variables.
    TString vars_string("");
    for (int var_i = 0; var_i < VAR_LIST_SIZE; ++var_i) {
        vars_string.Append(Form("%s", S_VAR_LIST[var_i]));
        if (var_i != VAR_LIST_SIZE-1) vars_string.Append(":");
    }

    // Create TTree and TNTuples.
    TTree *tree_in = file_in->Get<TTree>("Tree");
    if (tree_in == NULL) return 12;
    TNtuple *tree_out;
    tree_out = new TNtuple(TREENAME, TREENAME, vars_string);

    // Change n_events to number of entries if it is equal to -1 or invalid.
    if (n_events == -1 || n_events > tree_in->GetEntries()) {
        n_events = tree_in->GetEntries();
    }

    // Associate banks to TTree.
    Particle     bank_part  (tree_in);
    Track        bank_trk_dc(tree_in);
    Calorimeter  bank_cal   (tree_in);
    Cherenkov    bank_chkv  (tree_in);
    Scintillator bank_sci   (tree_in);
    FMT_Tracks   bank_trk_fmt;
    if (use_fmt) bank_trk_fmt.link_tree(tree_in);

    // Iterate through input file. Each TTree entry is one event.
    printf("Processing %d events from %s.\n", n_events, filename_in);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    // Particle counters.
    int cnt_trigger = 0;
    int cnt_part    = 0;

    // Loop through events in input file.
    for (int event = 0; event < n_events; ++event) {
        // Print fancy progress bar.
        if (!debug) {
            update_progress_bar(n_events, event, &evnsplitter, &divcntr);
        }

        // Get entries from input file.
        bank_part  .get_entries(tree_in, event);
        bank_trk_dc.get_entries(tree_in, event);
        bank_sci   .get_entries(tree_in, event);
        bank_cal   .get_entries(tree_in, event);
        bank_chkv  .get_entries(tree_in, event);
        if (use_fmt) bank_trk_fmt.get_entries(tree_in, event);

        // Filter events without the necessary banks.
        if (bank_part.vz->size() == 0 || bank_trk_dc.pindex->size() == 0) {
            continue;
        }

        // Check existence of trigger electron
        particle part_trigger;
        bool trigger_exist = false;
        unsigned int trigger_pos    = INT_MAX;
        unsigned int trigger_pindex = INT_MAX;
        double trigger_tof = -1.;
        for (unsigned int pos = 0; pos < bank_trk_dc.index->size(); ++pos) {
            unsigned int pindex = static_cast<unsigned int>(
                    bank_trk_dc.pindex->at(pos)
            );

            // Get reconstructed particle from DC and from FMT.
            if (!use_fmt) {
                part_trigger = particle_init(&bank_part, &bank_trk_dc, pos);
            }
            else {
                part_trigger = particle_init(
                        &bank_part, &bank_trk_dc, &bank_trk_fmt, pos
                );
            }

            // Skip particle if it doesn't fit requirements.
            if (!part_trigger.is_valid) continue;

            // Get energy deposited in calorimeters.
            double energy_PCAL, energy_ECIN, energy_ECOU;
            errcode = get_deposited_energy(
                    bank_cal, pindex, &energy_PCAL, &energy_ECIN, &energy_ECOU
            );
            if (errcode) return 13;

            // Get number of photoelectrons from Cherenkov counters.
            int nphe_HTCC, nphe_LTCC;
            errcode = count_photoelectrons(
                    bank_chkv, pindex, &nphe_HTCC, &nphe_LTCC
            );
            if (errcode) return 14;

            // Get time of flight from scintillators or calorimeters.
            double tof = get_tof(bank_sci, bank_cal, pindex);

            // Get miscellaneous data.
            int status  = bank_part.status->at(pindex);
            double chi2 = bank_trk_dc.chi2->at(pos);
            double ndf  = bank_trk_dc.ndf ->at(pos);

            // Assign PID.
            set_pid(
                    &part_trigger, bank_part.pid->at(pindex), status,
                    energy_PCAL + energy_ECIN + energy_ECOU, energy_PCAL,
                    nphe_HTCC, nphe_LTCC,
                    smplng_frctn_prmtrs[bank_trk_dc.sector->at(pos)]
            );

            // Skip particle if its not the trigger electron.
            if (!part_trigger.is_trigger_electron) continue;

            // Fill TNtuple with trigger electron information.
            Float_t arr[VAR_LIST_SIZE];
            fill_ntuples_arr(
                    arr, part_trigger, part_trigger, run_no, event, status,
                    energy_beam, chi2, ndf, energy_PCAL, energy_ECIN,
                    energy_ECOU, tof, tof, nphe_LTCC, nphe_HTCC
            );

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
        ++cnt_trigger;

        // Processing particles.
        for (unsigned int pos = 0; pos < bank_trk_dc.index->size(); ++pos) {
            // Currently pindex is always equal to pos, but this is not a given
            //     in the future of the reconstruction software development.
            unsigned int pindex =
                    static_cast<unsigned int>(bank_trk_dc.pindex->at(pos));

            // Avoid double-counting the trigger electron.
            if (trigger_pindex == pindex && trigger_pos == pos) {
                continue;
            }

            // Get reconstructed particle from DC and from FMT.
            particle part;
            if (!use_fmt) part = particle_init(&bank_part, &bank_trk_dc, pos);
            else {
                part = particle_init(
                        &bank_part, &bank_trk_dc, &bank_trk_fmt, pos
                );
            }

            // Skip particle if it doesn't fit requirements.
            if (!part.is_valid) continue;

            // Get energy deposited in calorimeters.
            double energy_PCAL, energy_ECIN, energy_ECOU;
            errcode = get_deposited_energy(
                    bank_cal, pindex, &energy_PCAL, &energy_ECIN, &energy_ECOU
            );
            if (errcode) return 13;

            // Get Cherenkov counters data.
            int nphe_HTCC, nphe_LTCC;
            errcode = count_photoelectrons(
                    bank_chkv, pindex, &nphe_HTCC, &nphe_LTCC
            );
            if (errcode) return 14;

            // Get time-of-flight (tof).
            double tof = get_tof(bank_sci, bank_cal, pindex);

            // Get miscellaneous data.
            int status  = bank_part.status->at(pindex);
            double chi2 = bank_trk_dc.chi2->at(pos);
            double ndf  = bank_trk_dc.ndf ->at(pos);

            // Assign PID.
            set_pid(
                    &part, bank_part.pid->at(pindex), status,
                    energy_PCAL + energy_ECIN + energy_ECOU, energy_PCAL,
                    nphe_HTCC, nphe_LTCC,
                    smplng_frctn_prmtrs[bank_trk_dc.sector->at(pos)]
            );

            // Fill TNtuples.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            Float_t arr[VAR_LIST_SIZE];
            fill_ntuples_arr(
                    arr, part, part_trigger, run_no, event, status, energy_beam,
                    chi2, ndf, energy_PCAL, energy_ECIN, energy_ECOU, tof,
                    trigger_tof, nphe_LTCC, nphe_HTCC
            );

            tree_out->Fill(arr);

            ++cnt_part;
        }
    }

    // Print number of particles found to detect errors early.
    printf("Triggers found:  %d\n", cnt_trigger);
    printf("Particles found: %d\n", cnt_trigger + cnt_part);

    // Create output file.
    char filename_out[PATH_MAX];
    if (!use_fmt) {
        sprintf(filename_out, "%s/ntuples_dc_%06d.root", work_dir, run_no);
    }
    else {
        sprintf(filename_out, "%s/ntuples_fmt_%06d.root", work_dir, run_no);
    }
    TFile *file_out = TFile::Open(filename_out, "RECREATE");

    // Write to output file.
    file_out->cd();
    tree_out->Write();

    // Clean up after ourselves.
    file_in ->Close();
    file_out->Close();

    return 0;
}

/** Print usage and exit. */
static int usage() {
    fprintf(stderr,
            "\n\nUsage: make_ntuples [-hDfn:w:d:] infile\n"
            " * -h         : show this message and exit.\n"
            " * -D         : activate debug mode.\n"
            " * -f         : define from which bank the tracking data is to be "
            "obtained. Set\n                to false to use DC data, set to "
            "true to use FMT data \n                If set to true and "
            "FMT::Tracks bank is not in the input file,\n                the "
            "program will crash.\n"
            " * -n nevents : number of events.\n"
            " * -w workdir : location where output root files are to be "
            "stored. Default\n                is root_io.\n"
            " * -d datadir : location where sampling fraction files are "
            "located. Default is\n                data.\n"
            " * infile     : input ROOT file. Expected file format: "
            "<text>run_no.root`.\n\n"
            "    Generate ntuples relevant to SIDIS analysis based on the "
            "reconstructed\n    variables from CLAS12 data.\n\n"
    );

    return 1;
}

/** Print error number and provide a short description of the error. */
static int handle_err(int errcode) {
    if (errcode > 1) fprintf(stderr, "Error %02d. ", errcode);
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            fprintf(stderr, "nevents should be a number greater than 0.");
            break;
        case 3:
            fprintf(stderr, "Bad usage of optional arguments.");
            break;
        case 4:
            fprintf(stderr, "No file name provided.");
            break;
        case 5:
            fprintf(stderr, "Input file should be in root format.");
            break;
        case 6:
            fprintf(stderr, "Input file does not exist.");
            break;
        case 7:
            fprintf(stderr, "Couldn't find dot position in input filename.");
            break;
        case 8:
            fprintf(stderr, "Couldn't find run number in input filename.");
            break;
        case 9:
            fprintf(stderr, "Run number not in constants. Add from RCDB.");
            break;
        case 10:
            // NOTE. In this scenario, a smoother behavior would be that the
            //       program calls extract_sf itself.
            fprintf(stderr, "No sampling fraction file is available for run "
            "number! Run extract_sf on this\nroot file before running "
            "make_ntuples.");
            break;
        case 11:
            fprintf(stderr, "Input file is not a valid root file.");
            break;
        case 12:
            fprintf(stderr, "Couldn't get relevant trees from input file.");
            break;
        case 13:
            fprintf(stderr, "Invalid Electronic Calorimeter layer. Check bank "
                            "integrity.\n");
            break;
        case 14:
            fprintf(stderr, "Invalid Cherenkov Counter ID. Check bank "
                            "integrity.");
            break;
        default:
            fprintf(stderr, "Error code not implemented!\n");
            return 1;
    }

    return usage();
}

/**
 * Handle arguments for make_ntuples using optarg. Error codes used are
 *     explained in the handle_err() function.
 */
static int handle_args(int argc, char **argv, char **filename_in,
        char **work_dir, char **data_dir, bool *debug, bool *use_fmt,
        int *n_events, int *run_no, double *energy_beam)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hDfn:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'D':
                *debug = true;
                break;
            case 'f':
                *use_fmt = true;
                break;
            case 'n':
                *n_events = atoi(optarg);
                if (*n_events <= 0) return 2; // Check if n_events is valid.
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
                return 3; // Bad usage of optional arguments.
        }
    }

    // Define workdir if undefined.
    // NOTE. We copy argv[0] because sprintf() writes over it.
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
    if (*filename_in == NULL) return 4;

    int check = handle_root_filename(*filename_in, run_no, energy_beam);
    if (check) return check + 4; // Shift errcode.

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *filename_in  = NULL;
    char *work_dir     = NULL;
    char *data_dir     = NULL;
    bool debug         = false;
    bool use_fmt       = false;
    int n_events       = -1;
    int run_no         = -1;
    double energy_beam = -1;

    int errcode = handle_args(
            argc, argv, &filename_in, &work_dir, &data_dir, &debug, &use_fmt,
            &n_events, &run_no, &energy_beam
    );

    // Run.
    if (errcode == 0) {
        errcode = run(
                filename_in, work_dir, data_dir, debug, use_fmt, n_events,
                run_no, energy_beam
        );
    }

    // Free up memory.
    if (filename_in != NULL) free(filename_in);
    if (work_dir    != NULL) free(work_dir);
    if (data_dir    != NULL) free(data_dir);

    // Return errcode.
    return handle_err(errcode);
}
