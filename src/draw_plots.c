#include <stdbool.h>
#include <stdio.h>
// #include <stdlib.h>
#include <regex.h>

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

int run(char * command, char * cuts, char * binning, int sample_size) {
    TFile *f_in  = TFile::Open("../root_io/out.root", "READ"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;

    // TODO. Turn everything into lowercase.


    // TODO. Prepare cuts.

    // TODO. Prepare binning.

    // Check if we are to make a 1D or 2D plot.
    int plt_n = 2;
    int px    = -1;
    const char * plt_str[plt_n] = {R_PLOT1D, R_PLOT2D};
    for (int pi = 0; pi < plt_n; ++pi) {
        regex_t plt_rx;
        if (regcomp(&plt_rx, plt_str[pi], 0)) return 1;
        if (regexec(&plt_rx, command, 0, NULL, 0) == 0) {
            if (px != -1) return 2;
            px = pi;
        }
    }
    if (px == -1) return 3;
    printf("Plot type is: %d\n", px);

    // Check variable(s) to be plotted.
    int n_vars = 24;
    const char * var_str[n_vars] = {R_RUNNO, R_EVENTNO, R_BEAME, R_PID, R_CHARGE, R_MASS, R_VX,
            R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI, R_BETA, R_PCAL_E, R_ECIN_E, R_ECOU_E,
            R_TOT_E, R_DTOF, R_Q2, R_NU, R_XB, R_W2};
    int vx = -1;
    int vy = -1;
    for (int vi = 0; vi < n_vars; ++vi) {
        regex_t var_rx;
        if (regcomp(&var_rx, var_str[vi], 0)) return 4;
        if (regexec(&var_rx, command, 0, NULL, 0) == 0) {
            if      (vx == -1) vx = vi;
            else if (vy == -1) vy = vi;
            else               return 5;
        }
    }
    if (px == 0 && (vx == -1 || vy != -1)) return 6;
    if (px == 1 && (vx == -1 || vy == -1)) return 6;
    printf("Vars are: %d, %d\n", vx, vy);

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
    int sample_size = -1;

    if (draw_plots_handle_args_err(draw_plots_handle_args(argc, argv, &command, &cuts, &binning,
                                   &sample_size), &command, &cuts, &binning))
        return 1;
    return draw_plots_err(run(command, cuts, binning, sample_size), &command, &cuts, &binning);
}
