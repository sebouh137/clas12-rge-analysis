// CLAS12 RG-E Analyser.
// Copyright (C) 2022-2023 Bruno Benkel
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You can see a copy of the GNU Lesser Public License under the LICENSE file.

// C.
#include <limits.h>
#include <libgen.h>

// ROOT.
#include <TFile.h>
#include <TH2.h>
#include <TNtuple.h>

// rge-analysis.
#include "../lib/rge_constants.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_progress.h"
#include "../lib/rge_pid_utils.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_math_utils.h"

static const char *USAGE_MESSAGE =
"Usage: draw_plots [-hp:cn:o:a:w:] infile\n"
" * -h          : show this message and exit.\n"
" * -p pid      : skip particle selection and draw plots for pid.\n"
" * -c          : apply all cuts (general, geometry, and DIS) instead of\n"
"                 asking which ones to apply while running.\n"
" * -n nentries : number of entries to process.\n"
" * -o outfile  : output file name. Default is plots_<run_no>.root.\n"
" * -a accfile  : apply acceptance correction using acc_filename.\n"
" * -A          : get acceptance correction plots without applying acceptance\n"
"                 correction. Requires -a to be set.\n"
" * -w workdir  : location where output root files are to be stored. Default\n"
"                 is root_io.\n"
" * infile      : input file produced by make_ntuples.\n\n"
"    Draw plots from a ROOT file built from make_ntuples. File should be\n"
"    named <text>run_no.root.\n";

/** Particle names array. */
#define PART_LIST_SIZE 5
#define A_PPOS 1
#define A_PNEG 2
#define A_PNEU 3
#define A_PPID 4
static const char *PART_LIST[PART_LIST_SIZE] = {
        "all", "+", "-", "neutral", "pid"
};

/** Plotting opts arrays. */
static const char *PLT_LIST[2] = {"1d", "2d"};
static const char *DIM_LIST[2] = {"x", "y"};
static const char *RAN_LIST[2] = {"lower", "upper"};

/** List of DIS variables. */
#define DIS_LIST_SIZE 5
static const char *DIS_LIST[DIS_LIST_SIZE] = {
        RGE_Q2.name, RGE_NU.name, RGE_XB.name, RGE_YB.name, RGE_W2.name
};

/** "Standard" plots data. */
#define STDPLT_LIST_SIZE 4
static const int STD_PX[STDPLT_LIST_SIZE] = {
        0, 0, 0, 0
};
static const int STD_VX[STDPLT_LIST_SIZE][2] = {
        {RGE_VZ.addr,  -1}, {RGE_THETA.addr, -1},
        {RGE_PHI.addr, -1}, {RGE_P.addr,     -1}
};
static const double STD_RX[STDPLT_LIST_SIZE][2][2] = {
        {{-30.,   20.},   {-1,-1}}, {{0.05, 0.9}, {-1,-1}},
        {{ -3.1415926535,  3.1415926535}, {-1,-1}}, {{0.,    9.}, {-1,-1}}
};
static const long STD_BX[STDPLT_LIST_SIZE][2] = {
        {50, -1}, {42, -1}, {72, -1}, {45, -1}
};

/** Acceptance correction plot data. */
#define ACCPLT_LIST_SIZE 5
static const int ACC_PX[ACCPLT_LIST_SIZE] = {
        0, 0, 0, 0, 0
};
static const int ACC_VX[ACCPLT_LIST_SIZE][2] = {
        {RGE_Q2.addr,-1}, {RGE_NU.addr,-1}, {RGE_ZH.addr,-1}, {RGE_PT2.addr,-1},
        {RGE_PHIPQ.addr,-1}
};

/**
 * Assign title to plots, recursively going through binnings. The title that
 *     will be given to the plots is <title> (<bin 1>) (<bin 2>) ... (<bin n>),
 *     where <title> is the base title given by the plot_title parameter.
 *
 *     TARGET VARIABLE.
 * @param plot_arr:    Array of plots to be created.
 *
 *     INTERNAL PARAMETERS.
 * @param dim_bins:    Binning dimension. Needed to compute how deep the
 *                     recursion should go before stopping.
 * @param idx:         Index in the plot array of the plot to be created. When
 *                     calling the function, this should be set to 0.
 * @param depth:       How deep along the number of bins we are. When calling
 *                     the function, this should be set to 0.
 *
 *     STRINGS.
 * @param plot_title:  Base for the title given to the plot.
 * @param x_var:       Variable name for the x axis.
 * @param y_var:       Variable name for the y axis.
 *
 *     PLOT PARAMETERS.
 * @param plot_type:   Dimensionality of plot. Set to 1 for a 1d plot, 2 for 2d.
 * @param plot_nbins:  2-dimensional array containing the number of bins for
 *                     each axis for each plot.
 * @param plot_range:  2-dimensional array containing the range for each axis
 *                     for each plot.
 *
 *     BINNING PARAMETERS.
 * @param bin_vars:    Array of variables for binning for each plot.
 * @param bin_nbins:   Array with number of dimensions for binning for each
 *                     binning.
 * @param bin_range:   2-dimensional array with lower and upper limits for each
 *                     binning variable.
 * @param bin_binsize: Array with size of each bin for each binning.
 *
 * @return:            Error code. Currently, can only return 0 (success).
 */
static int create_plots(
        TH1 *plot_arr[], luint dim_bins, int *idx, luint depth,
        TString *plot_title, const char *x_var, const char *y_var,
        int plot_type, luint plot_nbins[], double plot_range[][2],
        int bin_vars[], luint bin_nbins[], double bin_range[][2],
        double bin_binsize[]
) {
    if (depth == dim_bins) {
        // Create plot and increase index.
        if (plot_type == 0) {
            plot_arr[*idx] = new TH1F(
                    *plot_title, Form("%s;%s", plot_title->Data(), x_var),
                    plot_nbins[0], plot_range[0][0], plot_range[0][1]
            );
        }
        if (plot_type == 1) {
            plot_arr[*idx] = new TH2F(
                    *plot_title,
                    Form("%s;%s;%s", plot_title->Data(), x_var, y_var),
                    plot_nbins[0], plot_range[0][0], plot_range[0][1],
                    plot_nbins[1], plot_range[1][0], plot_range[1][1]
            );
        }
        ++(*idx);
        return 0;
    }

    for (luint bin_i = 0; bin_i < bin_nbins[depth]; ++bin_i) {
        // Find limits.
        double b_low  = bin_range[depth][0] + bin_binsize[depth]* bin_i;
        double b_high = bin_range[depth][0] + bin_binsize[depth]*(bin_i+1);

        // Append bin limits to title.
        TString name_cpy = plot_title->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", RGE_VARS[bin_vars[depth]],
                             b_low, b_high));

        // Continue down the recursive line...
        create_plots(
                plot_arr, dim_bins, idx, depth+1, &name_cpy, x_var, y_var,
                plot_type, plot_nbins, plot_range, bin_vars, bin_nbins,
                bin_range, bin_binsize
        );
    }

    return 0;
}

/** Copy of function above for acceptance corrected plots. */
static int create_acc_corr_plots(
        TH1 *plot_arr[], luint dim_bins, int *idx, luint depth,
        TString *plot_title, const char *x_var, const char *y_var,
        int plot_type, luint plot_nbins, double plot_edges[], int bin_vars[],
        luint bin_nbins[], double bin_range[][2], double bin_binsize[]
) {
    if (depth == dim_bins) {
        // Create plot and increase index.
        if (plot_type == 0) {
            plot_arr[*idx] = new TH1F(
                    *plot_title, Form("%s;%s", plot_title->Data(), x_var),
                    plot_nbins-1, plot_edges
            );
        }
        if (plot_type == 1) {
            rge_errno = RGEERR_2DACCEPTANCEPLOT;
            return 1; // Feature not available.
        }
        ++(*idx);
        return 0;
    }

    for (luint bin_i = 0; bin_i < bin_nbins[depth]; ++bin_i) {
        // Find limits.
        double b_low  = bin_range[depth][0] + bin_binsize[depth]* bin_i;
        double b_high = bin_range[depth][0] + bin_binsize[depth]*(bin_i+1);

        // Append bin limits to title.
        TString name_cpy = plot_title->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", RGE_VARS[bin_vars[depth]],
                             b_low, b_high));

        // Continue down the recursive line...
        if (create_acc_corr_plots(
                plot_arr, dim_bins, idx, depth+1, &name_cpy, x_var, y_var,
                plot_type, plot_nbins, plot_edges, bin_vars, bin_nbins,
                bin_range, bin_binsize
        )) return 1;
    }

    return 0;
}

/**
 * Find title of bin by recursively going through binnings and appending their
 *     range to the title.
 *
 *     TARGET VARIABLE.
 * @param plot_title:      Title of the plot to which we'll append each bin.
 *
 *     INTERNAL PARAMETERS.
 * @param dim_bins:        Binning dimension. Needed to compute how deep the
 *                         recursion should go before stopping.
 * @param idx:             Index in the plot array of the plot to be created.
 *                         When calling the function, this should be set to 0.
 * @param depth:           How deep along the number of bins we are. When
 *                         calling the function, this should be set to 0.
 * @param prev_dim_factor: Dimension factor of the previous binning in the
 *                         recursion.
 *
 *     BINNING PARAMETERS.
 * @param nplots:          Number of TH1F and TH2F plots, depends on number of
 *                         bins.
 * @param vars:            Array of variables for binning for each plot.
 * @param nbins:           Array with number of dimensions for binning for each
 *                         binning.
 * @param range:           2-dimensional array with lower and upper limits for
 *                         each binning variable.
 * @param binsize:         Array with size of each bin for each binning.
 *
 * @return:                Error code. Currently, can only return 0 (success).
 */
static int find_bin(
        TString *plot_title, luint dim_bins, luint idx, luint depth,
        luint prev_dim_factor, int nplots, int vars[], luint nbins[],
        double range[][2], double binsize[]
) {
    if (depth == dim_bins) return 0;

    // Find index in array (for this dimension).
    luint dim_factor = 1;
    for (luint depth_i = depth+1; depth_i < dim_bins; ++depth_i) {
        dim_factor *= nbins[depth_i];
    }
    luint bi = (idx%prev_dim_factor)/dim_factor;

    // Get limits.
    double low  = range[depth][0] + binsize[depth]* bi;
    double high = range[depth][0] + binsize[depth]*(bi+1);

    // Append dir to title.
    plot_title->Append(Form(
            "%s (%6.2f, %6.2f)/", RGE_VARS[vars[depth]], low, high
    ));

    return find_bin(
            plot_title, dim_bins, idx, depth+1, dim_factor,
            nplots, vars, nbins, range, binsize
    );
}

/**
 * Find index of plot in array, recursively going through binnings.
 *
 *     INTERNAL PARAMETERS.
 * @param dim_bins: Binning dimension. Needed to compute how deep the recursion
 *                  should go before stopping.
 * @param depth:    How deep along the number of bins we are. When calling the
 *                  function, this should always be 0.
 *
 *     BINNINGS PAREMETERS.
 * @param var:      Binning variables.
 * @param nbins:    Array with number of dimensions for binning for each
 *                  binning.
 * @param range:    2-dimensional array with lower and upper limits for each
 *                  binning variable.
 * @param binsize:  Array with size of each bin for each binning.
 *
 * @return:         Index of the bin we're looking for. Returns -1 if variable
 *                  is not within binning range.
 */
static lint find_idx(
        luint dim_bins, luint depth, Float_t var[], luint nbins[],
        double range[][2], double binsize[]
) {
    if (depth == dim_bins) return 0;
    for (luint bi = 0; bi < nbins[depth]; ++bi) {
        // Define bin limits.
        double low  = range[depth][0] + binsize[depth]* bi;
        double high = range[depth][0] + binsize[depth]*(bi+1);

        // Find bin for var.
        if (low < var[depth] && var[depth] < high) {
            luint dim_factor = 1;
            for (luint depth_i = depth + 1; depth_i < dim_bins; ++depth_i) {
                dim_factor *= nbins[depth_i];
            }
            lint idx =
                    find_idx(dim_bins, depth+1, var, nbins, range, binsize);
            if (idx < 0) return idx;
            else return static_cast<lint>(
                    bi*dim_factor + static_cast<luint>(idx)
            );
        }
    }

    return -1; // Variable is not within binning range.
}

/** run() function of the program. Check USAGE_MESSAGE for details. */
static int run(
        char *in_filename, char *out_filename, char *acc_filename,
        char *work_dir, int run_no, lint nentries, lint sel_pid,
        bool apply_all_cuts, bool apply_acc_corr
) {
    // Open input file.
    TFile *f_in  = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) {
        rge_errno = RGEERR_BADINPUTFILE;
        return 1;
    }

    // Get acceptance correction
    bool acc_plot = false;
    luint acc_nedges[5];
    luint acc_nbins;
    luint acc_npids;
    double **acc_edges;
    lint *acc_pids;
    int **acc_n_thrown;
    int **acc_n_simul;
    if (acc_filename != NULL) {
        acc_plot = true;
        if (rge_read_acc_corr_file(
                acc_filename, acc_nedges, &acc_edges, &acc_npids, &acc_nbins,
                &acc_pids, &acc_n_thrown, &acc_n_simul
        )) return 1;
    }

    // === PARTICLE SELECTION ==================================================
    int plot_particle = INT_MAX;
    int plot_charge   = INT_MAX;
    int plot_pid      = INT_MAX;
    if (sel_pid == 0) {
        printf("\nWhat particle should be plotted? Available cuts:\n[");
        for (int part_i = 0; part_i < PART_LIST_SIZE; ++part_i) {
            printf("%s, ", PART_LIST[part_i]);
        }
        printf("\b\b]\n");
        plot_particle = rge_catch_string(PART_LIST, PART_LIST_SIZE);
        if      (plot_particle == A_PPOS) plot_charge =  1;
        else if (plot_particle == A_PNEU) plot_charge =  0;
        else if (plot_particle == A_PNEG) plot_charge = -1;
        else if (plot_particle == A_PPID) {
            printf("\nSelect PID from:\n");
            rge_print_pid_names();
            plot_pid = rge_catch_long();
        }
    }
    else {
        plot_pid = sel_pid;
    }

    // If a PID was selected, check that it's valid.
    if (plot_pid != INT_MAX && rge_pid_invalid(plot_pid)) return 1;

    // Find selected particle PID in acceptance correction data. If not found,
    //     return an error.
    uint acc_pid_idx = UINT_MAX;
    if (acc_plot) {
        // Find index of plot_pid in acc_pids.
        for (uint pid_i = 0; pid_i < acc_npids; ++pid_i) {
            if (acc_pids[pid_i] == plot_pid) acc_pid_idx = pid_i;
        }
        if (acc_pid_idx == UINT_MAX) {
            rge_errno = RGEERR_NOACCDATA;
            return 1;
        }
    }

    // === SELECT CUTS =========================================================
    bool general_cuts  = false;
    bool geometry_cuts = false;
    bool dis_cuts      = false;
    if (!apply_all_cuts) {
        printf("\nApply all default cuts (general, geometry, DIS)? [y/n]\n");
        if (!rge_catch_yn()) {
            printf("\nApply general cuts? [y/n]\n");
            general_cuts = rge_catch_yn();
            printf("\nApply geometry cuts? [y/n]\n");
            geometry_cuts = rge_catch_yn();
            printf("\nApply DIS cuts? [y/n]\n");
            dis_cuts = rge_catch_yn();
        }
        else {
            apply_all_cuts = true;
        }
    }
    if (apply_all_cuts) {
        general_cuts  = true;
        geometry_cuts = true;
        dis_cuts      = true;
    }

    // === SETUP BINNING =======================================================
    printf("\nNumber of dimensions for binning?\n");
    luint dim_bins = static_cast<luint>(rge_catch_long());
    int    bin_vars[dim_bins];
    double bin_range[dim_bins][2];
    double bin_binsize[dim_bins];
    luint bin_nbins[dim_bins];
    for (luint bin_dim_i = 0; bin_dim_i < dim_bins; ++bin_dim_i) {
        // variable.
        printf(
                "\nDefine var for bin in dimension %ld by index. Available "
                "vars:\n", bin_dim_i
        );
        for (int var_i = 0; var_i < RGE_VARS_SIZE; ++var_i) {
            printf("  %2d. %s\n", var_i, RGE_VARS[var_i]);
        }
        bin_vars[bin_dim_i] = rge_catch_var(RGE_VARS, RGE_VARS_SIZE);

        // range.
        for (int range_i = 0; range_i < 2; ++range_i) {
            printf("\nDefine %s limit for bin in dimension %ld:\n",
                    RAN_LIST[range_i], bin_dim_i);
            bin_range[bin_dim_i][range_i] = rge_catch_double();
        }

        // nbins.
        printf(
                "\nDefine number of bins for bin in dimension %ld:\n", bin_dim_i
        );
        bin_nbins[bin_dim_i] = static_cast<luint>(rge_catch_long());

        // binning bin size.
        bin_binsize[bin_dim_i] =
                (bin_range[bin_dim_i][1] - bin_range[bin_dim_i][0]) /
                bin_nbins[bin_dim_i];
    }

    // === SETUP PLOT ==========================================================
    // Number of plots.
    luint plot_arr_size;

    // If acceptance correction is being made, only acceptance corrected
    //     variables (Q2, nu, zh, Pt2, and phiPQ) can be plotted.
    if (acc_plot) plot_arr_size = ACCPLT_LIST_SIZE;
    else {
        printf("\nDefine number of plots (Set to 0 to draw standard plots).\n");
        plot_arr_size = static_cast<luint>(rge_catch_long());
    }

    bool std_plot = false;
    if (plot_arr_size == 0) {
        std_plot = true;
        plot_arr_size = STDPLT_LIST_SIZE;
    }

    int plot_type[plot_arr_size];
    int plot_vars[plot_arr_size][2];
    double plot_range[plot_arr_size][2][2];
    luint plot_nbins[plot_arr_size][2];
    for (
            luint plot_i = 0;
            plot_i < plot_arr_size && !std_plot && !acc_plot;
            ++plot_i
    ) {
        // Check if we are to make a 1D or 2D plot.
        printf("\nPlot %ld type? [", plot_i);
        for (int var_i = 0; var_i < 2; ++var_i)
            printf("%s, ", PLT_LIST[var_i]);
        printf("\b\b]:\n");
        plot_type[plot_i] = rge_catch_string(PLT_LIST, 2);

        for (int dim_i = 0; dim_i < plot_type[plot_i]+1; ++dim_i) {
            // Check variable(s) to be plotted.
            printf(
                    "\nDefine var to be plotted on the %s axis by index. "
                    "Available vars:\n", DIM_LIST[dim_i]
            );
            for (int var_i = 0; var_i < RGE_VARS_SIZE; ++var_i) {
                printf("  %2d. %s\n", var_i, RGE_VARS[var_i]);
            }
            plot_vars[plot_i][dim_i] = rge_catch_var(RGE_VARS, RGE_VARS_SIZE);

            // Define ranges.
            for (int range_i = 0; range_i < 2; ++range_i) {
                printf("\nDefine %s limit for %s axis:\n",
                        RAN_LIST[range_i], DIM_LIST[dim_i]);
                plot_range[plot_i][dim_i][range_i] = rge_catch_double();
            }

            // Define number of bins in plot.
            printf("\nDefine number of bins for %s axis:\n", DIM_LIST[dim_i]);
            plot_nbins[plot_i][dim_i] = static_cast<luint>(rge_catch_long());
        }
    }

    // Setup standard plots.
    if (std_plot) {
        memcpy(plot_type,  STD_PX, sizeof plot_type);
        memcpy(plot_vars,  STD_VX, sizeof plot_vars);
        memcpy(plot_range, STD_RX, sizeof plot_range);
        memcpy(plot_nbins, STD_BX, sizeof plot_nbins);
    }

    // Setup acceptance corrected plots.
    if (acc_plot) {
        memcpy(plot_type,  ACC_PX, sizeof plot_type);
        memcpy(plot_vars,  ACC_VX, sizeof plot_vars);
    }

    // === SETUP NTUPLES =======================================================
    TNtuple *ntuple = static_cast<TNtuple *>(f_in->Get(RGE_TREENAMEDATA));
    if (ntuple == NULL) {
        rge_errno = RGEERR_BADROOTFILE;
        return 1;
    }

    Float_t vars[RGE_VARS_SIZE];
    for (int var_i = 0; var_i < RGE_VARS_SIZE; ++var_i) {
        ntuple->SetBranchAddress(RGE_VARS[var_i], &vars[var_i]);
    }

    // === APPLY CUTS ==========================================================
    printf("\nOpening file...\n");

    // Counters for fancy progress bar.
    if (nentries == -1 || nentries > ntuple->GetEntries()) {
        nentries = ntuple->GetEntries();
    }

    // Apply SIDIS cuts, checking which event numbers should be skipped.
    luint nevents = 0;

    // Prepare progress bar.
    rge_pbar_set_nentries(nentries);

    // Count number of events.
    for (lint entry = 0; entry < nentries; ++entry) {
        rge_pbar_update(entry);
        ntuple->GetEntry(entry);
        if (vars[RGE_EVENTNO.addr] > nevents) {
            nevents = static_cast<luint>(vars[RGE_EVENTNO.addr]+0.5);
        }
    }

    // Apply previously setup cuts.
    if (dis_cuts) printf("Applying cuts...\n");
    bool *valid_event = static_cast<bool *>(malloc(nevents * sizeof(bool)));
    Float_t current_evn = -1;
    bool no_tre_pass, Q2_pass, W2_pass, Yb_pass;

    // Fill valid_event array with false bools in case the next for loop doesn't
    //     fill every entry in it.
    for (luint event = 0; event < nevents && dis_cuts; ++event) {
        valid_event[event] = false;
    }

    // Since each entry is a particle, there are potentially many entries for
    //     one event. Then, we need to check beforehand which events are valid
    //     so that we can skip those when plotting. It is only necessary to do
    //     this if we're applying DIS cuts.
    rge_pbar_reset();
    for (lint entry = 0; entry < nentries && dis_cuts; ++entry) {
        rge_pbar_update(entry);

        ntuple->GetEntry(entry);
        if (vars[RGE_EVENTNO.addr] != current_evn) {
            current_evn = vars[RGE_EVENTNO.addr];
            valid_event[static_cast<luint>(vars[RGE_EVENTNO.addr]+0.5)] = false;
            no_tre_pass = false;
            Q2_pass     = true;
            W2_pass     = true;
            Yb_pass     = true;
        }

        if (
                (10.5 >= vars[RGE_PID.addr] || vars[RGE_PID.addr] > 11.5) ||
                vars[RGE_STATUS.addr] > 0
        ) {
            continue;
        }
        no_tre_pass = true;
        Q2_pass = vars[RGE_Q2.addr] >= RGE_Q2CUT;
        W2_pass = vars[RGE_W2.addr] >= RGE_W2CUT;
        Yb_pass = vars[RGE_YB.addr] <= RGE_YBCUT;

        valid_event[static_cast<luint>(vars[RGE_EVENTNO.addr]+0.5)] =
                no_tre_pass && Q2_pass && W2_pass && Yb_pass;
    }

    // === PLOT ================================================================
    // Create plots, separated by n-dimensional binning.
    luint bin_arr_size = 1;
    for (luint bin_dim_i = 0; bin_dim_i < dim_bins; ++bin_dim_i) {
        bin_arr_size *= bin_nbins[bin_dim_i];
    }

    TH1 *plot_arr[plot_arr_size][bin_arr_size];
    for (luint plot_i = 0; plot_i < plot_arr_size; ++plot_i) {
        TString plot_title;
        int idx = 0;
        if (acc_plot) { // Acceptance corrected plots have variable bin sizes.
            plot_title = Form("%s", RGE_VARS[plot_vars[plot_i][0]]);
            create_acc_corr_plots(
                    plot_arr[plot_i], dim_bins, &idx, 0, &plot_title,
                    RGE_VARS[plot_vars[plot_i][0]], "",
                    plot_type[plot_i], acc_nedges[plot_i], acc_edges[plot_i],
                    bin_vars, bin_nbins, bin_range, bin_binsize
            );
            continue;
        }
        if (plot_type[plot_i] == 0) { // 1D plot.
            plot_title = Form("%s", RGE_VARS[plot_vars[plot_i][0]]);
            create_plots(
                    plot_arr[plot_i], dim_bins, &idx, 0, &plot_title,
                    RGE_VARS[plot_vars[plot_i][0]], "",
                    plot_type[plot_i], plot_nbins[plot_i], plot_range[plot_i],
                    bin_vars, bin_nbins, bin_range, bin_binsize
            );
        }
        if (plot_type[plot_i] == 1) { // 2D plot.
            plot_title = Form("%s vs %s", RGE_VARS[plot_vars[plot_i][0]],
                    RGE_VARS[plot_vars[plot_i][1]]);
            create_plots(
                    plot_arr[plot_i], dim_bins, &idx, 0, &plot_title,
                    RGE_VARS[plot_vars[plot_i][0]],
                    RGE_VARS[plot_vars[plot_i][1]],
                    plot_type[plot_i], plot_nbins[plot_i], plot_range[plot_i],
                    bin_vars, bin_nbins, bin_range, bin_binsize
            );
        }
    }

    // Run through events.
    printf("Processing plots...\n");
    rge_pbar_reset();
    for (lint entry = 0; entry < nentries; ++entry) {
        rge_pbar_update(entry);
        ntuple->GetEntry(entry);

        // Apply particle cuts.
        if (plot_charge != INT_MAX) {
            if (plot_charge ==  1 && !(vars[RGE_CHARGE.addr] >  0)) continue;
            if (plot_charge ==  0 && !(vars[RGE_CHARGE.addr] == 0)) continue;
            if (plot_charge == -1 && !(vars[RGE_CHARGE.addr] <  0)) continue;
        }
        if (
                plot_pid != INT_MAX &&
                (
                        vars[RGE_PID.addr] - 0.5 >= plot_pid ||
                        plot_pid > vars[RGE_PID.addr] + 0.5
                )
        ) {
            continue;
        }

        // Apply geometry cuts.
        if (geometry_cuts) {
            if (
                    rge_calc_magnitude(vars[RGE_VX.addr], vars[RGE_VY.addr]) >
                    RGE_VXVYCUT
            ) {
                continue;
            }
            if (
                    RGE_VZLOWCUT > vars[RGE_VZ.addr] ||
                    vars[RGE_VZ.addr] > RGE_VZHIGHCUT
            ) {
                continue;
            }
        }

        // Apply miscellaneous cuts.
        if (general_cuts) {
            // Non-identified particle.
            if (-0.5 <= vars[RGE_PID.addr] && vars[RGE_PID.addr] <  0.5)
                continue;
            // Non-identified particle.
            if (44.5 <= vars[RGE_PID.addr] && vars[RGE_PID.addr] < 45.5)
                continue;
            // Ignore tracks with high chi2.
            if (vars[RGE_CHI2.addr]/vars[RGE_NDF.addr] >= RGE_CHI2NDFCUT)
                continue;
        }

        // Apply DIS cuts.
        if (
                dis_cuts &&
                !valid_event[static_cast<int>(vars[RGE_EVENTNO.addr]+0.5)]
        ) {
            continue;
        }

        // Remove DIS vars = 0.
        if (vars[RGE_Q2.addr] == 0 || vars[RGE_NU.addr] == 0) {
            continue;
        }
        // Remove SIDIS vars = 0 (for all but electrons!).
        if (
                (10.5 >= vars[RGE_PID.addr] || vars[RGE_PID.addr] > 11.5) &&
                (
                        vars[RGE_ZH.addr]    == 0 ||
                        vars[RGE_PT2.addr]   == 0 ||
                        vars[RGE_PHIPQ.addr] == 0
                )
        ) {
            continue;
        }

        // Prepare binning vars.
        Float_t bin_vars_idx[dim_bins];
        for (luint bin_dim_i = 0; bin_dim_i < dim_bins; ++bin_dim_i) {
            bin_vars_idx[bin_dim_i] = vars[bin_vars[bin_dim_i]];
        }

        // Fill plots.
        for (luint plot_i = 0; plot_i < plot_arr_size; ++plot_i) {
            // SIDIS variables only make sense for some particles.
            bool sidis_pass = true;
            for (int dim_i = 0; dim_i < plot_type[plot_i]+1; ++dim_i) {
                const char **plot_var = &RGE_VARS[plot_vars[plot_i][dim_i]];
                for (int list_i = 0; list_i < DIS_LIST_SIZE; ++list_i) {
                    if (
                            !strcmp(*plot_var, DIS_LIST[list_i]) &&
                            vars[plot_vars[plot_i][dim_i]] < 1e-9
                    ) {
                        sidis_pass = false;
                    }
                }
            }
            if (!sidis_pass) continue;

            // Find corresponding bin.
            lint idx = find_idx(
                    dim_bins, 0, bin_vars_idx, bin_nbins, bin_range, bin_binsize
            );
            if (idx == -1) continue;

            // Fill histogram.
            if (plot_type[plot_i] == 0) {
                plot_arr[plot_i][idx]->Fill(vars[plot_vars[plot_i][0]]);
            }
            if (plot_type[plot_i] == 1) {
                plot_arr[plot_i][idx]->Fill(
                        vars[plot_vars[plot_i][0]], vars[plot_vars[plot_i][1]]
                );
            }
        }
    }

    // === APPLY ACCEPTANCE CORRECTION =========================================
    // Array for storing number of bins (for simplicity).
    luint bn[5] = {
            acc_nedges[0]-1, acc_nedges[1]-1, acc_nedges[2]-1,
            acc_nedges[3]-1, acc_nedges[4]-1
    };

    // Interate through plot variables and bins.
    for (
            luint plot_i = 0;
            plot_i < plot_arr_size && acc_plot && apply_acc_corr;
            ++plot_i
    ) {
        for (luint bin_i = 0; bin_i < bin_arr_size; ++bin_i) {
            // Integrate through other variables.
            int y_thrown[bn[plot_i]];
            int y_simul [bn[plot_i]];
            for (
                    luint acc_bin_i = 0;
                    acc_bin_i < bn[plot_i];
                    ++acc_bin_i
            ) {
                y_thrown[acc_bin_i] = 0;
                y_simul [acc_bin_i] = 0;
            }

            // Go through each of the five acceptance correction bins.
            for (luint i0 = 0; i0 < acc_nedges[0]-1; ++i0) {
                for (luint i1 = 0; i1 < acc_nedges[1]-1; ++i1) {
                    for (luint i2 = 0; i2 < acc_nedges[2]-1; ++i2) {
                        for (luint i3 = 0; i3 < acc_nedges[3]-1; ++i3) {
                            for (luint i4 = 0; i4 < acc_nedges[4]-1; ++i4) {
                                // Find 1D bin position from 5 indices.
                                luint bin_pos =
                                        i0 * (bn[1]*bn[2]*bn[3]*bn[4]) +
                                        i1 * (bn[2]*bn[3]*bn[4]) +
                                        i2 * (bn[3]*bn[4]) +
                                        i3 * (bn[4]) +
                                        i4;

                                // Find which ID should be updated.
                                // This assumes the order of variables of ACC_VX
                                // to be Q2, nu, zh, pt2, phiPQ. If that
                                // changes, this should change as well.
                                luint sel_idx;
                                switch(plot_i) {
                                    case 0: sel_idx = i0; break;
                                    case 1: sel_idx = i1; break;
                                    case 2: sel_idx = i2; break;
                                    case 3: sel_idx = i3; break;
                                    case 4: sel_idx = i4; break;
                                    default: {
                                        rge_errno = RGEERR_WRONGACCVARS;
                                        return 1;
                                    }
                                }

                                // Increment appropriate counters.
                                y_thrown[sel_idx] +=
                                        acc_n_thrown[acc_pid_idx][bin_pos];
                                y_simul[sel_idx] +=
                                        acc_n_simul[acc_pid_idx][bin_pos];
                            }
                        }
                    }
                }
            }

            // Compute acceptance correction factor.
            double acc_corr_factor[bn[plot_i]];
            for (luint acc_bin_i = 0; acc_bin_i < bn[plot_i]; ++acc_bin_i) {
                acc_corr_factor[acc_bin_i] =
                        static_cast<double>(y_thrown[acc_bin_i]) /
                        static_cast<double>(y_simul[acc_bin_i]);
            }

            // Multiply each plot bin by its corresponding correction factor.
            for (luint plt_bin_i = 1; plt_bin_i <= bn[plot_i]; ++plt_bin_i) {
                double bin = plot_arr[plot_i][bin_i]->GetBinContent(plt_bin_i);
                plot_arr[plot_i][bin_i]->SetBinContent(
                        plt_bin_i, bin * acc_corr_factor[plt_bin_i-1]
                );
            }
        }
    }

    // === WRITE TO OUTPUT FILE ================================================
    // Create output file.
    TFile *f_out = TFile::Open(out_filename, "RECREATE");
    if (!f_out || f_out->IsZombie()) {
        rge_errno = RGEERR_OUTPUTROOTFAILED;
        return 1;
    }

    // Write plots to output file.
    for (luint bin_i = 0; bin_i < bin_arr_size; ++bin_i) {
        // Find dir.
        TString dir;
        find_bin(&dir, dim_bins, bin_i, 0, INT_MAX, bin_arr_size, bin_vars,
                bin_nbins, bin_range, bin_binsize);

        f_out->mkdir(dir);
        f_out->cd(dir);

        // Write plot(s).
        for (luint plot_i = 0; plot_i < plot_arr_size; ++plot_i) {
            plot_arr[plot_i][bin_i]->Write();
        }
    }

    printf("Done! Check out plots at %s.\n\n", out_filename);

    // === CLEAN-UP ============================================================
    f_in ->Close();
    f_out->Close();

    free(valid_event);

    if (acc_plot) {
        for (luint bin_i = 0; bin_i < 5; ++bin_i) {
            free(acc_edges[bin_i]);
        }
        free(acc_edges);
        free(acc_pids);
        for (luint plot_i = 0; plot_i < acc_npids; ++plot_i) {
            free(acc_n_thrown[plot_i]);
            free(acc_n_simul[plot_i]);
        }
        free(acc_n_thrown);
        free(acc_n_simul);
    }

    rge_errno = RGEERR_NOERR;
    return 0;
}

/**
 * Handle arguments for make_ntuples using optarg. Error codes used are
 *     explained in the handle_err() function.
 */
static int handle_args(
        int argc, char **argv, lint *sel_pid, bool *apply_all_cuts,
        lint *nentries, char **out_filename, char **acc_filename,
        bool *apply_acc_corr, char **work_dir, char **in_filename, int *run_no
) {
    // Handle arguments.
    int opt;
    char *tmp_out_filename = NULL;
    while ((opt = getopt(argc, argv, "-hp:cn:o:a:Aw:")) != -1) {
        switch (opt) {
            case 'h':
                rge_errno = RGEERR_USAGE;
                return 1;
            case 'p':
                if (rge_process_pid(sel_pid, optarg)) return 1;
                break;
            case 'c':
                *apply_all_cuts = true;
                break;
            case 'n':
                if (rge_process_nentries(nentries, optarg)) return 1;
                break;
            case 'o':
                tmp_out_filename =
                        static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(tmp_out_filename, optarg);
                break;
            case 'a':
                *acc_filename = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*acc_filename, optarg);
                break;
            case 'A':
                *apply_acc_corr = false;
                break;
            case 'w':
                *work_dir = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*work_dir, optarg);
                break;
            case 1:
                *in_filename = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*in_filename, optarg);
                break;
            default:
                rge_errno = RGEERR_BADOPTARGS;
                return 1;
        }
    }

    // Define workdir if undefined.
    if (*work_dir == NULL) {
        *work_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // -A is only valid if -a is also specified.
    if (*apply_acc_corr == false && *acc_filename == NULL) {
        rge_errno = RGEERR_INVALIDACCEPTANCEOPT;
        return 1;
    }

    // Check positional argument.
    if (*in_filename == NULL) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }

    // Check input filename validity and write run number to run_no.
    if (rge_handle_root_filename(*in_filename, run_no)) return 1;

    // Define tmp output filename if undefined.
    if (tmp_out_filename == NULL) {
        tmp_out_filename = static_cast<char *>(malloc(PATH_MAX));
        sprintf(tmp_out_filename, "plots_%06d.root", *run_no);
    }

    // Define final output filename, including work_dir.
    *out_filename = static_cast<char *>(malloc(PATH_MAX));
    sprintf(*out_filename, "%s/%s", *work_dir, tmp_out_filename);
    free(tmp_out_filename);

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    lint sel_pid        = 0;
    bool apply_all_cuts = false;
    lint nentries       = -1;
    char *out_filename  = NULL;
    char *acc_filename  = NULL;
    bool apply_acc_corr = true;
    char *work_dir      = NULL;
    char *in_filename   = NULL;
    int  run_no         = -1;

    int err = handle_args(
            argc, argv, &sel_pid, &apply_all_cuts, &nentries, &out_filename,
            &acc_filename, &apply_acc_corr, &work_dir, &in_filename, &run_no
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED && err == 0) {
        run(
                in_filename, out_filename, acc_filename, work_dir, run_no,
                nentries, sel_pid, apply_all_cuts, apply_acc_corr
        );
    }

    // Free up memory.
    if (in_filename  != NULL) free(in_filename);
    if (out_filename != NULL) free(out_filename);
    if (acc_filename != NULL) free(acc_filename);
    if (work_dir     != NULL) free(work_dir);

    // Return errcode.
    return rge_print_usage(USAGE_MESSAGE);
}
