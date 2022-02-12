#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>

#include <TFile.h>

#include <BankHist.h>

#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

int run(char *input_file, bool use_fmt, int nevents, int run_no, double beam_energy) {
    // Access input files.
    TChain fake("hipo");
    fake.Add(input_file);
    auto files = fake.GetListOfFiles();

    // Particle Constants. (TODO. Move this to its own file).
    const char *p_all = "All particles"; // All particles.
    const char *p_pos = "Positive particles"; // Positive particles.
    const char *p_neg = "Negative particles"; // Negative particles.
    const char *p_pip = "Pi+"; // pi plus.
    const char *p_pim = "Pi-"; // pi minus.
    const char *p_elc = "e-"; // electron.
    const char *p_tre = "Trigger e-"; // trigger electron.

    // Histogram Constants. (TODO. Move this to its own file).
    const char *h_pz      = "Pz";
    const char *h_beta    = "Beta";
    const char *h_beta_pz = "Beta vs Pz";

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
            {h_pz,      new TH1F(Form("%s - %s", hmap_it->first, h_pz), h_pz, 100, 0, 12)},
            {h_beta,    new TH1F(Form("%s - %s", hmap_it->first, h_beta), h_beta, 100, 0, 1)},
            {h_beta_pz, new TH2F(Form("%s - %s", hmap_it->first, h_beta_pz), h_beta_pz, 100, 0, 1, 100, 0, 12)},
        };
    }

    // Iterate through input files.
    for (int i = 0; i < files->GetEntries(); ++i) {
        clas12reader c12(files->At(i)->GetTitle(), {0}); // Create event reader.
        c12.setEntries(nevents / files->GetEntries());

        // Iterate through events in file.
        while (c12.next() == true) {
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
                // TODO. Figure out how to get DC data.

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
                    histos[hmap_it->first][h_pz]->Fill(p->getPz());
                    histos[hmap_it->first][h_beta]->Fill(p->getBeta());
                    histos[hmap_it->first][h_beta_pz]->Fill(p->getBeta(), p->getPz());
                }
            }
        }
    }

    // Create output file.
    TFile f("out/histos.root", "RECREATE");

    // Write to output file.
    for (hmap_it = histos.begin(); hmap_it != histos.end(); ++hmap_it) {
        TString dir = Form("%s/%s", hmap_it->first, "Vertex P");
        f.mkdir(dir);
        f.cd(dir);

        histos[hmap_it->first][h_pz]->Write();
        histos[hmap_it->first][h_beta]->Write();
        histos[hmap_it->first][h_beta_pz]->Write();

        f.cd("/");
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
