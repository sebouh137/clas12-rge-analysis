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
#include "TFile.h"
#include "TTree.h"
#include "../lib/io_handler.h"
#include "../lib/bank_containers.h"

int run(char *in_filename, int run_no, int nevn) {
    // Create output file. TODO. I should fix these paths ASAP.
    char out_filename[128];
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

    int evn = 0;
    while (reader.next() && (nevn == -1 || evn < nevn)) {
        evn++;
        if (evn % 10000 == 0) {
            if (evn != 10000) printf("\33[2K\r");
            printf("Read %8d events...", evn);
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
    printf("\33[2K\rRead %8d events... Done!\n", evn);

    // Clean up.
    tree->Write();
    f->Close();
    return 0;
}

int usage() {
    fprintf(stderr,
            "\nUsage: hipo2root [-h] [-n nevents] file\n"
            " * -h         : show this message and exit.\n"
            " * -n nevents : specify number of events.\n"
            " * file       : HIPO file. Expected file format is "
            "<text>run_no.hipo.\n\n"
            "    Convert a file from hipo to root format. This program only "
            "conserves the\n    banks that are useful for EG2 analysis, as "
            "specified in the\n    lib/bank_containers.h file.\n\n"
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
            fprintf(stderr, "Error %02d. No file name provided.\n\n", errcode);
            break;
        case 3:
            fprintf(stderr, "Error %02d. input file should be in hipo format."
                            "\n\n", errcode);
            break;
        case 4:
            fprintf(stderr, "Error %02d. file does not exist!\n\n", errcode);
            break;
        case 5:
            fprintf(stderr, "Error %02d. Bad usage of optional arguments.\n\n",
                    errcode);
            break;
        case 6:
            fprintf(stderr, "Error %02d. nevents should be a number greater "
                            "than 0", errcode);
            break;
        default:
            fprintf(stderr, "Error code %02d not implemented!\n\n", errcode);
            return 1;
    }
    return usage();
}

int handle_args(int argc, char **argv, char **input_file, int *run_no,
        int *nevents)
{
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'n':
                *nevents = atoi(optarg);
                break;
            case 1:
                *input_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*input_file, optarg);
                break;
            default:
                return 5;
        }
    }

    // Check that nevents is valid and that atoi performed correctly.
    if (*nevents == 0) return 6;

    // Check positional argument.
    if (*input_file == NULL) return 2;

    return handle_hipo_filename(*input_file, run_no);
}

int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename = NULL;
    int  run_no       = -1;
    int  nevn         = -1;

    int errcode = handle_args(argc, argv, &in_filename, &run_no, &nevn);

    // Run.
    if (errcode == 0) errcode = run(in_filename, run_no, nevn);

    // Free up memory.
    if (in_filename != NULL) free(in_filename);

    // Return errcode.
    return handle_err(errcode);
}
