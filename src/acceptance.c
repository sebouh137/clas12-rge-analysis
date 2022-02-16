#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>

#include <TFile.h>

#include <BankHist.h>

#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

// TODO. These #defines should be in their own file.
// Masses.
#define PIMASS  0.139570 // Pion mass.
#define PRTMASS 0.938272 // Proton mass.
#define NTRMASS 0.939565 // Neutron mass.
#define EMASS   0.000051 // Electron mass.

// Particle map keys.
#define PALL "All particles"
#define PPOS "Positive particles"
#define PNEG "Negative particles"
#define PPIP "Pi+"
#define PPIM "Pi-"
#define PELC "e-"
#define PTRE "Trigger e-"

// Histogram map keys.
#define VZ       "Vz"
#define VZPHI    "Vz vs phi"
#define VZTHETA  "Vz vs theta"

#define VP       "Vp"
#define BETA     "Beta"
#define BETAVP   "Beta vs Vp"

#define DTOF     "TOF Difference"
#define VPTOF    "Vp vs TOF Difference"

#define PDIVEP   "Vp/E vs Vp"
#define PDIVEE   "Vp/E vs E"
#define PPCALE   "Vp vs E (PCAL)"
#define PECINE   "Vp vs E (ECIN)"
#define PECOUE   "Vp vs E (ECOU)"
#define ECALPCAL "E (ECAL) vs E (PCAL)"
// #define SF "Sampling Fraction"

#define Q2       "Q2"
#define NU       "nu"
#define XB       "X_bjorken"

double to_deg(double radians) {return radians * (180.0 / M_PI);}
double calc_Q2(double beam_E, double momentum, double theta) {
    return pow(4 * beam_E * momentum * sin(theta/2), 2);
}
double calc_nu(double beam_E, double momentum) {
    return beam_E - momentum;
}
double calc_Xb(double beam_E, double momentum, double theta) {
    return (calc_Q2(beam_E, momentum, theta)/2) / (calc_nu(beam_E, momentum)/PRTMASS);
}

int run(char *input_file, bool use_fmt, int nevents, int run_no, double beam_E) {
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
            {VZ,       new TH1F(Form("%s - %s", k1, VZ),       VZ,      100, -50, 50)},
            {VZPHI,    new TH2F(Form("%s - %s", k1, VZPHI),    VZPHI,   100, -50, 50, 100, -180, 180)},
            {VZTHETA,  new TH2F(Form("%s - %s", k1, VZTHETA),  VZTHETA, 100, -50, 50, 100, 0, 50)},

            {VP,       new TH1F(Form("%s - %s", k1, VP),       VP,      100, 0, 12)},
            {BETA,     new TH1F(Form("%s - %s", k1, BETA),     BETA,    100, 0, 1)},
            {BETAVP,   new TH2F(Form("%s - %s", k1, BETAVP),   BETAVP,  100, 0, 1, 100, 0, 12)},

            {DTOF,     new TH1F(Form("%s - %s", k1, DTOF),     DTOF,    100, 0, 50)},
            {VPTOF,    new TH2F(Form("%s - %s", k1, VPTOF),    VPTOF,   100, 0, 12, 100, 0, 50)},

            {PDIVEE,   new TH2F(Form("%s - %s", k1, PDIVEE),   PDIVEE,   100, 0, 3, 100, 0, 0.4)},
            {PDIVEP,   new TH2F(Form("%s - %s", k1, PDIVEP),   PDIVEP,   100, 0, 12, 100, 0, 0.4)},
            {PPCALE,   new TH2F(Form("%s - %s", k1, PPCALE),   PPCALE,   100, 0, 12, 100, 0, 12)},
            {PECINE,   new TH2F(Form("%s - %s", k1, PECINE),   PECINE,   100, 0, 12, 100, 0, 12)},
            {PECOUE,   new TH2F(Form("%s - %s", k1, PECOUE),   PECOUE,   100, 0, 12, 100, 0, 12)},
            {ECALPCAL, new TH2F(Form("%s - %s", k1, ECALPCAL), ECALPCAL, 100, 0, 2, 100, 0, 2)},

            {Q2,       new TH1F(Form("%s - %s", k1, Q2),       Q2,       22, 0, 12)},
            {NU,       new TH1F(Form("%s - %s", k1, NU),       NU,       22, 0, 12)},
            {XB,       new TH1F(Form("%s - %s", k1, XB),       XB,       20, 0, 2)},
        };
    }

    // Iterate through input files.
    for (int i = 0; i < files->GetEntries(); ++i) {
        clas12reader c12(files->At(i)->GetTitle(), {0}); // Create event reader.
        c12.setEntries(nevents / files->GetEntries());

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
                    printf("[ERROR] A particles comes from an invalid detector.\n"); // Just in casae.
                    return(1);
                };

                // Get particle and associated data.
                particle *p = rp->par();

                // Apply PID cuts.
                if (
                        abs(p->getChi2Pid()) >= 3 // Ignore spurious particles.
                     || p->getPid() == 0          // Ignore badly identified particles.
                ) continue;

                // Apply geometry cuts. (TODO. Improve cut in z).
                if (
                        p->getVx()*p->getVx() + p->getVy()*p->getVy() > 4 // Too far from beamline.
                     || (p->getVz() < -40 || p->getVz() > 40)             // Too far from target.
                ) continue;

                // Apply FMT cuts. (TODO. Make sure that this is enough).
                if (use_fmt && (
                        (abs(p->getStatus())/1000) != 2 // Filter particles that pass through FMT.
                     // || rp->trk(FMT)->getNDF() == 3     // TODO. Figure out what this NDF is.
                )) continue;

                // TODO. Make sure that the particle bank has FMT data.
                //       UPDATE. It doesn't. Fix this.
                // TODO. Figure out how to get DC data.

                // Figure out which histograms are to be filled.
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

                    histos[hmap_it->first][VZPHI]->Fill(p->getVz(), to_deg(rp->getPhi()));
                    // NOTE. No beam alignment on runs yet, so we only use one sector.
                    if (rp->trk(FMT)->getSector() != 1) continue;

                    // Vertex z.
                    histos[hmap_it->first][VZ]     ->Fill(p->getVz());
                    histos[hmap_it->first][VZTHETA]->Fill(p->getVz(), to_deg(rp->getTheta()));

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

    // Write to output file.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        TString dir = Form("%s/%s", hmap_it->first, "Vertex Z");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][VZ]     ->Write();
        histos[hmap_it->first][VZPHI]  ->Write();
        histos[hmap_it->first][VZTHETA]->Write();

        dir = Form("%s/%s", hmap_it->first, "Vertex P");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][VP]    ->Write();
        histos[hmap_it->first][BETA]  ->Write();
        histos[hmap_it->first][BETAVP]->Write();

        dir = Form("%s/%s", hmap_it->first, "DTOF");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][DTOF] ->Write();
        histos[hmap_it->first][VPTOF]->Write();

        dir = Form("%s/%s", hmap_it->first, "CALs");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][PDIVEP]  ->Write();
        histos[hmap_it->first][PDIVEE]  ->Write();
        histos[hmap_it->first][PPCALE]  ->Write();
        histos[hmap_it->first][PECINE]  ->Write();
        histos[hmap_it->first][PECOUE]  ->Write();
        histos[hmap_it->first][ECALPCAL]->Write();

        dir = Form("%s/%s", hmap_it->first, "SIDIS");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][Q2]->Write();
        histos[hmap_it->first][NU]->Write();
        histos[hmap_it->first][XB]->Write();
    }

    return 0;
}

// Execute program from clas12root (`.x src/acceptance.c(filename, use_fmt, nevents)`).
int acceptance(char *input_file, bool use_fmt, int nevents) {
    int    run_no      = -1;
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
    double beam_E = -1;

    if (handle_args_err(
            handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_E),
            &input_file, run_no)
        ) return 1;

    return run(input_file, use_fmt, nevents, run_no, beam_E);
}
