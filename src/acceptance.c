#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TF1.h>
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

// TODO. Save both histograms and ntuples.
// TODO. Separate vz plot in z bins.
// TODO. Make this as a library similar to the Analyser.
// TODO. Evaluate acceptance in diferent regions.
// TODO. Get simulations from RG-F, understand how they're made to do acceptance correction.
// TODO. See simulations with Esteban.

int run(char *in_filename, bool use_fmt, bool debug, int nevn, int run_no, double beam_E) {
    // Access input file. TODO. Make this input file*s*.
    TFile *f_in = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 1;

    // Create and organize histos.
    std::map<const char *, std::map<const char *, TH1 *>> histos;
    histos.insert({PALL, {}});
    histos.insert({PPOS, {}});
    histos.insert({PNEG, {}});
    histos.insert({PNEU, {}});
    histos.insert({PPIP, {}});
    histos.insert({PPIM, {}});
    histos.insert({PELC, {}});
    histos.insert({PTRE, {}});

    std::map<const char *, std::map<const char *, TH1 *>>::iterator hmap_it;
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        const char *k1 = hmap_it->first;
        hmap_it->second = {};

        // Vertex z.
        insert_TH1F(&hmap_it->second, k1, VZ,      VZ,        500, -50, 50);
        insert_TH2F(&hmap_it->second, k1, VZPHI,   VZ, PHI,   500, -50, 50, 180, -180, 180);
        insert_TH2F(&hmap_it->second, k1, VZTHETA, VZ, THETA, 500, -50, 50, 200,    0,  50);

        // Vertex p.
        insert_TH1F(&hmap_it->second, k1, VP,     VP,         200, 0,    10);
        insert_TH1F(&hmap_it->second, k1, BETA,   BETA,       200, 0.94,  1);
        insert_TH2F(&hmap_it->second, k1, VPBETA, VP,   BETA, 200, 0,    10, 200, 0.94, 1);

        // Scintillator.
        insert_TH1F(&hmap_it->second, k1, DTOF,  DTOF,       200, 0, 20);
        insert_TH2F(&hmap_it->second, k1, VPTOF, VP,   DTOF, 200, 0, 10, 100, 0, 20);

        // Calorimeters.
        insert_TH2F(&hmap_it->second, k1, PEDIVP,   VP, EDIVP, 200, 0, 10, 200, 0, 0.4);
        insert_TH2F(&hmap_it->second, k1, EEDIVP,   E,  EDIVP, 200, 0,  3, 200, 0, 0.4);
        insert_TH2F(&hmap_it->second, k1, PPCALE,   VP, E,     200, 0, 10, 200, 0, 2);
        insert_TH2F(&hmap_it->second, k1, PECINE,   VP, E,     200, 0, 10, 200, 0, 2);
        insert_TH2F(&hmap_it->second, k1, PECOUE,   VP, E,     200, 0, 10, 200, 0, 2);
        insert_TH2F(&hmap_it->second, k1, ECALPCAL, E,  E,     200, 0,  2, 200, 0, 2);

        insert_TH1F(&hmap_it->second, k1, Q2, Q2, 22, 0, 12);
        insert_TH1F(&hmap_it->second, k1, NU, NU, 22, 0, 12);
        insert_TH1F(&hmap_it->second, k1, XB, XB, 20, 0,  2);
    }

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    REC_Particle     rp(t);
    REC_Track        rt(t);
    REC_Scintillator rs(t);
    REC_Calorimeter  rc(t);
    FMT_Tracks       ft(t);

    if (debug) {
        printf("PRINTING ALL INDEX NAMES IN HISTOS MAP:\n");
        for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
            const char *k1 = hmap_it->first;
            printf("  * %s:\n", k1);
            std::map<const char *, TH1 *>::iterator hmap_it2;
            for (hmap_it2 = histos[k1].begin(); hmap_it2 != histos[k1].end(); ++hmap_it2)
                printf("      * %s\n", hmap_it2->first);
        }
        printf("\n\n\n\n");
    }

    // Iterate through input file. Each TTree entry is one event.
    int evn;
    int divcntr = 0;
    int evnsplitter = 0;
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() : nevn, in_filename);
    for (evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
        if (!debug && evn >= evnsplitter) {
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
        rs.get_entries(t, evn);
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0) continue;

        // Find trigger electron's TOF.
        int tre_pindex = rt.pindex->at(0);
        double tre_tof = INFINITY;
        for (UInt_t i = 0; i < rs.pindex->size(); ++i) {
            if (rs.pindex->at(i) == tre_pindex && rs.time->at(i) < tre_tof) tre_tof = rs.time->at(i);
        }

        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index      = rt.index ->at(pos);
            int pindex     = rt.pindex->at(pos);
            int ndf        = rt.ndf   ->at(pos);
            double chi2    = rt.chi2  ->at(pos);

            int charge     = rp.charge ->at(pindex);
            int pid        = rp.pid    ->at(pindex);
            int status     = rp.status ->at(pindex);
            // double chi2pid = rp.chi2pid->at(pindex);

            // General cuts.
            // if ((int) abs(status)/1000 != 2) continue;
            // if (abs(chi2pid) >= 3) continue; // Spurious particle.
            if (pid == 0)       continue; // Non-identified particle.
            if (chi2/ndf >= 15) continue; // Ignore tracks with high chi2.

            // TODO. I should filter already processed pindexes so that I don't count a detector's
            //       data more than once.

            // Figure out which histograms are to be filled.
            // TODO. Choose these particules from cuts, not PID? Maybe using PID is fine for this
            //       early analysis...
            std::map<const char *, bool> truth_map;
            truth_map.insert({PALL, true});
            truth_map.insert({PPOS, charge > 0});
            truth_map.insert({PNEG, charge < 0});
            truth_map.insert({PNEU, charge == 0}); // Apparently there are no neutrals? Odd.
            truth_map.insert({PPIP, pid ==  211});
            truth_map.insert({PPIM, pid == -211});
            truth_map.insert({PELC, pid ==   11});
            truth_map.insert({PTRE, pid == 11 && status < 0});

            // Get reconstructed particle from either FMT or DC.
            // TODO. Create a particle struct with methods to update it.
            double vx, vy, vz;
            double px, py, pz;
            if (use_fmt) {
                // Apply FMT cuts.
                if (ft.vz->size() < 1)      continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) != 3) continue; // Track crossed 3 FMT layers.
                // if (ft.ndf->at(index) > 0) printf("NDF: %d\n", ft.ndf->at(index));

                vx = ft.vx->at(index); vy = ft.vy->at(index); vz = ft.vz->at(index);
                px = ft.px->at(index); py = ft.py->at(index); pz = ft.pz->at(index);
            }
            else {
                vx = rp.vx->at(pindex); vy = rp.vy->at(pindex); vz = rp.vz->at(pindex);
                px = rp.px->at(pindex); py = rp.py->at(pindex); pz = rp.pz->at(pindex);
            }

            // Geometry cuts.
            if (vx*vx + vy*vy > 4)   continue; // Too far from beamline.
            if (-40 > vz || vz > 40) continue; // Too far from target.

            // Get calorimeters data.
            double pcal_E = 0; // PCAL total deposited energy.
            double ecin_E = 0; // EC inner total deposited energy.
            double ecou_E = 0; // EC outer total deposited energy.
            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) == pindex) {
                    int lyr = (int) rc.layer->at(i);
                    // TODO. Add correction via sampling fraction.

                    if      (lyr == PCAL_LYR) pcal_E += rc.energy->at(i);
                    else if (lyr == ECIN_LYR) ecin_E += rc.energy->at(i);
                    else if (lyr == ECOU_LYR) ecou_E += rc.energy->at(i);
                    else return 2;
                }
            }
            double tot_E = pcal_E + ecin_E + ecou_E;

            // Get scintillators data.
            double tof = INFINITY;
            for (UInt_t i = 0; i < rs.pindex->size(); ++i)
                if (rs.pindex->at(i) == pindex && rs.time->at(i) < tof) tof = rs.time->at(i);

            // Walk through histos
            for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
                const char *k1 = hmap_it->first;
                if (!truth_map[k1]) continue;

                // Vertex z.
                histos[k1][VZ]     ->Fill(vz);
                histos[k1][VZPHI]  ->Fill(vz, to_deg(calc_phi(px, py)));
                histos[k1][VZTHETA]->Fill(vz, to_deg(calc_theta(px, py, pz)));

                // Vertex p.
                histos[k1][VP]    ->Fill(pz);
                histos[k1][BETA]  ->Fill(rp.beta->at(pindex));
                histos[k1][VPBETA]->Fill(pz, rp.beta->at(pindex));

                // TOF. (TODO. Check FTOF resolution).
                double dtof = tof - tre_tof;
                if (tre_tof > 0 && dtof > 0) { // Only fill if trigger electron's TOF was found.
                    histos[k1][DTOF] ->Fill(dtof);
                    histos[k1][VPTOF]->Fill(calc_P(px,py,pz), dtof);
                }

                // Calorimeters.
                double tot_P = calc_P(px,py,pz);
                if (tot_E > 0) {
                    histos[k1][PEDIVP]->Fill(tot_P, tot_E/tot_P);
                    histos[k1][EEDIVP]->Fill(tot_E, tot_E/tot_P);
                }
                if (pcal_E > 0) histos[k1][PPCALE]->Fill(tot_P, pcal_E);
                if (ecin_E > 0) histos[k1][PECINE]->Fill(tot_P, ecin_E);
                if (ecou_E > 0) histos[k1][PECOUE]->Fill(tot_P, ecou_E);
                if (pcal_E > 0 && ecin_E > 0 && ecou_E > 0)
                    histos[k1][ECALPCAL]->Fill(ecin_E+ecou_E, pcal_E);

                // SIDIS variables.
                if (pid == 11) {
                    double calc_theta(double px, double py, double pz);
                    histos[k1][Q2]->Fill(calc_Q2(beam_E, calc_P(px,py,pz), calc_theta(px,py,pz)));
                    histos[k1][NU]->Fill(calc_nu(beam_E, calc_P(px,py,pz)));
                    histos[k1][XB]->Fill(calc_Xb(beam_E, calc_P(px,py,pz), calc_theta(px,py,pz)));
                }
            }
        }
    }
    if (!debug) {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("[==================================================] 100%%\n");
    }

    // Fit histograms.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        const char *k1 = hmap_it->first;

        // Vz upstream fit.
        TH1 *vz = histos[k1][VZ];
        TString vz_fit_name = Form("%s %s", k1, "vz fit");
        TF1 *vz_fit = new TF1(vz_fit_name,
                "[0]*TMath::Gaus(x,[1],[2])+[3]*TMath::Gaus(x,[1]-2.4,[2])+[4]+[5]*x+[6]*x*x",
                -36,-30);
        vz->GetXaxis()->SetRange(70,100); // Set range to fitted range.
        vz_fit->SetParameter(0 /* amp1  */, vz->GetBinContent(vz->GetMaximumBin()));
        vz_fit->SetParameter(1 /* mean  */, vz->GetXaxis()->GetBinCenter(vz->GetMaximumBin()));
        vz_fit->SetParameter(2 /* sigma */, 0.5);
        vz_fit->SetParameter(3 /* amp2 */,  0);
        vz_fit->SetParameter(4 /* p0 */,    0);
        vz_fit->SetParameter(5 /* p1 */,    0);
        vz_fit->SetParameter(6 /* p2 */,    0);
        vz->GetXaxis()->SetRange(0,500);
        histos[k1][VZ]->Fit(vz_fit_name, "Q", "", -36., -30.);

        // Vp vs beta theoretical curve.
        double mass = 0;
        if      (!strcmp(k1, PPIP) || !strcmp(k1, PPIM)) mass = PIMASS;
        else if (!strcmp(k1, PELC) || !strcmp(k1, PTRE)) mass = EMASS;
        else continue;
        TString beta_vp_curve_name = Form("%s %s", k1, "beta vs vp curve");
        TF1 *beta_vp_curve =
                new TF1(beta_vp_curve_name, "(x)/(sqrt([m]*[m] + x*x))", 0, 12);
        beta_vp_curve->FixParameter(0, mass);
        histos[k1][VPBETA]->Fit(beta_vp_curve_name, "Q", "", 0, 12);
    }

    // Create output file.
    TFile *f_out = TFile::Open("../root_io/out.root", "RECREATE");

    // Write to output file.
    TString dir;
    TCanvas *gcvs = new TCanvas();
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        const char *k1 = hmap_it->first;

        dir = Form("%s/%s", k1, "Vertex Z");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[k1][VZ]     ->Write();
        histos[k1][VZPHI]  ->Draw("colz"); gcvs->Write(VZPHI);
        histos[k1][VZTHETA]->Draw("colz"); gcvs->Write(VZTHETA);

        dir = Form("%s/%s", k1, "Vertex P");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[k1][VP]    ->Write();
        histos[k1][BETA]  ->Write();
        histos[k1][VPBETA]->Draw("colz"); gcvs->Write(VPBETA);

        dir = Form("%s/%s", k1, "DTOF");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[k1][DTOF] ->Write();
        histos[k1][VPTOF]->Draw("colz"); gcvs->Write(VPTOF);

        dir = Form("%s/%s", k1, "CALs");
        f_out->mkdir(dir);
        f_out->cd(dir);
        histos[k1][PEDIVP]  ->Draw("colz"); gcvs->Write(PEDIVP);
        histos[k1][EEDIVP]  ->Draw("colz"); gcvs->Write(EEDIVP);
        histos[k1][PPCALE]  ->Draw("colz"); gcvs->Write(PPCALE);
        histos[k1][PECINE]  ->Draw("colz"); gcvs->Write(PECINE);
        histos[k1][PECOUE]  ->Draw("colz"); gcvs->Write(PECOUE);
        histos[k1][ECALPCAL]->Draw("colz"); gcvs->Write(ECALPCAL);

        if (!strcmp(k1, PELC) || !strcmp(k1, PTRE)) {
            dir = Form("%s/%s", k1, "SIDIS");
            f_out->mkdir(dir);
            f_out->cd(dir);
            histos[k1][Q2]->Write();
            histos[k1][NU]->Write();
            histos[k1][XB]->Write();
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
    bool debug        = false;
    int nevn          = -1;
    int run_no        = -1;
    double beam_E     = -1;
    char *in_filename = NULL;

    if (acceptance_handle_args_err(acceptance_handle_args(argc, argv, &use_fmt, &debug, &nevn,
            &in_filename, &run_no, &beam_E), &in_filename, run_no))
        return 1;
    return acceptance_err(run(in_filename, use_fmt, debug, nevn, run_no, beam_E), &in_filename);
}
