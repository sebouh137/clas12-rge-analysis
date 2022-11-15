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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TStyle.h>
#include <TTree.h>

#include "../lib/bank_containers.h"
#include "../lib/constants.h"
#include "../lib/err_handler.h"
#include "../lib/file_handler.h"
#include "../lib/io_handler.h"
#include "../lib/utilities.h"

int run(char *in_filename, bool use_fmt, int nevn, int run_no) {
    gStyle->SetOptFit();

    // Access input file.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 1;

    // Create and organize histos and name arrays.
    std::map<const char *, TH1 *> histos;

    const int ncals = sizeof(CALNAME)/sizeof(CALNAME[0]);
    char *sf1D_name_arr[ncals][NSECTORS][(int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)];
    char *sf2D_name_arr[ncals][NSECTORS];
    TGraphErrors *sf_dotgraph[ncals][NSECTORS];
    char *sf2Dfit_name_arr[ncals][NSECTORS];
    TF1 *sf_polyfit[ncals][NSECTORS];
    double sf_fitresults[ncals][NSECTORS][SF_NPARAMS][2];

    int ci = -1;
    for (const char *cal : SFARR2D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            // Initialize dotgraphs.
            char * tmp_str = Form("%s%d)", cal, si+1);
            sf2D_name_arr[ci][si] = (char *) malloc(strlen(tmp_str)+1);
            strncpy(sf2D_name_arr[ci][si], tmp_str, strlen(tmp_str));
            insert_TH2F(&histos, R_PALL, sf2D_name_arr[ci][si], S_P, S_EDIVP,
                        200, 0, 10, 200, 0, 0.4);
            sf_dotgraph[ci][si] = new TGraphErrors();
            sf_dotgraph[ci][si]->SetMarkerStyle(kFullCircle);
            sf_dotgraph[ci][si]->SetMarkerColor(kRed);

            // Initialize fits.
            sf2Dfit_name_arr[ci][si] = (char *) malloc(strlen(tmp_str)+1);
            strncpy(sf2Dfit_name_arr[ci][si], tmp_str, strlen(tmp_str));
            sf_polyfit[ci][si] = new TF1(sf2Dfit_name_arr[ci][si],
                    "[0]*([1]+[2]/x + [3]/(x*x))", SF_PMIN+SF_PSTEP,
                    SF_PMAX-SF_PSTEP);
            sf_polyfit[ci][si]->SetParameter(0 /* p0 */, 0.25);
            sf_polyfit[ci][si]->SetParameter(1 /* p1 */, 1);
            sf_polyfit[ci][si]->SetParameter(2 /* p2 */, 0);
            sf_polyfit[ci][si]->SetParameter(3 /* p3 */, 0);
        }
    }

    ci = -1;
    for (const char *cal : SFARR1D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            int pi = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                pi++;
                char * tmp_str = Form("%s%d (%5.2f < p < %5.2f)", cal, si+1, p,
                        p+SF_PSTEP);
                sf1D_name_arr[ci][si][pi] = (char *) malloc(strlen(tmp_str)+1);
                strncpy(sf1D_name_arr[ci][si][pi], tmp_str, strlen(tmp_str));
                insert_TH1F(&histos, R_PALL, sf1D_name_arr[ci][si][pi], S_EDIVP,
                        200, 0, 0.4);
            }
        }
    }

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    REC_Particle     rp(t);
    REC_Track        rt(t);
    REC_Calorimeter  rc(t);
    FMT_Tracks       ft(t);

    // Iterate through input file. Each TTree entry is one event.
    int evn;
    int divcntr = 0;
    int evnsplitter = 0;
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() :
            nevn, in_filename);
    for (evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
        if (evn >= evnsplitter) {
            if (evn != 0) printf("\33[2K\r");
            printf("[");
            for (int i = 0; i <= 50; ++i) {
                if (i <= divcntr/2) printf("=");
                else                printf(" ");
            }
            printf("] %2d%%", divcntr);
            fflush(stdout);
            divcntr++;
            evnsplitter = nevn == -1 ? (t->GetEntries() / 100) * divcntr :
                    (nevn/100) * divcntr;
        }

        rp.get_entries(t, evn);
        rt.get_entries(t, evn);
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0 ||
                rc.pindex->size() == 0) continue;

        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index  = rt.index ->at(pos);
            int pindex = rt.pindex->at(pos);

            // Get particle momentum from either FMT or DC.
            double px, py, pz;
            if (use_fmt) {
                // Apply FMT cuts.
                // Track reconstructed by FMT.
                if (ft.pz->size() < 1)      continue;
                // Track crossed 3 FMT layers.
                if (ft.ndf->at(index) != 3) continue;

                px = ft.px->at(index);
                py = ft.py->at(index);
                pz = ft.pz->at(index);
            }
            else {
                px = rp.px->at(pindex);
                py = rp.py->at(pindex);
                pz = rp.pz->at(pindex);
            }
            double tot_P = calc_magnitude(px, py, pz);

            // Compute energy deposited in each calorimeter per sector.
            double sf_E[ncals][NSECTORS];
            for (int ci = 0; ci < ncals; ++ci) {
                for (int si = 0; si < NSECTORS; ++si) sf_E[ci][si] = 0;
            }

            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) != pindex) continue;

                // Get sector.
                int si = rc.sector->at(i) - 1;
                if      (si == -1)                   continue;
                else if (si < -1 || si > NSECTORS-1) return 3;

                // Get detector.
                switch(rc.layer->at(i)) {
                    case PCAL_LYR: sf_E[PCAL_IDX][si] += rc.energy->at(i); break;
                    case ECIN_LYR: sf_E[ECIN_IDX][si] += rc.energy->at(i); break;
                    case ECOU_LYR: sf_E[ECOU_IDX][si] += rc.energy->at(i); break;
                    default:       return 2;
                }
            }

            for (int ci = 0; ci < ncals-1; ++ci) {
                for (int si = 0; si < NSECTORS; ++si)
                    sf_E[CALS_IDX][si] += sf_E[ci][si];
            }

            // Get momentum bin.
            if (tot_P < SF_PMIN || tot_P > SF_PMAX) continue;
            int pi = -1;
            for (double p_cnt = SF_PMIN; p_cnt <= SF_PMAX; p_cnt += SF_PSTEP) {
                if (tot_P < p_cnt) break;
                pi++;
            }

            // Write to histograms.
            for (int ci = 0; ci < ncals; ++ci) {
                for (int si = 0; si < NSECTORS; ++si) {
                    if (sf_E[ci][si] <= 0) continue;
                    histos[sf2D_name_arr[ci][si]]->Fill(tot_P, sf_E[ci][si]/tot_P);
                    histos[sf1D_name_arr[ci][si][pi]]->Fill(sf_E[ci][si]/tot_P);
                }
            }
        }
    }
    printf("\33[2K\r");
    printf("[==================================================] 100%%\n");

    // Fit histograms.
    ci = -1;
    for (const char *cal : SFARR1D) {
        ci++;
        for (int si = 0; si < NSECTORS; ++si) {
            int pi = -1;
            int point_index = 0;
            // Fit 1D plots for each momentum bin to fill dotgraphs.
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                pi++;

                // Get ref to histogram.
                TH1 *EdivP = histos[sf1D_name_arr[ci][si][pi]];

                // Form fit string name.
                char * tmp_str = Form("%s%d (%5.2f < p < %5.2f) fit", cal, si+1,
                        p, p+SF_PSTEP);

                // Fit.
                TF1 *sf_gaus = new TF1(tmp_str,
                        "[0]*TMath::Gaus(x,[1],[2]) + [3]*x*x + [4]*x + [5]",
                        PLIMITSARR[ci][0], PLIMITSARR[ci][1]);
                sf_gaus->SetParameter(0 /* amp   */,
                        EdivP->GetBinContent(EdivP->GetMaximumBin()));
                sf_gaus->SetParLimits(1, PLIMITSARR[ci][0], PLIMITSARR[ci][1]);
                sf_gaus->SetParameter(1 /* mean  */,
                        (PLIMITSARR[ci][1] + PLIMITSARR[ci][0])/2);
                sf_gaus->SetParLimits(2, 0., 0.1);
                sf_gaus->SetParameter(2 /* sigma */, 0.05);
                sf_gaus->SetParameter(3 /* p0 */,    0);
                sf_gaus->SetParameter(4 /* p1 */,    0);
                sf_gaus->SetParameter(5 /* p2 */,    0);
                EdivP->Fit(sf_gaus, "QR", "",
                        PLIMITSARR[ci][0], PLIMITSARR[ci][1]);

                // Extract mean and sigma from fit and add it to 2D plots.
                double mean  = sf_gaus->GetParameter(1);
                double sigma = sf_gaus->GetParameter(2);

                // Only add points within PLIMITSARR borders and with an
                // acceptable chi2.
                if ((mean - 2*sigma > PLIMITSARR[ci][0] &&
                        mean + 2*sigma < PLIMITSARR[ci][1]) &&
                        (sf_gaus->GetChisquare() / sf_gaus->GetNDF() <
                        SF_CHI2CONFORMITY)) {
                        // Older root compatibility fix
                        sf_dotgraph[ci][si]->SetPoint(point_index,
                                p + SF_PSTEP/2, mean);
                        point_index++;
                }
            }

            // Fit dotgraphs.
            if (sf_dotgraph[ci][si]->GetN() > 0)
                sf_dotgraph[ci][si]->Fit(sf_polyfit[ci][si], "QR", "",
                                         SF_PMIN+SF_PSTEP, SF_PMAX-SF_PSTEP);

            // Extract and save dotgraph fits parameters to make cuts from them.
            for (int pi = 0; pi < sf_polyfit[ci][si]->GetNpar(); ++pi) {
                // sf.
                sf_fitresults[ci][si][pi][0] = sf_polyfit[ci][si]->GetParameter(pi);
                // sfs.
                sf_fitresults[ci][si][pi][1] = sf_polyfit[ci][si]->GetParError(pi);
            }
        }
    }

    // Create output file.
    char*  out_filename = (char *) malloc(128 * sizeof(char));
    sprintf(out_filename, "../root_io/sf_study_%06d.root", run_no);

    TFile *f_out = TFile::Open(out_filename, "RECREATE");
    // Write to output file.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (ci = 0; ci < ncals; ++ci) {
        dir = Form("%s", CALNAME[ci]);
        f_out->mkdir(dir);
        f_out->cd(dir);
        for (int si = 0; si < NSECTORS; ++si) {
            dir = Form("%s/sector %d", CALNAME[ci], si+1);
            f_out->mkdir(dir);
            f_out->cd(dir);

            histos[sf2D_name_arr[ci][si]]->Draw("colz");
            sf_dotgraph[ci][si]->Draw("Psame");
            sf_polyfit[ci][si]->Draw("same");
            gcvs->Write(sf2D_name_arr[ci][si]);
            free(sf2D_name_arr[ci][si]);
            for (int pi = 0; pi < ((int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)); ++pi) {
                histos[sf1D_name_arr[ci][si][pi]]->Write();
                free(sf1D_name_arr[ci][si][pi]);
            }
            free(sf2Dfit_name_arr[ci][si]);
        }
    }

    // Write results to file.
    FILE *t_out = fopen(Form("../data/sf_params_%06d.txt", run_no), "w");

    if (t_out == NULL) return 4;
    for (int ci = 3; ci < 4; ++ci) { // NOTE. Only writing ECAL sf results.
        for (int si = 0; si < NSECTORS; ++si) {
            for (int ppi = 0; ppi < 2; ++ppi) { // sf and sfs.
                for (int pi = 0; pi < SF_NPARAMS; ++pi) {
                    fprintf(t_out, "%011.8f ", sf_fitresults[ci][si][pi][0]);
                }
            }
            fprintf(t_out, "\n");
        }
    }

    fclose(t_out);
    f_in ->Close();
    f_out->Close();
    free(in_filename);
    free(out_filename);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool use_fmt      = false;
    int nevn          = -1;
    char *in_filename = NULL;
    int run_no        = -1;

    if (extractsf_handle_args_err(extractsf_handle_args(argc, argv, &use_fmt,
            &nevn, &in_filename, &run_no), &in_filename))
        return 1;

    return extractsf_err(run(in_filename, use_fmt, nevn, run_no), &in_filename);
}
