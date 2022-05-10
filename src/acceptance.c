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

#include "../lib/bank_containers.h"
#include "../lib/constants.h"
#include "../lib/err_handler.h"
#include "../lib/file_handler.h"
#include "../lib/io_handler.h"
#include "../lib/particle.h"
#include "../lib/utilities.h"

// TODO. Check and fix theoretical curves. -> Ask Raffa.
// TODO. See why I'm not seeing any neutrals. -> ask Raffa.
// TODO. Get simulations from RG-F, understand how they're made to do acceptance correction.
//           -> ask Raffa.

// TODO. Add a cut on momentum vs beta to remove deuteron?
// TODO. Separate in z bins and see what happens.
// TODO. Make this as a library similar to the Analyser.
// TODO. Evaluate acceptance in diferent regions.
// TODO. See simulations with Esteban.
// NOTE. Adding a functionality to be able to request a plot and get it done in one line would be
//       the gold standard for this program.

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
        insert_TH1F(&hmap_it->second, k1, BETA,   BETA,       200, 0,     1.1);
        insert_TH2F(&hmap_it->second, k1, VPBETA, VP,   BETA, 200, 0,    10, 200, 0, 1.1);

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

        // SIDIS.
        insert_TH1F(&hmap_it->second, k1, Q2_STR, Q2_STR, 22, 0, 12);
        insert_TH1F(&hmap_it->second, k1, NU_STR, NU_STR, 22, 0, 12);
        insert_TH1F(&hmap_it->second, k1, XB_STR, XB_STR, 20, 0,  2);
        insert_TH1F(&hmap_it->second, k1, W2_STR,  W2_STR,  200, 0, 20);
        insert_TH2F(&hmap_it->second, k1, Q2NU_STR, Q2_STR, NU_STR, 200, 0, 12, 200, 0, 12);
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

    // Counters for fancy progress bar.
    int divcntr = 0;
    int evnsplitter = 0;

    // Counters for SIDIS cuts.
    int tot_cntr    = 0;
    int pass_cntr   = 0;
    int no_tre_cntr = 0;
    int Q2_cntr     = 0;
    int W_cntr      = 0;
    // Iterate through input file. Each TTree entry is one event.
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() : nevn, in_filename);
    for (int evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
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

        // Trigger electron cuts.
        bool no_tre_pass = false;
        bool Q2_pass     = true;
        bool W_pass      = true;
        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get reconstructed particle from either FMT or DC.
            int pindex = rt.pindex->at(pos);
            if (rp.pid->at(pindex) != 11 || rp.status->at(pindex) > 0) continue;

            no_tre_pass = true;
            particle p;
            if (use_fmt) {
                int index = rt.index->at(pos);
                // Apply FMT cuts.
                if (ft.vz->size() < 1)               continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) < FMTNLYRSCUT) continue; // Track crossed 3 FMT layers.
                // if (ft.ndf->at(index) > 0) printf("NDF: %d\n", ft.ndf->at(index));

                p = particle_init(rp.pid->at(pindex), rp.charge->at(pindex), rp.beta->at(pindex),
                                  rp.status->at(pindex), rt.sector->at(pos),
                                  ft.vx->at(index), ft.vy->at(index), ft.vz->at(index),
                                  ft.px->at(index), ft.py->at(index), ft.pz->at(index));
            }
            else {
                p = particle_init(rp.pid->at(pindex), rp.charge->at(pindex), rp.beta->at(pindex),
                                  rp.status->at(pindex), rt.sector->at(pos),
                                  rp.vx->at(pindex), rp.vy->at(pindex), rp.vz->at(pindex),
                                  rp.px->at(pindex), rp.py->at(pindex), rp.pz->at(pindex));
            }

            Q2_pass = Q2(p, beam_E) >= Q2CUT;
            W_pass  = W( p, beam_E) >= WCUT;
        }

        // Eliminate elastic scattering events and count passing events.
        tot_cntr++;
        if (!no_tre_pass) no_tre_cntr++;
        if (!Q2_pass)     Q2_cntr++;
        if (!W_pass)      W_cntr++;
        if (!no_tre_pass || !Q2_pass || !W_pass) continue;
        pass_cntr++;

        // Process DIS event.
        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            // Get basic data from track and particle banks.
            int index      = rt.index ->at(pos);
            int pindex     = rt.pindex->at(pos); // pindex is always equal to pos!
            int ndf        = rt.ndf   ->at(pos);
            double chi2    = rt.chi2  ->at(pos);
            // double chi2pid = rp.chi2pid->at(pindex);

            // Get reconstructed particle from either FMT or DC.
            particle p;
            if (use_fmt) {
                // Apply FMT cuts.
                if (ft.vz->size() < 1)      continue; // Track reconstructed by FMT.
                if (ft.ndf->at(index) != 3) continue; // Track crossed 3 FMT layers.
                // if (ft.ndf->at(index) > 0) printf("NDF: %d\n", ft.ndf->at(index));

                p = particle_init(rp.pid->at(pindex), rp.charge->at(pindex), rp.beta->at(pindex),
                                  rp.status->at(pindex), rt.sector->at(pos),
                                  ft.vx->at(index), ft.vy->at(index), ft.vz->at(index),
                                  ft.px->at(index), ft.py->at(index), ft.pz->at(index));
            }
            else {
                p = particle_init(rp.pid->at(pindex), rp.charge->at(pindex), rp.beta->at(pindex),
                                  rp.status->at(pindex), rt.sector->at(pos),
                                  rp.vx->at(pindex), rp.vy->at(pindex), rp.vz->at(pindex),
                                  rp.px->at(pindex), rp.py->at(pindex), rp.pz->at(pindex));
            }

            // General cuts.
            // if ((int) abs(status)/1000 != 2) continue;
            // if (abs(chi2pid) >= 3) continue; // Spurious particle.
            if (p.pid == 0)       continue; // Non-identified particle.
            if (chi2/ndf >= CHI2NDFCUT) continue; // Ignore tracks with high chi2.

            // Geometry cuts.
            if (d_from_beamline(p) > VXVYCUT)  continue; // Too far from beamline.
            if (VZLOWCUT > p.vz || p.vz > VZHIGHCUT) continue; // Too far from target.

            // Figure out which histograms are to be filled.
            // NOTE. Normally for analysis its preferred for me to find my own cuts to get PID, but
            //       perhaps for this very early analysis using the banks' PID is fine.
            std::map<const char *, bool> truth_map;
            truth_map.insert({PALL, true});
            truth_map.insert({PPOS, p.q > 0});
            truth_map.insert({PNEG, p.q < 0});
            truth_map.insert({PNEU, p.q == 0}); // NOTE. Apparently there are no neutrals? Very odd.
            truth_map.insert({PPIP, p.pid ==  211});
            truth_map.insert({PPIM, p.pid == -211});
            truth_map.insert({PELC, p.pid ==   11});
            truth_map.insert({PTRE, p.is_trigger_electron});

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
                histos[k1][VZ]     ->Fill(p.vz);
                histos[k1][VZPHI]  ->Fill(p.vz, to_deg(phi_lab(p)));
                histos[k1][VZTHETA]->Fill(p.vz, to_deg(theta_lab(p)));

                // Vertex p.
                histos[k1][VP]    ->Fill(p.pz);
                histos[k1][BETA]  ->Fill(p.beta);
                histos[k1][VPBETA]->Fill(p.pz, p.beta);

                // TOF. (TODO. Check FTOF resolution).
                double dtof = tof - tre_tof;
                if (tre_tof > 0 && dtof > 0) { // Only fill if trigger electron's, Q2CUT TOF was found.
                    histos[k1][DTOF] ->Fill(dtof);
                    histos[k1][VPTOF]->Fill(P(p), dtof);
                }

                // Calorimeters.
                if (tot_E > 0) {
                    histos[k1][PEDIVP]->Fill(P(p), tot_E/P(p));
                    histos[k1][EEDIVP]->Fill(tot_E, tot_E/P(p));
                }
                if (pcal_E > 0) histos[k1][PPCALE]->Fill(P(p), pcal_E);
                if (ecin_E > 0) histos[k1][PECINE]->Fill(P(p), ecin_E);
                if (ecou_E > 0) histos[k1][PECOUE]->Fill(P(p), ecou_E);
                if (pcal_E > 0 && ecin_E > 0 && ecou_E > 0)
                    histos[k1][ECALPCAL]->Fill(ecin_E+ecou_E, pcal_E);

                // SIDIS variables.
                if (p.is_trigger_electron) {
                    histos[k1][Q2_STR]->Fill(Q2(p, beam_E));
                    histos[k1][NU_STR]->Fill(nu(p, beam_E));
                    histos[k1][XB_STR]->Fill(Xb(p, beam_E));
                    histos[k1][W2_STR] ->Fill(W2(p, beam_E));
                    histos[k1][Q2NU_STR]->Fill(Q2(p, beam_E), nu(p, beam_E));
                }
            }
        }
    }
    if (!debug) {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("[==================================================] 100%% \n");
    }

    // Print statistics.
    printf("Processed events (with particles): %8d/%8d\n", pass_cntr, tot_cntr);
    printf("  * no trigger electron : %8d events\n", no_tre_cntr);
    printf("  * Q^2 < %2d            : %8d events\n", Q2CUT, Q2_cntr);
    printf("  * W   < %2d            : %8d events\n\n", WCUT, W_cntr);

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
        double masses[4] = {0, 0, 0, 0};
        if      (!strcmp(k1, PPIP) || !strcmp(k1, PPIM)) masses[0] = PIMASS;
        else if (!strcmp(k1, PELC) || !strcmp(k1, PTRE)) masses[0] = EMASS;
        else if (!strcmp(k1, PPOS) || !strcmp(k1, PNEG)) {
             masses[0] = PRTMASS;
             masses[1] = KMASS;
             masses[2] = PIMASS;
             masses[3] = DMASS;
        }
        else continue;
        for (uint mi = 0; mi < sizeof(masses)/sizeof(masses[0]); ++mi) {
            if (masses[mi] == 0) continue;
            TString beta_vp_curve_name = Form("%s %s (%d)", k1, "beta vs vp curve", mi);
            TF1 *beta_vp_curve =
                new TF1(beta_vp_curve_name, "(x)/(sqrt([m]*[m] + x*x))", 0, 12);
            beta_vp_curve->FixParameter(0, masses[mi]);
            histos[k1][VPBETA]->Fit(beta_vp_curve_name, "Q+", "", 0, 12);
        }
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

        if (!strcmp(k1, PTRE)) {
            dir = Form("%s/%s", k1, "SIDIS");
            f_out->mkdir(dir);
            f_out->cd(dir);
            histos[k1][Q2_STR]->Write();
            histos[k1][NU_STR]->Write();
            histos[k1][XB_STR]->Write();
            histos[k1][W2_STR] ->Write();
            histos[k1][Q2NU_STR]->Draw("colz"); gcvs->Write(Q2NU_STR);
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
