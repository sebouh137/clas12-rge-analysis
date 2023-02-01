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

#include <libgen.h>
#include <limits.h>
#include <TFile.h>
#include <TNtuple.h>
#include "../lib/io_handler.h"
#include "../lib/utilities.h"

/**
 * Return position of value v inside a binning array b of size s. If v is not
 *     inside b, return -1.
 */
int find_pos(double v, double *b, int size) {
    for (int i = 0; i < size; ++i) if (b[i] < v && v < b[i+1]) return i;
    return -1;
}

/**
 * Count number of events in a tree for each bin, for a given pid. The number of
 *     bins is equal to the multiplication of the size-1 of each binning.
 *
 * @param evn_cnt:  array of integers that count events in each bin.
 * @param tree:     TTree containing the data we're to process.
 * @param pid:      pid of the particle for which we're counting events.
 * @param nbins:    number of bins, or size of the evn_cnt array.
 * @param bsizes:   size of each binning, should be an array of size 5.
 * @param binnings: 2-dimensional array of binnings.
 * @param in_deg:   boolean telling us if thrown events are in degrees --
 *                  default is radians.
 * @return:         success code (0).
 */
int count_events(int *evn_cnt, TTree *tree, int pid, int nbins, int *bsizes,
        double **binnings, bool in_deg)
{
    for (int i = 0; i < nbins; ++i) evn_cnt[i] = 0;

    Float_t s_pid;
    Float_t s_binning[5] = {0, 0, 0, 0, 0};
    tree->SetBranchAddress(S_PID,   &s_pid);
    tree->SetBranchAddress(S_Q2,    &(s_binning[0]));
    tree->SetBranchAddress(S_NU,    &(s_binning[1]));
    tree->SetBranchAddress(S_ZH,    &(s_binning[2]));
    tree->SetBranchAddress(S_PT2,   &(s_binning[3]));
    tree->SetBranchAddress(S_PHIPQ, &(s_binning[4]));
    for (int evn = 0; evn < tree->GetEntries(); ++evn) {
        tree->GetEntry(evn);
        if (pid-0.5 < s_pid && s_pid < pid+0.5) continue;

        // Find position of event.
        int idx[5];
        bool kill = false;
        for (int bi = 0; bi < 5 && !kill; ++bi) {
            if (bi == 4 && in_deg)
                idx[bi] = find_pos(to_rad(s_binning[bi]), binnings[bi],
                        bsizes[bi]-1);
            else
                idx[bi] = find_pos(s_binning[bi], binnings[bi], bsizes[bi]-1);

            if (idx[bi] < 0) kill = true;
        }
        if (kill) continue;

        // Increase counter.
        ++evn_cnt[
                idx[0]*(bsizes[1]-1)*(bsizes[2]-1)*(bsizes[3]-1)*(bsizes[4]-1) +
                idx[1]*(bsizes[2]-1)*(bsizes[3]-1)*(bsizes[4]-1) +
                idx[2]*(bsizes[3]-1)*(bsizes[4]-1) +
                idx[3]*(bsizes[4]-1) +
                idx[4]
        ];
    }

    return 0;
}

/** run() function of the program. Check usage() for details. */
int run(char *gen_file, char *sim_file, char *data_dir, int *bsizes,
        double **binnings, bool use_fmt, bool in_deg)
{
    // Open input files and load TTrees.
    printf("\nOpening generated events file...\n");
    TFile *t_in = TFile::Open(gen_file, "READ");
    if (!t_in || t_in->IsZombie()) return 10;
    TNtuple *thrown = t_in->Get<TNtuple>("ntuple_thrown");
    if (thrown == NULL) return 11;

    printf("Opening simulated events file...\n");
    TFile *s_in = TFile::Open(sim_file, "READ");
    if (!s_in || s_in->IsZombie()) return 12;
    TTree *simul = use_fmt ? s_in->Get<TTree>("fmt") : s_in->Get<TTree>("dc");
    if (simul == NULL) return 13;

    // Create output file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/acc_corr.txt", data_dir);
    if (!access(out_file, F_OK)) return 14;
    FILE *t_out = fopen(out_file, "w");

    // Write binning bsizes to output file.
    for (int bi = 0; bi < 5; ++bi) fprintf(t_out, "%d ", bsizes[bi]);
    fprintf(t_out, "\n");

    // Write binnings to output file.
    for (int bi = 0; bi < 5; ++bi) {
        for (int bii = 0; bii < bsizes[bi]; ++bii) {
            fprintf(t_out, "%12.9f ", binnings[bi][bii]);
        }
        fprintf(t_out, "\n");
    }

    // Get list of PIDs.
    // NOTE. We assume that we'll deal with at most 256 PIDs.
    printf("Getting list of PIDs from generated file...\n");
    Float_t s_pid;
    double pidlist[256];
    int pidlist_size = 0;
    thrown->SetBranchAddress(S_PID, &s_pid);

    for (int evn = 0; evn < thrown->GetEntries(); ++evn) {
        thrown->GetEntry(evn);
        bool found = false;
        for (int pi = 0; pi < pidlist_size; ++pi) {
            if (pidlist[pi] - 0.5 <= s_pid && s_pid <= pidlist[pi] + 0.5) {
                found = true;
            }
        }
        if (found) continue;
        pidlist[pidlist_size] = s_pid;
        ++pidlist_size;
    }

    // Write list of PIDs to output file.
    fprintf(t_out, "%d\n", pidlist_size);
    for (int pi = 0; pi < pidlist_size; ++pi)
        fprintf(t_out, "%d ", (int) pidlist[pi]);
    fprintf(t_out, "\n");

    // Get number of bins.
    int nbins = 1;
    for (int bi = 0; bi < 5; ++bi) nbins *= bsizes[bi] - 1;

    // Count and write number of thrown and simulated events in each bin.
    for (int pi = 0; pi < pidlist_size; ++pi) {
        int pid = (int) pidlist[pi];
        printf("Working on PID %5d...\n", pid);
        fflush(stdout);

        int t_evn[nbins];
        printf("  Counting thrown events...\n");
        count_events(t_evn, thrown, pid, nbins, bsizes, binnings, in_deg);
        for (int i = 0; i < nbins; ++i) fprintf(t_out, "%d ", t_evn[i]);
        fprintf(t_out, "\n");

        int s_evn[nbins];
        printf("  Counting simulated events...\n");
        count_events(s_evn, simul,  pid, nbins, bsizes, binnings, false);
        for (int i = 0; i < nbins; ++i) fprintf(t_out, "%d ", s_evn[i]);
        fprintf(t_out, "\n");

        printf("  Done!\n");
    }

    // Clean up after ourselves.
    t_in->Close();
    s_in->Close();
    fclose(t_out);
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);

    return 0;
}

/** Print usage and exit. */
int usage() {
    fprintf(stderr,
            "\n\nUsage: acc_corr [-hq:n:z:p:f:g:s:d:FD]\n"
            " * -h         : show this message and exit.\n"
            " * -q ...     : Q2 bins.\n"
            " * -n ...     : nu bins.\n"
            " * -z ...     : z_h bins.\n"
            " * -p ...     : Pt2 bins.\n"
            " * -f ...     : phi_PQ bins (in degrees).\n"
            " * -g genfile : generated events ROOT file.\n"
            " * -s simfile : simulated events ROOT file.\n"
            " * -d datadir : location where sampling fraction files are "
            "located. Default is\n                data.\n"
            " * -F         : flag to tell program to use FMT data instead of DC"
            " data from\n                the simulation file.\n"
            " * -D         : flag to tell program that generated events are in "
            "degrees\n                instead of radians.\n"
            "    Get the 5-dimensional acceptance correction factors for Q2, nu"
            ", z_h, Pt2, and\n    phi_PQ. For each optional argument, an array "
            "of doubles is expected. The first\n    double will be the lower "
            "limit of the leftmost bin, the final double will be\n    the upper"
            " limit of the rightmost bin, and all doubles inbetween will be the"
            "\n    separators between each bin.\n\n"
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
            fprintf(stderr, "All binnings should be specified.");
            break;
        case 3:
            fprintf(stderr, "All binnings should have *at least* two values -- "
                            "a minimum and a maximum.");
            break;
        case 4:
            fprintf(stderr, "Generated file must be specified.");
            break;
        case 5:
            fprintf(stderr, "Generated file should be in root format.");
            break;
        case 6:
            fprintf(stderr, "Generated file does not exist.");
            break;
        case 7:
            fprintf(stderr, "Simulated file must be specified.");
            break;
        case 8:
            fprintf(stderr, "Simulated file should be in root format.");
            break;
        case 9:
            fprintf(stderr, "Simulated file does not exist.");
            break;
        case 10:
            fprintf(stderr, "Generated file is not a valid root file.");
            break;
        case 11:
            fprintf(stderr, "Generated file is badly formatted.");
            break;
        case 12:
            fprintf(stderr, "Simulated file is not a valid root file.");
            break;
        case 13:
            fprintf(stderr, "Simualted file is badly formatted.");
            break;
        case 14:
            fprintf(stderr, "Output file already exists.");
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
int handle_args(int argc, char **argv, char **gen_file, char **sim_file,
        char **data_dir, int *bsizes, double **binnings, bool *use_fmt,
        bool *in_deg)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "hq:n:z:p:f:g:s:d:FD")) != -1) {
        switch (opt) {
        case 'h':
            return 1;
        case 'q':
            grab_multiarg(argc, argv, &optind, &(bsizes[0]), &(binnings[0]));
            break;
        case 'n':
            grab_multiarg(argc, argv, &optind, &(bsizes[1]), &(binnings[1]));
            break;
        case 'z':
            grab_multiarg(argc, argv, &optind, &(bsizes[2]), &(binnings[2]));
            break;
        case 'p':
            grab_multiarg(argc, argv, &optind, &(bsizes[3]), &(binnings[3]));
            break;
        case 'f':
            grab_multiarg(argc, argv, &optind, &(bsizes[4]), &(binnings[4]));
            break;
        case 'g':
            grab_str(optarg, gen_file);
            break;
        case 's':
            grab_str(optarg, sim_file);
            break;
        case 'd':
            *data_dir = (char *) malloc(strlen(optarg) + 1);
            strcpy(*data_dir, optarg);
            break;
        case 'F':
            *use_fmt = true;
            break;
        case 'D':
            *in_deg = true;
            break;
        default:
            break;
        }
    }

    // Check that all arrays have *at least* two values.
    for (int bi = 0; bi < 5; ++bi) if (bsizes[bi] == -1) return 2;
    for (int bi = 0; bi < 5; ++bi) if (bsizes[bi] < 2)   return 3;

    // Convert phi_PQ binning to radians.
    for (int bbi = 0; bbi < bsizes[4]; ++bbi)
        binnings[4][bbi] = to_rad(binnings[4][bbi]);

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = (char *) malloc(PATH_MAX);
        sprintf(*data_dir, "%s/../data", dirname(argv[0]));
    }

    // Check genfile.
    if (*gen_file == NULL) return 4;
    int errcode = check_root_filename(*gen_file);
    if (errcode) return errcode + 4; // Shift errcode.

    // Check simfile.
    if (*sim_file == NULL) return 7;
    errcode = check_root_filename(*sim_file);
    if (errcode) return errcode + 7; // Shift errcode.

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *gen_file = NULL;
    char *sim_file = NULL;
    char *data_dir = NULL;
    bool use_fmt   = false;
    bool in_deg    = false;
    int bsizes[5]  = {-1, -1, -1, -1, -1};
    double **binnings;

    binnings = (double **) malloc(5 * sizeof(*binnings));
    int errcode = handle_args(argc, argv, &gen_file, &sim_file, &data_dir,
            bsizes, binnings, &use_fmt, &in_deg);

    // Run.
    if (errcode == 0)
        errcode = run(gen_file, sim_file, data_dir, bsizes, binnings, use_fmt,
                in_deg);

    // Free up memory.
    if (gen_file != NULL) free(gen_file);
    if (sim_file != NULL) free(sim_file);
    if (data_dir != NULL) free(data_dir);

    // Return errcode.
    return handle_err(errcode);
}
