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

#include <TFile.h>
#include <TNtuple.h>
#include "../lib/io_handler.h"

// Return position of val in vec or -1 if val is not inside vec.
int find_pos(double val, std::vector<double> vec, int size) {
    for (int i = 0; i < size; ++i) if (vec[i] < val && val < vec[i+1]) return i;
    return -1;
}

int count_events(int *evn_cnt, TTree *tree, int *sizes,
        std::vector<double> &b_Q2, std::vector<double> &b_nu,
        std::vector<double> &b_zh, std::vector<double> &b_Pt2,
        std::vector<double> &b_pPQ)
{
    for (int i = 0; i < sizes[5]; ++i) evn_cnt[i] = 0;

    Float_t s_Q2, s_nu, s_zh, s_Pt2, s_pPQ;
    tree->SetBranchAddress(S_Q2,    &s_Q2);
    tree->SetBranchAddress(S_NU,    &s_nu);
    tree->SetBranchAddress(S_ZH,    &s_zh);
    tree->SetBranchAddress(S_PT2,   &s_Pt2);
    tree->SetBranchAddress(S_PHIPQ, &s_pPQ);
    for (int evn = 0; evn < tree->GetEntries(); ++evn) {
        tree->GetEntry(evn);

        // Find position of event.
        int i0, i1, i2, i3, i4;
        i0 = find_pos(s_Q2,  b_Q2,  sizes[0]);
        i1 = find_pos(s_nu,  b_nu,  sizes[1]);
        i2 = find_pos(s_zh,  b_zh,  sizes[2]);
        i3 = find_pos(s_Pt2, b_Pt2, sizes[3]);
        i4 = find_pos(s_pPQ, b_pPQ, sizes[4]);
        if (i0 < 0 || i1 < 0 || i2 < 0 || i3 < 0 || i4 < 0) continue;

        // Increase counter.
        ++evn_cnt[
                i0*sizes[1]*sizes[2]*sizes[3]*sizes[4] +
                i1*sizes[2]*sizes[3]*sizes[4] +
                i2*sizes[3]*sizes[4] +
                i3*sizes[4] +
                i4
        ];
    }

    return 0;
}

int run(char *gen_file, char *sim_file, std::vector<double> &b_Q2,
        std::vector<double> &b_nu,  std::vector<double> &b_zh,
        std::vector<double> &b_Pt2, std::vector<double> &b_pPQ, bool use_fmt)
{
    // Open input files and load TTrees.
    TFile *t_in = TFile::Open(gen_file, "READ");
    if (!t_in || t_in->IsZombie()) return 9;

    TFile *s_in = TFile::Open(sim_file, "READ");
    if (!s_in || s_in->IsZombie()) return 10;

    // Open output file.
    const char *out_file = "../data/acc_corr.txt";
    if (!access(out_file, F_OK)) return 11;
    FILE *t_out = fopen("../data/acc_corr.txt", "w");

    // Open TTrees.
    TNtuple *thrown = t_in->Get<TNtuple>("ntuple_thrown");
    if (thrown == NULL) return 12;

    TTree *simul;
    if (!use_fmt) simul = s_in->Get<TTree>("dc");
    else          simul = s_in->Get<TTree>("fmt");
    if (simul == NULL) return 13;

    // Count # of thrown and simulated events in each bin.
    // Helper variables.
    int sizes[6];
    sizes[0] = b_Q2.size()-1;
    sizes[1] = b_nu.size()-1;
    sizes[2] = b_zh.size()-1;
    sizes[3] = b_Pt2.size()-1;
    sizes[4] = b_pPQ.size()-1;
    sizes[5] = sizes[0]*sizes[1]*sizes[2]*sizes[3]*sizes[4];

    int t_evn[sizes[5]];
    int s_evn[sizes[5]];
    count_events(t_evn, thrown, sizes, b_Q2, b_nu, b_zh, b_Pt2, b_pPQ);
    count_events(s_evn, simul,  sizes, b_Q2, b_nu, b_zh, b_Pt2, b_pPQ);

    // Compute and save acceptance ratios.
    for (int i = 0; i < sizes[5]; ++i) {
        double acc = (double)s_evn[i] / (double)t_evn[i];
        if (std::fpclassify(acc) != FP_NORMAL || acc > 1) acc = 0;
        fprintf(t_out, "%.12f\n", acc);
    }

    // Clean up after ourselves.
    t_in->Close();
    s_in->Close();
    fclose(t_out);
    free(gen_file);
    free(sim_file);

    return 0;
}

int usage() {
    fprintf(stderr,
            "Usage: acc_corr [q:n:z:p:f:] genfile simfile\n"
            " * -q ...     : Q2 bins.\n"
            " * -n ...     : nu bins.\n"
            " * -z ...     : z_h bins.\n"
            " * -p ...     : Pt2 bins.\n"
            " * -f ...     : phi_PQ bins.\n"
            " * -g genfile : generated events ROOT file.\n"
            " * -s simfile : simulated events ROOT file.\n"
            " * -F         : flag to tell program to use FMT data instead of DC"
            " data from\n                the simulation file.\n\n"
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
        std::vector<double> &b_Q2, std::vector<double> &b_nu,
        std::vector<double> &b_zh, std::vector<double> &b_Pt2,
        std::vector<double> &b_pPQ, bool *use_fmt)
{
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "q:n:z:p:f:g:s:F")) != -1) {
        switch (opt) {
            case 'q': grab_multiarg(argc, argv, &optind, b_Q2);  break;
            case 'n': grab_multiarg(argc, argv, &optind, b_nu);  break;
            case 'z': grab_multiarg(argc, argv, &optind, b_zh);  break;
            case 'p': grab_multiarg(argc, argv, &optind, b_Pt2); break;
            case 'f': grab_multiarg(argc, argv, &optind, b_pPQ); break;
            case 'g': grab_filename(optarg, gen_file);           break;
            case 's': grab_filename(optarg, sim_file);           break;
            case 'F': *use_fmt = true;                           break;
            default: break;
        }
    }

    // Check that all vectors have *at least* two values.
    if (b_Q2.size() < 2 || b_nu.size() < 2 || b_zh.size() < 2 ||
            b_Pt2.size() < 2 || b_pPQ.size() < 2) {
        return 2;
    }

    // Check input file existence and validity.
    if (!(*gen_file)) return 7;
    int errcode = check_root_filename(*gen_file);
    if (errcode) return errcode;
    if (!(*sim_file)) return 8;
    errcode     = check_root_filename(*sim_file);
    if (errcode) return errcode + 2;

    return 0;
}

int main(int argc, char **argv) {
    bool use_fmt   = false;
    char *gen_file = NULL;
    char *sim_file = NULL;
    std::vector<double> b_Q2;
    std::vector<double> b_nu;
    std::vector<double> b_zh;
    std::vector<double> b_Pt2;
    std::vector<double> b_pPQ;

    int errcode = handle_args(argc, argv, &gen_file, &sim_file, b_Q2, b_nu,
            b_zh, b_Pt2, b_pPQ, &use_fmt);
    if (handle_err(errcode)) return 1;

    return handle_err(run(gen_file, sim_file, b_Q2, b_nu, b_zh, b_Pt2, b_pPQ,
            use_fmt));
}
