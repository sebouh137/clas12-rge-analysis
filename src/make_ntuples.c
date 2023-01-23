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
 * @param rsci:  instance of the Scintillator class.
 * @param rcal:  instance of the Calorimeter class.
 * @param pindex: particle index of the particle we're studying.
 * @return:       the most accurate TOF available in the scintillator and
 *                calorimeter banks.
 */
double get_tof(Scintillator rsci, Calorimeter rcal, int pindex) {
    int    most_precise_lyr = 0;
    double tof              = INFINITY;
    for (UInt_t i = 0; i < rsci.pindex->size(); ++i) {
        // Filter out incorrect pindex and hits not from FTOF.
        if (rsci.pindex->at(i) != pindex || rsci.detector->at(i) != FTOF_ID)
            continue;
        if (rsci.layer->at(i) == FTOF1B_LYR) {
            most_precise_lyr = FTOF1B_LYR;
            tof = rsci.time->at(i);
            break; // Things won't get better than this.
        }
        else if (rsci.layer->at(i) == FTOF1A_LYR) {
            if (most_precise_lyr == FTOF1A_LYR) continue;
            most_precise_lyr = FTOF1A_LYR;
            tof = rsci.time->at(i);
        }
        else if (rsci.layer->at(i) == FTOF2_LYR) {
            // We already have a similar or better hit.
            if (most_precise_lyr != 0) continue;
            most_precise_lyr = FTOF2_LYR;
            tof = rsci.time->at(i);
        }
    }
    if (most_precise_lyr == 0) { // No hits from FTOF, let's try ECAL.
        for (UInt_t i = 0; i < rcal.pindex->size(); ++i) {
            // Filter out incorrect pindex.
            if (rcal.pindex->at(i) != pindex) continue;
            if (rcal.layer->at(i) == PCAL_LYR) {
                most_precise_lyr = 10 + PCAL_LYR;
                tof = rcal.time->at(i);
                break; // Things won't get better than this.
            }
            else if (rcal.layer->at(i) == ECIN_LYR) {
                if (most_precise_lyr == 10 + ECIN_LYR) continue;
                most_precise_lyr = 10 + ECIN_LYR;
                tof = rcal.time->at(i);
            }
            else if (rcal.layer->at(i) == ECOU_LYR) {
                if (most_precise_lyr != 0) continue;
                most_precise_lyr = 10 + ECOU_LYR;
                tof = rcal.time->at(i);
            }
        }
    }

    return tof;
}

/** run() function of the program. Check usage() for details. */
int run(char *in_file, char *work_dir, char *data_dir, bool debug, int nevn,
        int run_no, double beam_E)
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
    TFile *f_in  = TFile::Open(in_file, "READ");
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

    // Associate banks to TTree.
    Particle     rpart(t_in);
    Track        rtrk (t_in);
    Calorimeter  rcal (t_in);
    Cherenkov    rche (t_in);
    Scintillator rsci (t_in);

    // Counters for PID assignment quality assessment.
    int pid_n[NPIDS];
    int pid_qa[NPIDS][NPIDS];
    if (debug) {
        for (int i = 0; i < NPIDS; ++i) pid_n[i] = 0;
        for (int i = 0; i < NPIDS; ++i)
            for (int j = 0; j < NPIDS; ++j) pid_qa[i][j] = 0;
    }

    // Iterate through input file. Each TTree entry is one event.
    printf("Reading %lld events from %s.\n", nevn == -1 ? t_in->GetEntries() :
            nevn, in_file);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    for (int evn = 0; (evn < t_in->GetEntries()) && (nevn == -1 || evn < nevn);
            ++evn)
    {
        // Print fancy progress bar.
        if (!debug) update_progress_bar(nevn == -1 ? t_in->GetEntries() : nevn,
                evn, &evnsplitter, &divcntr);

        // Get entries from input file.
        rpart.get_entries(t_in, evn);
        rtrk .get_entries(t_in, evn);
        rsci .get_entries(t_in, evn);
        rcal .get_entries(t_in, evn);
        rche .get_entries(t_in, evn);

        // Filter events without the necessary banks.
        if (rpart.vz->size() == 0 || rtrk.pindex->size() == 0) continue;

        // Find trigger electron's TOF.
        float tre_tof = get_tof(rsci, rcal, rtrk.pindex->at(0));

        // Check existence of trigger electron
        particle p_el[2];
        bool    trigger_exist  = false;
        UInt_t  trigger_pos    = -1;
        int     trigger_pindex = -1;
        for (UInt_t pos = 0; pos < rtrk.index->size(); ++pos) {
            int pindex = rtrk.pindex->at(pos);

            // Get reconstructed particle from DC and from FMT.
            p_el[0] = particle_init(&rpart, &rtrk, pos, false); // DC.
            p_el[1] = particle_init(&rpart, &rtrk, pos, true);  // FMT.

            // Get deposited energy.
            float pcal_E = 0; // PCAL total deposited energy.
            float ecin_E = 0; // EC inner total deposited energy.
            float ecou_E = 0; // EC outer total deposited energy.
            for (UInt_t i = 0; i < rcal.pindex->size(); ++i) {
                if (rcal.pindex->at(i) != pindex) continue;
                int lyr = (int) rcal.layer->at(i);

                if      (lyr == PCAL_LYR) pcal_E += rcal.energy->at(i);
                else if (lyr == ECIN_LYR) ecin_E += rcal.energy->at(i);
                else if (lyr == ECOU_LYR) ecou_E += rcal.energy->at(i);
                else return 13;
            }

            // Get Cherenkov counters data.
            int htcc_nphe = 0; // Number of photoelectrons deposited in htcc.
            int ltcc_nphe = 0; // Number of photoelectrons deposited in ltcc.
            for (UInt_t i = 0; i < rche.pindex->size(); ++i) {
                if (rche.pindex->at(i) == pindex) {
                    int detector = rche.detector->at(i);
                    if      (detector == HTCC_ID) htcc_nphe += rche.nphe->at(i);
                    else if (detector == LTCC_ID) ltcc_nphe += rche.nphe->at(i);
                    else return 14;
                }
            }

            // Get TOF.
            float tof = get_tof(rsci, rcal, pindex);

            // Get miscellaneous data.
            int status = rpart.status->at(pindex);
            float chi2 = rtrk.chi2   ->at(pos);
            float ndf  = rtrk.ndf    ->at(pos);

            // Assign PID.
            for (int pi = 0; pi < 2; ++pi) {
                set_pid(&(p_el[pi]), rpart.pid->at(pindex), status,
                        pcal_E + ecin_E + ecou_E, pcal_E, htcc_nphe, ltcc_nphe,
                        sf_params[rtrk.sector->at(pos)]);
            }

            // Fill TNtuples with trigger electron information.
            for (int pi = 0; pi < 2; ++pi) {
                if (!(p_el[pi].is_valid && p_el[pi].is_trigger_electron))
                    continue;
                trigger_exist = true;

                Float_t arr[VAR_LIST_SIZE];
                fill_ntuples_arr(arr, p_el[pi], p_el[pi], run_no, evn, status,
                        beam_E, chi2, ndf, pcal_E, ecin_E, ecou_E, tof,
                        tre_tof);

                t_out[pi]->Fill(arr);
            }
            if (trigger_exist) {
                trigger_pindex = pindex;
                trigger_pos    = pos;
                break;
            }
        }

        // In case no trigger e was found, initiate p_el as dummy particles.
        if (!trigger_exist) {
            p_el[0] = particle_init();
            p_el[1] = particle_init();
        }

        // Processing particles.
        for (UInt_t pos = 0; pos < rtrk.index->size(); ++pos) {
            // Currently pindex is always equal to pos, but this is not a given
            //     in the future of reconstruction development. Better safe than
            //     sorry!
            int pindex = rtrk.pindex->at(pos);

            // Avoid double-counting the trigger electron.
            if (trigger_pindex == pindex && trigger_pos == pos) continue;

            // Get reconstructed particle from DC and from FMT.
            particle p[2];
            p[0] = particle_init(&rpart, &rtrk, pos, false); // DC.
            p[1] = particle_init(&rpart, &rtrk, pos, true);  // FMT.

            // Get deposited energy.
            float pcal_E = 0; // PCAL total deposited energy.
            float ecin_E = 0; // EC inner total deposited energy.
            float ecou_E = 0; // EC outer total deposited energy.
            for (UInt_t i = 0; i < rcal.pindex->size(); ++i) {
                if (rcal.pindex->at(i) != pindex) continue;
                int lyr = (int) rcal.layer->at(i);

                if      (lyr == PCAL_LYR) pcal_E += rcal.energy->at(i);
                else if (lyr == ECIN_LYR) ecin_E += rcal.energy->at(i);
                else if (lyr == ECOU_LYR) ecou_E += rcal.energy->at(i);
                else return 13;
            }

            // Get Cherenkov counters data.
            int htcc_nphe = 0; // Number of photoelectrons deposited in htcc.
            int ltcc_nphe = 0; // Number of photoelectrons deposited in ltcc.
            for (UInt_t i = 0; i < rche.pindex->size(); ++i) {
                if (rche.pindex->at(i) == pindex) {
                    int detector = rche.detector->at(i);
                    if      (detector == HTCC_ID) htcc_nphe += rche.nphe->at(i);
                    else if (detector == LTCC_ID) ltcc_nphe += rche.nphe->at(i);
                    else return 14;
                }
            }

            // Get TOF.
            float tof = get_tof(rsci, rcal, pindex);

            // Get miscellaneous data.
            int status = rpart.status->at(pindex);
            float chi2 = rtrk.chi2   ->at(pos);
            float ndf  = rtrk.ndf    ->at(pos);

            // Assign PID.
            for (int pi = 0; pi < 2; ++pi) {
                set_pid(&(p[pi]), rpart.pid->at(pindex), status,
                        pcal_E + ecin_E + ecou_E, pcal_E, htcc_nphe, ltcc_nphe,
                        sf_params[rtrk.sector->at(pos)]);
            }

            // Test PID assignment precision.
            if (debug
                    && PID_QA.find(abs(rpart.pid->at(pindex))) != PID_QA.end()
                    && PID_QA.find(abs(p[0].pid)) != PID_QA.end())
            {
                pid_n[PID_QA.at(abs(rpart.pid->at(pindex)))]++;
                pid_qa[PID_QA.at(abs(rpart.pid->at(pindex)))]
                        [PID_QA.at(abs(p[0].pid))]++;
            }

            // Fill TNtuples.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            for (int pi = 0; pi < 2; ++pi) {
                if (!p[pi].is_valid) continue;
                Float_t arr[VAR_LIST_SIZE];
                fill_ntuples_arr(arr, p[pi], p_el[pi], run_no, evn, status,
                        beam_E, chi2, ndf, pcal_E, ecin_E, ecou_E, tof,
                        tre_tof);

                t_out[pi]->Fill(arr);
            }
        }
    }

    if (debug) {
        printf("\nparticle identification matrix:\n        e     pi    K     "
               "p     n     gamma\n");
        for (int i = 0; i < NPIDS; ++i) {
            if (i == 0) printf("    e  ");
            if (i == 1) printf("   pi  ");
            if (i == 2) printf("    K  ");
            if (i == 3) printf("    p  ");
            if (i == 4) printf("    n  ");
            if (i == 5) printf("gamma  ");
            for (int j = 0; j < NPIDS; ++j) {
                printf("%5.2f ", ((double) pid_qa[j][i])/((double) pid_n[j]));
            }
            printf("\n");
        }
        printf("\n");
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
            "\n\nUsage: make_ntuples [-hDn:w:d:] infile\n"
            " * -h         : show this message and exit.\n"
            " * -D         : activate debug mode.\n"
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
int handle_args(int argc, char **argv, char **in_file, char **work_dir,
        char **data_dir, bool *debug, int *nevn, int *run_no, double *beam_E)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hDn:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'D':
                *debug = true;
                break;
            case 'n':
                *nevn = atoi(optarg);
                if (*nevn <= 0) return 2; // Check if nevn is valid.
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
                *in_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*in_file, optarg);
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
    if (*in_file == NULL) return 4;

    int check = handle_root_filename(*in_file, run_no, beam_E);
    if (check) return check + 4; // Shift errcode.

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_file  = NULL;
    char *work_dir = NULL;
    char *data_dir = NULL;
    bool debug     = false;
    int nevn       = -1;
    int run_no     = -1;
    double beam_E  = -1;

    int errcode = handle_args(argc, argv, &in_file, &work_dir, &data_dir,
            &debug, &nevn, &run_no, &beam_E);

    // Run.
    if (errcode == 0)
        errcode = run(in_file, work_dir, data_dir, debug, nevn, run_no, beam_E);

    // Free up memory.
    if (in_file  != NULL) free(in_file);
    if (work_dir != NULL) free(work_dir);
    if (data_dir != NULL) free(data_dir);

    // Return errcode.
    return handle_err(errcode);
}
