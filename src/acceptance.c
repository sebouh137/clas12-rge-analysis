#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

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

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    // NOTE. Program goes here.

    free(input_file);
    return 0;
}

// Call program from ROOT interactive sesh.
int run(bool use_fmt, int nevents, char *input_file) {
    int    run_no      = -1;
    double beam_energy = -1;

    if (handle_args_err(handle_filename(input_file, &run_no, &beam_energy), &input_file, run_no))
        return 1;

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    // NOTE. Program goes here.
    // BankHist bankDraw("/home/twig/data/out_clas_011983.hipo");
    // bankDraw.Hist1D("REC::Particle::Pz",100,0,10,"")->Draw();

    free(input_file);
    return 0;
}

int run(char *input_file)               { return run(false, -1, input_file);      }
int run(bool use_fmt, char *input_file) { return run(use_fmt, -1, input_file);    }
int run(int nevents, char *input_file)  { return run(false, nevents, input_file); }
