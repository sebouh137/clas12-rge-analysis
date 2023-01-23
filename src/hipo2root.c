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
#include "TFile.h"
#include "../lib/io_handler.h"
#include "../lib/bank_containers.h"

/** run() function of the program. Check usage() for details. */
int run(char *in_filename, char *work_dir, int run_no, int event_max) {
    // Create output file.
    char out_file[PATH_MAX];
    sprintf(out_file, "%s/banks_%06d.root", work_dir, run_no);

    TFile *f = TFile::Open(out_file, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    // Create tree for output file and bank containers to read hipo file.
    TTree *tree = new TTree("Tree", "Tree");
    Particle     particle;
    Track        track;
    Calorimeter  calorimeter;
    Cherenkov    cherenkov;
    Scintillator scintillator;

    // Link bank container to tree branches.
    particle    .link_branches(tree);
    track       .link_branches(tree);
    calorimeter .link_branches(tree);
    cherenkov   .link_branches(tree);
    scintillator.link_branches(tree);

    // Open input file and get hipo schemas.
    hipo::reader reader;
    reader.open(in_filename);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::event event;
    hipo::bank particle_bank    (factory.getSchema("REC::Particle"));
    hipo::bank track_bank       (factory.getSchema("REC::Track"));
    hipo::bank calorimeter_bank (factory.getSchema("REC::Calorimeter"));
    hipo::bank cherenkov_bank   (factory.getSchema("REC::Cherenkov"));
    hipo::bank scintillator_bank(factory.getSchema("REC::Scintillator"));

    // Get stuff from hipo file and write to root file.
    if (event_max == -1) event_max = reader.getEntries();
    printf("Reading %d events from %s.\n", event_max, in_filename);

    int event_no = 0;

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    while (reader.next() && event_no < event_max) {
        // Print fancy progress bar.
        update_progress_bar(event_max, event_no, &evnsplitter, &divcntr);
        ++event_no;

        // Read next event.
        reader.read(event);

        // Get bank structures from hipo event.
        event.getStructure(particle_bank);
        event.getStructure(track_bank);
        event.getStructure(calorimeter_bank);
        event.getStructure(cherenkov_bank);
        event.getStructure(scintillator_bank);

        // Fill banks from hipo event.
        particle    .fill(particle_bank);
        track       .fill(track_bank);
        calorimeter .fill(calorimeter_bank);
        cherenkov   .fill(cherenkov_bank);
        scintillator.fill(scintillator_bank);

        // Write to tree *if* event is not empty.
        int total_nrows = particle.get_nrows() +
                          track.get_nrows() +
                          calorimeter.get_nrows() +
                          cherenkov.get_nrows() +
                          scintillator.get_nrows();

        if (total_nrows > 0) tree->Fill();
    }
    printf("\33[2K\rRead %8d events... Done!\n", event_no);

    // Write to root tree and clean up after ourselves.
    tree->Write();
    f->Close();

    return 0;
}

/** Print usage and exit. */
int usage() {
    fprintf(stderr,
            "\n\nUsage: hipo2root [-hn:w:] infile\n"
            " * -h         : show this message and exit.\n"
            " * -n nevents : number of events.\n"
            " * -w workdir : location where output root files are to be "
            "stored. Default\n                is root_io.\n"
            " * infile     : input HIPO file. Expected file format is "
            "<text>run_no.hipo.\n\n"
            "    Convert a file from hipo to root format. This program only "
            "conserves the\n    banks that are useful for RG-E analysis, as "
            "specified in the\n    lib/bank_containers.h file.\n\n"
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
            fprintf(stderr, "nevents should be a number greater than 0");
            break;
        case 3:
            fprintf(stderr, "Bad usage of optional arguments.");
            break;
        case 4:
            fprintf(stderr, "No input filename provided.");
            break;
        case 5:
            fprintf(stderr, "Input file should be in hipo format.");
            break;
        case 6:
            fprintf(stderr, "Input file does not exist.");
            break;
        case 7:
            // NOTE. It's technically impossible to get here, this error should
            //     be fully covered by errcode 5. Better safe than sorry.
            fprintf(stderr, "Couldn't find extension in input filename.");
            break;
        case 8:
            fprintf(stderr, "Couldn't find run number in input filename.");
            break;
        default:
            fprintf(stderr, "Error code not implemented!\n");
            return 1;
    }
    return usage();
}

/**
 * Handle arguments for hipo2root using optarg. Error codes used are explained
 *     in the handle_err() function.
 */
int handle_args(int argc, char **argv, char **in_filename, char **work_dir,
        int *run_no, int *event_max)
{
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hn:w:")) != -1) {
        switch (opt) {
            case 'h':
                return 1;
            case 'n':
                *event_max = atoi(optarg);
                if (*event_max <= 0) return 2; // Check if event_max is valid.
                break;
            case 'w':
                *work_dir = (char *) malloc(strlen(optarg) + 1);
                strcpy(*work_dir, optarg);
                break;
            case 1:
                *in_filename = (char *) malloc(strlen(optarg) + 1);
                strcpy(*in_filename, optarg);
                break;
            default:
                return 3;
        }
    }

    // Define workdir if undefined.
    if (*work_dir == NULL) {
        *work_dir = (char *) malloc(PATH_MAX);
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Check that a positional argument was given.
    if (*in_filename == NULL) return 4;

    int check = handle_hipo_filename(*in_filename, run_no);
    if (check) return check + 4; // Shift errcode.

    return 0;
}

/** Entry point of hipo2root. Check usage() for details. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename = NULL;
    char *work_dir    = NULL;
    int  run_no       = -1;
    int  event_max    = -1;

    int errcode = handle_args(argc, argv, &in_filename, &work_dir, &run_no,
            &event_max);

    // Run.
    if (errcode == 0) errcode = run(in_filename, work_dir, run_no, event_max);

    // Free up memory.
    if (in_filename != NULL) free(in_filename);
    if (work_dir    != NULL) free(work_dir);

    // Return errcode.
    return handle_err(errcode);
}
