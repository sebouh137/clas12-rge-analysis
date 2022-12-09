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
#include <algorithm>
#include <TFile.h>
#include <TNtuple.h>
#include "../lib/io_handler.h"
#include "../lib/utilities.h"

// Return position of val in vec or -1 if val is not inside vec.
int find_pos(double val, double *b, int size) {
    for (int i = 0; i < size; ++i) if (b[i] < val && val < b[i+1]) return i;
    return -1;
}

// Count number of events in tree for each bin for a given pid.
int count_events(int *evn_cnt, TTree *tree, int pid, int tsize, int *sizes,
        double **binnings, bool in_deg)
{
    for (int i = 0; i < tsize; ++i) evn_cnt[i] = 0;

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
                        sizes[bi]-1);
            else
                idx[bi] = find_pos(s_binning[bi], binnings[bi], sizes[bi]-1);

            if (idx[bi] < 0) kill = true;
        }
        if (kill) continue;

        // Increase counter.
        ++evn_cnt[
                idx[0]*(sizes[1]-1)*(sizes[2]-1)*(sizes[3]-1)*(sizes[4]-1) +
                idx[1]*(sizes[2]-1)*(sizes[3]-1)*(sizes[4]-1) +
                idx[2]*(sizes[3]-1)*(sizes[4]-1) +
                idx[3]*(sizes[4]-1) +
                idx[4]
        ];
    }

    return 0;
}

int run(char *gen_file, char *sim_file, char *data_dir, int *sizes,
        double **binnings, bool use_fmt, bool in_deg)
{
    // Open input files and load TTrees.
    TFile *t_in = TFile::Open(gen_file, "READ");
    if (!t_in || t_in->IsZombie()) return 9;
    TNtuple *thrown = t_in->Get<TNtuple>("ntuple_thrown");
    if (thrown == NULL) return 12;

    TFile *s_in = TFile::Open(sim_file, "READ");
    if (!s_in || s_in->IsZombie()) return 10;
    TTree *simul = use_fmt ? s_in->Get<TTree>("fmt") : s_in->Get<TTree>("dc");
    if (simul == NULL) return 13;

    // Create output file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/acc_corr.txt", data_dir);
    if (!access(out_file, F_OK)) return 11;
    FILE *t_out = fopen(out_file, "w");

    // Write binning sizes to output file.
    for (int bi = 0; bi < 5; ++bi) fprintf(t_out, "%d ", sizes[bi]);
    fprintf(t_out, "\n");

    // Write binnings to output file.
    for (int bi = 0; bi < 5; ++bi) {
        for (int bii = 0; bii < sizes[bi]; ++bii) {
            fprintf(t_out, "%12.9f ", binnings[bi][bii]);
        }
        fprintf(t_out, "\n");
    }

    // Get list of PIDs.
    Float_t s_pid;
    std::vector<double> pid_list;
    thrown->SetBranchAddress(S_PID, &s_pid);
    for (int evn = 0; evn < thrown->GetEntries(); ++evn) {
        thrown->GetEntry(evn);
        if (std::find(pid_list.begin(),pid_list.end(),s_pid) == pid_list.end())
            pid_list.push_back(s_pid);
    }

    // Write list of PIDs to output file.
    fprintf(t_out, "%ld\n", pid_list.size());
    for (double pid : pid_list) fprintf(t_out, "%d ", (int) pid);
    fprintf(t_out, "\n");

    // Count # of thrown and simulated events in each bin.
    int tsize = 1;
    for (int bi = 0; bi < 5; ++bi) tsize *= sizes[bi] - 1;

    for (double pid_dbl : pid_list) {
        int pid = (int) pid_dbl;
        printf("Working on PID %5d...", pid);
        fflush(stdout);

        int t_evn[tsize];
        int s_evn[tsize];
        count_events(t_evn, thrown, pid, tsize, sizes, binnings, in_deg);
        count_events(s_evn, simul,  pid, tsize, sizes, binnings, false);

        // Compute and save acceptance ratios.
        for (int i = 0; i < tsize; ++i) {
            double acc = (double)s_evn[i] / (double)t_evn[i];
            if (std::fpclassify(acc) != FP_NORMAL || acc > 1) acc = 0;
            fprintf(t_out, "%.12f ", acc);
        }
        fprintf(t_out, "\n");
        printf(" Done!\n");
    }

    // Clean up after ourselves.
    t_in->Close();
    s_in->Close();
    fclose(t_out);
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);

    return 0;
}

int usage() {
    fprintf(stderr,
            "\nUsage: acc_corr [hq:n:z:p:f:g:s:d:FD]\n"
            " * -h         : show this message and exit.\n"
            " * -q ...     : Q2 bins.\n"
            " * -n ...     : nu bins.\n"
            " * -z ...     : z_h bins.\n"
            " * -p ...     : Pt2 bins.\n"
            " * -f ...     : phi_PQ bins.\n"
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

int handle_err(int errcode) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            break;
        case 2:
            fprintf(stderr, "Error. All binnings should have *at least* two "
                            "values -- a minimum and a\n maximum.\n\n");
            break;
        case 3:
            fprintf(stderr, "Error. Generated file should be in root "
                            "format.\n\n");
            break;
        case 4:
            fprintf(stderr, "Error. Generated file does not exist!\n\n");
            break;
        case 5:
            fprintf(stderr, "Error. Simulated file should be in root "
                            "format.\n\n");
            break;
        case 6:
            fprintf(stderr, "Error. Simulated file does not exist!\n\n");
            break;
        case 7:
            fprintf(stderr, "Error. Please specify a generated file.\n\n");
            break;
        case 8:
            fprintf(stderr, "Error. Please specify a simulated file.\n\n");
            break;
        case 9:
            fprintf(stderr, "Error. Generated file is not a valid ROOT file."
                            "\n\n");
            break;
        case 10:
            fprintf(stderr, "Error. Simulated file is not a valid ROOT file."
                            "\n\n");
            break;
        case 11:
            fprintf(stderr, "Error. Output file already exists.\n\n");
            break;
        case 12:
            fprintf(stderr, "Error. Generated file is badly formatted.\n\n");
            break;
        case 13:
            fprintf(stderr, "Error. Simualted file is badly formatted.\n\n");
            break;
        default:
            fprintf(stderr, "Error code %d not implemented!\n\n", errcode);
            return 1;
    }
    return usage();
}

int handle_args(int argc, char **argv, char **gen_file, char **sim_file,
        char **data_dir, int *sizes, double **binnings, bool *use_fmt,
        bool *in_deg)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "hq:n:z:p:f:g:s:d:FD")) != -1) {
        switch (opt) {
        case 'h':
            return 1;
        case 'q':
            grab_multiarg(argc, argv, &optind, &(sizes[0]), &(binnings[0]));
            break;
        case 'n':
            grab_multiarg(argc, argv, &optind, &(sizes[1]), &(binnings[1]));
            break;
        case 'z':
            grab_multiarg(argc, argv, &optind, &(sizes[2]), &(binnings[2]));
            break;
        case 'p':
            grab_multiarg(argc, argv, &optind, &(sizes[3]), &(binnings[3]));
            break;
        case 'f':
            grab_multiarg(argc, argv, &optind, &(sizes[4]), &(binnings[4]));
            break;
        case 'g':
            grab_filename(optarg, gen_file);
            break;
        case 's':
            grab_filename(optarg, sim_file);
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

    // Check that all vectors have *at least* two values.
    for (int bi = 0; bi < 5; ++bi) if (sizes[bi] < 2) return 2;

    // Define datadir if undefined.
    if (*data_dir == NULL) {
        *data_dir = (char *) malloc(PATH_MAX);
        sprintf(*data_dir, "%s/../data", dirname(argv[0]));
    }

    // Check genfile.
    if (*gen_file == NULL) return 7;
    int errcode = check_root_filename(*gen_file);
    if (errcode) return errcode;

    // Check simfile.
    if (*sim_file == NULL) return 8;
    errcode = check_root_filename(*sim_file);
    if (errcode) return errcode + 2;

    return 0;
}

int main(int argc, char **argv) {
    // Handle arguments.
    char *gen_file = NULL;
    char *sim_file = NULL;
    char *data_dir = NULL;
    bool use_fmt   = false;
    bool in_deg    = false;
    int sizes[5];
    double **binnings;

    binnings = (double **) malloc(5 * sizeof(*binnings));
    int errcode = handle_args(argc, argv, &gen_file, &sim_file, &data_dir,
            sizes, binnings, &use_fmt, &in_deg);

    // Run.
    if (errcode == 0)
        errcode = run(gen_file, sim_file, data_dir, sizes, binnings, use_fmt,
                in_deg);

    // Free up memory.
    if (gen_file != NULL) free(gen_file);
    if (sim_file != NULL) free(sim_file);
    if (data_dir != NULL) free(data_dir);

    // Return errcode.
    return handle_err(errcode);
}
