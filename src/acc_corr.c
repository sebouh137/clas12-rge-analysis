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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

int run() {
    return 0;
}

int usage() {
    fprintf(stderr,
            "Usage: acc_corr [q:n:z:p:f:g:s:]\n"
            " * -q ...     : Q2 bins.\n"
            " * -n ...     : nu bins.\n"
            " * -z ...     : z_h bins.\n"
            " * -p ...     : Pt2 bins.\n"
            " * -f ...     : phi_PQ bins.\n"
            " * -g genfile : generated events ROOT file.\n"
            " * -s simfile : simulated events ROOT file.\n\n"
            "    Get the 5-dimensional acceptance correction factors for Q2, nu"
            ", z_h, Pt2, and\n    phi_PQ. For each optional argument, an array "
            "of doubles is expected. The first\n    double will be the lower "
            "limit of the leftmost bin, the final double will be\n    the upper"
            " limit of the rightmost bin, and all doubles inbetween will be the"
            "\n    separators between each bin.\n\n"
    );
    return 1;
}

// Grab multiple arguments and fill a vector of doubles with it.
int grab_multiarg(int argc, char **argv, int *optind, std::vector<double> &b) {
    // Initialize and fill array.
    int idx = *optind - 1;
    char *next;
    while (idx < argc) {
        next = strdup(argv[idx++]);
        if (next[0] >= '0' && next[0] <= '9') b.push_back(atof(next));
        else                                  break;
    }

    // Continue with getopts.
    *optind = idx - 1;
    return 0;
}

int grab_filename(char *optarg, char **file) {
    *file = (char *) malloc(strlen(optarg) + 1);
    strcpy(*file, optarg);
    return 0;
}

int handle_err(int errcode) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            break;
    }
    return usage();
}

int handle_args(int argc, char **argv, char **gen_file, char **sim_file,
        std::vector<double> &b_Q2, std::vector<double> &b_nu,
        std::vector<double> &b_zh, std::vector<double> &b_Pt2,
        std::vector<double> &b_phiPQ)
{
    int opt;
    while ((opt = getopt(argc, argv, "-q:n:z:p:f:g:s:")) != -1) {
        switch (opt) {
            case 'q': grab_multiarg(argc, argv, &optind, b_Q2);    break;
            case 'n': grab_multiarg(argc, argv, &optind, b_nu);    break;
            case 'z': grab_multiarg(argc, argv, &optind, b_zh);    break;
            case 'p': grab_multiarg(argc, argv, &optind, b_Pt2);   break;
            case 'f': grab_multiarg(argc, argv, &optind, b_phiPQ); break;
            case 'g': grab_filename(optarg, gen_file);             break;
            case 's': grab_filename(optarg, sim_file);             break;
            default: return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    char *gen_file  = NULL;
    char *sim_file  = NULL;
    std::vector<double> b_Q2;
    std::vector<double> b_nu;
    std::vector<double> b_zh;
    std::vector<double> b_Pt2;
    std::vector<double> b_phiPQ;

    int errcode = handle_args(argc, argv, &gen_file, &sim_file, b_Q2, b_nu,
            b_zh, b_Pt2, b_phiPQ);

    printf("b_Q2    = [");
    for (const double &i : b_Q2) printf("%5.2f, ", i);
    printf("]\nb_nu    = [");
    for (const double &i : b_nu) printf("%5.2f, ", i);
    printf("]\nb_zh    = [");
    for (const double &i : b_zh) printf("%5.2f, ", i);
    printf("]\nb_Pt2   = [");
    for (const double &i : b_Pt2) printf("%5.2f, ", i);
    printf("]\nb_phiPQ = [");
    for (const double &i : b_phiPQ) printf("%5.2f, ", i);
    printf("]\ngen_file = %s\n", gen_file);
    printf("sim_file = %s\n", sim_file);
    printf("errcode = %d\n\n", errcode);

    if (handle_err(errcode)) return 1;

    // return handle_err(run(file, b_Q2, b_nu, b_zh, b_Pt2, b_phiPQ), &file);
    return 0;
}
