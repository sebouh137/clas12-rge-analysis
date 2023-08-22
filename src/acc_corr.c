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
#include <libgen.h>
#include <limits.h>

// ROOT.
#include <TFile.h>
#include <TNtuple.h>

// rge-analysis.
#include "../lib/rge_constants.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_math_utils.h"

static const char *USAGE_MESSAGE =
"Usage: acc_corr [-hq:n:z:p:f:g:s:d:FD]\n"
" * -h         : show this message and exit.\n"
" * -q ...     : Q2 bins.\n"
" * -n ...     : nu bins.\n"
" * -z ...     : z_h bins.\n"
" * -p ...     : Pt2 bins.\n"
" * -f ...     : phi_PQ bins (in degrees).\n"
" * -g genfile : generated events ROOT file.\n"
" * -s simfile : simulated events ROOT file.\n"
" * -d datadir : location where sampling fraction files are found. Default is\n"
"                data.\n"
" * -D         : flag to tell program that generated events are in degrees\n"
"                instead of radians.\n\n"
"    Get the 5-dimensional acceptance correction factors for Q2, nu, z_h,\n"
"    Pt2, and phi_PQ. For each optional argument, an array of doubles is\n"
"    expected. The first double will be the lower limit of the leftmost bin,\n"
"    the final double will be the upper limit of the rightmost bin, and all\n"
"    doubles between them will be the separators between each bin.\n";

/** Data tree name in generated (thrown) file. */
#define RGE_TREENAMETHRN "ntuple_thrown"
#define RGE_TREENAMETHRNELECTRONS "ntuple_thrown_electrons"

/**
 * List of PIDs that can appear in thrown events, but are not interesting for
 * SIDIS analysis.
 */
#define BADPIDS_SIZE 11
static int BADPIDS[BADPIDS_SIZE] = {
        2112, -2112, 2212, -2212, -12, 12, -13, 13, 321, -321, 130
};

/**
 * Thrown file variable names, in case they are not the same as the ones in
 * constants.
 */
#define THROWN_Q2    "Q2"
#define THROWN_NU    "#nu"
#define THROWN_ZH    "z_{h}"
#define THROWN_PT2   "Pt2"
#define THROWN_PHIPQ "#phi_{PQ}"
#define THROWN_W     "W"
#define THROWN_YB    "y"

/** Available entry count types. */
#define THROWN_ELECTRON -2
#define THROWN_HADRON   -1
#define SIMUL_HADRON     1
#define SIMUL_ELECTRON   2

/**
* Return position of value v inside a doubles array b of size s. If v is not
*     inside b, return -1.
*/
static int find_pos(double v, double *b, int size) {
    for (int i = 0; i < size; ++i) if (b[i] < v && v < b[i+1]) return i;
    return -1;
}

/**
 * Count number of events in a tree for each bin, for a given pid. The number of
 *     bins is equal to the multiplication of the size-1 of each binning.
 *
 * @param file:   file where we'll write the output data.
 * @param tree:   TTree containing the data we're to process.
 * @param pid:    pid of the particle for which we're counting events.
 * @param nbins:  array containing number of bins.
 * @param edges:  2-dimensional array of edges.
 * @param in_deg: boolean telling us if thrown events are in degrees -- default
 *                is radians.
 * @param type:   int describing type of processing to be done.
 *                  * -2: thrown electron.
 *                  * -1: thrown hadron.
 *                  *  1: simulated hadron.
 *                  *  2: simulated electron.
 * @return:       success code (0).
 */
static int count_entries(
        FILE *file, TTree *tree, int pid, luint *nbins, double **edges,
        bool in_deg, int type
) {
    if (
            type != THROWN_ELECTRON && type != SIMUL_ELECTRON &&
            type != THROWN_HADRON   && type != SIMUL_HADRON
    ) {
        rge_errno = RGEERR_WRONGENTRYTYPE;
        return 1;
    }

    // Store total number of bins for simplicity.
    luint total_nbins = 1;
    for (int i = 0; i < 5; ++i) total_nbins *= nbins[i];

    // Create and initialize evn_cnt.
    // NOTE. Variable-length arrays (vla) are technically not permitted in C++
    //       (-Werror=vla from -pedantic), but *we know* that the array won't be
    //       rambunctiously large, and I'd rather have my memory contiguous than
    //       keeping a 5D array of non-contiguous pointers or -- even worse --
    //       C++ vectors.
    //          -Bruno
    int evn_cnt[nbins[0]][nbins[1]][nbins[2]][nbins[3]][nbins[4]];
    int *iterator = &evn_cnt[0][0][0][0][0]; // Auxiliary iterator for evn_cnt.

    // Set everything in evn_cnt to 0.
    for (luint bin_i = 0; bin_i < total_nbins; ++bin_i) {
        *iterator = 0;
        ++iterator;
    }

    // Get PID.
    Float_t s_pid;
    if (type == THROWN_ELECTRON || type == SIMUL_ELECTRON) {
        s_pid = 11;
    }
    else {
        tree->SetBranchAddress(RGE_PID.name, &s_pid);
    }

    // Get W2.
    Float_t s_W, s_W2;
    if (type == THROWN_ELECTRON || type == THROWN_HADRON) {
        tree->SetBranchAddress(THROWN_W, &s_W);
    }
    else {
        tree->SetBranchAddress(RGE_W2.name, &s_W2);
    }

    // Get Yb.
    Float_t s_Yb;
    if (type == THROWN_ELECTRON || type == THROWN_HADRON) {
        tree->SetBranchAddress(THROWN_YB, &s_Yb);
    }
    else {
        tree->SetBranchAddress(RGE_YB.name, &s_Yb);
    }

    // Get binning variables: Q2, nu, zh, Pt2, phiPQ.
    Float_t s_bin[5] = {0, 0, 0, 0, 0};
    if (type == THROWN_ELECTRON || type == THROWN_HADRON) {
        tree->SetBranchAddress(THROWN_Q2, &(s_bin[0]));
        tree->SetBranchAddress(THROWN_NU, &(s_bin[1]));
    }
    if (type == THROWN_HADRON) {
        tree->SetBranchAddress(THROWN_ZH,    &(s_bin[2]));
        tree->SetBranchAddress(THROWN_PT2,   &(s_bin[3]));
        tree->SetBranchAddress(THROWN_PHIPQ, &(s_bin[4]));
    }
    if (type == SIMUL_ELECTRON || type == SIMUL_HADRON) {
        tree->SetBranchAddress(RGE_Q2.name, &(s_bin[0]));
        tree->SetBranchAddress(RGE_NU.name, &(s_bin[1]));
    }
    if (type == SIMUL_HADRON) {
        tree->SetBranchAddress(RGE_ZH.name,    &(s_bin[2]));
        tree->SetBranchAddress(RGE_PT2.name,   &(s_bin[3]));
        tree->SetBranchAddress(RGE_PHIPQ.name, &(s_bin[4]));
    }

    for (int evn = 0; evn < tree->GetEntries(); ++evn) {
        tree->GetEntry(evn);

        // Only count the selected PID.
        if (pid - 0.5 >= s_pid || s_pid > pid + 0.5) continue;

        // Apply Q2 cut.
        if (s_bin[0] < RGE_Q2CUT) continue; // Q2 > 1.

        // Apply W2 cut.
        if (type == THROWN_ELECTRON || type == THROWN_HADRON) {
            s_W2 = s_W * s_W;
        }
        if (s_W2 < RGE_W2CUT) continue; // W2 > 4.

        // Apply Yb cut.
        if (s_Yb > RGE_YBCUT) continue; // Yb < 0.85.

        // Remove kinematic variables == 0.
        if (s_bin[1] == 0) continue;
        if (
                (type == THROWN_HADRON || type == SIMUL_HADRON) &&
                (s_bin[2] == 0 || s_bin[3] == 0 || s_bin[4] == 0)
        ) {
            continue;
        }

        // Convert phiPQ to radians if necessary.
        if (in_deg) {
            double tmp; // s_bin[4] is Float_t, so we need this conversion step.
            if (rge_to_rad(s_bin[4], &tmp)) return 1;
            s_bin[4] = tmp;
        }

        // Find position of event.
        int idx[5] = {0, 0, 0, 0, 0};
        bool kill = false; // If kill is true, var falls outside of bin range.
        // Hadrons use 5 kinematic variables, electrons can only use 2.
        int nvars = (type == THROWN_HADRON || type == SIMUL_HADRON) ? 5 : 2;
        for (int bi = 0; bi < nvars && !kill; ++bi) {
            idx[bi] = find_pos(s_bin[bi], edges[bi], nbins[bi]);
            if (idx[bi] < 0) kill = true;
        }
        if (kill) continue;

        // Increase counter.
        ++evn_cnt[idx[0]][idx[1]][idx[2]][idx[3]][idx[4]];
    }

    // Write evn_cnt to file.
    iterator = &evn_cnt[0][0][0][0][0];
    for (luint bin_i = 0; bin_i < total_nbins; ++bin_i) {
        fprintf(file, "%d ", *iterator);
        ++iterator;
    }
    fprintf(file, "\n");

    return 0;
}

/** run() function of the program. Check USAGE_MESSAGE for details. */
static int run(
        char *thrown_filename, char *simul_filename, char *data_dir,
        luint *nedges, double **edges, bool in_deg
) {
    // Open input files and load TTrees.
    printf("\nOpening generated events file...\n");
    TFile *thrown_file = TFile::Open(thrown_filename, "READ");
    if (!thrown_file || thrown_file->IsZombie()) {
        rge_errno = RGEERR_WRONGGENFILE;
        return 1;
    }
    TNtuple *thrown    = thrown_file->Get<TNtuple>(RGE_TREENAMETHRN);
    TNtuple *thrown_el = thrown_file->Get<TNtuple>(RGE_TREENAMETHRNELECTRONS);
    if (thrown == NULL || thrown_el == NULL) {
        rge_errno = RGEERR_BADGENFILE;
        return 1;
    }

    printf("Opening simulated events file...\n");
    TFile *simul_file = TFile::Open(simul_filename, "READ");
    if (!simul_file || simul_file->IsZombie()) {
        rge_errno = RGEERR_WRONGSIMFILE;
        return 1;
    }
    TTree *simul = simul_file->Get<TTree>(RGE_TREENAMEDATA);
    if (simul == NULL) {
        rge_errno = RGEERR_BADSIMFILE;
        return 1;
    }

    // Create output file.
    char out_filename[PATH_MAX];
    sprintf(out_filename, "%s/acc_corr.txt", data_dir);
    if (!access(out_filename, F_OK)) {
        rge_errno = RGEERR_OUTFILEEXISTS;
        return 1;
    }
    FILE *out_file = fopen(out_filename, "w");

    // Write binning nedges to output file.
    for (int bi = 0; bi < 5; ++bi) fprintf(out_file, "%lu ", nedges[bi]);
    fprintf(out_file, "\n");

    // Write edges to output file.
    for (int bi = 0; bi < 5; ++bi) {
        for (luint bii = 0; bii < nedges[bi]; ++bii) {
            fprintf(out_file, "%12.9f ", edges[bi][bii]);
        }
        fprintf(out_file, "\n");
    }

    // Get list of PIDs.
    // We assume that we'll deal with at most 256 PIDs. Extend this list if that
    //     stops being the case.
    printf("Getting list of PIDs from generated file...\n");
    Float_t s_pid;
    double pidlist[256];
    int pidlist_size = 0;
    thrown->SetBranchAddress(RGE_PID.name, &s_pid);

    // Add electron to PID list.
    pidlist[pidlist_size++] = 11;

    for (int evn = 0; evn < thrown->GetEntries(); ++evn) {
        thrown->GetEntry(evn);
        bool skip = false;

        // Check that PID is useful for SIDIS analysis.
        for (int pid_i = 0; pid_i < BADPIDS_SIZE; ++pid_i) {
            if (BADPIDS[pid_i] - .5 <= s_pid && s_pid <= BADPIDS[pid_i] + .5) {
                skip = true;
                break;
            }
        }
        if (skip) continue;

        // Check if we have already found this PID.
        for (int pid_i = 0; pid_i < pidlist_size; ++pid_i) {
            if (pidlist[pid_i] - .5 <= s_pid && s_pid <= pidlist[pid_i] + .5) {
                skip = true;
                break;
            }
        }
        if (skip) continue;

        // Add PID to list.
        pidlist[pidlist_size++] = s_pid;
    }

    // Write list of PIDs to output file.
    fprintf(out_file, "%d\n", pidlist_size);
    for (int pid_i = 0; pid_i < pidlist_size; ++pid_i) {
        fprintf(out_file, "%d ", static_cast<int>(pidlist[pid_i]));
    }
    fprintf(out_file, "\n");

    // Get number of bins.
    luint nbins[5];
    for (int bin_dim_i = 0; bin_dim_i < 5; ++bin_dim_i) {
        nbins[bin_dim_i] = static_cast<luint>(nedges[bin_dim_i]-1);
    }

    // Count and write number of thrown and simulated events in each bin.
    for (int pid_i = 0; pid_i < pidlist_size; ++pid_i) {
        int pid = static_cast<int>(pidlist[pid_i]);
        printf("Working on PID %5d (%2d/%2d)...\n", pid, pid_i+1, pidlist_size);

        printf("  Counting thrown events...\n");
        int err = 0;
        if (pid_i == 0) { // electron.
            err = count_entries(
                    out_file, thrown_el, pid, nbins, edges, in_deg,
                    THROWN_ELECTRON
            );
        }
        else {
            err = count_entries(
                    out_file, thrown, pid, nbins, edges, in_deg, THROWN_HADRON
            );
        }
        if (err != 0) return 1;

        printf("  Counting simulated events...\n");
        if (pid_i == 0) {
            err = count_entries(
                    out_file, simul, pid, nbins, edges, false, SIMUL_ELECTRON
            );
        }
        else {
            err = count_entries(
                    out_file, simul, pid, nbins, edges, false, SIMUL_HADRON
            );
        }
        if (err != 0) return 1;

        printf("  Done!\n");
    }

    // Clean up after ourselves.
    thrown_file->Close();
    simul_file->Close();
    fclose(out_file);
    for (int bi = 0; bi < 5; ++bi) free(edges[bi]);
    free(edges);

    rge_errno = RGEERR_NOERR;
    return 0;
}

/** Handle arguments for make_ntuples using optarg. */
static int handle_args(
        int argc, char **argv, char **thrown_filename, char **simul_filename,
        char **data_dir, luint *nedges, double **edges, bool *in_deg
) {
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "hq:n:z:p:f:g:s:d:D")) != -1) {
        switch (opt) {
        case 'h':
            rge_errno = RGEERR_USAGE;
            return 1;
        case 'q':
            rge_grab_multiarg(argc, argv, &optind, &(nedges[0]), &(edges[0]));
            break;
        case 'n':
            rge_grab_multiarg(argc, argv, &optind, &(nedges[1]), &(edges[1]));
            break;
        case 'z':
            rge_grab_multiarg(argc, argv, &optind, &(nedges[2]), &(edges[2]));
            break;
        case 'p':
            rge_grab_multiarg(argc, argv, &optind, &(nedges[3]), &(edges[3]));
            break;
        case 'f':
            rge_grab_multiarg(argc, argv, &optind, &(nedges[4]), &(edges[4]));
            break;
        case 'g':
            rge_grab_string(optarg, thrown_filename);
            break;
        case 's':
            rge_grab_string(optarg, simul_filename);
            break;
        case 'd':
            *data_dir = static_cast<char *>(malloc(strlen(optarg) + 1));
            strcpy(*data_dir, optarg);
            break;
        case 'D':
            *in_deg = true;
            break;
        default:
            break;
        }
    }

    // Check that all arrays were defined.
    for (int bi = 0; bi < 5; ++bi) {
        if (nedges[bi] == 0) {
            rge_errno = RGEERR_NOEDGE;
            return 1;
        }
    }

    // Check that all arrays have *at least* two values.
    for (int bi = 0; bi < 5; ++bi) {
        if (nedges[bi]  < 2) {
            rge_errno = RGEERR_BADEDGES;
            return 1;
        }
    }

    // Convert phi_PQ binning to radians.
    for (luint bbi = 0; bbi < nedges[4]; ++bbi) {
        double tmp;
        if (rge_to_rad(edges[4][bbi], &tmp)) return 1;
        edges[4][bbi] = tmp;
    }

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*data_dir, "%s/../data", dirname(argv[0]));
    }

    // Check genfile.
    if (*thrown_filename == NULL) {
        rge_errno = RGEERR_NOGENFILE;
        return 1;
    }
    if (rge_check_root_filename(*thrown_filename)) return 1;

    // Check simfile.
    if (*simul_filename == NULL) {
        rge_errno = RGEERR_NOSIMFILE;
        return 1;
    }
    if (rge_check_root_filename(*simul_filename)) return 1;

    return 0;
}

/** Entry point of the program. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *thrown_filename = NULL;
    char *simul_filename  = NULL;
    char *data_dir        = NULL;
    bool in_deg           = false;
    luint nedges[5] = {0, 0, 0, 0, 0};
    double **edges;

    edges = static_cast<double **>(malloc(5 * sizeof(*edges)));
    int err = handle_args(
            argc, argv, &thrown_filename, &simul_filename, &data_dir, nedges,
            edges, &in_deg
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED && err == 0) {
        run(thrown_filename, simul_filename, data_dir, nedges, edges, in_deg);
    }

    // Free up memory.
    if (thrown_filename != NULL) free(thrown_filename);
    if (simul_filename  != NULL) free(simul_filename);
    if (data_dir        != NULL) free(data_dir);

    // Return errcode.
    return rge_print_usage(USAGE_MESSAGE);
}
