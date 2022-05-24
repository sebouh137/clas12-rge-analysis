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

    // TODO. This function could receive a few arguments to speed things up. Pre-configured cuts,
    //       binnings, and corrections would be nice.
    // TODO. Prepare cuts.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).
    // TODO. Prepare binning.

    // === PLOTTING ================================================================================
    // Check if we are to make a 1D or 2D plot.
    printf("\nPlot type? [");
    for (int pi = 0; pi < PLOT_LIST_SIZE; ++pi) printf("%s, ", PLOT_LIST[pi]);
    printf("\b\b]:\n");
    int px = catch_string(PLOT_LIST, PLOT_LIST_SIZE);
    int pn = px + 1; // Number of histogram axes.

    // Check variable(s) to be plotted.
    int vx[pn];
    char * vx_tuplename[pn];
    for (int pi = 0; pi < pn; ++pi) {
        printf("\nDefine var to be plotted on the %s axis. Available vars:\n[", DIM_LIST[pi]);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        vx[pi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);
        find_ntuple(&vx_tuplename[pi], vx[pi]);
    }

    // Define ranges.
    double rx[pn][2];
    for (int pi = 0; pi < pn; ++pi) {
        for (int ri = 0; ri < 2; ++ri) {
            printf("\nDefine %s limit for %s axis:\n", RAN_LIST[ri], DIM_LIST[pi]);
            rx[pi][ri] = catch_double();
        }
    }

    // Define number of bins in plot.
    long bx[pn];
    for (int pi = 0; pi < pn; ++pi) {
        printf("\nDefine number of bins for %s axis:\n", DIM_LIST[pi]);
        bx[pi] = catch_long();
    }

    // Plot.
    TCanvas * gcvs = new TCanvas();
    TH1 * plt;
    if (px == 0)
        plt = new TH1F(S_VAR_LIST[vx[0]], S_VAR_LIST[vx[0]], bx[0], rx[0][0], rx[0][1]);
    if (px == 1) {
        TString name = Form("%s vs %s", S_VAR_LIST[vx[0]], S_VAR_LIST[vx[1]]);
        plt = new TH2F(name, name, bx[0], rx[0][0], rx[0][1], bx[1], rx[1][0], rx[1][1]);
    }

    TNtuple * t[pn];
    Float_t var[pn];
    for (int pi = 0; pi < pn; ++pi) {
        t[pi] = (TNtuple *) f_in->Get(vx_tuplename[pi]);
        t[pi]->SetBranchAddress(S_VAR_LIST[vx[pi]], &var[pi]);
    }
    for (int i = 0; i < t[0]->GetEntries(); ++i) {
        for (int pi = 0; pi < pn; ++pi) t[pi]->GetEntry(i);
        if (px == 0) plt->Fill(var[0]);
        if (px == 1) plt->Fill(var[0], var[1]);
    }
    if (px == 0) plt->Write();
    if (px == 1) {
        plt->Draw("colz");
        gcvs->Write();
    }

    // === CLEAN-UP ================================================================================
    f_in ->Close();
    f_out->Close();
    for (int pi = 0; pi < pn; ++pi) free(vx_tuplename[pi]);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}
