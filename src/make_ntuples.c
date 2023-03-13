// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
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
double get_tof(Scintillator scintillator, Calorimeter calorimeter, int pindex)
{
    int    most_precise_lyr = 0;
    double tof              = INFINITY;
    for (UInt_t i = 0; i < scintillator.pindex->size(); ++i) {
        // Filter out incorrect pindex and hits not from FTOF.
        if (
                scintillator.pindex->at(i) != pindex ||
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
    for (UInt_t i = 0; i < calorimeter.pindex->size(); ++i) {
        // Filter out incorrect pindex.
        if (calorimeter.pindex->at(i) != pindex) continue;

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
 * @param pcal_E:      pointer to double to which we'll write the PCAL energy.
 * @param ecin_E:      pointer to double to which we'll write the ECIN energy.
 * @param ecou_E:      pointer to double to which we'll write the ECOU energy.
 * @return             error code. 0 if successful, 1 otherwise. The function
 *                     only returns 1 if there's an invalid layer in the
 *                     Calorimeter instance, suggesting corruption or a change
 *                     in the REC::Calorimeter bank.
 */
int get_deposited_energy(Calorimeter calorimeter, int pindex, double *pcal_E,
        double *ecin_E, double *ecou_E)
{
    *pcal_E = 0;
    *ecin_E = 0;
    *ecou_E = 0;

    for (UInt_t i = 0; i < calorimeter.pindex->size(); ++i) {
        if (calorimeter.pindex->at(i) != pindex) continue;
        int lyr = (int) calorimeter.layer->at(i);

        if      (lyr == PCAL_LYR) *pcal_E += calorimeter.energy->at(i);
        else if (lyr == ECIN_LYR) *ecin_E += calorimeter.energy->at(i);
        else if (lyr == ECOU_LYR) *ecou_E += calorimeter.energy->at(i);
        else return 1;
    }

    return 0;
}

/**
 * Count number of photoelectrons deposited on HTCC and LTCC detectors.
 *
 * @param cherenkov: Instance of the Cherenkov class.
 * @param pindex:    particle index of the particle we're studying.
 * @param htcc_nphe: pointer to int where we'll write the number of
 *                   photoelectrons deposited on HTCC.
 * @param ltcc_nphe: pointer to int where we'll write the number of
 *                   photoelectrons deposited on LTCC.
 * @return           error code. 0 if successful, 1 otherwise. The function only
 *                   returns 1 if there's an invalid detector ID in the
 *                   Cherenkov instance, suggesting data corruption or a change
 *                   in the REC::Cherenkov bank.
 */
int count_photoelectrons(Cherenkov cherenkov, int pindex, int *htcc_nphe,
        int *ltcc_nphe)
{
    *htcc_nphe = 0;
    *ltcc_nphe = 0;

    for (UInt_t i = 0; i < cherenkov.pindex->size(); ++i) {
        if (cherenkov.pindex->at(i) != pindex) continue;

        int detector = cherenkov.detector->at(i);
        if      (detector == HTCC_ID) *htcc_nphe += cherenkov.nphe->at(i);
        else if (detector == LTCC_ID) *ltcc_nphe += cherenkov.nphe->at(i);
        else return 1;
    }

    return 0;
}

/** run() function of the program. Check usage() for details. */
int run(char *filename_in, char *work_dir, char *data_dir, bool debug,
        bool use_fmt, int n_events, int run_no, double energy_beam)
{
    // Get sampling fraction.
    char t_file[PATH_MAX];
    sprintf(t_file, "%s/sf_params_%06d.txt", data_dir, run_no);
    double sf_params[NSECTORS][SF_NPARAMS][2];
    int errcode = get_sf_params(t_file, sf_params);
    if (errcode) return 10;

    // Create output file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/ntuples_%06d.root", work_dir, run_no);
    TFile *f_out = TFile::Open(out_file, "RECREATE");

    // Access input file.
    TFile *f_in  = TFile::Open(filename_in, "READ");
    if (!f_in || f_in->IsZombie()) return 11;

    // Return to top directory (weird root stuff).
    gROOT->cd();

    // Generate lists of variables.
    TString vars("");
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) {
        vars.Append(Form("%s", S_VAR_LIST[vi]));
        if (vi != VAR_LIST_SIZE-1) vars.Append(":");
    }

    // Create TTree and TNTuples.
    TTree *t_in = f_in->Get<TTree>("Tree");
    if (t_in == NULL) return 12;
    TNtuple *t_out[2];
    t_out[0] = new TNtuple(S_DC,  S_DC,  vars);
    t_out[1] = new TNtuple(S_FMT, S_FMT, vars);

    // Change n_events to number of entries if it is equal to -1.
    if (n_events == -1) n_events = t_in->GetEntries();

    // Associate banks to TTree.
    Particle     b_particle    (t_in);
    Track        b_track       (t_in);
    Calorimeter  b_calorimeter (t_in);
    Cherenkov    b_cherenkov   (t_in);
    Scintillator b_scintillator(t_in);
    FMT_Tracks   b_fmt_tracks;
    if (use_fmt) b_fmt_tracks.link_tree(t_in);

    // Iterate through input file. Each TTree entry is one event.
    printf("Reading %d events from %s.\n", n_events, filename_in);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    for (int evn = 0; evn < n_events; ++evn) {
        // Print fancy progress bar.
        if (!debug) update_progress_bar(n_events, evn, &evnsplitter, &divcntr);

        // Get entries from input file.
        b_particle    .get_entries(t_in, evn);
        b_track       .get_entries(t_in, evn);
        b_scintillator.get_entries(t_in, evn);
        b_calorimeter .get_entries(t_in, evn);
        b_cherenkov   .get_entries(t_in, evn);
        if (use_fmt) b_fmt_tracks.get_entries(t_in, evn);

        // Filter events without the necessary banks.
        if (b_particle.vz->size() == 0 || b_track.pindex->size() == 0) continue;

        // Check existence of trigger electron
        particle p_el[2];
        bool    trigger_exist  = false;
        UInt_t  trigger_pos    = -1;
        int     trigger_pindex = -1;
        double   trigger_tof    = -1.;
        for (UInt_t pos = 0; pos < b_track.index->size(); ++pos) {
            int pindex = b_track.pindex->at(pos);

            // Get reconstructed particle from DC and from FMT.
            p_el[0] = particle_init(&b_particle, &b_track, pos);
            if (use_fmt) {
                p_el[1] = particle_init(&b_particle, &b_track, &b_fmt_tracks,
                        pos);
            }
            else {
                p_el[1] = particle_init();
            }

            // Get deposited energy in PCAL, ECIN, and ECOU.
            double pcal_E, ecin_E, ecou_E;
            if (get_deposited_energy(b_calorimeter, pindex, &pcal_E, &ecin_E,
                                     &ecou_E)
            ) return 13;

            // Get Cherenkov counters data.
            int htcc_nphe, ltcc_nphe;
            if (count_photoelectrons(b_cherenkov, pindex, &htcc_nphe,
                                     &ltcc_nphe)
            ) return 14;

            // Get time of flight.
            double tof = get_tof(b_scintillator, b_calorimeter, pindex);

            // Get miscellaneous data.
            int status = b_particle.status->at(pindex);
            double chi2 = b_track.chi2     ->at(pos);
            double ndf  = b_track.ndf      ->at(pos);

            // Assign PID.
            for (int pi = 0; pi < 2; ++pi) {
                set_pid(&(p_el[pi]), b_particle.pid->at(pindex), status,
                        pcal_E + ecin_E + ecou_E, pcal_E, htcc_nphe, ltcc_nphe,
                        sf_params[b_track.sector->at(pos)]);
            }

            // Fill TNtuples with trigger electron information.
            for (int pi = 0; pi < 2; ++pi) {
                if (!(p_el[pi].is_valid && p_el[pi].is_trigger_electron))
                    continue;
                trigger_exist = true;

                Float_t arr[VAR_LIST_SIZE];
                fill_ntuples_arr(
                        arr, p_el[pi], p_el[pi], run_no, evn, status,
                        energy_beam, chi2, ndf, pcal_E, ecin_E, ecou_E, tof, tof
                );

                t_out[pi]->Fill(arr);
            }
            if (trigger_exist) {
                trigger_pindex = pindex;
                trigger_pos    = pos;
                trigger_tof    = tof;
                break;
            }
        }

        // In case no trigger e was found, initiate p_el as dummy particles.
        if (!trigger_exist) {
            p_el[0] = particle_init();
            p_el[1] = particle_init();
        }

        // Processing particles.
        for (UInt_t pos = 0; pos < b_track.index->size(); ++pos) {
            // Currently pindex is always equal to pos, but this is not a given
            //     in the future of reconstruction development. Better safe than
            //     sorry!
            int pindex = b_track.pindex->at(pos);

            // Avoid double-counting the trigger electron.
            if (trigger_pindex == pindex && trigger_pos == pos) continue;

            // Get reconstructed particle from DC and from FMT.
            particle p[2];
            p[0] = particle_init(&b_particle, &b_track, pos);
            if (use_fmt)
                p[1] = particle_init(&b_particle, &b_track, &b_fmt_tracks, pos);
            else
                p[1] = particle_init();

            // Get deposited energy in PCAL, ECIN, and ECOU.
            double pcal_E, ecin_E, ecou_E;
            if (get_deposited_energy(b_calorimeter, pindex, &pcal_E, &ecin_E,
                                     &ecou_E)
            ) return 13;

            // Get Cherenkov counters data.
            int htcc_nphe, ltcc_nphe;
            if (count_photoelectrons(b_cherenkov, pindex, &htcc_nphe,
                                     &ltcc_nphe)
            ) return 14;

            // Get TOF.
            double tof = get_tof(b_scintillator, b_calorimeter, pindex);

            // Get miscellaneous data.
            int status = b_particle.status->at(pindex);
            double chi2 = b_track.chi2     ->at(pos);
            double ndf  = b_track.ndf      ->at(pos);

            // Assign PID.
            for (int pi = 0; pi < 2; ++pi) {
                set_pid(&(p[pi]), b_particle.pid->at(pindex), status,
                        pcal_E + ecin_E + ecou_E, pcal_E, htcc_nphe, ltcc_nphe,
                        sf_params[b_track.sector->at(pos)]);
            }

            // Fill TNtuples.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            for (int pi = 0; pi < 2; ++pi) {
                if (!p[pi].is_valid) continue;
                Float_t arr[VAR_LIST_SIZE];
                fill_ntuples_arr(arr, p[pi], p_el[pi], run_no, evn, status,
                        energy_beam, chi2, ndf, pcal_E, ecin_E, ecou_E, tof,
                        trigger_tof);

                t_out[pi]->Fill(arr);
            }
        }
    }

    // Write to output file.
    f_out->cd();
    t_out[0]->Write();
    t_out[1]->Write();

    // Clean up after ourselves.
    f_in ->Close();
    f_out->Close();

    return 0;
}

/** Print usage and exit. */
int usage() {
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
int handle_err(int errcode) {
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
int handle_args(int argc, char **argv, char **filename_in, char **work_dir,
        char **data_dir, bool *debug, bool *use_fmt, int *n_events, int *run_no,
        double *energy_beam)
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
                *work_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*work_dir, optarg);
                break;
            case 'd':
                *data_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*data_dir, optarg);
                break;
            case 1:
                *filename_in = (char *) malloc(strlen(optarg) + 1);
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
        *work_dir = (char *) malloc(PATH_MAX);
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = (char *) malloc(PATH_MAX);
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
