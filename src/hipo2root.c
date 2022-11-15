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

#include <cstdlib>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "reader.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

#include "../lib/err_handler.h"
#include "../lib/io_handler.h"
#include "../lib/bank_containers.h"

int run(char *in_filename, int run_no) {
    char *out_filename = (char *) malloc(128 * sizeof(char));

    // TODO. I should fix these paths ASAP.
    sprintf(out_filename, "../root_io/banks_%06d.root", run_no);

    TFile *f = TFile::Open(out_filename, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    TTree *tree = new TTree("Tree", "Tree");
    REC_Particle     rprt; rprt.link_branches(tree);
    REC_Track        rtrk; rtrk.link_branches(tree);
    REC_Calorimeter  rcal; rcal.link_branches(tree);
    REC_Cherenkov    rche; rche.link_branches(tree);
    REC_Scintillator rsci; rsci.link_branches(tree);
    FMT_Tracks       ftrk; ftrk.link_branches(tree);

    // Setup.
    hipo::reader reader;
    reader.open(in_filename);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank rprt_b(factory.getSchema("REC::Particle"));
    hipo::bank rtrk_b(factory.getSchema("REC::Track"));
    hipo::bank rcal_b(factory.getSchema("REC::Calorimeter"));
    hipo::bank rche_b(factory.getSchema("REC::Cherenkov"));
    hipo::bank rsci_b(factory.getSchema("REC::Scintillator"));
    hipo::bank ftrk_b(factory.getSchema("FMT::Tracks"));
    hipo::event event;

    int c = 0;
    while (reader.next() && c < 840000) {
        c++;
        if (c % 10000 == 0) {
            if (c != 10000) printf("\33[2K\r");
            printf("Read %8d events...", c);
            fflush(stdout);
        }
        reader.read(event);

        event.getStructure(rprt_b); rprt.fill(rprt_b);
        event.getStructure(rtrk_b); rtrk.fill(rtrk_b);
        event.getStructure(rcal_b); rcal.fill(rcal_b);
        event.getStructure(rche_b); rche.fill(rche_b);
        event.getStructure(rsci_b); rsci.fill(rsci_b);
        event.getStructure(ftrk_b); ftrk.fill(ftrk_b);
        if (rprt.get_nrows() + rtrk.get_nrows() + rcal.get_nrows() +
                rche.get_nrows() + rsci.get_nrows() + ftrk.get_nrows() > 0)
            tree->Fill();
    }
    printf("\33[2K\rRead %8d events... Done!\n", c);

    // Clean up.
    tree->Write();
    f->Close();
    free(in_filename);
    free(out_filename);
    return 0;
}

int usage() {
    fprintf(stderr,
            "\nUsage: hipo2root filename\n\n"
            "    Convert a file from the hipo format to the root format.\n\n"
    );

    return 1;
}

int handle_errs(int errcode, char **in_filename) {
    switch (errcode) {
        case 0:
            return 0;
        case 1:
            fprintf(stderr, "Error. No file name provided.\n");
            break;
        case 2:
            fprintf(stderr, "Error. Too many arguments.\n");
            break;
        case 3:
            fprintf(stderr, "Error. input file should be in hipo format.\n");
            break;
        case 4:
            fprintf(stderr, "Error. file does not exist!\n");
            break;
        default:
            fprintf(stderr, "Error code %d not implemented!\n", errcode);
            return 1;
    }
    if (errcode > 2) free(*in_filename);
    return usage();
}

int handle_args(int argc, char **argv, char **input_file, int *run_no) {
    if (argc < 2) return 1;
    if (argc > 2) return 2;

    *input_file = (char *) malloc(strlen(argv[1]) + 1);
    strcpy(*input_file, argv[1]);
    return handle_hipo_filename(*input_file, run_no);
}

int main(int argc, char **argv) {
    char *in_filename = NULL;
    int  run_no       = -1;

    int errcode = handle_args(argc, argv, &in_filename, &run_no);
    if (handle_errs(errcode, &in_filename)) return 1;

    return handle_errs(run(in_filename, run_no), &in_filename);
}
