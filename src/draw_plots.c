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

// TODO. See simulations with Esteban or get them from RG-F.
// TODO. Give the program the capacity to output more than one plot per run.

// Assign name to plots, recursively going through binnings.
int name_plt(TH1 * plt[], TString * name, int * idx, long dbins, long depth, int px, long bx[],
             double rx[][2], int bvx[], long bbx[], double brx[][2], double b_interval[]) {
    if (depth == dbins) {
        if (px == 0) plt[* idx] =
                new TH1F(* name, * name, bx[0], rx[0][0], rx[0][1]);
        if (px == 1) plt[* idx] =
                new TH2F(* name, * name, bx[0], rx[0][0], rx[0][1], bx[1], rx[1][0], rx[1][1]);
        ++(* idx);
        return 0;
    }

    for (int bbi = 0; bbi < bbx[depth]; ++bbi) {
        double b_low  = brx[depth][0] + b_interval[depth]* bbi;
        double b_high = brx[depth][0] + b_interval[depth]*(bbi+1);

        TString name_cpy = name->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", S_VAR_LIST[bvx[depth]], b_low, b_high));
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
        double low  = rx[depth][0] + interval[depth]* bi;
        double high = rx[depth][0] + interval[depth]*(bi+1);

        if (low < var[depth] && var[depth] < high) {
            int dim_factor = 1;
            for (int di = depth+1; di < dbins; ++di) dim_factor *= bx[di];
            return bi*dim_factor + find_idx(dbins, depth+1, var, bx, rx, interval);
        }
    }

    return -1; // Variable is not within binning range.
}

// TODO. Draw all standard plots.
int draw_std_plots() {
    return 1;
}

int run() {
    TFile * f_in  = TFile::Open("../root_io/ntuples.root", "READ");   // NOTE. This path sucks.
    TFile * f_out = TFile::Open("../root_io/plots.root", "RECREATE"); // NOTE. Again. This path sucks.

    if (!f_in || f_in->IsZombie()) return 1;

    // NOTE. This function could receive a few arguments to speed IO up. Pre-configured cuts,
    //       binnings, and corrections would be nice.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).

    // === CUT SETUP ===============================================================================
    // TODO. What particles should be used? All? e-? positive? k+? trigger e-? etc...

    bool general_cuts  = false;
    bool geometry_cuts = false;
    bool sidis_cuts    = false;
    printf("\nApply all default cuts (general, geometry, SIDIS)? [y/n]\n");
    if (!catch_yn()) {
        printf("\nApply general cuts? [y/n]\n");
        general_cuts = catch_yn();
        printf("\nApply geometry cuts? [y/n]\n");
        geometry_cuts = catch_yn();
        printf("\nApply SIDIS cuts? [y/n]\n");
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
    long dbins = catch_long();
    int    bvx[dbins];
    char * bvx_tuplename[dbins];
    double brx[dbins][2];
    double b_interval[dbins];
    long   bbx[dbins];
    for (long bdi = 0; bdi < dbins; ++bdi) {
        // variable.
        printf("\nDefine var for bin in dimension %ld. Available vars:\n[", bdi);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        bvx[bdi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);
        find_ntuple(&bvx_tuplename[bdi], bvx[bdi]);

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
    // TODO. Check if we are to make a particular plot or all standard plots.

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

    // === APPLY CUTS ==============================================================================
    // Apply SIDIS cuts, checking which event numbers should be skipped.
    // int nruns   =  1; // TODO.
    int nevents = -1;
    // Count number of events. NOTE. There's probably a cleaner way to do this.
    TNtuple * cuts = (TNtuple *) f_in->Get(S_CUTS);
    Float_t c_evn; cuts->SetBranchAddress(S_EVENTNO, &c_evn);
    for (int i = 0; i < cuts->GetEntries(); ++i) {
        cuts->GetEntry(i);
        if (c_evn > nevents) nevents = (int) (c_evn+0.5);
    }

    bool valid_event[nevents];
    Float_t c_pid;    cuts->SetBranchAddress(S_PID,     &c_pid);
    Float_t c_status; cuts->SetBranchAddress(S_STATUS,  &c_status);
    Float_t c_q2;     cuts->SetBranchAddress(S_Q2,      &c_q2);
    Float_t c_w2;     cuts->SetBranchAddress(S_W2,      &c_w2);

    Float_t current_evn = -1;
    bool no_tre_pass, Q2_pass, W2_pass;
    for (int i = 0; i < cuts->GetEntries(); ++i) {
        cuts->GetEntry(i);
        if (c_evn != current_evn) {
            current_evn = c_evn;
            valid_event[(int) (c_evn+0.5)] = false;
            no_tre_pass = false;
            Q2_pass     = true;
            W2_pass     = true;
        }

        if (c_pid != 11 || c_status > 0) continue;
        no_tre_pass = true;
        Q2_pass = c_q2 >= Q2CUT;
        W2_pass = c_w2 >= W2CUT;

        valid_event[(int) (c_evn+0.5)] = no_tre_pass && Q2_pass && W2_pass;
    }

    // === SETUP NTUPLES ===========================================================================
    // TNtuples for cuts.
    Float_t c_ndf;  cuts->SetBranchAddress(S_NDF,     &c_ndf);
    Float_t c_chi2; cuts->SetBranchAddress(S_CHI2,    &c_chi2);
    Float_t c_vx;   cuts->SetBranchAddress(S_VX,      &c_vx);
    Float_t c_vy;   cuts->SetBranchAddress(S_VY,      &c_vy);
    Float_t c_vz;   cuts->SetBranchAddress(S_VZ,      &c_vz);

    // TNtuples of plotted variables.
    TNtuple * t[pn];
    Float_t var[pn];
    for (int pi = 0; pi < pn; ++pi) {
        t[pi] = (TNtuple *) f_in->Get(vx_tuplename[pi]);
        t[pi]->SetBranchAddress(S_VAR_LIST[vx[pi]], &var[pi]);
    }

    // TNtuples of binning variable (TODO. Make this variable*s*).
    TNtuple * bt[dbins];
    Float_t b_var[dbins];
    for (long bdi = 0; bdi < dbins; ++bdi) {
        bt[bdi] = (TNtuple *) f_in->Get(bvx_tuplename[bdi]);
        bt[bdi]->SetBranchAddress(S_VAR_LIST[bvx[bdi]], &b_var[bdi]);
    }

    // === PLOT ====================================================================================
    // Create plots, separated by n-dimensional binning.
    long plt_size = 1;
    for (int bdi = 0; bdi < dbins; ++bdi) plt_size *= bbx[bdi];

    TH1 * plt[plt_size];
    TString name;
    int idx = 0;
    if (px == 0) name = Form("%s", S_VAR_LIST[vx[0]]);
    if (px == 1) name = Form("%s vs %s", S_VAR_LIST[vx[0]], S_VAR_LIST[vx[1]]);
    name_plt(plt, &name, &idx, dbins, 0, px, bx, rx, bvx, bbx, brx, b_interval);

    // Run through events.
    for (int i = 0; i < t[0]->GetEntries(); ++i) {
        cuts->GetEntry(i);
        for (int pi = 0; pi < pn; ++pi) t[pi]->GetEntry(i);
        for (long bdi = 0; bdi < dbins; ++bdi) bt[bdi]->GetEntry(i);

        // Apply cuts.
        if (general_cuts) {
            if (-0.5 < c_pid && c_pid < 0.5) continue; // Non-identified particle.
            if (c_chi2/c_ndf >= CHI2NDFCUT)  continue; // Ignore tracks with high chi2.
        }

        if (geometry_cuts) {
            if (sqrt(c_vx*c_vx + c_vy*c_vy) > VXVYCUT) continue; // Too far from beamline.
            if (VZLOWCUT > c_vz || c_vz > VZHIGHCUT)   continue; // Too far from target.
        }

        if (sidis_cuts && !valid_event[(int) (c_evn+0.5)]) continue; // Event didn't pass SIDIS cut.

        // SIDIS variables only make sense for some particles.
        bool sidis_pass = true;
        for (int pi = 0; pi < pn; ++pi) {
            for (int li = 0; li < SIDIS_LIST_SIZE; ++li) {
                if (!strcmp(R_VAR_LIST[vx[pi]], SIDIS_LIST[li]) && var[pi] < 1e-9)
                    sidis_pass = false;
            }
        }
        if (!sidis_pass) continue;

        // Fill histogram in its corresponding bin.
        int idx = find_idx(dbins, 0, b_var, bbx, brx, b_interval);
        if (idx == -1) continue;

        if (px == 0) plt[idx]->Fill(var[0]);
        if (px == 1) plt[idx]->Fill(var[0], var[1]);
    }

    for (int plti = 0; plti < plt_size; ++plti) {
        // Find dir.
        TString dir;
        find_bin(&dir, plt_size, plti, dbins, 0, INT_MAX, bvx, bbx, brx, b_interval);
        f_out->mkdir(dir);
        f_out->cd(dir);

        // Write plot(s).
        plt[plti]->Write();
    }

    // === CLEAN-UP ================================================================================
    f_in ->Close();
    f_out->Close();
    for (int  pi  = 0; pi  < pn;    ++pi)  free( vx_tuplename[pi]);
    for (long bdi = 0; bdi < dbins; ++bdi) free(bvx_tuplename[bdi]);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    return run();
}
