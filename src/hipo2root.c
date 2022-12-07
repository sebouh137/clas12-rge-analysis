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
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include "TFile.h"
#include "TTree.h"
#include "../lib/io_handler.h"
#include "../lib/bank_containers.h"

int run(char *in_file, char *work_dir, int run_no, int nevn) {
    // Create output file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/banks_%06d.root", work_dir, run_no);

    TFile *f = TFile::Open(out_file, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    // Create tree for output file.
    TTree *tree = new TTree("Tree", "Tree");
    REC_Particle     rprt; rprt.link_branches(tree);
    REC_Track        rtrk; rtrk.link_branches(tree);
    REC_Calorimeter  rcal; rcal.link_branches(tree);
    REC_Cherenkov    rche; rche.link_branches(tree);
    REC_Scintillator rsci; rsci.link_branches(tree);
    FMT_Tracks       ftrk; ftrk.link_branches(tree);

    // Open input file and get hipo schemas.
    hipo::reader reader;
    reader.open(in_file);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank rprt_b(factory.getSchema("REC::Particle"));
    hipo::bank rtrk_b(factory.getSchema("REC::Track"));
    hipo::bank rcal_b(factory.getSchema("REC::Calorimeter"));
    hipo::bank rche_b(factory.getSchema("REC::Cherenkov"));
    hipo::bank rsci_b(factory.getSchema("REC::Scintillator"));
    hipo::bank ftrk_b(factory.getSchema("FMT::Tracks"));
    hipo::event event;

    // Get stuff from hipo file and write to root file.
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
            "\nUsage: hipo2root [-hn:w:] infile\n"
            " * -h         : show this message and exit.\n"
            " * -n nevents : number of events.\n"
            " * -w workdir : location where output root files are to be "
            "stored. Default\n                is root_io.\n"
            " * infile     : input HIPO file. Expected file format is "
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
            fprintf(stderr, "Error %02d. No file name provided.\n", errcode);
            break;
        case 3:
            fprintf(stderr, "Error %02d. input file should be in hipo format."
                            "\n", errcode);
            break;
        case 4:
            fprintf(stderr, "Error %02d. file does not exist!\n", errcode);
            break;
        case 5:
            fprintf(stderr, "Error %02d. Bad usage of optional arguments.\n",
                    errcode);
            break;
        case 6:
            fprintf(stderr, "Error %02d. nevents should be a number greater "
                            "than 0\n", errcode);
            break;
        default:
            fprintf(stderr, "Error code %02d not implemented!\n", errcode);
            return 1;
    }
    return usage();
}

int handle_args(int argc, char **argv, char **in_file, char **work_dir,
        int *run_no, int *nevents)
{
    // Handle optional arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:w:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'n':
                *nevents = atoi(optarg);
                break;
            case 'w':
                *work_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*work_dir, optarg);
                break;
            case 1:
                *in_file = (char *) malloc(strlen(optarg) + 1);
                strcpy(*in_file, optarg);
                break;
            default:
                return 5;
        }
    }

    // Check that nevents is valid and that atoi performed correctly.
    if (*nevents == 0) return 6;

    // Define workdir if undefined.
    if (*work_dir == NULL) {
        *work_dir = (char *) malloc(PATH_MAX);
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Check positional argument.
    if (*in_file == NULL) return 2;

    return handle_hipo_filename(*in_file, run_no);
}

int main(int argc, char **argv) {
    // Handle arguments.
    char *in_file  = NULL;
    char *work_dir = NULL;
    int  run_no    = -1;
    int  nevn      = -1;

    int errcode = handle_args(argc, argv, &in_file, &work_dir, &run_no, &nevn);

    // Run.
    if (errcode == 0) errcode = run(in_file, work_dir, run_no, nevn);

    // Free up memory.
    if (in_file  != NULL) free(in_file);
    if (work_dir != NULL) free(work_dir);

    // Return errcode.
    return handle_err(errcode);
}
