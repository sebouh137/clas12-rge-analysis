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
#include "libgen.h"

// C++.
#include <map>
#include <utility>

// ROOT.
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>

// rge-analysis.
#include "../lib/constants.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_hipo_bank.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_math_utils.h"
#include "../lib/rge_progress.h"

static const char *USAGE_MESSAGE =
"Usage: extract_sf [-hn:w:d:] infile\n"
" * -h         : show this message and exit.\n"
" * -n nevents : number of events\n"
" * -w workdir : location where output root files are to be stored. Default\n"
"                is root_io.\n"
" * -d datadir : location where sampling fraction files are stored. Default\n"
"                is data.\n"
" * infile     : input ROOT file. Expected file format: <text>run_no.root.\n\n"
"    Obtain the EC sampling fraction from an input file.\n";

/** Histogram name. */
#define S_EDIVP "E/p"

/** Sampling fraction array pre-defined integers. */
#define NCALS 4    /** Number of calorimeters (PCAL, ECIN, ECOU, ALL). */
#define PCAL_IDX 0 /** PCAL idx in arrays. */
#define ECIN_IDX 1 /** ECIN idx in arrays. */
#define ECOU_IDX 2 /** ECOU idx in arrays. */
#define CALS_IDX 3 /** CALs idx in arrays. */

/** Momentum bin parameters, in GeV. */
#define SF_PMIN  1.0 /** Minimum value. */
#define SF_PMAX  9.0 /** Maximum value. */
#define SF_PSTEP 0.4 /** Step size to separate each bin. */

/** Chi2 conformity for sampling fraction fits. */
#define SF_CHI2CONFORMITY 2

/** Calorimeter names. */
static const char *CALNAME[NCALS] = {
        "PCAL", "ECIN", "ECOU", "ALL"
};

/** Sampling fraction 1D and 2D array names. */
static const char *SFARR1D[NCALS] = {
        "PCAL Sampling Fraction sector ",
        "ECIN Sampling Fraction sector ",
        "ECOU Sampling Fraction sector ",
        "CALs Sampling Fraction sector "
};
static const char *SFARR2D[NCALS] = {
        "Vp vs E/Vp (PCAL sector ",
        "Vp vs E/Vp (ECIN sector ",
        "Vp vs E/Vp (ECOU sector ",
        "Vp vs E/Vp (CALs sector "
};

/** Momentum limits for 1D sampling fraction fits. */
static const double PLIMITSARR[NCALS][2] = {
        {0.060, 0.250},
        {0.015, 0.120},
        {0.000, 0.400},
        {0.150, 0.300}
};

/**
 * Insert a 1-dimensional histogram of floats into a map.
 *
 * @param map  : Map onto which we'll insert the histogram.
 * @param k    : Name of the particle (or set of particles) in the histogram.
 * @param n    : Name of the histogram to be inserted.
 * @param xn   : Name of the variable in the histogram's x axis.
 * @param bins : Number of bins in the histogram.
 * @param min  : Minimum value for the x axis of the histogram.
 * @param max  : Maximum value for the x axis of the histogram.
 * @return     : Success code (0).
 */
static int insert_TH1F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *xn, int bins, double min, double max
) {
    map->insert(std::pair<const char *, TH1 *> (n, new TH1F(
            Form("%s: %s", k, n), Form("%s;%s", n, xn), bins, min, max
    )));
    return 0;
}

/**
 * Insert a 2-dimensional histogram of floats into a map.
 *
 * @param map   : Map onto which we'll insert the histogram.
 * @param k     : Name of the particle (or set of particles) in the histogram.
 * @param n     : Name of the histogram to be inserted.
 * @param nx    : Name of the variable in the histogram's x axis.
 * @param ny    : Name of the variable in the histogram's y axis.
 * @param xbins : Number of bins in the x axis of the histogram.
 * @param ybins : Number of bins in the y axis of the histogram.
 * @param xmin  : Minimum value for the x axis of the histogram.
 * @param xmax  : Maximum value for the x axis of the histogram.
 * @param ymin  : Minimum value for the y axis of the histogram.
 * @param ymax  : Maximum value for the y axis of the histogram.
 * @return      : Success code (0).
 */
static int insert_TH2F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *nx, const char *ny, int xbins, double xmin, double xmax,
        int ybins, double ymin, double ymax
) {
    map->insert(std::pair<const char *, TH1 *> (n, new TH2F(
            Form("%s: %s", k, n), Form("%s;%s;%s", n, nx, ny),
            xbins, xmin, xmax, ybins, ymin, ymax
    )));
    return 0;
}

/** run() function of the program. Check USAGE_MESSAGE for details. */
static int run(
        char *in_filename, char *work_dir, char *data_dir, lint nevn, int run_no
) {
    // Configure ROOT fitting.
    gStyle->SetOptFit();

    // Create output root file.
    char out_rootfilename[PATH_MAX];
    sprintf(out_rootfilename, "%s/sf_study_%06d.root", work_dir, run_no);
    TFile *out_rootfile = TFile::Open(out_rootfilename, "RECREATE");
    if (!out_rootfile || out_rootfile->IsZombie()) {
        rge_errno = RGEERR_OUTPUTROOTFAILED;
        return 1;
    }

    // Create output data file.
    char out_textfilename[PATH_MAX];
    sprintf(out_textfilename, "%s/sf_params_%06d.txt", data_dir, run_no);
    FILE *out_textfile = fopen(out_textfilename, "w");
    if (out_textfile == NULL) {
        rge_errno = RGEERR_OUTPUTTEXTFAILED;
        return 1;
    }

    // Access input file.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) {
        rge_errno = RGEERR_BADINPUTFILE;
        return 1;
    }

    // Create histogram and name arrays.
    std::map<const char *, TH1 *> histos;

    char *sf1D_name_arr[NCALS][NSECTORS][
            static_cast<luint>(((SF_PMAX - SF_PMIN)/SF_PSTEP))
    ];
    char *sf2D_name_arr[NCALS][NSECTORS];
    TGraphErrors *sf_dotgraph[NCALS][NSECTORS];
    char *sf2Dfit_name_arr[NCALS][NSECTORS];
    TF1 *sf_polyfit[NCALS][NSECTORS];
    double sf_fitresults[NCALS][NSECTORS][SF_NPARAMS][2];

    // Configure 2D histogram arrays.
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

    // Configure 1D histogram arrays.
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
    TTree *t = f_in->Get<TTree>(RGE_TREENAMEDATA);
    rge_hipobank particle    = rge_hipobank_init(RGE_RECPARTICLE,    t);
    rge_hipobank track       = rge_hipobank_init(RGE_RECTRACK,       t);
    rge_hipobank calorimeter = rge_hipobank_init(RGE_RECCALORIMETER, t);

    // Iterate through input file. Each TTree entry is one event.
    if (nevn == -1 || t->GetEntries() < nevn) nevn = t->GetEntries();
    rge_pbar_set_nentries(nevn);

    printf("Reading %ld events from %s.\n", nevn, in_filename);
    for (lint evn = 0; evn < nevn; ++evn) {
        rge_pbar_update(evn);

        // Get entries from bank containers.
        rge_get_entries(&particle,    t, evn);
        rge_get_entries(&track,       t, evn);
        rge_get_entries(&calorimeter, t, evn);

        // Skip events without the necessary banks.
        if (
                rge_get_size(&particle,    "vz")     == 0 ||
                rge_get_size(&track,       "pindex") == 0 ||
                rge_get_size(&calorimeter, "pindex") == 0
        ) {
            continue;
        }

        // Iterate through entries and write data to histograms.
        luint npos = rge_get_size(&track, "pindex");
        for (luint pos = 0; pos < npos; ++pos) {
            // Get basic data from track and particle banks.
            luint pindex = static_cast<luint>(
                    track.entries.at("pindex").data->at(pos)
            );

            // Get particle momentum.
            double px = rge_get_entry(&particle, "px", pindex);
            double py = rge_get_entry(&particle, "py", pindex);
            double pz = rge_get_entry(&particle, "pz", pindex);
            double total_p = rge_calc_magnitude(px, py, pz);

            // Compute energy deposited in each calorimeter per sector.
            double sf_E[NCALS][NSECTORS];
            for (int cal_i = 0; cal_i < NCALS; ++cal_i) {
                for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
                    sf_E[cal_i][sector_i] = 0;
                }
            }

            for (
                    luint entry_i = 0;
                    entry_i < rge_get_size(&calorimeter, "pindex");
                    ++entry_i
            ) {
                if (static_cast<luint>(
                        rge_get_entry(&calorimeter, "pindex", entry_i)
                ) != pindex) {
                    continue;
                }

                // Get sector.
                int sector_i =
                        rge_get_entry(&calorimeter, "sector", entry_i) - 1;
                if (sector_i == -1) continue;
                if (sector_i < -1 || sector_i > NSECTORS-1) {
                    rge_errno = RGEERR_INVALIDCALSECTOR;
                    return 1;
                }

                // Get detector.
                double energy = rge_get_entry(&calorimeter, "energy", entry_i);
                switch(static_cast<int>(
                        rge_get_entry(&calorimeter, "layer", entry_i)
                )) {
                    case PCAL_LYR:
                        sf_E[PCAL_IDX][sector_i] += energy;
                        break;
                    case ECIN_LYR:
                        sf_E[ECIN_IDX][sector_i] += energy;
                        break;
                    case ECOU_LYR:
                        sf_E[ECOU_IDX][sector_i] += energy;
                        break;
                    default:
                        rge_errno = RGEERR_INVALIDCALLAYER;
                        return 1;
                }
            }

            for (int cal_i = 0; cal_i < NCALS-1; ++cal_i) {
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
            for (int cal_i = 0; cal_i < NCALS; ++cal_i) {
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

    // Write to output root file to visualize the fits.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (int cal_i = 0; cal_i < NCALS; ++cal_i) {
        dir = Form("%s", CALNAME[cal_i]);
        out_rootfile->mkdir(dir);
        out_rootfile->cd(dir);
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            dir = Form("%s/sector %d", CALNAME[cal_i], sector_i+1);
            out_rootfile->mkdir(dir);
            out_rootfile->cd(dir);

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

    // Write ECAL sf results to output text file.
    for (int cal_i = 3; cal_i < 4; ++cal_i) {
        for (int sector_i = 0; sector_i < NSECTORS; ++sector_i) {
            for (int sf_i = 0; sf_i < 2; ++sf_i) { // sf and sfs.
                for (int param_i = 0; param_i < SF_NPARAMS; ++param_i) {
                    fprintf(
                            out_textfile, "%011.8f ",
                            sf_fitresults[cal_i][sector_i][param_i][0]
                    );
                }
            }
            fprintf(out_textfile, "\n");
        }
    }

    // Clean up after ourselves.
    fclose(out_textfile);
    f_in ->Close();
    out_rootfile->Close();

    // Exit.
    rge_errno = RGEERR_NOERR;
    return 0;
}

/**
 * Handle arguments for make_ntuples using optarg. Error codes used are
 *     explained in the handle_err() function.
 */
static int handle_args(
        int argc, char **argv, char **in_filename, char **work_dir,
        char **data_dir, int *run_no, lint *nevn
) {
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:w:d:")) != -1) {
        switch (opt) {
            case 'h':
                rge_errno = RGEERR_USAGE;
                return 1;
            case 'n':
                if (rge_process_nentries(nevn, optarg)) return 1;
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
                *in_filename = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*in_filename, optarg);
                break;
            default:
                rge_errno = RGEERR_BADOPTARGS;
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
    if (*in_filename == NULL) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }

    // Handle input filename.
    if (rge_handle_root_filename(*in_filename, run_no)) return 1;

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename = NULL;
    char *work_dir    = NULL;
    char *data_dir    = NULL;
    lint nevn         = -1;
    int run_no        = -1;

    int err = handle_args(
            argc, argv, &in_filename, &work_dir, &data_dir, &run_no, &nevn
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED && err == 0) {
        run(in_filename, work_dir, data_dir, nevn, run_no);
    }

    // Free up memory.
    if (in_filename != NULL) free(in_filename);
    if (work_dir    != NULL) free(work_dir);
    if (data_dir    != NULL) free(data_dir);

    // Return errcode.
    return rge_print_usage(USAGE_MESSAGE);
}
