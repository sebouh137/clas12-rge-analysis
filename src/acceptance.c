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
    char const *p_all = "all"; // All particles.
    char const *p_pos = "pos"; // Positive particles.
    char const *p_neg = "neg"; // Negative particles.
    char const *p_pip = "pip"; // pi plus.
    char const *p_pim = "pim"; // pi minus.
    char const *p_tre = "tre"; // trigger electron.

    // Histogram Constants. (TODO. Move this to its own file).
    char const *h_pz      = "pz";
    char const *h_beta    = "beta";
    char const *h_beta_pz = "beta v pz";

    // Add histos.
    std::map<char const *, std::map<char const *, TH1 *>> histos;
    histos.insert({p_all, {}}); // All particles.
    histos.insert({p_pos, {}}); // Positive particles.
    histos.insert({p_neg, {}}); // Negative particles.
    histos.insert({p_pip, {}}); // pi plus.
    histos.insert({p_pim, {}}); // pi minus.
    histos.insert({p_tre, {}}); // trigger electron.

    {
        std::map<char const *, std::map<char const *, TH1 *>>::iterator it;
        int d = 0;
        for (it = histos.begin(); it != histos.end(); ++it) {
            it->second = {
                {h_pz,      new TH1F(Form("%d", d+1), "Pz", 100, 0, 12)},
                {h_beta,    new TH1F(Form("%d", d+2), "Beta", 100, 0, 1)},
                {h_beta_pz, new TH2F(Form("%d", d+3), "Pz vs Beta", 100, 0, 1, 100, 0, 12)},
            };
            d += 1000;
        }
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

                // TODO. For loop per PID.
                // TODO. Add PID cuts.
                histos[p_all][h_pz]->Fill(p->getPz());
                histos[p_all][h_beta]->Fill(p->getBeta());
                histos[p_all][h_beta_pz]->Fill(p->getBeta(), p->getPz());
            }
        }
    }

    // Create output file.
    TFile f("out/histos.root", "RECREATE");

    // Write to output file.
    f.mkdir("Vertex P");
    f.cd("Vertex P");
    histos[p_all][h_pz]->Write();
    histos[p_all][h_beta]->Write();
    histos[p_all][h_beta_pz]->Write();
    f.cd("/");

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
