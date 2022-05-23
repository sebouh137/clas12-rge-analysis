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

int run() {
    TFile *f_in  = TFile::Open("../root_io/out.root", "READ"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;

    // TODO. Prepare cuts.

    // TODO. Prepare binning.

    // Check if we are to make a 1D or 2D plot.
    int plt_n =  2;
    const char * plt_str[plt_n] = {R_PLOT1D, R_PLOT2D};
    int px    = -1;

    while (true) {
        char plt_type[3];
        printf("\nPlot type? [");
        for (int pi = 0; pi < plt_n; ++pi) printf("%s, ", plt_str[pi]);
        printf("\b\b]:\n>>> ");
        scanf("%6s", plt_type);

        for (int pi = 0; pi < plt_n; ++pi) if (!strcmp(plt_type, plt_str[pi])) px = pi;
        if (px != -1) break;
    }

    // Check variable(s) to be plotted.
    int n_vars = 24;
    const char * var_str[n_vars] = {R_RUNNO, R_EVENTNO, R_BEAME, R_PID, R_CHARGE, R_MASS, R_VX,
            R_VY, R_VZ, R_PX, R_PY, R_PZ, R_P, R_THETA, R_PHI, R_BETA, R_PCAL_E, R_ECIN_E, R_ECOU_E,
            R_TOT_E, R_DTOF, R_Q2, R_NU, R_XB, R_W2};
    int vx = -1;
    int vy = -1;

    while (true) {
        char varx[32];
        printf("\nDefine variable to be plotted on the x axis. Available variables:\n[");
        for (int vi = 0; vi < n_vars; ++vi) printf("%s, ", var_str[vi]);
        printf("\b\b]\n>>> ");
        scanf("%31s", varx);

        for (int vi = 0; vi < n_vars; ++vi) if (!strcmp(varx, var_str[vi])) vx = vi;
        if (vx != -1) break;
    }
    if (px == 1) {
        while (true) {
            char vary[32];
            printf("\nDefine variable to be plotted on the y axis. Available variables:\n[");
            for (int vi = 0; vi < n_vars; ++vi) printf("%s, ", var_str[vi]);
            printf("\b\b]\n>>> ");
            scanf("%31s", vary);

            for (int vi = 0; vi < n_vars; ++vi) if (!strcmp(varx, var_str[vi])) vy = vi;
            if (vy != -1) break;
        }
    }

    // TODO. Define ranges.
    // double rx[2] = {0., 0.};
    // double ry[2] = {0., 0.};
    // {
    //     // regmatch_t pos_rx[2];
    //     regex_t ran_rx[2];
    //     if (regcomp(&ran_rx[0], "xm_([^[:space:]]*)/g", 0))      return 7; // Match the first number.
    //     if (regcomp(&ran_rx[1], "xr\[[^,]+,[ ]*([^\]]*)", 0)) return 7; // Match the second number.
    //
    //     printf("command: \"%s\"\n", command);
    //
    //     regmatch_t pmatch[2];
    //     if (regexec(&ran_rx[0], command, 0, pmatch, 0) == 0) printf("AHAA!\n");
    //
    //     // TEST
    //     char * result;
    //     result = (char *) malloc(pmatch[1].rm_eo - pmatch[1].rm_so);
    //     strncpy(result, &command[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
    //     printf("a matched substring \"%s\" is found at position %d to %d.\n",
    //             result, pmatch[1].rm_so, pmatch[1].rm_eo - 1);
    //
    //     // later on ...
    //     free(result);
    //
    //     // for (int ri = 0; ri < 3; ++ri)
    //     //     regexec(&ran_rx[ri], command, 0, &pos_rx[ri], 0);
    //     //
    //     // printf("pos_rx:\n");
    //     // for (int ri = 0; ri < 3; ++ri)
    //     //     printf("  * [%d, %d]\n", pos_rx[ri][0], pos_rx[ri][1]);
    //
    //     // regex_t ran_ry[2];
    //     // if (regcomp(&ran_ry[0], "yr\[([^,]*)", 0))        return 7;
    //     // if (regcomp(&ran_ry[1], "yr\[[^,;]+,[ ]*([^]]*)", 0))  return 7;
    // }

    // TODO. Define number of bins in plot.

    // TODO. Plot.

    // Clean up after ourselves.
    f_in->Close();

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return draw_plots_err(run());
}
