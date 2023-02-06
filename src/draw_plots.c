// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
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

#include <climits>
#include <libgen.h>
#include <TFile.h>
#include <TNtuple.h>
#include "../lib/io_handler.h"
#include "../lib/utilities.h"

// TODO. Do acceptance correction.
// TODO. See what happens to low-momentum particles inside CLAS12 through
//       simulation and see if they are reconstructed.
// TODO. Evaluate **acceptance** in diferent regions.
// TODO. Separate in vz bins. Start from -40 to 40 cm, 4-cm bins.

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
int create_plots(TH1 *plot_arr[], long dim_bins, int *idx, long depth,
        TString *plot_title, const char *x_var, const char *y_var,
        int plot_type, long plot_nbins[], double plot_range[][2],
        int bin_vars[], long bin_nbins[], double bin_range[][2],
        double bin_binsize[])
{
    if (depth == dim_bins) {
        // Create plot and increase index.
        if (plot_type == 0) {
            plot_arr[*idx] = new TH1F(*plot_title,
                    Form("%s;%s", plot_title->Data(), x_var),
                    plot_nbins[0], plot_range[0][0], plot_range[0][1]
            );

        }
        if (plot_type == 1) {
            plot_arr[*idx] = new TH2F(*plot_title,
                    Form("%s;%s;%s", plot_title->Data(), x_var, y_var),
                    plot_nbins[0], plot_range[0][0], plot_range[0][1],
                    plot_nbins[1], plot_range[1][0], plot_range[1][1]
            );
        }
        ++(*idx);
        return 0;
    }

    for (int bbi = 0; bbi < bin_nbins[depth]; ++bbi) {
        // Find limits.
        double b_low  = bin_range[depth][0] + bin_binsize[depth]* bbi;
        double b_high = bin_range[depth][0] + bin_binsize[depth]*(bbi+1);

        // Append bin limits to title.
        TString name_cpy = plot_title->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", S_VAR_LIST[bin_vars[depth]],
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
 * @param vars:        Array of variables for binning for each plot.
 * @param nbins:       Array with number of dimensions for binning for each
 *                         binning.
 * @param range:       2-dimensional array with lower and upper limits for
 *                         each binning variable.
 * @param binsize:     Array with size of each bin for each binning.
 *
 * @return:                Error code. Currently, can only return 0 (success).
 */
int find_bin(TString *plot_title,
        long dim_bins, int idx, long depth, int prev_dim_factor,
        int nplots, int vars[], long nbins[], double range[][2],
        double binsize[])
{
    if (depth == dim_bins) return 0;

    // Find index in array (for this dimension).
    int dim_factor = 1;
    for (int di = depth+1; di < dim_bins; ++di) dim_factor *= nbins[di];
    int bi = (idx%prev_dim_factor)/dim_factor;

    // Get limits.
    double low  = range[depth][0] + binsize[depth]* bi;
    double high = range[depth][0] + binsize[depth]*(bi+1);

    // Append dir to title.
    plot_title->Append(Form("%s (%6.2f, %6.2f)/", S_VAR_LIST[vars[depth]],
            low, high));

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
int find_idx(long dim_bins, long depth, Float_t var[], long nbins[],
        double range[][2], double binsize[])
{
    if (depth == dim_bins) return 0;
    for (int bi = 0; bi < nbins[depth]; ++bi) {
        // Define bin limits.
        double low  = range[depth][0] + binsize[depth]* bi;
        double high = range[depth][0] + binsize[depth]*(bi+1);

        // Find bin for var.
        if (low < var[depth] && var[depth] < high) {
            int dim_factor = 1;
            for (int di = depth+1; di < dim_bins; ++di) dim_factor *= nbins[di];
            return bi*dim_factor +
                    find_idx(dim_bins, depth+1, var, nbins, range, binsize);
        }
    }

    return -1; // Variable is not within binning range.
}

/** run() function of the program. Check usage() for details. */
int run(char *in_filename, char *acc_filename, char *work_dir, int run_no,
        int entry_max)
{
    // Open input file.
    TFile *f_in  = TFile::Open(in_filename, "READ");
    if (!f_in || f_in->IsZombie()) return 8;

    // Get acceptance correction
    bool accplt = false;
    long int acc_binsizes[5];
    long int acc_nbins;
    double **acc_binnings;
    long int acc_pids_size;
    long int *acc_pids;
    int **acc_n_thrown;
    int **acc_n_simul;
    if (acc_filename != NULL) {
        accplt = true;
        int errcode = read_acc_corr_file(acc_filename, acc_binsizes,
                &acc_binnings, &acc_pids_size, &acc_nbins, &acc_pids,
                &acc_n_thrown, &acc_n_simul);
        if (errcode != 0) return 9;
    }

    // NOTE. This function could receive a few arguments to speed IO up.
    //       Pre-configured cuts, binnings, and corrections would be nice.
    // TODO. Prepare corrections (acceptance, radiative, Feynman, etc...).

    // === CUT SETUP ===========================================================
    printf("\nUse DC or FMT data? [");
    for (int ti = 0; ti < TRK_LIST_SIZE; ++ti) printf("%s, ", TRK_LIST[ti]);
    printf("\b\b]\n");
    int trk = catch_string(TRK_LIST, TRK_LIST_SIZE);

    printf("\nWhat particle should be plotted? Available cuts:\n[");
    for (int pi = 0; pi < PART_LIST_SIZE; ++pi) printf("%s, ", PART_LIST[pi]);
    printf("\b\b]\n");
    int part = catch_string(PART_LIST, PART_LIST_SIZE);
    int p_charge = INT_MAX;
    int p_pid    = INT_MAX;
    if      (part == A_PPOS) p_charge =  1;
    else if (part == A_PNEU) p_charge =  0;
    else if (part == A_PNEG) p_charge = -1;
    else if (part == A_PPID) {
        printf("\nSelect PID from:\n");
        for (std::map<int, const char*>::const_iterator it = PID_NAME.begin();
                it != PID_NAME.end(); ++it)
        {
            printf("  * %5d (%s).\n", it->first, it->second);
        }
        p_pid = catch_long();
    }

    bool general_cuts  = false;
    bool geometry_cuts = false;
    bool dis_cuts      = false;
    printf("\nApply all default cuts (general, geometry, DIS)? [y/n]\n");
    if (!catch_yn()) {
        printf("\nApply general cuts? [y/n]\n");
        general_cuts = catch_yn();
        printf("\nApply geometry cuts? [y/n]\n");
        geometry_cuts = catch_yn();
        printf("\nApply DIS cuts? [y/n]\n");
        dis_cuts = catch_yn();
    }
    else {
        general_cuts  = true;
        geometry_cuts = true;
        dis_cuts      = true;
    }

    // TODO. Apply custom cuts.

    // === BINNING SETUP =======================================================

    // TODO. If accplt == true, limit binning in Q2, nu, z_h, Pt2, and
    //       phi_PQ to acceptance correction bins.

    printf("\nNumber of dimensions for binning?\n");
    long   dim_bins = catch_long();
    int    bin_vars[dim_bins];
    double bin_range[dim_bins][2];
    double bin_binsize[dim_bins];
    long   bin_nbins[dim_bins];
    for (long bdi = 0; bdi < dim_bins; ++bdi) {
        // variable.
        printf("\nDefine var for bin in dimension %ld. Available vars:\n[",
                bdi);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi)
            printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        bin_vars[bdi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

        // range.
        for (int ri = 0; ri < 2; ++ri) {
            printf("\nDefine %s limit for bin in dimension %ld:\n",
                    RAN_LIST[ri], bdi);
            bin_range[bdi][ri] = catch_double();
        }

        // nbins.
        printf("\nDefine number of bins for bin in dimension %ld:\n", bdi);
        bin_nbins[bdi] = catch_long();

        // binning bin size.
        bin_binsize[bdi] = (bin_range[bdi][1]-bin_range[bdi][0])/bin_nbins[bdi];
    }

    // === PLOT SETUP ==========================================================
    // Number of plots.
    long pn;
    if (accplt) {
        pn = ACCPLT_LIST_SIZE;
    }
    else {
        printf("\nDefine number of plots (Set to 0 to draw standard plots).\n");
        pn = catch_long();
    }

    bool stdplt = false;
    if (pn == 0) {
        stdplt = true;
        pn     = STDPLT_LIST_SIZE;
    }

    int    plot_type[pn];
    int    plot_vars[pn][2];
    double plot_range[pn][2][2];
    long   plot_nbins[pn][2];
    for (long pi = 0; pi < pn && !stdplt && !accplt; ++pi) {
        // Check if we are to make a 1D or 2D plot.
        printf("\nPlot %ld type? [", pi);
        for (int vi = 0; vi < PLOT_LIST_SIZE; ++vi)
            printf("%s, ", PLOT_LIST[vi]);
        printf("\b\b]:\n");
        plot_type[pi] = catch_string(PLOT_LIST, PLOT_LIST_SIZE);

        for (int di = 0; di < plot_type[pi]+1; ++di) {
            // Check variable(s) to be plotted.
            printf("\nDefine var to be plotted on the %s axis. Available "
                   "vars:\n[", DIM_LIST[di]);
            for (int vi = 0; vi < VAR_LIST_SIZE; ++vi)
                printf("%s, ", R_VAR_LIST[vi]);
            printf("\b\b]\n");
            plot_vars[pi][di] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

            // Define ranges.
            for (int ri = 0; ri < 2; ++ri) {
                printf("\nDefine %s limit for %s axis:\n",
                        RAN_LIST[ri], DIM_LIST[di]);
                plot_range[pi][di][ri] = catch_double();
            }

            // Define number of bins in plot.
            printf("\nDefine number of bins for %s axis:\n", DIM_LIST[di]);
            plot_nbins[pi][di] = catch_long();
        }
    }
    if (stdplt) { // Setup standard plots.
        memcpy(plot_type, STD_PX, sizeof plot_type);
        memcpy(plot_vars, STD_VX, sizeof plot_vars);
        memcpy(plot_range, STD_RX, sizeof plot_range);
        memcpy(plot_nbins, STD_BX, sizeof plot_nbins);
    }
    if (accplt) { // Setup acceptance corrected plots.
        memcpy(plot_type, ACC_PX, sizeof plot_type);
        memcpy(plot_vars, ACC_VX, sizeof plot_vars);
        memcpy(plot_range, ACC_RX, sizeof plot_range);
        memcpy(plot_nbins, ACC_BX, sizeof plot_nbins);
        // NOTE. plot_nbins is given by acc_nbins and acc_binnings!
    }

    // === NTUPLES SETUP =======================================================
    TNtuple *t = (TNtuple *) f_in->Get(trk == 0 ? S_DC : S_FMT);
    Float_t vars[VAR_LIST_SIZE];
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi)
        t->SetBranchAddress(S_VAR_LIST[vi], &vars[vi]);

    // === APPLY CUTS ==========================================================
    printf("\nOpening file...\n");
    // Counters for fancy progress bar.
    if (entry_max == -1) entry_max = t->GetEntries();
    int divcntr     = 0;
    int evnsplitter = 0;

    // Apply SIDIS cuts, checking which event numbers should be skipped.
    int nevents = -1;
    // Count number of events.
    for (int entry = 0; entry < entry_max; ++entry) {
        update_progress_bar(entry_max, entry, &evnsplitter, &divcntr);

        t->GetEntry(entry);
        if (vars[A_EVENTNO] > nevents) nevents = (int) (vars[A_EVENTNO]+0.5);
    }

    printf("Applying cuts...\n");
    divcntr     = 0;
    evnsplitter = 0;

    bool *valid_event = (bool *) malloc(nevents * sizeof(bool));
    Float_t current_evn = -1;
    bool no_tre_pass, Q2_pass, W2_pass, zh_pass;
    for (int entry = 0; entry < entry_max; ++entry) {
        update_progress_bar(entry_max, entry, &evnsplitter, &divcntr);

        t->GetEntry(entry);
        if (vars[A_EVENTNO] != current_evn) {
            current_evn = vars[A_EVENTNO];
            valid_event[(int) (vars[A_EVENTNO]+0.5)] = false;
            no_tre_pass = false;
            Q2_pass     = true;
            W2_pass     = true;
            zh_pass     = true;
        }

        if (vars[A_PID] != 11 || vars[A_STATUS] > 0) continue;
        no_tre_pass = true;
        Q2_pass = vars[A_Q2] >= Q2CUT;
        W2_pass = vars[A_W2] >= W2CUT;
        // zh_pass = vars[A_ZH] <= ZHCUT;

        valid_event[(int) (vars[A_EVENTNO]+0.5)] =
                no_tre_pass && Q2_pass && W2_pass && zh_pass;
    }

    // === PLOT ================================================================
    // Create plots, separated by n-dimensional binning.
    long nplots = 1;
    for (int bdi = 0; bdi < dim_bins; ++bdi) nplots *= bin_nbins[bdi];

    TH1 *plot_arr[pn][nplots];
    for (int pi = 0; pi < pn; ++pi) {
        TString plot_title;
        int idx = 0;
        if (plot_type[pi] == 0) { // 1D plot.
            plot_title = Form("%s", S_VAR_LIST[plot_vars[pi][0]]);
            create_plots(
                    plot_arr[pi], dim_bins, &idx, 0, &plot_title,
                    S_VAR_LIST[plot_vars[pi][0]], "",
                    plot_type[pi], plot_nbins[pi], plot_range[pi],
                    bin_vars, bin_nbins, bin_range, bin_binsize
            );
        }
        if (plot_type[pi] == 1) { // 2D plot.
            plot_title = Form("%s vs %s", S_VAR_LIST[plot_vars[pi][0]],
                    S_VAR_LIST[plot_vars[pi][1]]);
            create_plots(
                    plot_arr[pi], dim_bins, &idx, 0, &plot_title,
                    S_VAR_LIST[plot_vars[pi][0]], S_VAR_LIST[plot_vars[pi][1]],
                    plot_type[pi], plot_nbins[pi], plot_range[pi],
                    bin_vars, bin_nbins, bin_range, bin_binsize
            );
        }
    }

    divcntr     = 0;
    evnsplitter = 0;

    // Run through events.
    printf("Processing plots...\n");
    for (int entry = 0; entry < entry_max; ++entry) {
        update_progress_bar(entry_max, entry, &evnsplitter, &divcntr);
        t->GetEntry(entry);

        // Apply particle cuts.
        if (p_charge != INT_MAX) {
            if (p_charge ==  1 && !(vars[A_CHARGE] >  0)) continue;
            if (p_charge ==  0 && !(vars[A_CHARGE] == 0)) continue;
            if (p_charge == -1 && !(vars[A_CHARGE] <  0)) continue;
        }
        if (p_pid != INT_MAX && vars[A_PID] != p_pid) continue;

        // Apply geometry cuts.
        if (geometry_cuts) {
            if (calc_magnitude(vars[A_VX], vars[A_VY]) > VXVYCUT) continue;
            if (VZLOWCUT > vars[A_VZ] || vars[A_VZ] > VZHIGHCUT)  continue;
        }

        // Apply miscellaneous cuts.
        if (general_cuts) {
            // Non-identified particle.
            if (-0.5 < vars[A_PID] && vars[A_PID] <  0.5) continue;
            // Non-identified particle.
            if (44.5 < vars[A_PID] && vars[A_PID] < 45.5) continue;
            // Ignore tracks with high chi2.
            if (vars[A_CHI2]/vars[A_NDF] >= CHI2NDFCUT)   continue;
        }

        // Apply DIS cuts.
        if (dis_cuts && !valid_event[(int) (vars[A_EVENTNO]+0.5)]) continue;

        // Remove DIS vars = 0.
        if (vars[A_Q2] == 0 || vars[A_NU] == 0 || vars[A_ZH] == 0 ||
                vars[A_PT2] == 0 || vars[A_PHIPQ] == 0)
            continue;

        // Prepare binning vars.
        Float_t bin_vars_idx[dim_bins];
        for (long bdi = 0; bdi < dim_bins; ++bdi)
            bin_vars_idx[bdi] = vars[bin_vars[bdi]];

        // Fills plots.
        for (int pi = 0; pi < pn; ++pi) {
            // SIDIS variables only make sense for some particles.
            bool sidis_pass = true;
            for (int di = 0; di < plot_type[pi]+1; ++di) {
                for (int li = 0; li < DIS_LIST_SIZE; ++li) {
                    if (!strcmp(R_VAR_LIST[plot_vars[pi][di]], DIS_LIST[li]) &&
                            vars[plot_vars[pi][di]] < 1e-9)
                        sidis_pass = false;
                }
            }
            if (!sidis_pass) continue;

            // Find corresponding bin.
            int idx = find_idx(
                    dim_bins, 0, bin_vars_idx, bin_nbins, bin_range, bin_binsize
            );
            if (idx == -1) continue;

            // Fill histogram.
            if (plot_type[pi] == 0) {
                plot_arr[pi][idx]->Fill(vars[plot_vars[pi][0]]);
            }
            if (plot_type[pi] == 1) {
                plot_arr[pi][idx]->Fill(
                        vars[plot_vars[pi][0]], vars[plot_vars[pi][1]]
                );
            }
        }
    }

    // if (accplt) {
    //     printf("\n\n\n\n");
    //     for (int i = 0; i < acc_binsizes[0]; ++i) {
    //         printf("acc_binnings[0][%02d] = %5.2f\n", i, acc_binnings[0][i]);
    //     }
    //     printf("\n\n\n\n");
    // }

    // === APPLY ACCEPTANCE CORRECTION =========================================
    // TODO...
    // p_pid: PID of the selected particle.

    // TODO. Calculate acceptance correction in each bin from n_thrown and
    //       n_simul for each of the 5 variables.
    // TODO. Multiply each plot by its corresponding acceptance correction.

    // === WRITE TO OUTPUT FILE ================================================
    // Create output file.
    char out_file[PATH_MAX];
    if (p_pid == INT_MAX)
        sprintf(out_file, "%s/plots_%06d_%s_%s.root", work_dir, run_no,
                TRK_LIST[trk], PART_LIST[part]);
    else
        sprintf(out_file, "%s/plots_%06d_%s_pid%d.root", work_dir, run_no,
                TRK_LIST[trk], p_pid);

    TFile *f_out = TFile::Open(out_file, "RECREATE");

    // Write plots to output file.
    for (int plti = 0; plti < nplots; ++plti) {
        // Find dir.
        TString dir;
        find_bin(&dir, dim_bins, plti, 0, INT_MAX, nplots, bin_vars, bin_nbins,
                bin_range, bin_binsize);

        f_out->mkdir(dir);
        f_out->cd(dir);

        // Write plot(s).
        for (int pi = 0; pi < pn; ++pi) plot_arr[pi][plti]->Write();
    }

    printf("Done! Check out plots at %s.\n\n", out_file);

    // === CLEAN-UP ============================================================
    f_in ->Close();
    f_out->Close();

    free(valid_event);

    if (accplt) {
        for (int bi = 0; bi < 5; ++bi) free(acc_binnings[bi]);
        free(acc_binnings);
        free(acc_pids);
        for (int pi = 0; pi < acc_pids_size; ++pi) {
            free(acc_n_thrown[pi]);
            free(acc_n_simul[pi]);
        }
        free(acc_n_thrown);
        free(acc_n_simul);
    }

    return 0;
}

/** Print usage and exit. */
int usage() {
    fprintf(stderr,
            "\n\nUsage: draw_plots [-hn:a:w:] infile\n"
            " * -h          : show this message and exit.\n"
            " * -n nentries : number of entries to process.\n"
            " * -a accfile  : apply acceptance correction using acc_filename.\n"
            " * -w workdir  : location where output root files are to be "
            "stored. Default\n                 is root_io.\n"
            " * infile      : input file produced by make_ntuples.\n\n"
            "    Draw plots from a ROOT file built from make_ntuples. File "
            "should be named\n     <text>run_no.root.\n\n"
    );

    return 1;
}

/** Print error number and provide a short description of the error. */
int handle_err(int errcode) {
    if (errcode > 1) fprintf(stderr, "Error %02d. ", errcode);
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            fprintf(stderr, "Bad usage of optional arguments.");
            break;
        case 3:
            fprintf(stderr, "No input file provided.");
            break;
        case 4:
            fprintf(stderr, "Input file should be a root file.");
            break;
        case 5:
            fprintf(stderr, "Input file wasn't found.");
            break;
        case 6:
            fprintf(stderr, "Couldn't find extension in input filename.");
            break;
        case 7:
            fprintf(stderr, "Couldn't find run number from input filename.");
            break;
        case 8:
            fprintf(stderr, "Input file is not a valid root file.");
            break;
        case 9:
            fprintf(stderr, "Acceptance correction text file couldn't be "
                            "opened.");
            break;
        case 10:
            fprintf(stderr, "Number of entries should be greater than 0.");
            break;
        default:
            fprintf(stderr, "Error code not implemented!\n");
            return 1;
    }

    return usage();
}

/**
 * Handle arguments for make_ntuples using optarg. Error codes used are
 *     explained in the handle_err() function.
 */
int handle_args(int argc, char **argv, char **in_filename, char **acc_filename,
        char **work_dir, int *run_no, int *entry_max)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:a:w:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'n':
                *entry_max = atoi(optarg);
                if (*entry_max <= 0) return 10; // Check if entry_max is valid.
                break;
            case 'a':
                *acc_filename = (char *) malloc(strlen(optarg) + 1);
                strcpy(*acc_filename, optarg);
                break;
            case 'w':
                *work_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*work_dir, optarg);
            case 1:
                *in_filename = (char *) malloc(strlen(optarg) + 1);
                strcpy(*in_filename, optarg);
                break;
            default:
                return 2;
        }
    }

    // Define workdir if undefined.
    if (*work_dir == NULL) {
        *work_dir = (char *) malloc(PATH_MAX);
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Check positional argument.
    if (*in_filename == NULL) return 3;

    int check = handle_root_filename(*in_filename, run_no);
    if (!check || check == 5) return 0;
    else                      return check + 3; // Shift errcode.
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename  = NULL;
    char *acc_filename = NULL;
    char *work_dir = NULL;
    int run_no     = -1;
    int entry_max  = -1;

    int errcode = handle_args(argc, argv, &in_filename, &acc_filename,
            &work_dir, &run_no, &entry_max);

    // Run.
    if (errcode == 0)
        errcode = run(in_filename, acc_filename, work_dir, run_no, entry_max);

    // Free up memory.
    if (in_filename  != NULL) free(in_filename);
    if (acc_filename != NULL) free(acc_filename);
    if (work_dir != NULL) free(work_dir);

    // Return errcode.
    return handle_err(errcode);
}
