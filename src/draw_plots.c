// #include <stdbool.h>
// #include <stdio.h>
// #include <stdlib.h>

#include <TFile.h>
// #include <TNtuple.h>

// #include "../lib/constants.h"
#include "../lib/err_handler.h"
// #include "../lib/file_handler.h"
#include "../lib/io_handler.h"
// #include "../lib/particle.h"
// #include "../lib/utilities.h"

// TODO. Check and fix theoretical curves. -> Ask Raffa.
// TODO. See why I'm not seeing any neutrals. -> Ask Raffa.
// TODO. Get simulations from RG-F, understand how they're made to do acceptance correction.
//           -> Ask Raffa.

// TODO. Separate in z bins and see what happens.
// TODO. Evaluate acceptance in diferent regions.
// TODO. See simulations with Esteban.
// NOTE. Adding a functionality to be able to request a plot and get it done in one line would be
//       the gold standard for this program.

int run(char * command, char * cuts, char * binning, bool debug, int sample_size) {
    TFile *f_in  = TFile::Open("../root_io/out.root", "READ"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;



    // Clean up after ourselves.
    f_in->Close();
    if (command != NULL) free(command);
    if (cuts    != NULL) free(cuts);
    if (binning != NULL) free(binning);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    char * command  = NULL;
    char * cuts     = NULL;
    char * binning  = NULL;
    bool debug      = false;
    int sample_size = -1;

    if (draw_plots_handle_args_err(draw_plots_handle_args(argc, argv, &command, &cuts, &binning,
                                   &debug, &sample_size), &command, &cuts, &binning))
        return 1;
    return draw_plots_err(run(command, cuts, binning, debug, sample_size), &command, &cuts,
                          &binning);
}
