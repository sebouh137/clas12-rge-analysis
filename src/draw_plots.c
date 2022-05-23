#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <TFile.h>
// #include <TNtuple.h>

// #include "../lib/constants.h"
// #include "../lib/err_handler.h"
// #include "../lib/file_handler.h"
// #include "../lib/io_handler.h"
// #include "../lib/particle.h"
#include "../lib/utilities.h"

// TODO. Check and fix theoretical curves. -> Ask Raffa.
// TODO. See why I'm not seeing any neutrals. -> Ask Raffa.
// TODO. Get simulations from RG-F, understand how they're made to do acceptance correction.
//           -> Ask Raffa.

// TODO. Separate in z bins and see what happens.
// TODO. Evaluate acceptance in diferent regions.
// TODO. See simulations with Esteban.
// NOTE. Adding a functionality to be able to request a plot and get it done in one line would be
//       the gold standard for this program.

int run() {
    TFile *f_in  = TFile::Open("../root_io/out.root", "READ"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;

    // TODO. Prepare cuts.

    // TODO. Prepare binning.

    // Check if we are to make a 1D or 2D plot.
    int plt_n =  2;
    const char * plt_str[plt_n] = {R_PLOT1D, R_PLOT2D};

    printf("\nPlot type? [");
    for (int pi = 0; pi < plt_n; ++pi) printf("%s, ", plt_str[pi]);
    printf("\b\b]:\n");
    int px = catch_string(plt_str, plt_n);

    // Check variable(s) to be plotted.
    int n_vars = 24;
    const char * var_str[n_vars] = {R_RUNNO, R_EVENTNO, R_BEAME, R_PID, R_CHARGE, R_MASS, R_VX,
            R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI, R_BETA, R_PCAL_E, R_ECIN_E, R_ECOU_E,
            R_TOT_E, R_DTOF, R_Q2, R_NU, R_XB, R_W2};

    printf("\nDefine variable to be plotted on the x axis. Available variables:\n[");
    for (int vi = 0; vi < n_vars; ++vi) printf("%s, ", var_str[vi]);
    printf("\b\b]\n");
    int vx = catch_string(var_str, n_vars);

    int vy;
    if (px == 1) {
        printf("\nDefine variable to be plotted on the y axis. Available variables:\n[");
        for (int vi = 0; vi < n_vars; ++vi) printf("%s, ", var_str[vi]);
        printf("\b\b]\n");
        vy = catch_string(var_str, n_vars);
    }

    // Define ranges.
    double rx[2] = {0, 0};
    double ry[2] = {0, 0};

    printf("\nDefine lower limit for x axis:\n");
    rx[0] = catch_double();
    printf("\nDefine upper limit for x axis:\n");
    rx[1] = catch_double();

    if (px == 1) {
        printf("\nDefine lower limit for y axis:\n");
        ry[0] = catch_double();
        printf("\nDefine upper limit for y axis:\n");
        ry[1] = catch_double();
    }

    // TODO. Define number of bins in plot.

    // TODO. Plot.

    // Clean up after ourselves.
    f_in->Close();

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}
