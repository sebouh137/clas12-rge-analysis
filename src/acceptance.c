#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TFile.h>

#include <BankHist.h>

#include "constants.h"
#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

double to_deg(double radians) {return radians * (180.0 / M_PI);}
double calc_Q2(double beam_E, double momentum, double theta) {
    return 4 * beam_E * momentum * pow(sin(theta/2), 2);
}
double calc_nu(double beam_E, double momentum) {
    return beam_E - momentum;
}
double calc_Xb(double beam_E, double momentum, double theta) {
    return (calc_Q2(beam_E, momentum, theta)/2) / (calc_nu(beam_E, momentum)/PRTMASS);
}

int run(char *input_file, bool use_fmt, int nevents, int run_no, double beam_E) {
    // Sampling Fraction. TODO. Temporary code.
    double p1[6] = { 0.25149,  0.25186,  0.24912,  0.24747,  0.24649,  0.25409};
    double p2[6] = { 1.00000,  1.00000,  1.00000,  1.00000,  1.00000,  1.00000};
    double p3[6] = {-0.03427, -0.03771, -0.02627, -0.03163, -0.02723, -0.04000};
    double p4[6] = { 0.00070,  0.00070,  0.00070,  0.00070,  0.00070,  0.00070};

    // Access input files.
    TChain fake("hipo");
    fake.Add(input_file);
    auto files = fake.GetListOfFiles();

    // Add histos.
    std::map<const char *, std::map<const char *, TH1 *>> histos;
    histos.insert({PALL, {}});
    histos.insert({PPOS, {}});
    histos.insert({PNEG, {}});
    histos.insert({PPIP, {}});
    histos.insert({PPIM, {}});
    histos.insert({PELC, {}});
    histos.insert({PTRE, {}});

    std::map<const char *, std::map<const char *, TH1 *>>::iterator hmap_it;
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        const char *k1 = hmap_it->first;
        hmap_it->second = {
            {VZ,       new TH1F(Form("%s: %s", k1, VZ),       VZ,      500, -50, 50)},
            {VZPHI,    new TH2F(Form("%s: %s", k1, VZPHI),    VZPHI,   100, -50, 50, 100, -180, 180)},
            {VZTHETA,  new TH2F(Form("%s: %s", k1, VZTHETA),  VZTHETA, 100, -50, 50, 100, 0, 50)},

            {VP,       new TH1F(Form("%s: %s", k1, VP),       VP,      100, 0, 12)},
            {BETA,     new TH1F(Form("%s: %s", k1, BETA),     BETA,    100, 0.9, 1)},
            {BETAVP,   new TH2F(Form("%s: %s", k1, BETAVP),   BETAVP,  100, 0.9, 1, 100, 0, 12)},

            {DTOF,     new TH1F(Form("%s: %s", k1, DTOF),     DTOF,    100, 0, 5)},
            {VPTOF,    new TH2F(Form("%s: %s", k1, VPTOF),    VPTOF,   100, 0, 12, 100, 0, 5)},

            {PDIVEE,   new TH2F(Form("%s: %s", k1, PDIVEE),   PDIVEE,   100, 0, 3, 100, 0, 0.4)},
            {PDIVEP,   new TH2F(Form("%s: %s", k1, PDIVEP),   PDIVEP,   100, 0, 12, 100, 0, 0.4)},
            {PPCALE,   new TH2F(Form("%s: %s", k1, PPCALE),   PPCALE,   100, 0, 12, 100, 0, 2)},
            {PECINE,   new TH2F(Form("%s: %s", k1, PECINE),   PECINE,   100, 0, 12, 100, 0, 2)},
            {PECOUE,   new TH2F(Form("%s: %s", k1, PECOUE),   PECOUE,   100, 0, 12, 100, 0, 2)},
            {ECALPCAL, new TH2F(Form("%s: %s", k1, ECALPCAL), ECALPCAL, 100, 0, 2, 100, 0, 2)},

            {PCALSF1,  new TH1F(Form("%s: %s", k1, PCALSF1), PCALSF1, 100, 0, 0.5)},
            {PCALSF2,  new TH1F(Form("%s: %s", k1, PCALSF2), PCALSF2, 100, 0, 0.5)},
            {PCALSF3,  new TH1F(Form("%s: %s", k1, PCALSF3), PCALSF3, 100, 0, 0.5)},
            {PCALSF4,  new TH1F(Form("%s: %s", k1, PCALSF4), PCALSF4, 100, 0, 0.5)},
            {PCALSF5,  new TH1F(Form("%s: %s", k1, PCALSF5), PCALSF5, 100, 0, 0.5)},
            {PCALSF6,  new TH1F(Form("%s: %s", k1, PCALSF6), PCALSF6, 100, 0, 0.5)},
            {ECINSF1,  new TH1F(Form("%s: %s", k1, ECINSF1), ECINSF1, 100, 0, 0.5)},
            {ECINSF2,  new TH1F(Form("%s: %s", k1, ECINSF2), ECINSF2, 100, 0, 0.5)},
            {ECINSF3,  new TH1F(Form("%s: %s", k1, ECINSF3), ECINSF3, 100, 0, 0.5)},
            {ECINSF4,  new TH1F(Form("%s: %s", k1, ECINSF4), ECINSF4, 100, 0, 0.5)},
            {ECINSF5,  new TH1F(Form("%s: %s", k1, ECINSF5), ECINSF5, 100, 0, 0.5)},
            {ECINSF6,  new TH1F(Form("%s: %s", k1, ECINSF6), ECINSF6, 100, 0, 0.5)},
            {ECOUSF1,  new TH1F(Form("%s: %s", k1, ECOUSF1), ECOUSF1, 100, 0, 0.5)},
            {ECOUSF2,  new TH1F(Form("%s: %s", k1, ECOUSF2), ECOUSF2, 100, 0, 0.5)},
            {ECOUSF3,  new TH1F(Form("%s: %s", k1, ECOUSF3), ECOUSF3, 100, 0, 0.5)},
            {ECOUSF4,  new TH1F(Form("%s: %s", k1, ECOUSF4), ECOUSF4, 100, 0, 0.5)},
            {ECOUSF5,  new TH1F(Form("%s: %s", k1, ECOUSF5), ECOUSF5, 100, 0, 0.5)},
            {ECOUSF6,  new TH1F(Form("%s: %s", k1, ECOUSF6), ECOUSF6, 100, 0, 0.5)},

            {Q2,       new TH1F(Form("%s: %s", k1, Q2),       Q2,       22, 0, 12)},
            {NU,       new TH1F(Form("%s: %s", k1, NU),       NU,       22, 0, 12)},
            {XB,       new TH1F(Form("%s: %s", k1, XB),       XB,       20, 0, 2)},
        };
    }

    // Iterate through input files.
    for (int i = 0; i < files->GetEntries(); ++i) {
        clas12reader c12(files->At(i)->GetTitle(), {0}); // Create event reader.
        if (nevents != 0) c12.setEntries(nevents / files->GetEntries());

        // Iterate through events in file.
        while (c12.next() == true) {
            // Find trigger electron's TOF.
            double tre_tof = -1;
            for (region_particle *rp : c12.getDetParticles()) {
                if (rp->par()->getPid() == 11 && rp->par()->getStatus() < 0)
                    tre_tof = rp->sci(FTOF)->getTime();
            }

            // Iterate through particles in event.
            for (region_particle *rp : c12.getDetParticles()) {
                // Make sure that particle comes from FD.
                switch (rp->getRegion()) {
                case FD: break;    // Forward Detector.
                case FT: continue; // Forward Tagger.
                case CD: continue; // Central Detector.
                default:
                    printf("[ERROR] A particles comes from an invalid detector.\n"); // Just in case.
                    return(1);
                };

                // Get particle and associated data.
                particle *p = rp->par();

                // Apply PID cuts.
                if (    abs(p->getChi2Pid()) >= 3 // Ignore spurious particles.
                     || p->getPid() == 0          // Ignore badly identified particles.
                ) continue;

                // Apply geometry cuts. (TODO. Improve cut in z).
                if (    p->getVx()*p->getVx() + p->getVy()*p->getVy() > 4 // Too far from beamline.
                     || (p->getVz() < -40 || p->getVz() > 40)             // Too far from target.
                ) continue;

                // Apply FMT cuts. (TODO. Make sure that this is enough).
                if (use_fmt && (
                        (abs(p->getStatus())/1000) != 2 // Filter particles that pass through FD.
                     // || rp->trk(FMT)->getNDF() == 3     // TODO. Figure out what this NDF is.
                )) continue;

                // TODO. Make sure that the particle bank has FMT data.
                //       UPDATE. It doesn't. Fix this.
                // TODO. Figure out how to get DC data.

                // Figure out which histograms are to be filled.
                // TODO. Choose these particules from cuts, not PID.
                std::map<const char *, bool> truth_map;
                truth_map.insert({PALL, true});
                truth_map.insert({PPOS, p->getCharge() > 0  ? true : false});
                truth_map.insert({PPOS, p->getCharge() < 0  ? true : false});
                truth_map.insert({PPIP, p->getPid() ==  211 ? true : false});
                truth_map.insert({PPIM, p->getPid() == -211 ? true : false});
                truth_map.insert({PELC, p->getPid() ==   11 ? true : false});
                truth_map.insert({PTRE, (p->getPid() == 11 && p->getStatus() < 0) ? true : false});

                for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
                    if (!truth_map[hmap_it->first]) continue; // Only write to appropiate histograms.

                    // Vertex z.
                    histos[hmap_it->first][VZPHI]->Fill(p->getVz(), to_deg(rp->getPhi()));
                    if (rp->trk(FMT)->getSector() == 1) { // No beam alignment on runs yet.
                        histos[hmap_it->first][VZ]     ->Fill(p->getVz());
                        histos[hmap_it->first][VZTHETA]->Fill(p->getVz(), to_deg(rp->getTheta()));
                    }

                    // Vertex P.
                    histos[hmap_it->first][VP]    ->Fill(p->getP());
                    histos[hmap_it->first][BETA]  ->Fill(p->getBeta());
                    histos[hmap_it->first][BETAVP]->Fill(p->getBeta(), p->getP());

                    // TOF. (TODO. Check FTOF resolution).
                    if (tre_tof >= 0) { // Only fill if trigger electron's TOF was found.
                        double dtof = rp->sci(FTOF)  ->getTime() - tre_tof;
                        histos[hmap_it->first][DTOF] ->Fill(dtof);
                        histos[hmap_it->first][VPTOF]->Fill(p->getP(), dtof);
                    }

                    // Calorimeters.
                    double pcal_E  = rp->cal(PCAL) ->getEnergy();
                    double ecin_E  = rp->cal(ECIN) ->getEnergy();
                    double ecou_E  = rp->cal(ECOUT)->getEnergy();
                    double total_E = pcal_E + ecin_E + ecou_E;
                    histos[hmap_it->first][PDIVEP]  ->Fill(p->getP()/total_E, p->getP());
                    histos[hmap_it->first][PDIVEE]  ->Fill(p->getP()/total_E, total_E);
                    histos[hmap_it->first][PPCALE]  ->Fill(p->getP(), pcal_E);
                    histos[hmap_it->first][PECINE]  ->Fill(p->getP(), ecin_E);
                    histos[hmap_it->first][PECOUE]  ->Fill(p->getP(), ecou_E);
                    histos[hmap_it->first][ECALPCAL]->Fill(ecin_E+ecou_E, pcal_E);

                    // Sampling Fraction.
                    int s; double sf;
                    s  = rp->cal(PCAL)->getSector();
                    sf = p1[s-1] * (p2[s-1] + p3[s-1]/pcal_E + pow(p4[s-1]/pcal_E, 2));
                    switch (s) { // NOTE. Bad solution because I'm lazy. I should change this.
                    case 0: break;
                    case 1: histos[hmap_it->first][PCALSF1]->Fill(sf); break;
                    case 2: histos[hmap_it->first][PCALSF2]->Fill(sf); break;
                    case 3: histos[hmap_it->first][PCALSF3]->Fill(sf); break;
                    case 4: histos[hmap_it->first][PCALSF4]->Fill(sf); break;
                    case 5: histos[hmap_it->first][PCALSF5]->Fill(sf); break;
                    case 6: histos[hmap_it->first][PCALSF6]->Fill(sf); break;
                    default:
                        printf("[ERROR] A particles is in invalid sector %d.\n", s); // Just in case.
                        return(1);
                    };

                    s  = rp->cal(ECIN)->getSector();
                    sf = p1[s-1] * (p2[s-1] + p3[s-1]/pcal_E + pow(p4[s-1]/pcal_E, 2));
                    switch (s) { // NOTE. Bad solution because I'm lazy. I should change this.
                    case 0: break;
                    case 1: histos[hmap_it->first][ECINSF1]->Fill(sf); break;
                    case 2: histos[hmap_it->first][ECINSF2]->Fill(sf); break;
                    case 3: histos[hmap_it->first][ECINSF3]->Fill(sf); break;
                    case 4: histos[hmap_it->first][ECINSF4]->Fill(sf); break;
                    case 5: histos[hmap_it->first][ECINSF5]->Fill(sf); break;
                    case 6: histos[hmap_it->first][ECINSF6]->Fill(sf); break;
                    default:
                        printf("[ERROR] A particles is in invalid sector %d.\n", s); // Just in case.
                        return(1);
                    };

                    s  = rp->cal(ECOUT)->getSector();
                    sf = p1[s-1] * (p2[s-1] + p3[s-1]/pcal_E + pow(p4[s-1]/pcal_E, 2));
                    switch (s) { // NOTE. Bad solution because I'm lazy. I should change this.
                    case 0: break;
                    case 1: histos[hmap_it->first][ECOUSF1]->Fill(sf); break;
                    case 2: histos[hmap_it->first][ECOUSF2]->Fill(sf); break;
                    case 3: histos[hmap_it->first][ECOUSF3]->Fill(sf); break;
                    case 4: histos[hmap_it->first][ECOUSF4]->Fill(sf); break;
                    case 5: histos[hmap_it->first][ECOUSF5]->Fill(sf); break;
                    case 6: histos[hmap_it->first][ECOUSF6]->Fill(sf); break;
                    default:
                        printf("[ERROR] A particles is in invalid sector %d.\n", s); // Just in case.
                        return(1);
                    };

                    // SIDIS variables.
                    if (p->getPid() == 11) {
                        histos[hmap_it->first][Q2]->Fill(calc_Q2(beam_E, p->getP(), rp->getTheta()));
                        histos[hmap_it->first][NU]->Fill(calc_nu(beam_E, p->getP()));
                        histos[hmap_it->first][XB]->Fill(calc_Xb(beam_E, p->getP(), rp->getTheta()));
                    }
                }
            }
        }
    }

    // Create output file.
    TFile f("out/histos.root", "RECREATE");

    // Fit histograms.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        // Vz upstream fit.
        TH1 *vz = histos[hmap_it->first][VZ];
        TString vz_fit_name = Form("%s %s", hmap_it->first, "vz fit");
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
        histos[hmap_it->first][VZ]->Fit(vz_fit_name, "", "", -36., -30.);

        // Vp vs beta theoretical curve.
        double mass = 0;
        if      (hmap_it->first == PPIP || hmap_it->first == PPIM) mass = PIMASS;
        else if (hmap_it->first == PELC || hmap_it->first == PTRE) mass = EMASS;
        else continue;
        TString vp_beta_curve_name = Form("%s %s", hmap_it->first, "vp vs beta curve");
        TF1 *vp_beta_curve = new TF1(vp_beta_curve_name, "[m]*x/(sqrt(1-x))", 0.9, 1.0);
        vp_beta_curve->FixParameter(0, mass);
        histos[hmap_it->first][BETAVP]->Fit(vp_beta_curve_name, "", "", 0.9, 1.0);
    }

    // Write to output file.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        TCanvas *gcvs = new TCanvas();

        TString dir = Form("%s/%s", hmap_it->first, "Vertex Z");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][VZ]     ->Write();
        histos[hmap_it->first][VZPHI]  ->Draw("colz"); gcvs->Write(VZPHI);
        histos[hmap_it->first][VZTHETA]->Draw("colz"); gcvs->Write(VZTHETA);

        dir = Form("%s/%s", hmap_it->first, "Vertex P");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][VP]    ->Write();
        histos[hmap_it->first][BETA]  ->Write();
        histos[hmap_it->first][BETAVP]->Draw("colz"); gcvs->Write(BETAVP);

        dir = Form("%s/%s", hmap_it->first, "DTOF");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][DTOF] ->Write();
        histos[hmap_it->first][VPTOF]->Draw("colz"); gcvs->Write(VPTOF);

        dir = Form("%s/%s", hmap_it->first, "CALs");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][PDIVEP]  ->Draw("colz"); gcvs->Write(PDIVEP);
        histos[hmap_it->first][PDIVEE]  ->Draw("colz"); gcvs->Write(PDIVEE);
        histos[hmap_it->first][PPCALE]  ->Draw("colz"); gcvs->Write(PPCALE);
        histos[hmap_it->first][PECINE]  ->Draw("colz"); gcvs->Write(PECINE);
        histos[hmap_it->first][PECOUE]  ->Draw("colz"); gcvs->Write(PECOUE);
        histos[hmap_it->first][ECALPCAL]->Draw("colz"); gcvs->Write(ECALPCAL);

        dir = Form("%s/%s/%s", hmap_it->first, "CALs", "Sampling Fraction");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][PCALSF1]->Write();
        histos[hmap_it->first][PCALSF2]->Write();
        histos[hmap_it->first][PCALSF3]->Write();
        histos[hmap_it->first][PCALSF4]->Write();
        histos[hmap_it->first][PCALSF5]->Write();
        histos[hmap_it->first][PCALSF6]->Write();
        histos[hmap_it->first][ECINSF1]->Write();
        histos[hmap_it->first][ECINSF2]->Write();
        histos[hmap_it->first][ECINSF3]->Write();
        histos[hmap_it->first][ECINSF4]->Write();
        histos[hmap_it->first][ECINSF5]->Write();
        histos[hmap_it->first][ECINSF6]->Write();
        histos[hmap_it->first][ECOUSF1]->Write();
        histos[hmap_it->first][ECOUSF2]->Write();
        histos[hmap_it->first][ECOUSF3]->Write();
        histos[hmap_it->first][ECOUSF4]->Write();
        histos[hmap_it->first][ECOUSF5]->Write();
        histos[hmap_it->first][ECOUSF6]->Write();

        dir = Form("%s/%s", hmap_it->first, "SIDIS");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][Q2]->Write();
        histos[hmap_it->first][NU]->Write();
        histos[hmap_it->first][XB]->Write();
    }
    f.Close();

    return 0;
}

// Execute program from clas12root (`.x src/acceptance.c(filename, use_fmt, nevents)`).
int acceptance(char *input_file, bool use_fmt, int nevents) {
    int    run_no = -1;
    double beam_E = -1;
    if (handle_args_err(handle_filename(input_file, &run_no, &beam_E), &input_file, run_no))
        return 1;

    return run(input_file, use_fmt, nevents, run_no, beam_E);
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool   use_fmt     = false;
    int    nevents     = -1;
    char   *input_file = NULL;
    int    run_no      = -1;
    double beam_E      = -1;

    if (handle_args_err(
            handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_E),
            &input_file, run_no)
        ) return 1;

    return run(input_file, use_fmt, nevents, run_no, beam_E);
}
