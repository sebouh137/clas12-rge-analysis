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

#define PIMASS  0.139570 // Pion mass.
#define PRTMASS 0.938272 // Proton mass.
#define NTRMASS 0.939565 // Neutron mass.
#define EMASS   0.000051 // Electron mass.

double to_deg(double radians) {return radians * (180.0 / M_PI);}
double calc_Q2(double beam_energy, double momentum, double theta) {
    return pow(4 * beam_energy * momentum * sin(theta/2), 2);
}
double calc_nu(double beam_energy, double momentum) {
    return beam_energy - momentum;
}
double calc_Xb(double beam_energy, double momentum, double theta) {
    return (calc_Q2(beam_energy, momentum, theta)/2) / (calc_nu(beam_energy, momentum)/PRTMASS);
}

int run(char *input_file, bool use_fmt, int nevents, int run_no, double beam_energy) {
    // Access input files.
    TChain fake("hipo");
    fake.Add(input_file);
    auto files = fake.GetListOfFiles();

    // Particle Constants. (TODO. Move this to its own file).
    const char *p_all = "All particles";      // All particles.
    const char *p_pos = "Positive particles"; // Positive particles.
    const char *p_neg = "Negative particles"; // Negative particles.
    const char *p_pip = "Pi+";                // pi plus.
    const char *p_pim = "Pi-";                // pi minus.
    const char *p_elc = "e-";                 // electron.
    const char *p_tre = "Trigger e-";         // trigger electron.

    // Histogram Constants. (TODO. Move this to its own file).
    const char *h_vz             = "Vz";
    const char *h_vz_phi         = "Vz vs phi";
    const char *h_vz_theta       = "Vz vs theta";

    const char *h_vp             = "Vp";
    const char *h_beta           = "Beta";
    const char *h_beta_vp        = "Beta vs Vp";

    const char *h_dtof           = "TOF Difference";
    const char *h_vp_dtof        = "Vp vs TOF Difference";

    const char *h_pdivE_vs_p     = "Vp/E vs Vp";
    const char *h_pdivE_vs_E     = "Vp/E vs E";
    const char *h_p_vs_PCALE     = "Vp vs E (PCAL)";
    const char *h_p_vs_ECINE     = "Vp vs E (ECIN)";
    const char *h_p_vs_ECOUE     = "Vp vs E (ECOU)";
    const char *h_ECALE_vs_PCALE = "E (ECAL) vs E (PCAL)";
    // const char *h_sf         = "Sampling Fraction";

    // Add histos.
    std::map<const char *, std::map<const char *, TH1 *>> histos;
    histos.insert({p_all, {}});
    histos.insert({p_pos, {}});
    histos.insert({p_neg, {}});
    histos.insert({p_pip, {}});
    histos.insert({p_pim, {}});
    histos.insert({p_elc, {}});
    histos.insert({p_tre, {}});

    std::map<const char *, std::map<const char *, TH1 *>>::iterator hmap_it;
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        hmap_it->second = {
            const char *k1 = hmap_it->first;
            {h_vz,       new TH1F(Form("%s - %s", k1, h_vz),       h_vz,       100, -50, 50)},
            {h_vz_phi,   new TH2F(Form("%s - %s", k1, h_vz_phi),   h_vz_phi,   100, -50, 50, 100, -180, 180)},
            {h_vz_theta, new TH2F(Form("%s - %s", k1, h_vz_theta), h_vz_theta, 100, -50, 50, 100, 0, 50)},

            {h_vp,       new TH1F(Form("%s - %s", k1, h_vp),       h_vp,       100, 0, 12)},
            {h_beta,     new TH1F(Form("%s - %s", k1, h_beta),     h_beta,     100, 0, 1)},
            {h_beta_vp,  new TH2F(Form("%s - %s", k1, h_beta_vp),  h_beta_vp,  100, 0, 1, 100, 0, 12)},

            {h_dtof,     new TH1F(Form("%s - %s", k1, h_dtof),     h_dtof,     100, 0, 50)},
            {h_vp_dtof,  new TH2F(Form("%s - %s", k1, h_vp_dtof),  h_vp_dtof,  100, 0, 12, 100, 0, 50)},

            {h_pdivE_vs_p,     new TH2F(Form("%s - %s", k1, h_pdivE_vs_p),     h_pdivE_vs_p,     100, 0, 12, 100, 0, 0.4)},
            {h_pdivE_vs_E,     new TH2F(Form("%s - %s", k1, h_pdivE_vs_E),     h_pdivE_vs_E,     100, 0, 3, 100, 0, 0.4)},
            {h_p_vs_PCALE,     new TH2F(Form("%s - %s", k1, h_p_vs_PCALE),     h_p_vs_PCALE,     100, 0, 12, 100, 0, 12)},
            {h_p_vs_ECINE,     new TH2F(Form("%s - %s", k1, h_p_vs_ECINE),     h_p_vs_ECINE,     100, 0, 12, 100, 0, 12)},
            {h_p_vs_ECOUE,     new TH2F(Form("%s - %s", k1, h_p_vs_ECOUE),     h_p_vs_ECOUE,     100, 0, 12, 100, 0, 12)},
            {h_ECALE_vs_PCALE, new TH2F(Form("%s - %s", k1, h_ECALE_vs_PCALE), h_ECALE_vs_PCALE, 100, 0, 2, 100, 0, 2)},
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
                truth_map.insert({p_all, true});
                truth_map.insert({p_pos, p->getCharge() > 0  ? true : false});
                truth_map.insert({p_pos, p->getCharge() < 0  ? true : false});
                truth_map.insert({p_pip, p->getPid() ==  211 ? true : false});
                truth_map.insert({p_pim, p->getPid() == -211 ? true : false});
                truth_map.insert({p_elc, p->getPid() ==   11 ? true : false});
                truth_map.insert({p_tre, (p->getPid() == 11 && p->getStatus() < 0) ? true : false});

                for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
                    if (!truth_map[hmap_it->first]) continue; // Only write to appropiate histograms.

                    histos[hmap_it->first][h_vz_phi]  ->Fill(p->getVz(), to_deg(rp->getPhi()));
                    // NOTE. No beam alignment on runs yet, so we only use one sector.
                    if (rp->trk(FMT)->getSector() != 1) continue;

                    // Vertex z.
                    histos[hmap_it->first][h_vz]      ->Fill(p->getVz());
                    histos[hmap_it->first][h_vz_theta]->Fill(p->getVz(), to_deg(rp->getTheta()));

                    // Vertex P.
                    histos[hmap_it->first][h_vp]     ->Fill(p->getP());
                    histos[hmap_it->first][h_beta]   ->Fill(p->getBeta());
                    histos[hmap_it->first][h_beta_vp]->Fill(p->getBeta(), p->getP());

                    // TOF. (TODO. Check FTOF resolution).
                    if (tre_tof >= 0) { // Only fill if trigger electron's TOF was found.
                        double dtof = rp->sci(FTOF)->getTime() - tre_tof;
                        histos[hmap_it->first][h_dtof]   ->Fill(dtof);
                        histos[hmap_it->first][h_vp_dtof]->Fill(p->getP(), dtof);
                    }

                    // Calorimeters.
                    double pcal_E  = rp->cal(PCAL) ->getEnergy();
                    double ecin_E  = rp->cal(ECIN) ->getEnergy();
                    double ecou_E  = rp->cal(ECOUT)->getEnergy();
                    double total_E = pcal_E + ecin_E + ecou_E;
                    histos[hmap_it->first][h_pdivE_vs_p]    ->Fill(p->getP()/total_E, p->getP());
                    histos[hmap_it->first][h_pdivE_vs_E]    ->Fill(p->getP()/total_E, total_E);
                    histos[hmap_it->first][h_p_vs_PCALE]    ->Fill(p->getP(), pcal_E);
                    histos[hmap_it->first][h_p_vs_ECINE]    ->Fill(p->getP(), ecin_E);
                    histos[hmap_it->first][h_p_vs_ECOUE]    ->Fill(p->getP(), ecou_E);
                    histos[hmap_it->first][h_ECALE_vs_PCALE]->Fill(ecin_E+ecou_E, pcal_E);
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
        histos[hmap_it->first][h_vz]      ->Write();
        histos[hmap_it->first][h_vz_phi]  ->Write();
        histos[hmap_it->first][h_vz_theta]->Write();

        dir = Form("%s/%s", hmap_it->first, "Vertex P");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][h_vp]      ->Write();
        histos[hmap_it->first][h_beta]    ->Write();
        histos[hmap_it->first][h_beta_vp] ->Write();

        dir = Form("%s/%s", hmap_it->first, "DTOF");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][h_dtof]    ->Write();
        histos[hmap_it->first][h_vp_dtof] ->Write();

        dir = Form("%s/%s", hmap_it->first, "CALs");
        f.mkdir(dir);
        f.cd(dir);
        histos[hmap_it->first][h_pdivE_vs_p]    ->Write();
        histos[hmap_it->first][h_pdivE_vs_E]    ->Write();
        histos[hmap_it->first][h_p_vs_PCALE]    ->Write();
        histos[hmap_it->first][h_p_vs_ECINE]    ->Write();
        histos[hmap_it->first][h_p_vs_ECOUE]    ->Write();
        histos[hmap_it->first][h_ECALE_vs_PCALE]->Write();
    }

    return 0;
}

// Execute program from clas12root (`.x src/acceptance.c(filename, use_fmt, nevents)`).
int acceptance(char *input_file, bool use_fmt, int nevents) {
    int    run_no      = -1;
    double beam_energy = -1;
    if (handle_args_err(handle_filename(input_file, &run_no, &beam_energy), &input_file, run_no))
        return 1;

    return run(input_file, use_fmt, nevents, run_no, beam_energy);
}

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool   use_fmt     = false;
    int    nevents     = -1;
    char   *input_file = NULL;
    int    run_no      = -1;
    double beam_energy = -1;

    if (handle_args_err(
            handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_energy),
            &input_file, run_no)
        ) return 1;

    return run(input_file, use_fmt, nevents, run_no, beam_energy);
}
