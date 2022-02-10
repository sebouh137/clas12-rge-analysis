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
    // Create output file.
    TFile f("out/histos.root", "RECREATE");

    // Open relevant data banks.
    // std::map<char*, std::map<char*, BankHist>> banks;
    std::map<char const *, BankHist *> banks = {
            {"REC::Particle", new BankHist(input_file)},
            // {"", new BankHist(input_file)},
            // {"", new BankHist(input_file)},
            // {"", new BankHist(input_file)},
            // {"", new BankHist(input_file)},
            // {"", new BankHist(input_file)},
    };

    banks["REC::Particle"]->SetEntries(nevents);

    // BankHist rec_part(input_file);
    // BankHist rec_trk (input_file);
    // BankHist rec_traj(input_file);
    // BankHist fmt_trks(input_file);
    // BankHist rec_ecal(input_file);
    // BankHist rec_tof (input_file);
    // if (nevents != 0) {
    //     rec_part.SetEntries(nevents);
    //     rec_trk .SetEntries(nevents);
    //     rec_traj.SetEntries(nevents);
    //     fmt_trks.SetEntries(nevents);
    //     rec_ecal.SetEntries(nevents);
    //     rec_tof .SetEntries(nevents);
    // }

    banks["REC::Particle"]->Hist1D("REC::Particle::Pz", 100, 0, 12, "");
    banks["REC::Particle"]->Draw();

    // Write into file.
    f.Write();

    // Do not free up memory since ROOT seems to hate it.
    // free(input_file);
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
