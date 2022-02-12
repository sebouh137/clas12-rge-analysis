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
    char const *p_all = "All particles"; // All particles.
    char const *p_pos = "Positive particles"; // Positive particles.
    char const *p_neg = "Negative particles"; // Negative particles.
    char const *p_pip = "Pi+"; // pi plus.
    char const *p_pim = "Pi-"; // pi minus.
    char const *p_elc = "e-"; // electron.
    char const *p_tre = "Trigger e-"; // trigger electron.

    // Histogram Constants. (TODO. Move this to its own file).
    char const *h_pz      = "Pz";
    char const *h_beta    = "Beta";
    char const *h_beta_pz = "Beta vs Pz";

    // Add histos.
    std::map<char const *, std::map<char const *, TH1 *>> histos;
    histos.insert({p_all, {}});
    histos.insert({p_pos, {}});
    histos.insert({p_neg, {}});
    histos.insert({p_pip, {}});
    histos.insert({p_pim, {}});
    histos.insert({p_elc, {}});
    histos.insert({p_tre, {}});

    std::map<char const *, std::map<char const *, TH1 *>>::iterator hmap_it;
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
        c12.setEntries(nevents);

        // Iterate through events in file.
        while (c12.next() == true) {
            // Iterate through particles in event.
            for (region_particle *rp : c12.getDetParticles()) {
                // Make sure that particle comes from FD.
                bool exit = false;
                switch (rp->getRegion()) {
                case FD: // Forward Detector.
                    break;
                case FT: // Forward Tagger.
                case CD: // Central Detector.
                    exit = true;
                };
                if (exit) continue;

                // Get particle and associated data.
                particle *p = rp->par();

                // Apply general cuts.
                if ((abs(p->getStatus())/1000) != 2) continue; // Filter particles that pass through FMT.
                // if (abs(chi2pid) >= 3) continue; // Ignore spurious particles.
                if (p->getPid() == 0) continue; // Ignore bad particles.
                // if (vz < -40 || vz > 40) continue; // Geometry cut.
                if (rp->trk(FMT)->getNDF() != 3); // Only use particles detected by 3 FMT layers.

                // TODO. MAKE SURE THAT THE APPLIED CUTS ARE ENOUGH TO KNOW THAT THE PARTICLE BANK
                //       HAS USED FMT DATA AND NOT ONLY DC!!!

                std::map<char const *, bool> truth_map;
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

// int run_old(char *input_file, bool use_fmt, int nevents, int run_no, double beam_energy) {
//     // Create output file.
//     TFile f("out/histos.root", "RECREATE");
//
//     // Open relevant data banks.
//     // std::map<char*, std::map<char*, BankHist>> banks;
//     std::map<char const *, BankHist *> banks = {
//             {"REC::Particle",     new BankHist(input_file)},
//             {"REC::Track",        new BankHist(input_file)},
//             {"REC::Traj",         new BankHist(input_file)},
//             {"REC::Calorimeter",  new BankHist(input_file)},
//             {"REC::Scintillator", new BankHist(input_file)},
//             {"FMT::Tracks",       new BankHist(input_file)},
//     };
//
//     // Setup banks.
//     if (nevents != 0) {
//         for (std::pair<char const *, BankHist *> it : banks) it.second->SetEntries(nevents);
//     }
//
//     banks["REC::Particle"]->Hist1D("REC::Particle::Pz", 100, 0, 12, "");
//     banks["REC::Particle"]->Hist1D("REC::Particle::Beta", 100, 0, 1, "");
//     banks["REC::Particle"]->Hist2D("REC::Particle::Beta:REC::Particle::Pz", 100, 0, 1, 100, 0, 12, "");
//     banks["REC::Particle"]->Draw();
//
//     // Write into file.
//     f.Write();
//
//     // Do not free up memory since ROOT seems to hate it.
//     // free(input_file);
//     return 0;
// }

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
