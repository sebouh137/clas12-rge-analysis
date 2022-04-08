#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>

#include "bank_containers.h"
#include "constants.h"
#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"
#include "utilities.h"

int run(char *in_filename, bool use_fmt, int nevn) {
    // Access input file. TODO. Make this input file*s*.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 1;

    // Create and organize histos and name arrays.
    std::map<const char *, TH1 *> histos;

    const int ncals = sizeof(CALNAME)/sizeof(CALNAME[0]);
    char *sf1D_name_arr[ncals][NSECTORS][(int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)];
    char *sf2D_name_arr[ncals][NSECTORS];
    TGraph *sf_dotgraph_top[ncals][NSECTORS];
    TGraph *sf_dotgraph_bot[ncals][NSECTORS];

    int cal_i = -1;
    for (const char *cal : SFARR2D) {
        cal_i++;
        for (int s = 1; s <= NSECTORS; ++s) {
            std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
            oss << cal << s << ")";
            sf2D_name_arr[cal_i][s-1] = (char *) malloc(strlen(oss.str().c_str())+1);
            strncpy(sf2D_name_arr[cal_i][s-1], oss.str().c_str(), strlen(oss.str().c_str()));
            insert_TH2F(&histos, PALL, sf2D_name_arr[cal_i][s-1], VP, EDIVP, 200, 0, 10, 200, 0, 0.4);
            sf_dotgraph_top[cal_i][s-1] = new TGraph();
            sf_dotgraph_top[cal_i][s-1]->SetMarkerStyle(kFullCircle);
            sf_dotgraph_top[cal_i][s-1]->SetMarkerColor(kRed);
            sf_dotgraph_bot[cal_i][s-1] = new TGraph();
            sf_dotgraph_bot[cal_i][s-1]->SetMarkerStyle(kFullCircle);
            sf_dotgraph_bot[cal_i][s-1]->SetMarkerColor(kRed);
        }
    }

    cal_i = -1;
    for (const char *cal : SFARR1D) {
        cal_i++;
        for (int s = 1; s <= NSECTORS; ++s) {
            int p_i = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                p_i++;
                std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
                oss << cal << s << " (" << p << " < P_{tot} < " << p+SF_PSTEP << ")";
                sf1D_name_arr[cal_i][s-1][p_i] = (char *) malloc(strlen(oss.str().c_str())+1);
                strncpy(sf1D_name_arr[cal_i][s-1][p_i], oss.str().c_str(), strlen(oss.str().c_str()));
                insert_TH1F(&histos, PALL, sf1D_name_arr[cal_i][s-1][p_i], EDIVP, 200, 0, 0.4);
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
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() : nevn, in_filename);
    for (evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
        if (evn >= evnsplitter) {
            if (evn != 0) {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            }
            printf("[");
            for (int i = 0; i <= 50; ++i) {
                if (i <= divcntr/2) printf("=");
                else                printf(" ");
            }
            printf("] %2d%%", divcntr);
            fflush(stdout);
            divcntr++;
            evnsplitter = nevn == -1 ? (t->GetEntries() / 100) * divcntr : (nevn/100) * divcntr;
        }

        rp.get_entries(t, evn);
        rt.get_entries(t, evn);
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0 || rc.pindex->size() == 0) continue;

        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index      = rt.index ->at(pos);
            int pindex     = rt.pindex->at(pos);

            // Get particle momentum from either FMT or DC.
            double px, py, pz;
            if (use_fmt) {
                // Apply FMT cuts.
                if (ft.pz->size() < 1)      continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) != 3) continue; // Track crossed 3 FMT layers.

                px = ft.px->at(index); py = ft.py->at(index); pz = ft.pz->at(index);
            }
            else {
                px = rp.px->at(pindex); py = rp.py->at(pindex); pz = rp.pz->at(pindex);
            }
            double tot_P = calc_P(px, py, pz);

            // Compute energy deposited in each calorimeter per sector.
            double sf_pcal_E[] = {0, 0, 0, 0, 0, 0};
            double sf_ecin_E[] = {0, 0, 0, 0, 0, 0};
            double sf_ecou_E[] = {0, 0, 0, 0, 0, 0};
            double sf_cals_E[] = {0, 0, 0, 0, 0, 0};

            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) != pindex) continue;

                // Get sector.
                int si = rc.sector->at(i) - 1;
                if      (si == -1)                   continue;
                else if (si < -1 || si > NSECTORS-1) return 3;

                // Get detector.
                switch(rc.layer->at(i)) {
                    case PCAL_LYR: sf_pcal_E[si] += rc.energy->at(i); break;
                    case ECIN_LYR: sf_ecin_E[si] += rc.energy->at(i); break;
                    case ECOU_LYR: sf_ecou_E[si] += rc.energy->at(i); break;
                    default:       return 2;
                }
            }

            for (int si = 0; si < NSECTORS; ++si)
                sf_cals_E[si] = sf_pcal_E[si] + sf_ecin_E[si] + sf_ecou_E[si];

            // Get momentum bin.
            if (tot_P < SF_PMIN || tot_P > SF_PMAX) continue;
            int pi = -1;
            for (double p_cnt = SF_PMIN; p_cnt <= SF_PMAX; p_cnt += SF_PSTEP) {
                if (tot_P < p_cnt) break;
                pi++;
            }

            // Write to histograms.
            for (int si = 0; si < NSECTORS; ++si) {
                if (sf_pcal_E[si] > 0) {
                    histos[sf2D_name_arr[PCAL_IDX][si]]->Fill(tot_P, sf_pcal_E[si]/tot_P);
                    histos[sf1D_name_arr[PCAL_IDX][si][pi]]->Fill(sf_pcal_E[si]/tot_P);
                }
                if (sf_ecin_E[si] > 0) {
                    histos[sf2D_name_arr[ECIN_IDX][si]]->Fill(tot_P, sf_ecin_E[si]/tot_P);
                    histos[sf1D_name_arr[ECIN_IDX][si][pi]]->Fill(sf_ecin_E[si]/tot_P);
                }
                if (sf_ecou_E[si] > 0) {
                    histos[sf2D_name_arr[ECOU_IDX][si]]->Fill(tot_P, sf_ecou_E[si]/tot_P);
                    histos[sf1D_name_arr[ECOU_IDX][si][pi]]->Fill(sf_ecou_E[si]/tot_P);
                }
                if (sf_cals_E[si] > 0) {
                    histos[sf2D_name_arr[CALS_IDX][si]]->Fill(tot_P, sf_cals_E[si]/tot_P);
                    histos[sf1D_name_arr[CALS_IDX][si][pi]]->Fill(sf_cals_E[si]/tot_P);
                }
            }
        }
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("[==================================================] 100%%\n");

    // Fit histograms.
    cal_i = -1;
    for (const char *cal : SFARR1D) {
        cal_i++;
        for (int s = 1; s <= NSECTORS; ++s) {
            int p_i = -1;
            for (double p = SF_PMIN; p < SF_PMAX; p += SF_PSTEP) {
                p_i++;

                // Get ref to histogram.
                TH1 *EdivP = histos[sf1D_name_arr[cal_i][s-1][p_i]];

                // Form fit string name.
                std::ostringstream oss; // TODO. Change this to Form() because I hate c++.
                oss << cal << s << " (" << p << " < P_{tot} < " << p+SF_PSTEP << ") fit";

                // Fit.
                TF1 *sf_gaus = new TF1(oss.str().c_str(),
                                       "[0]*TMath::Gaus(x,[1],[2]) + [3]*x*x + [4]*x + [5]", 0.06, 0.25);
                sf_gaus->SetParameter(0 /* amp   */, EdivP->GetBinContent(EdivP->GetMaximumBin()));
                sf_gaus->SetParameter(1 /* mean  */, 0.15);
                sf_gaus->SetParLimits(1, 0.06, 0.25);
                sf_gaus->SetParameter(2 /* sigma */, 0.05);
                sf_gaus->SetParLimits(2, 0., 0.1);
                sf_gaus->SetParameter(3 /* p0 */,    0);
                sf_gaus->SetParameter(4 /* p1 */,    0);
                sf_gaus->SetParameter(5 /* p2 */,    0);
                EdivP->Fit(sf_gaus, "QR", "", 0.06, 0.25);

                // Extract mean and sigma from fit and add it to 2D plots.
                double mean  = sf_gaus->GetParameter(1);
                double sigma = sf_gaus->GetParameter(2);
                sf_dotgraph_top[cal_i][s-1]->AddPoint(p + SF_PSTEP/2, mean + 2*sigma);
                sf_dotgraph_bot[cal_i][s-1]->AddPoint(p + SF_PSTEP/2, mean - 2*sigma);
            }
        }
    }

    // Create output file.
    TFile *f_out = TFile::Open("../root_io/sf_study.root", "RECREATE");

    // Write to output file.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (cal_i = 0; cal_i < ncals; ++cal_i) {
        dir = Form("%s", CALNAME[cal_i]);
        f_out->mkdir(dir);
        f_out->cd(dir);
        for (int s_i = 0; s_i < NSECTORS; ++s_i) {
            dir = Form("%s/sector %d", CALNAME[cal_i], s_i+1);
            f_out->mkdir(dir);
            f_out->cd(dir);

            histos[sf2D_name_arr[cal_i][s_i]]->Draw("colz");
            sf_dotgraph_top[cal_i][s_i]->Draw("Psame");
            sf_dotgraph_bot[cal_i][s_i]->Draw("Psame");
            gcvs->Write(sf2D_name_arr[cal_i][s_i]);
            free(sf2D_name_arr[cal_i][s_i]);
            for (int p_i = 0; p_i < ((int) ((SF_PMAX - SF_PMIN)/SF_PSTEP)); ++p_i) {
                histos[sf1D_name_arr[cal_i][s_i][p_i]]->Write();
                free(sf1D_name_arr[cal_i][s_i][p_i]);
            }
        }
    }

    f_in ->Close();
    f_out->Close();
    free(in_filename);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool use_fmt      = false;
    int nevn          = -1;
    char *in_filename = NULL;

    if (extractsf_handle_args_err(extractsf_handle_args(argc, argv, &use_fmt, &nevn, &in_filename),
            &in_filename))
        return 1;
    return extractsf_err(run(in_filename, use_fmt, nevn), &in_filename);
}
