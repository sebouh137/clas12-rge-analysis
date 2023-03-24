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
#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TStyle.h>
#include "../lib/bank_containers.h"
#include "../lib/io_handler.h"
#include "../lib/utilities.h"

/** run() function of the program. Check usage() for details. */
static int run(char *in_file, char *work_dir, char *data_dir, int nevn,
        int run_no) {
    gStyle->SetOptFit();

    // Create output root file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/sf_study_%06d.root", work_dir, run_no);
    TFile *f_out = TFile::Open(out_file, "RECREATE");
    if (!f_out || f_out->IsZombie()) return 8;

    // Create output data file.
    char t_file[PATH_MAX];
    sprintf(t_file, "%s/sf_params_%06d.txt", data_dir, run_no);
    FILE *t_out = fopen(t_file, "w");
    if (t_out == NULL) return 9;

    // Access input file.
    TFile *f_in = TFile::Open(in_file, "READ");
    if (!f_in || f_in->IsZombie()) return 10;

    // Create and organize histos and name arrays.
    std::map<const char *, TH1 *> histos;

    const int ncals = sizeof(CALNAME)/sizeof(CALNAME[0]);
    char *sf1D_name_arr[ncals][NSECTORS][
            static_cast<long unsigned int>(((SF_PMAX - SF_PMIN)/SF_PSTEP))
    ];
    char *sf2D_name_arr[ncals][NSECTORS];
    TGraphErrors *sf_dotgraph[ncals][NSECTORS];
    char *sf2Dfit_name_arr[ncals][NSECTORS];
    TF1 *sf_polyfit[ncals][NSECTORS];
    double sf_fitresults[ncals][NSECTORS][SF_NPARAMS][2];

    int cal_idx = -1;
    for (const char *cal : SFARR2D) {
        cal_idx++;
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            // Initialize dotgraphs.
            char *tmp_str = Form("%s%d)", cal, sector_i+1);
            sf2D_name_arr[cal_idx][sector_i] =
                    static_cast<char *>(malloc(strlen(tmp_str)+1));
            strncpy(sf2D_name_arr[cal_idx][sector_i], tmp_str, strlen(tmp_str));
            insert_TH2F(
                    &histos, R_PALL, sf2D_name_arr[cal_idx][sector_i], S_P,
                    S_EDIVP, 200, 0, 10, 200, 0, 0.4
            );
            sf_dotgraph[cal_idx][sector_i] = new TGraphErrors();
            sf_dotgraph[cal_idx][sector_i]->SetMarkerStyle(kFullCircle);
            sf_dotgraph[cal_idx][sector_i]->SetMarkerColor(kRed);

            // Initialize fits.
            sf2Dfit_name_arr[cal_idx][sector_i] =
                    static_cast<char *>(malloc(strlen(tmp_str)+1));
            strncpy(
                    sf2Dfit_name_arr[cal_idx][sector_i], tmp_str,
                    strlen(tmp_str)
            );
            sf_polyfit[cal_idx][sector_i] = new TF1(
                    sf2Dfit_name_arr[cal_idx][sector_i],
                    "[0]*([1]+[2]/x + [3]/(x*x))", SF_PMIN+SF_PSTEP,
                    SF_PMAX-SF_PSTEP
            );
            sf_polyfit[cal_idx][sector_i]->SetParameter(0 /* p0 */, 0.25);
            sf_polyfit[cal_idx][sector_i]->SetParameter(1 /* p1 */, 1);
            sf_polyfit[cal_idx][sector_i]->SetParameter(2 /* p2 */, 0);
            sf_polyfit[cal_idx][sector_i]->SetParameter(3 /* p3 */, 0);
        }
    }

    cal_idx = -1;
    for (const char *cal : SFARR1D) {
        cal_idx++;
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            int param_i = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                param_i++;
                char *tmp_str = Form(
                        "%s%d (%5.2f < p < %5.2f)", cal, sector_i + 1, p,
                        p + SF_PSTEP
                );
                sf1D_name_arr[cal_idx][sector_i][param_i] =
                        static_cast<char *>(malloc(strlen(tmp_str)+1));
                strncpy(
                        sf1D_name_arr[cal_idx][sector_i][param_i], tmp_str,
                        strlen(tmp_str)
                );
                insert_TH1F(
                        &histos, R_PALL,
                        sf1D_name_arr[cal_idx][sector_i][param_i], S_EDIVP,
                        200, 0, 0.4
                );
            }
        }
    }

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    Particle particle(t);
    Track track(t);
    Calorimeter calorimeter(t);

    // Iterate through input file. Each TTree entry is one event.
    int divcntr = 0;
    int evnsplitter = 0;
    if (nevn == -1 || t->GetEntries() < nevn) nevn = t->GetEntries();

    printf("Reading %d events from %s.\n", nevn, in_file);
    for (int evn = 0; evn < nevn; ++evn) {
        update_progress_bar(nevn, evn, &evnsplitter, &divcntr);

        // Get entries from bank containers.
        particle.get_entries(t, evn);
        track.get_entries(t, evn);
        calorimeter.get_entries(t, evn);

        // Skip events without the necessary banks.
        if (
                particle.vz->size() == 0 ||
                track.pindex->size() == 0 ||
                calorimeter.pindex->size() == 0
        ) {
            continue;
        }

        for (long unsigned int pos = 0; pos < track.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            long unsigned int pindex =
                    static_cast<long unsigned int>(track.pindex->at(pos));

            // Get particle momentum.
            double px = particle.px->at(pindex);
            double py = particle.py->at(pindex);
            double pz = particle.pz->at(pindex);
            double total_p = calc_magnitude(px, py, pz);

            // Compute energy deposited in each calorimeter per sector.
            double sf_E[ncals][NSECTORS];
            for (int cal_i = 0; cal_i < ncals; ++cal_i) {
                for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
                    sf_E[cal_i][sector_i] = 0;
                }
            }

            for (
                    unsigned long int entry_i = 0;
                    entry_i < calorimeter.pindex->size();
                    ++entry_i
            ) {
                if (
                        static_cast<long unsigned int>(
                                calorimeter.pindex->at(entry_i)
                        ) != pindex
                ) {
                    continue;
                }

                // Get sector.
                int sector_i = calorimeter.sector->at(entry_i) - 1;
                if (sector_i == -1) continue;
                if (sector_i < -1 || sector_i > NSECTORS-1) return 11;

                // Get detector.
                switch(calorimeter.layer->at(entry_i)) {
                    case PCAL_LYR:
                        sf_E[PCAL_IDX][sector_i] +=
                                calorimeter.energy->at(entry_i);
                        break;
                    case ECIN_LYR:
                        sf_E[ECIN_IDX][sector_i] +=
                                calorimeter.energy->at(entry_i);
                        break;
                    case ECOU_LYR:
                        sf_E[ECOU_IDX][sector_i] +=
                                calorimeter.energy->at(entry_i);
                        break;
                    default:
                        return 12;
                }
            }

            for (int cal_i = 0; cal_i < ncals-1; ++cal_i) {
                for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
                    sf_E[CALS_IDX][sector_i] += sf_E[cal_i][sector_i];
                }
            }

            // Get momentum bin.
            if (total_p < SF_PMIN || total_p > SF_PMAX) continue;
            int param_i = -1;
            for (double p_cnt = SF_PMIN; p_cnt <= SF_PMAX; p_cnt += SF_PSTEP) {
                if (total_p < p_cnt) break;
                ++param_i;
            }

            // Write to histograms.
            for (int cal_i = 0; cal_i < ncals; ++cal_i) {
                for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
                    if (sf_E[cal_i][sector_i] <= 0) continue;
                    histos[sf2D_name_arr[cal_i][sector_i]]->Fill(
                            total_p, sf_E[cal_i][sector_i]/total_p
                    );
                    histos[sf1D_name_arr[cal_i][sector_i][param_i]]->Fill(
                            sf_E[cal_i][sector_i]/total_p
                    );
                }
            }
        }
    }

    // Fit histograms.
    cal_idx = -1;
    for (const char *cal : SFARR1D) {
        ++cal_idx;
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            int param_idx = -1;
            int point_idx = 0;
            // Fit 1D plots for each momentum bin to fill dotgraphs.
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                ++param_idx;

                // Get ref to histogram.
                TH1 *EdivP = histos[
                        sf1D_name_arr[cal_idx][sector_i][param_idx]
                ];

                // Form fit string name.
                char *tmp_str = Form(
                        "%s%d (%5.2f < p < %5.2f) fit", cal, sector_i+1,
                        p, p+SF_PSTEP
                );

                // Fit.
                TF1 *sf_gaus = new TF1(tmp_str,
                        "[0]*TMath::Gaus(x,[1],[2]) + [3]*x*x + [4]*x + [5]",
                        PLIMITSARR[cal_idx][0], PLIMITSARR[cal_idx][1]);
                sf_gaus->SetParameter(0 /* amp   */,
                        EdivP->GetBinContent(EdivP->GetMaximumBin()));
                sf_gaus->SetParLimits(1,
                        PLIMITSARR[cal_idx][0], PLIMITSARR[cal_idx][1]
                );
                sf_gaus->SetParameter(1 /* mean  */,
                        (PLIMITSARR[cal_idx][1] + PLIMITSARR[cal_idx][0])/2
                );
                sf_gaus->SetParLimits(2, 0., 0.1);
                sf_gaus->SetParameter(2 /* sigma */, 0.05);
                sf_gaus->SetParameter(3 /* p0 */,    0);
                sf_gaus->SetParameter(4 /* p1 */,    0);
                sf_gaus->SetParameter(5 /* p2 */,    0);
                EdivP->Fit(
                        sf_gaus, "QR", "",
                        PLIMITSARR[cal_idx][0], PLIMITSARR[cal_idx][1]
                );

                // Extract mean and sigma from fit and add it to 2D plots.
                double mean  = sf_gaus->GetParameter(1);
                double sigma = sf_gaus->GetParameter(2);

                // Only add points within PLIMITSARR borders and with an
                // acceptable chi2.
                if (
                        (
                                mean - 2*sigma > PLIMITSARR[cal_idx][0] &&
                                mean + 2*sigma < PLIMITSARR[cal_idx][1]
                        ) &&
                        (
                                sf_gaus->GetChisquare() / sf_gaus->GetNDF() <
                                SF_CHI2CONFORMITY
                        )
                ) {
                    sf_dotgraph[cal_idx][sector_i]->SetPoint(
                            point_idx, p + SF_PSTEP/2, mean
                    );
                    point_idx++;
                }
            }

            // Fit dotgraphs.
            if (sf_dotgraph[cal_idx][sector_i]->GetN() > 0) {
                sf_dotgraph[cal_idx][sector_i]->Fit(
                        sf_polyfit[cal_idx][sector_i], "QR", "",
                        SF_PMIN+SF_PSTEP, SF_PMAX-SF_PSTEP
                );
            }

            // Extract and save dotgraph fits parameters to make cuts from them.
            for (
                    int param_i = 0;
                    param_i < sf_polyfit[cal_idx][sector_i]->GetNpar();
                    ++param_i
            ) {
                // Sampling fraction (sf in CCDB).
                sf_fitresults[cal_idx][sector_i][param_i][0] =
                        sf_polyfit[cal_idx][sector_i]->GetParameter(param_i);
                // Sampling fraction sigma (sfs in CCDB).
                sf_fitresults[cal_idx][sector_i][param_i][1] =
                        sf_polyfit[cal_idx][sector_i]->GetParError(param_i);
            }
        }
    }

    // Write to output file.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (int cal_i = 0; cal_i < ncals; ++cal_i) {
        dir = Form("%s", CALNAME[cal_i]);
        f_out->mkdir(dir);
        f_out->cd(dir);
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            dir = Form("%s/sector %d", CALNAME[cal_i], sector_i+1);
            f_out->mkdir(dir);
            f_out->cd(dir);

            histos[sf2D_name_arr[cal_i][sector_i]]->Draw("colz");
            sf_dotgraph[cal_i][sector_i]->Draw("Psame");
            sf_polyfit[cal_i][sector_i]->Draw("same");
            gcvs->Write(sf2D_name_arr[cal_i][sector_i]);

            // Free arrays after writing.
            free(sf2D_name_arr[cal_i][sector_i]);
            for (
                    int param_i = 0;
                    param_i < static_cast<int>(((SF_PMAX-SF_PMIN) / SF_PSTEP));
                    ++param_i
            ) {
                histos[sf1D_name_arr[cal_i][sector_i][param_i]]->Write();
                free(sf1D_name_arr[cal_i][sector_i][param_i]);
            }
            free(sf2Dfit_name_arr[cal_i][sector_i]);
        }
    }

    // Write results to data file.
    // NOTE. Only writing ECAL sf results.
    for (int cal_i = 3; cal_i < 4; ++cal_i) {
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            for (int sf_i = 0; sf_i < 2; ++sf_i) { // sf and sfs.
                for (int param_i = 0; param_i < SF_NPARAMS; ++param_i) {
                    fprintf(
                            t_out, "%011.8f ",
                            sf_fitresults[cal_i][sector_i][param_i][0]
                    );
                }
            }
            fprintf(t_out, "\n");
        }
    }

    fclose(t_out);
    f_in ->Close();
    f_out->Close();

    return 0;
}

/** Print usage and exit. */
static int usage() {
    fprintf(stderr,
            "\n\nUsage: extract_sf [-hn:w:d:] infile\n"
            " * -h         : show this message and exit.\n"
            " * -n nevents : number of events\n"
            " * -w workdir : location where output root files are to be "
            "stored. Default\n                is root_io.\n"
            " * -d datadir : location where sampling fraction files are "
            "located. Default\n                is data.\n"
            " * infile     : input ROOT file. Expected file format: "
            "<text>run_no.root.\n\n"
            "    Obtain the EC sampling fraction from an input file.\n\n"
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
            fprintf(stderr, "nevents should greater than 0.");
            break;
        case 3:
            fprintf(stderr, "No file name provided.");
            break;
        case 4:
            fprintf(stderr, "Input file should be in root format.");
            break;
        case 5:
            fprintf(stderr, "Input file does not exist.");
            break;
        case 6:
            fprintf(stderr, "Couldn't find dot position in filename.");
            break;
        case 7:
            fprintf(stderr, "Couldn't get run number from input file.");
            break;
        case 8:
            fprintf(stderr, "Couldn't create output root file.");
            break;
        case 9:
            fprintf(stderr, "Couldn't create output text file.");
            break;
        case 10:
            fprintf(stderr, "Couldn't open input root file.");
            break;
        case 11:
            fprintf(stderr, "Invalid particle sector. Check bank integrity.");
            break;
        case 12:
            fprintf(stderr, "Invalid EC layer. Check bank integrity.");
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
static int handle_args(int argc, char **argv, char **in_file, char **work_dir,
        char **data_dir, int *run_no, int *nevn)
{
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'n':
                *nevn = atoi(optarg);
                if (*nevn <= 0) return 2;
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
                *in_file = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*in_file, optarg);
                break;
            default:
                return 1;
        }
    }

    // Define workdir if undefined.
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
    if (*in_file == NULL) return 3;

    // Handle input filename.
    int check = handle_root_filename(*in_file, run_no);
    if (!check || check == 5) return 0;
    else                      return check + 3; // Shift errcode.
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_file  = NULL;
    char *work_dir = NULL;
    char *data_dir = NULL;
    int nevn       = -1;
    int run_no     = -1;

    int errcode = handle_args(
            argc, argv, &in_file, &work_dir, &data_dir, &run_no, &nevn
    );

    // Run.
    if (errcode == 0) {
        errcode = run(in_file, work_dir, data_dir, nevn, run_no);
    }

    // Free up memory.
    if (in_file  != NULL) free(in_file);
    if (work_dir != NULL) free(work_dir);
    if (data_dir != NULL) free(data_dir);

    // Return errcode.
    return handle_err(errcode);
}
