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

#include "../lib/constants.h"
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
// TODO. Evaluate **acceptance** in diferent regions.
// TODO. See simulations with Esteban or get them from RG-F.

// TODO. Add status cut.
// TODO. Evaluate the most basic cuts and implement the necessary ones.
// TODO. Check what happens with the acceptance of different particles (like pi+ and pi-) when you
//       reverse the magnetic fields.
// TODO. Check if we can run high luminosity with reverse fields.
// TODO. Check if RG-F or RG-M ran with reverse field.
// TODO. See what happens to low-momentum particles inside CLAS12 through simulation and see if they
//       are reconstructed.

// Assign name to plots, recursively going through binnings.
int name_plt(TH1 * plt[], TString * name, int * idx, long dbins, long depth, int px, long bx[],
             double rx[][2], int bvx[], long bbx[], double brx[][2], double b_interval[]) {
    if (depth == dbins) {
        // Create plot and increase index.
        if (px == 0) plt[* idx] =
                new TH1F(* name, * name, bx[0], rx[0][0], rx[0][1]);
        if (px == 1) plt[* idx] =
                new TH2F(* name, * name, bx[0], rx[0][0], rx[0][1], bx[1], rx[1][0], rx[1][1]);
        ++(* idx);
        return 0;
    }

    for (int bbi = 0; bbi < bbx[depth]; ++bbi) {
        // Find limits.
        double b_low  = brx[depth][0] + b_interval[depth]* bbi;
        double b_high = brx[depth][0] + b_interval[depth]*(bbi+1);

        // Append bin limits to name.
        TString name_cpy = name->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", S_VAR_LIST[bvx[depth]], b_low, b_high));

        // Continue down the line.
        name_plt(plt, &name_cpy, idx, dbins, depth+1, px, bx, rx, bvx, bbx, brx, b_interval);
    }

    return 0;
}

int find_bin(TString * name, int plt_size, int idx, long dbins, long depth, int prev_dim_factor,
             int vx[], long bx[], double rx[][2], double interval[]) {
    if (depth == dbins) return 0;

    // Find index in array (for this dimension).
    int dim_factor = 1;
    for (int di = depth+1; di < dbins; ++di) dim_factor *= bx[di];
    int bi = (idx%prev_dim_factor)/dim_factor;

    // Get limits.
    double low  = rx[depth][0] + interval[depth]* bi;
    double high = rx[depth][0] + interval[depth]*(bi+1);

    // Append dir to name.
    name->Append(Form("%s (%6.2f, %6.2f)/", S_VAR_LIST[vx[depth]], low, high));

    return find_bin(name, plt_size, idx, dbins, depth+1, dim_factor, vx, bx, rx, interval);
}

// Find index of plot in array, recursively going through binnings.
int find_idx(long dbins, long depth, Float_t var[], long bx[], double rx[][2], double interval[]) {
    if (depth == dbins) return 0;
    for (int bi = 0; bi < bx[depth]; ++bi) {
        // Define bin limits.
        double low  = rx[depth][0] + interval[depth]* bi;
        double high = rx[depth][0] + interval[depth]*(bi+1);

        // Find bin for var.
        if (low < var[depth] && var[depth] < high) {
            int dim_factor = 1;
            for (int di = depth+1; di < dbins; ++di) dim_factor *= bx[di];
            return bi*dim_factor + find_idx(dbins, depth+1, var, bx, rx, interval);
        }
    }

    return -1; // Variable is not within binning range.
}

int run() {
    TFile * f_in  = TFile::Open("../root_io/ntuples.root", "READ");   // NOTE. This path sucks.
    TFile * f_out = TFile::Open("../root_io/plots.root", "RECREATE"); // NOTE. This path sucks.

    if (!f_in || f_in->IsZombie()) return 1;

    // NOTE. This function could receive a few arguments to speed IO up. Pre-configured cuts,
    //       binnings, and corrections would be nice.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).

    // === CUT SETUP ===============================================================================
    // TODO. What particles should be used? All? e-? positive? k+? trigger e-? etc...
    printf("\nWhat particle should be plotted? Available cuts:\n[");
    for (int pi = 0; pi < PART_LIST_SIZE; ++pi) printf("%s, ", PART_LIST[pi]);
    printf("\b\b]\n");
    int part = catch_string(PART_LIST, PART_LIST_SIZE);
    int p_charge;
    if      (part == A_PPOS || part == A_PPST || part == A_PPIP) p_charge = 1;
    else if (part == A_PNEU || part == A_PPIN) p_charge = 0;
    else if (part == A_PNEG || part == A_PTRE || part == A_PELC || part == A_PPIM) p_charge = -1;
    else p_charge = 2;

    bool general_cuts  = false;
    bool geometry_cuts = false;
    bool sidis_cuts    = false;
    printf("\nApply all default cuts (general, geometry, DIS)? [y/n]\n");
    if (!catch_yn()) {
        printf("\nApply general cuts? [y/n]\n");
        general_cuts = catch_yn();
        printf("\nApply geometry cuts? [y/n]\n");
        geometry_cuts = catch_yn();
        printf("\nApply DIS cuts? [y/n]\n");
        sidis_cuts = catch_yn();
    }
    else {
        general_cuts  = true;
        geometry_cuts = true;
        sidis_cuts    = true;
    }

    // TODO.
    // printf("\nApply any custom cut? [y/n]\n");
    // bool custom_cuts = catch_yn();

    // === BINNING SETUP ===========================================================================
    printf("\nNumber of dimensions for binning?\n");
    long   dbins = catch_long();
    int    bvx[dbins];
    double brx[dbins][2];
    double b_interval[dbins];
    long   bbx[dbins];
    for (long bdi = 0; bdi < dbins; ++bdi) {
        // variable.
        printf("\nDefine var for bin in dimension %ld. Available vars:\n[", bdi);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        bvx[bdi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

        // range.
        for (int ri = 0; ri < 2; ++ri) {
            printf("\nDefine %s limit for bin in dimension %ld:\n", RAN_LIST[ri], bdi);
            brx[bdi][ri] = catch_double();
        }

        // nbins.
        printf("\nDefine number of bins for bin in dimension %ld:\n", bdi);
        bbx[bdi] = catch_long();

        // binning interval.
        b_interval[bdi] = (brx[bdi][1] - brx[bdi][0])/bbx[bdi];
    }

    // === PLOT SETUP ==============================================================================
    // Number of plots.
    printf("\nDefine number of plots (Set to 0 to just draw standard plots).\n");
    long pn = catch_long();

    bool stdplt = false;
    if (pn == 0) {
        stdplt = true;
        pn     = STDPLT_LIST_SIZE;
    }

    int    px[pn];
    int    vx[pn][2];
    double rx[pn][2][2];
    long   bx[pn][2];
    for (long pi = 0; pi < pn && !stdplt; ++pi) {
        // Check if we are to make a 1D or 2D plot.
        printf("\nPlot %ld type? [", pi);
        for (int vi = 0; vi < PLOT_LIST_SIZE; ++vi) printf("%s, ", PLOT_LIST[vi]);
        printf("\b\b]:\n");
        px[pi] = catch_string(PLOT_LIST, PLOT_LIST_SIZE);

        for (int di = 0; di < px[pi]+1; ++di) {
            // Check variable(s) to be plotted.
            printf("\nDefine var to be plotted on the %s axis. Available vars:\n[", DIM_LIST[di]);
            for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
            printf("\b\b]\n");
            vx[pi][di] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

            // Define ranges.
            for (int ri = 0; ri < 2; ++ri) {
                printf("\nDefine %s limit for %s axis:\n", RAN_LIST[ri], DIM_LIST[di]);
                rx[pi][di][ri] = catch_double();
            }

            // Define number of bins in plot.
            printf("\nDefine number of bins for %s axis:\n", DIM_LIST[di]);
            bx[pi][di] = catch_long();
        }
    }
    if (stdplt) { // Setup standard plots.
        memcpy(px, STD_PX, sizeof px);
        memcpy(vx, STD_VX, sizeof vx);
        memcpy(rx, STD_RX, sizeof rx);
        memcpy(bx, STD_BX, sizeof bx);
    }

    // === NTUPLES SETUP ===========================================================================
    TNtuple * t = (TNtuple *) f_in->Get(S_PARTICLE);
    Float_t vars[VAR_LIST_SIZE];
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) t->SetBranchAddress(S_VAR_LIST[vi], &vars[vi]);

    // === APPLY CUTS ==============================================================================
    // Apply SIDIS cuts, checking which event numbers should be skipped.
    // int nruns   =  1; // TODO.
    int nevents = -1;
    // Count number of events. NOTE. There's probably a cleaner way to do this.
    for (int i = 0; i < t->GetEntries(); ++i) {
        t->GetEntry(i);
        if (vars[A_EVENTNO] > nevents) nevents = (int) (vars[A_EVENTNO]+0.5);
    }

    bool valid_event[nevents];
    Float_t current_evn = -1;
    bool no_tre_pass, Q2_pass, W2_pass;
    for (int i = 0; i < t->GetEntries(); ++i) {
        t->GetEntry(i);
        if (vars[A_EVENTNO] != current_evn) {
            current_evn = vars[A_EVENTNO];
            valid_event[(int) (vars[A_EVENTNO]+0.5)] = false;
            no_tre_pass = false;
            Q2_pass     = true;
            W2_pass     = true;
        }

        if (vars[A_PID] != 11 || vars[A_STATUS] > 0) continue;
        no_tre_pass = true;
        Q2_pass = vars[A_Q2] >= Q2CUT;
        W2_pass = vars[A_W2] >= W2CUT;

        valid_event[(int) (vars[A_EVENTNO]+0.5)] = no_tre_pass && Q2_pass && W2_pass;
    }

    // === PLOT ====================================================================================
    // Create plots, separated by n-dimensional binning.
    long plt_size = 1;
    for (int bdi = 0; bdi < dbins; ++bdi) plt_size *= bbx[bdi];

    TH1 * plt[pn][plt_size];
    for (int pi = 0; pi < pn; ++pi) {
        TString name;
        int idx = 0;
        if (px[pi] == 0) name = Form("%s", S_VAR_LIST[vx[pi][0]]);
        if (px[pi] == 1) name = Form("%s vs %s", S_VAR_LIST[vx[pi][0]], S_VAR_LIST[vx[pi][1]]);
        name_plt(plt[pi], &name, &idx, dbins, 0, px[pi], bx[pi], rx[pi], bvx, bbx, brx, b_interval);
    }

    // Run through events.
    for (int i = 0; i < t->GetEntries(); ++i) {
        t->GetEntry(i);

        // Apply cuts.
        if (p_charge ==  1 && !(vars[A_CHARGE] >  0)) continue; // TODO. This one isn't working...?
        if (p_charge ==  0 && !(vars[A_CHARGE] == 0)) continue;
        if (p_charge == -1 && !(vars[A_CHARGE] <  0)) continue;

        if (general_cuts) {
            if (-0.5 < vars[A_PID] && vars[A_PID] < 0.5) continue; // Non-identified particle.
            if (vars[A_CHI2]/vars[A_NDF] >= CHI2NDFCUT)  continue; // Ignore tracks with high chi2.
        }

        if (geometry_cuts) {
            if (calc_magnitude(vars[A_VX], vars[A_VY]) > VXVYCUT) continue;
            if (VZLOWCUT > vars[A_VZ] || vars[A_VZ] > VZHIGHCUT) continue;
        }

        if (sidis_cuts && !valid_event[(int) (vars[A_EVENTNO]+0.5)]) continue;

        // Prepare binning vars.
        Float_t b_vars[dbins];
        for (long bdi = 0; bdi < dbins; ++bdi) b_vars[bdi] = vars[bvx[bdi]];

        for (int pi = 0; pi < pn; ++pi) {
            // SIDIS variables only make sense for some particles.
            bool sidis_pass = true;
            for (int di = 0; di < px[pi]+1; ++di) {
                for (int li = 0; li < DIS_LIST_SIZE; ++li) {
                    if (!strcmp(R_VAR_LIST[vx[pi][di]], DIS_LIST[li]) && vars[vx[pi][di]] < 1e-9)
                        sidis_pass = false;
                }
            }
            if (!sidis_pass) continue;

            // Find corresponding bin.
            int idx = find_idx(dbins, 0, b_vars, bbx, brx, b_interval);
            if (idx == -1) continue;

            // Fill histogram.
            if (px[pi] == 0) plt[pi][idx]->Fill(vars[vx[pi][0]]);
            if (px[pi] == 1) plt[pi][idx]->Fill(vars[vx[pi][0]], vars[vx[pi][1]]);
        }
    }

    for (int plti = 0; plti < plt_size; ++plti) {
        // Find dir.
        TString dir;
        find_bin(&dir, plt_size, plti, dbins, 0, INT_MAX, bvx, bbx, brx, b_interval);
        f_out->mkdir(dir);
        f_out->cd(dir);

        // Write plot(s).
        for (int pi = 0; pi < pn; ++pi) plt[pi][plti]->Write();
    }

    // === CLEAN-UP ================================================================================
    f_in ->Close();
    f_out->Close();

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}
