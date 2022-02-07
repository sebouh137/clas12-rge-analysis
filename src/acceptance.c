#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <TFile.h>

#include <BankHist.h>

#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

int run(char *input_file, bool use_fmt, int nevets, int run_no, double beam_energy) {
    // Create output file.
    TFile f("out/histos.root", "RECREATE");

    // Open input file.
    BankHist bankDraw(input_file);

    // Draw some shit.
    bankDraw.Hist1D("REC::Particle::Pz", 100, 0, 10, "");
    bankDraw.Draw();

    // Write into file.
    f.Write();

    // Do not free up memory since ROOT seems to hate it.
    // free(input_file);
    return 0;
}

// Execute program from CLAS12ROOT (`.x src/acceptance.c(filename, use_fmt, nevents)`).
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
