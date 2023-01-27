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
 * Assign name to plots, recursively going through binnings.
 *
 * @param plt:        List of plots to be named.
 * @param name:       Base for the name given to the plot. The final name will
 *                    be "<name> (<bin 1>) (<bin 2>) ... (<bin n>)".
 * @param nx:         Variable name for the x axis..
 * @param ny:         Variable name for the y axis.
 * @param idx:        Index in the plot array of the plot to be named. When
 *                    calling the function, should be 0.
 * @param dbins:      Binning dimension. Needed to compute how deep the
 *                    recursion should go before stopping.
 * @param depth:      How deep along the number of bins we are. When calling the
 *                    function, this should always be 0.
 * @param px:         Dimensionality of plot. Set to 1 for 1d plot, 2 for 2d.
 * @param bx:         2-dimensional array containing the number of bins for each
 *                    axis for each plot.
 * @param rx:         2-dimensional array containing the range for each axis for
 *                    each plot.
 * @param bvx:        Array of variables for binning for each plot.
 * @param bbx:        Array with number of dimensions for binning for each
 *                    binning.
 * @param brx:        2-dimensional array with lower and upper limits for each
 *                    binning variable.
 * @param b_interval: Array with size of each bin for each binning.
 * @return:           Error code. Currently, can only return 0 (success).
 */
int name_plt(TH1 *plt[], TString *name, const char *nx, const char *ny,
        int *idx, long dbins, long depth, int px, long bx[], double rx[][2],
        int bvx[], long bbx[], double brx[][2], double b_interval[])
{
    if (depth == dbins) {
        // Create plot and increase index.
        if (px == 0) plt[*idx] =
                new TH1F(*name, Form("%s;%s", name->Data(), nx), bx[0],
                         rx[0][0], rx[0][1]);
        if (px == 1) plt[*idx] =
                new TH2F(*name, Form("%s;%s;%s", name->Data(), nx, ny), bx[0],
                         rx[0][0], rx[0][1], bx[1], rx[1][0], rx[1][1]);
        ++(*idx);
        return 0;
    }

    for (int bbi = 0; bbi < bbx[depth]; ++bbi) {
        // Find limits.
        double b_low  = brx[depth][0] + b_interval[depth]* bbi;
        double b_high = brx[depth][0] + b_interval[depth]*(bbi+1);

        // Append bin limits to name.
        TString name_cpy = name->Copy();
        name_cpy.Append(Form(" (%s: %6.2f, %6.2f)", S_VAR_LIST[bvx[depth]],
                             b_low, b_high));

        // Continue down the line.
        name_plt(plt, &name_cpy, nx, ny, idx, dbins, depth+1, px, bx, rx, bvx,
                 bbx, brx, b_interval);
    }

    return 0;
}

/**
 * Find name of bin by recursively going through binnings and appending their
 *     range to the name.
 *
 * @param name:            Name to which we append each bin.
 * @param plt_size:        Number of "versions" of a plot, depending on number
 *                         of bins.
 * @param idx:             Index in the plot array of the plot to be named. When
 *                         calling the function, should be 0.
 * @param dbins:           Binning dimension. Needed to compute how deep the
 *                         recursion should go before stopping.
 * @param depth:           How deep along the number of bins we are. When
 *                         calling the function, this should always be 0.
 * @param prev_dim_factor: Dimension factor
 * @param vx:              Array of variables for binning for each plot.
 * @param bx:              Array with number of dimensions for binning for each
 *                         binning.
 * @param rx:              2-dimensional array with lower and upper limits for
 *                         each binning variable.
 * @param interval:        Array with size of each bin for each binning.
 * @return:                Success code (0).
 */
int find_bin(TString *name, int plt_size, int idx, long dbins, long depth,
        int prev_dim_factor, int vx[], long bx[], double rx[][2],
        double interval[])
{
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

    return find_bin(name, plt_size, idx, dbins, depth+1, dim_factor, vx, bx, rx,
            interval);
}

/**
 * Find index of plot in array, recursively going through binnings.
 *
 * @param dbins:    Binning dimension. Needed to compute how deep the recursion
 *                  should go before stopping.
 * @param depth:    How deep along the number of bins we are. When calling the
 *                  function, this should always be 0.
 * @param var:      Binning variables.
 * @param bx:       Array with number of dimensions for binning for each
 *                  binning.
 * @param rx:       2-dimensional array with lower and upper limits for each
 *                  binning variable.
 * @param interval: Array with size of each bin for each binning.
 */
int find_idx(long dbins, long depth, Float_t var[], long bx[], double rx[][2],
        double interval[])
{
    if (depth == dbins) return 0;
    for (int bi = 0; bi < bx[depth]; ++bi) {
        // Define bin limits.
        double low  = rx[depth][0] + interval[depth]* bi;
        double high = rx[depth][0] + interval[depth]*(bi+1);

        // Find bin for var.
        if (low < var[depth] && var[depth] < high) {
            int dim_factor = 1;
            for (int di = depth+1; di < dbins; ++di) dim_factor *= bx[di];
            return bi*dim_factor +
                    find_idx(dbins, depth+1, var, bx, rx, interval);
        }
    }

    return -1; // Variable is not within binning range.
}

/** run() function of the program. Check usage() for details. */
int run(char *in_file, char *acc_file, char *work_dir, int run_no,
        int entry_max)
{
    // Open input file.
    TFile *f_in  = TFile::Open(in_file, "READ");
    if (!f_in || f_in->IsZombie()) return 8;

    // Get acceptance correction
    long int b_sizes[5];
    long int nbins;
    double **binnings;
    long int pids_size;
    long int *pids;
    double **acc_corr;

    if (acc_file != NULL) {
        if (access(acc_file, F_OK) != 0) return 9;
        FILE *ac_file = fopen(acc_file, "r");

        binnings = (double **) malloc(5 * sizeof(*binnings));
        get_binnings(ac_file, b_sizes, binnings, &pids_size);

        nbins = 1;
        for (int bi = 0; bi < 5; ++bi) nbins *= b_sizes[bi] - 1;

        for (int bi = 0; bi < 5; ++bi) {
            printf("binning[%ld]: [", b_sizes[bi]);
            for (int bii = 0; bii < b_sizes[bi]; ++bii)
                printf("%lf, ", binnings[bi][bii]);
            printf("\b\b]\n");
        }

        pids = (long int *) malloc(pids_size * sizeof(*pids));
        acc_corr = (double **) malloc(pids_size * sizeof(*acc_corr));

        get_acc_corr(ac_file, pids_size, nbins, pids, acc_corr);

        printf("pids[%ld] = [", pids_size);
        for (int pi = 0; pi < pids_size; ++pi) {
            printf("%ld ", pids[pi]);
        }
        printf("\b\b]\n");

        for (int pi = 0; pi < pids_size; ++pi) {
            printf("acc_corr[%ld]: [", nbins);
            for (int bii = 0; bii < nbins; ++bii)
                printf("%lf ", acc_corr[pi][bii]);
            printf("\b\b]\n");
        }

        fclose(ac_file);
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

    // TODO.
    // printf("\nApply any custom cut? [y/n]\n");
    // bool custom_cuts = catch_yn();

    // === BINNING SETUP =======================================================
    // TODO. If acc_file != NULL, limit binning in Q2, nu, z_h, Pt2, and phi_PQ
    //       to acceptance correction bins.
    printf("\nNumber of dimensions for binning?\n");
    long   dbins = catch_long();
    int    bvx[dbins];
    double brx[dbins][2];
    double b_interval[dbins];
    long   bbx[dbins];
    for (long bdi = 0; bdi < dbins; ++bdi) {
        // variable.
        printf("\nDefine var for bin in dimension %ld. Available vars:\n[", bdi);
        for (int vi = 0; vi < VAR_LIST_SIZE; ++vi)
            printf("%s, ", R_VAR_LIST[vi]);
        printf("\b\b]\n");
        bvx[bdi] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

        // range.
        for (int ri = 0; ri < 2; ++ri) {
            printf("\nDefine %s limit for bin in dimension %ld:\n",
                    RAN_LIST[ri], bdi);
            brx[bdi][ri] = catch_double();
        }

        // nbins.
        printf("\nDefine number of bins for bin in dimension %ld:\n", bdi);
        bbx[bdi] = catch_long();

        // binning interval.
        b_interval[bdi] = (brx[bdi][1] - brx[bdi][0])/bbx[bdi];
    }

    // === PLOT SETUP ==========================================================
    // Number of plots.
    printf("\nDefine number of plots (Set to 0 to draw standard plots).\n");
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
        for (int vi = 0; vi < PLOT_LIST_SIZE; ++vi)
            printf("%s, ", PLOT_LIST[vi]);
        printf("\b\b]:\n");
        px[pi] = catch_string(PLOT_LIST, PLOT_LIST_SIZE);

        for (int di = 0; di < px[pi]+1; ++di) {
            // Check variable(s) to be plotted.
            printf("\nDefine var to be plotted on the %s axis. Available "
                   "vars:\n[", DIM_LIST[di]);
            for (int vi = 0; vi < VAR_LIST_SIZE; ++vi)
                printf("%s, ", R_VAR_LIST[vi]);
            printf("\b\b]\n");
            vx[pi][di] = catch_string(R_VAR_LIST, VAR_LIST_SIZE);

            // Define ranges.
            for (int ri = 0; ri < 2; ++ri) {
                printf("\nDefine %s limit for %s axis:\n",
                        RAN_LIST[ri], DIM_LIST[di]);
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
    long plt_size = 1;
    for (int bdi = 0; bdi < dbins; ++bdi) plt_size *= bbx[bdi];

    TH1 *plt[pn][plt_size];
    for (int pi = 0; pi < pn; ++pi) {
        TString name;
        int idx = 0;
        if (px[pi] == 0) {
            name = Form("%s", S_VAR_LIST[vx[pi][0]]);
            name_plt(plt[pi], &name, S_VAR_LIST[vx[pi][0]], "", &idx, dbins, 0,
                     px[pi], bx[pi], rx[pi], bvx, bbx, brx, b_interval);
        }
        if (px[pi] == 1) {
            name = Form("%s vs %s", S_VAR_LIST[vx[pi][0]],
                    S_VAR_LIST[vx[pi][1]]);
            name_plt(plt[pi], &name, S_VAR_LIST[vx[pi][0]],
                    S_VAR_LIST[vx[pi][1]], &idx, dbins, 0, px[pi], bx[pi],
                    rx[pi], bvx, bbx, brx, b_interval);
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
        Float_t b_vars[dbins];
        for (long bdi = 0; bdi < dbins; ++bdi) b_vars[bdi] = vars[bvx[bdi]];

        // Fills plots.
        for (int pi = 0; pi < pn; ++pi) {
            // SIDIS variables only make sense for some particles.
            bool sidis_pass = true;
            for (int di = 0; di < px[pi]+1; ++di) {
                for (int li = 0; li < DIS_LIST_SIZE; ++li) {
                    if (!strcmp(R_VAR_LIST[vx[pi][di]], DIS_LIST[li]) &&
                            vars[vx[pi][di]] < 1e-9)
                        sidis_pass = false;
                }
            }
            if (!sidis_pass) continue;

            // Find corresponding bin.
            int idx = find_idx(dbins, 0, b_vars, bbx, brx, b_interval);
            if (idx == -1) continue;

            // Fill histogram.
            if (px[pi] == 0)
                plt[pi][idx]->Fill(vars[vx[pi][0]]);
            if (px[pi] == 1)
                plt[pi][idx]->Fill(vars[vx[pi][0]], vars[vx[pi][1]]);
        }
    }

    // === APPLY ACCEPTANCE CORRECTION =========================================
    // TODO...

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
    for (int plti = 0; plti < plt_size; ++plti) {
        // Find dir.
        TString dir;
        find_bin(&dir, plt_size, plti, dbins, 0, INT_MAX, bvx, bbx, brx,
                 b_interval);
        f_out->mkdir(dir);
        f_out->cd(dir);

        // Write plot(s).
        for (int pi = 0; pi < pn; ++pi) plt[pi][plti]->Write();
    }

    printf("Done! Check out plots at %s.\n\n", out_file);

    // === CLEAN-UP ============================================================
    f_in ->Close();
    f_out->Close();

    free(valid_event);

    if (acc_file != NULL) {
        for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
        free(binnings);
        free(pids);
        for (int pi = 0; pi < pids_size; ++pi) free(acc_corr[pi]);
        free(acc_corr);
    }

    return 0;
}

/** Print usage and exit. */
int usage() {
    fprintf(stderr,
            "\n\nUsage: draw_plots [-hn:a:w:] infile\n"
            " * -h          : show this message and exit.\n"
            " * -n nentries : number of entries to process.\n"
            " * -a accfile  : apply acceptance correction using acc_file.\n"
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
int handle_args(int argc, char **argv, char **in_file, char **acc_file,
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
                *acc_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*acc_file, optarg);
                break;
            case 'w':
                *work_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*work_dir, optarg);
            case 1:
                *in_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*in_file, optarg);
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
    if (*in_file == NULL) return 3;

    int check = handle_root_filename(*in_file, run_no);
    if (!check || check == 5) return 0;
    else                      return check + 3; // Shift errcode.
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_file  = NULL;
    char *acc_file = NULL;
    char *work_dir = NULL;
    int run_no     = -1;
    int entry_max  = -1;

    int errcode = handle_args(argc, argv, &in_file, &acc_file, &work_dir,
            &run_no, &entry_max);

    // Run.
    if (errcode == 0)
        errcode = run(in_file, acc_file, work_dir, run_no, entry_max);

    // Free up memory.
    if (in_file  != NULL) free(in_file);
    if (acc_file != NULL) free(acc_file);
    if (work_dir != NULL) free(work_dir);

    // Return errcode.
    return handle_err(errcode);
}
