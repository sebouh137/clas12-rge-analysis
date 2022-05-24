#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TNtuple.h>

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
    TFile *f_in  = TFile::Open("../root_io/ntuples.root", "READ");   // NOTE. This path sucks.
    TFile *f_out = TFile::Open("../root_io/plots.root", "RECREATE"); // NOTE. Again. This path sucks.

    if (!f_in || f_in->IsZombie()) return 1;

    // TODO. A smart struct would simplify the code here and reduce the amount of constants and
    //       arrays by A LOT.
    // TODO. This function could receive a few arguments to speed things up. Pre-configured cuts,
    //       binnings, and corrections would be nice.
    // TODO. Prepare cuts.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).
    // TODO. Prepare binning.

    // Check if we are to make a 1D or 2D plot.
    printf("\nPlot type? [");
    for (int pi = 0; pi < PLOT_LIST_SIZE; ++pi) printf("%s, ", PLOT_LIST[pi]);
    printf("\b\b]:\n");
    int px = catch_string(PLOT_LIST, PLOT_LIST_SIZE);

    // Check variable(s) to be plotted.
    printf("\nDefine variable to be plotted on the x axis. Available variables:\n[");
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
    printf("\b\b]\n");
    int vx = catch_string(R_VAR_LIST, VAR_LIST_SIZE);
    char * vx_tuplename;
    find_ntuple(&vx_tuplename, vx);

    int vy;
    char * vy_tuplename;
    if (px == 1) {
        printf("\nDefine variable to be plotted on the y axis. Available variables:\n[");
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        vy = catch_string(R_VAR_LIST, VAR_LIST_SIZE);
        find_ntuple(&vy_tuplename, vy);
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

    // Define number of bins in plot.
    long bx = -1;
    printf("\nDefine number of bins for x axis:\n");
    bx = catch_long();

    long by = -1;
    if (px == 1) {
        printf("\nDefine number of bins for y axis:\n");
        by = catch_long();
    }

    if (px == 0) { // 1D plot.
        // TODO. Use the strings from "TITLE_STR" for fancier axes names and titles...
        TH1F * plt = new TH1F(S_VAR_LIST[vx], S_VAR_LIST[vx], bx, rx[0], rx[1]);
        TNtuple * t = (TNtuple *) f_in->Get(vx_tuplename);
        Float_t var;
        t->SetBranchAddress(S_VAR_LIST[vx], &var);
        for (int i = 0; i < t->GetEntries(); ++i) {
            t->GetEntry(i);
            plt->Fill(var);
        }
        plt->Write();
    }
    else if (px == 1) { // 2D plot.
        // TODO. Use the strings from "TITLE_STR" for fancier axes names and titles...
        TCanvas *gcvs = new TCanvas();
        TH2F * plt = new TH2F(S_VAR_LIST[vx], Form("%s vs %s", S_VAR_LIST[vx], S_VAR_LIST[vy]),
                             bx, rx[0], rx[1], by, ry[0], ry[1]);
        TNtuple * tx = (TNtuple *) f_in->Get(vx_tuplename);
        TNtuple * ty = (TNtuple *) f_in->Get(vy_tuplename);
        Float_t varx, vary;
        tx->SetBranchAddress(S_VAR_LIST[vx], &varx);
        ty->SetBranchAddress(S_VAR_LIST[vy], &vary);
        for (int i = 0; i < tx->GetEntries(); ++i) {
            tx->GetEntry(i);
            ty->GetEntry(i);
            plt->Fill(varx, vary);
        }
        plt->Draw("colz");
        gcvs->Write();
    }

    // Clean up after ourselves.
    f_in ->Close();
    f_out->Close();
    free(vx_tuplename);
    if (px == 1) free(vy_tuplename);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}
