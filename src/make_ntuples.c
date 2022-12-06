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

#include <TFile.h>
#include <TNtuple.h>
#include <TROOT.h>
#include "../lib/io_handler.h"
#include "../lib/particle.h"

// Find most precise TOF. In order of decreasing precision, the detectors are
//     FTOF1B > FTOF1A > FTOF2 > PCAL > ECIN > ECOU.
double get_tof(REC_Scintillator rsci, REC_Calorimeter rcal, int pindex) {
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

int run(char *in_filename, bool debug, int nevn, char *ac_filename, int run_no,
        double beam_E)
{
    // Get sampling fraction.
    double sf_params[NSECTORS][SF_NPARAMS][2];
    if (get_sf_params(Form("../data/sf_params_%06d.txt", run_no), sf_params))
        return 11;

    // Get acceptance correction
    long int b_sizes[5];
    long int tsize;
    double **binnings;
    long int pids_size;
    long int *pids;
    double **acc_corr;

    if (ac_filename != NULL) {
        if (access(ac_filename, F_OK) != 0) return 12;
        FILE *ac_file = fopen(ac_filename, "r");

        binnings = (double **) malloc(5 * sizeof(*binnings));
        get_binnings(ac_file, b_sizes, binnings, &pids_size);

        tsize = 1;
        for (int bi = 0; bi < 5; ++bi) tsize *= b_sizes[bi] - 1;

        for (int bi = 0; bi < 5; ++bi) {
            printf("binning[%ld]: [", b_sizes[bi]);
            for (int bii = 0; bii < b_sizes[bi]; ++bii)
                printf("%lf, ", binnings[bi][bii]);
            printf("\b\b]\n");
        }

        pids = (long int *) malloc(pids_size * sizeof(*pids));
        acc_corr = (double **) malloc(pids_size * sizeof(*acc_corr));

        get_acc_corr(ac_file, pids_size, tsize, pids, acc_corr);

        printf("pids[%ld] = [", pids_size);
        for (int pi = 0; pi < pids_size; ++pi) {
            printf("%ld ", pids[pi]);
        }
        printf("\b\b]\n");

        for (int pi = 0; pi < pids_size; ++pi) {
            printf("acc_corr[%ld]: [", tsize);
            for (int bii = 0; bii < tsize; ++bii)
                printf("%lf ", acc_corr[pi][bii]);
            printf("\b\b]\n");
        }

        fclose(ac_file);
    }

    // Create output file.
    char *out_filename = (char *) malloc(128 * sizeof(char));
    sprintf(out_filename, "../root_io/ntuples.root");

    // Access input file.
    TFile *f_in  = TFile::Open(in_filename, "READ");
    TFile *f_out = TFile::Open(out_filename, "RECREATE");

    // Return to top directory.
    gROOT->cd();

    if (!f_in || f_in->IsZombie()) return 8;

    // Generate lists of variables.
    TString vars("");
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) {
        vars.Append(Form("%s", S_VAR_LIST[vi]));
        if (vi != VAR_LIST_SIZE-1) vars.Append(":");
    }

    // Create TTree and TNTuples.
    TTree   *t_in = f_in->Get<TTree>("Tree");
    if (t_in == NULL) return 8;
    TNtuple *t_out[2];
    t_out[0] = new TNtuple(S_DC,  S_DC,  vars);
    t_out[1] = new TNtuple(S_FMT, S_FMT, vars);

    // Associate banks to TTree.
    REC_Particle     rpart(t_in);
    REC_Track        rtrk (t_in);
    REC_Calorimeter  rcal (t_in);
    REC_Cherenkov    rche (t_in);
    REC_Scintillator rsci (t_in);
    FMT_Tracks       ftrk (t_in);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

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
            nevn, in_filename);

    // test of electrons
    for (int evn = 0; (evn < t_in->GetEntries()) &&
            (nevn == -1 || evn < nevn); ++evn) {
        if (!debug && evn >= evnsplitter) {
            if (evn != 0) printf("\33[2K\r");
            printf("[");
            for (int i = 0; i <= 50; ++i) {
                if (i <= divcntr/2) printf("=");
                else                printf(" ");
            }
            printf("] %2d%%", divcntr);
            fflush(stdout);
            divcntr++;
            evnsplitter = nevn == -1 ? (t_in->GetEntries() / 100) * divcntr :
                    (nevn/100) * divcntr;
        }

        rpart.get_entries(t_in, evn);
        rtrk .get_entries(t_in, evn);
        rsci .get_entries(t_in, evn);
        rcal .get_entries(t_in, evn);
        rche .get_entries(t_in, evn);
        ftrk .get_entries(t_in, evn);

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
            int pindex = rtrk.pindex->at(pos); // pindex is always equal to pos!

            // Get reconstructed particle from DC and from FMT.
            p_el[0] = particle_init(&rpart, &rtrk, pos);        // DC.
            p_el[1] = particle_init(&rpart, &rtrk, &ftrk, pos); // FMT.

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
                else return 9;
            }
            float tot_E = pcal_E + ecin_E + ecou_E;

            // Get Cherenkov counters data.
            int htcc_nphe = 0; // Number of photoelectrons deposited in htcc.
            int ltcc_nphe = 0; // Number of photoelectrons deposited in ltcc.
            for (UInt_t i = 0; i < rche.pindex->size(); ++i) {
                if (rche.pindex->at(i) == pindex) {
                    int detector = rche.detector->at(i);
                    if      (detector == HTCC_ID) htcc_nphe += rche.nphe->at(i);
                    else if (detector == LTCC_ID) ltcc_nphe += rche.nphe->at(i);
                    else return 10;
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
                set_pid(&(p_el[pi]), rpart.pid->at(pindex), status, tot_E,
                        pcal_E, htcc_nphe, ltcc_nphe,
                        sf_params[rtrk.sector->at(pos)]);
            }

            // Fill TNtuples with trigger electron info
            for (int pi = 0; pi < 2; ++pi) {
                if (!(p_el[pi].is_valid&&p_el[pi].is_trigger_electron))
                    continue;
                trigger_exist = true;
                Float_t v[VAR_LIST_SIZE] = {
                        (Float_t) run_no, (Float_t) evn, (Float_t) beam_E,
                        (Float_t) p_el[pi].pid, (Float_t) status,
                        (Float_t) p_el[pi].q, p_el[pi].mass, p_el[pi].vx,
                        p_el[pi].vy, p_el[pi].vz, p_el[pi].px, p_el[pi].py,
                        p_el[pi].pz, P(p_el[pi]), theta_lab(p_el[pi]),
                        phi_lab(p_el[pi]), p_el[pi].beta, chi2, ndf, pcal_E,
                        ecin_E, ecou_E, tot_E, (tof - tre_tof),
                        Q2(p_el[pi], beam_E), nu(p_el[pi], beam_E),
                        Xb(p_el[pi], beam_E), W2(p_el[pi], beam_E),
                        0, 0, 0, 0, 0
                };
                t_out[pi]->Fill(v);
            }
            if (trigger_exist){
                trigger_pindex = pindex;
                trigger_pos    = pos;
                break;
            }
        }

        // In case no trigger e was found, initiate p_el as dummy particles.
        if (!trigger_exist){
            p_el[0] = particle_init();
            p_el[1] = particle_init();
        }

        // Processing particles.
        for (UInt_t pos = 0; pos < rtrk.index->size(); ++pos) {
            int pindex = rtrk.pindex->at(pos); // pindex is always equal to pos!

            // Conditional to avoid trigger electron double counting.
            if (trigger_pindex == pindex && trigger_pos == pos) continue;

            // Get reconstructed particle from DC and from FMT.
            particle p[2];
            p[0] = particle_init(&rpart, &rtrk, pos);        // DC.
            p[1] = particle_init(&rpart, &rtrk, &ftrk, pos); // FMT.

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
                else return 9;
            }
            float tot_E = pcal_E + ecin_E + ecou_E;

            // Get Cherenkov counters data.
            int htcc_nphe = 0; // Number of photoelectrons deposited in htcc.
            int ltcc_nphe = 0; // Number of photoelectrons deposited in ltcc.
            for (UInt_t i = 0; i < rche.pindex->size(); ++i) {
                if (rche.pindex->at(i) == pindex) {
                    int detector = rche.detector->at(i);
                    if      (detector == HTCC_ID) htcc_nphe += rche.nphe->at(i);
                    else if (detector == LTCC_ID) ltcc_nphe += rche.nphe->at(i);
                    else return 10;
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
                set_pid(&(p[pi]), rpart.pid->at(pindex), status, tot_E, pcal_E,
                        htcc_nphe, ltcc_nphe, sf_params[rtrk.sector->at(pos)]);
            }

            // Test PID assignment precision.
            if (debug
                    && PID_QA.find(abs(rpart.pid->at(pindex))) != PID_QA.end()
                    && PID_QA.find(abs(p[0].pid)) != PID_QA.end()) {
                pid_n[PID_QA.at(abs(rpart.pid->at(pindex)))]++;
                pid_qa[PID_QA.at(abs(rpart.pid->at(pindex)))]
                        [PID_QA.at(abs(p[0].pid))]++;
            }

            // Fill TNtuples.
            // TODO. This probably should be implemented more elegantly.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            for (int pi = 0; pi < 2; ++pi) {
                if (!p[pi].is_valid) continue;
                Float_t v[VAR_LIST_SIZE] = {
                        (Float_t) run_no, (Float_t) evn, (Float_t) beam_E,
                        (Float_t) p[pi].pid, (Float_t) status,
                        (Float_t) p[pi].q, p[pi].mass, p[pi].vx, p[pi].vy,
                        p[pi].vz, p[pi].px, p[pi].py, p[pi].pz, P(p[pi]),
                        theta_lab(p[pi]), phi_lab(p[pi]), p[pi].beta, chi2, ndf,
                        pcal_E, ecin_E, ecou_E, tot_E, (tof - tre_tof),
                        // DIS e- functions just use the trigger electron.
                        Q2(p_el[pi], beam_E), nu(p_el[pi], beam_E),
                        Xb(p_el[pi], beam_E), W2(p_el[pi], beam_E),
                        // SIDIS functions use both particle and electron.
                        zh(p[pi], p_el[pi], beam_E),
                        Pt2(p[pi], p_el[pi], beam_E),
                        Pl2(p[pi], p_el[pi], beam_E),
                        phi_pq(p[pi], p_el[pi], beam_E),
                        theta_pq(p[pi], p_el[pi], beam_E)
                };

                t_out[pi]->Fill(v);
            }
        }
    }
    if (!debug) {
        printf("\33[2K\r");
        printf("[==================================================] 100%% \n");
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
    free(in_filename);

    if (ac_filename != NULL) {
        free(ac_filename);
        for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
        free(binnings);
        free(pids);
        for (int pi = 0; pi < pids_size; ++pi) free(acc_corr[pi]);
        free(acc_corr);
    }

    return 0;
}

int usage() {
    fprintf(stderr,
            "\nUsage: make_ntuples [-dh] [-n nevents] [-a accfile] file\n"
            " * -d         : activate debug mode.\n"
            " * -h         : show this message and exit.\n"
            " * -n nevents : number of events.\n"
            " * -a accfile : apply acceptance correction using accfile.\n"
            " * file       : ROOT file. Expected file format: "
            "<text>run_no.root`.\n\n"
            "    Generate ntuples relevant to SIDIS analysis based on the "
            "reconstructed\n    variables from CLAS12 data.\n\n"
    );

    return 1;
}

int handle_err(int errcode, char **file) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. Bad usage of optional arguments.\n\n");
            break;
        case 2:
            fprintf(stderr, "Error. nevents should be a number greater than"
                            " 0.\n\n");
            break;
        case 3:
            fprintf(stderr, "Error. input file should be in root format.\n\n");
            break;
        case 4:
            fprintf(stderr, "Error. file does not exist!\n\n");
            break;
        case 5:
            fprintf(stderr, "Error. Run number could not be extracted from "
                            "filename.\n\n");
            break;
        case 6:
            fprintf(stderr, "Error. Run number not in database. Add from "
                            "RCDB.\n\n");
            break;
        case 7:
            fprintf(stderr, "Error. No file name provided.\n\n");
            break;
        case 8:
            fprintf(stderr, "Error. %s is not a valid ROOT file.\n\n", *file);
            break;
        case 9:
            fprintf(stderr, "Error. Invalid EC layer. Check bank integrity."
                            "\n\n");
            break;
        case 10:
            fprintf(stderr, "Error. Invalid Cherenkov Counter ID. Check bank "
                            "integrity.\n\n");
            break;
        case 11:
            // NOTE. In this scenario, a smoother behavior would be that the
            //       program calls extract_sf itself!
            fprintf(stderr, "Error. No sampling fraction available for run "
                            "number! Run extract_sf before\ngenerating the "
                            "ntuples.\n\n");
            break;
        case 12:
            fprintf(stderr, "Error. Acceptance correction file was not found."
                            "\n\n");
            break;
        case 13:
            return usage();
        default:
            fprintf(stderr, "Error code %d not implemented!\n\n", errcode);
            return 1;
    }

    if (errcode > 2) free(*file);
    return usage();
}

int handle_args(int argc, char **argv, bool *debug, int *nevents,
        char **accfile, char **file, int *run_no, double *beam_energy)
{
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-dhn:a:")) != -1) {
        switch (opt) {
            case 'd': *debug   = true;         break;
            case 'h': return 13;
            case 'n': *nevents = atoi(optarg); break;
            case 'a':
                *accfile = (char *) malloc(strlen(optarg) + 1);
                strcpy(*accfile, optarg);
                break;
            case  1 :
                *file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*file, optarg);
                break;
            default: return 1; // Bad usage of optional arguments.
        }
    }
    // Check that nevents is valid and that atoi performed correctly.
    if (*nevents == 0) return 2;

    // Handle positional argument.
    if (argc < 2) return 7;

    return handle_root_filename(*file, run_no, beam_energy);
}

int main(int argc, char **argv) {
    bool dbg      = false;
    int nevn      = -1;
    char *accfile = NULL;
    int run_no    = -1;
    double beam_E = -1;
    char *file    = NULL;

    int errcode = handle_args(argc, argv, &dbg, &nevn, &accfile, &file, &run_no,
            &beam_E);
    if (errcode == 0) errcode = run(file, dbg, nevn, accfile, run_no, beam_E);
    return handle_err(errcode, &file);
}
