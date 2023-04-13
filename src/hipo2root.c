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

#include <libgen.h>
#include "TFile.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_progress.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_hipo_bank.h"

const char *usage_message =
"Usage: hipo2root [-hfn:w:] infile\n"
" * -h         : show this message and exit.\n"
" * -f         : set this to true to process FMT::Tracks bank. If this is set\n"
"                and FMT::Tracks bank is not present in the HIPO file, the\n"
"                program will crash.\n"
" * -n nevents : number of events.\n"
" * -w workdir : location where output root files are to be stored. Default\n"
"                is root_io.\n"
" * infile     : input HIPO file. Expected format is <text>run_no.hipo.\n\n"
"    Convert a file from hipo to root format. This program only conserves the\n"
"    banks that are useful for RG-E analysis, as specified in the\n"
"    lib/bank_containers.h file.\n";

/** run() function of the program. Check usage_message for details. */
static int run(
        char *in_filename, char *work_dir, bool use_fmt, int run_no,
        long int event_max
) {
    // Create output file.
    char out_filename[PATH_MAX];
    sprintf(out_filename, "%s/banks_%06d.root", work_dir, run_no);

    TFile *out_file = TFile::Open(out_filename, "RECREATE");
    out_file->SetCompressionAlgorithm(ROOT::kLZ4);

    // Create tree for output file and bank containers to read hipo file.
    TTree *tree = new TTree(TREENAMEDATA, TREENAMEDATA);
    rge_hipobank particle     = rge_recparticle_init();
    rge_hipobank track        = rge_rectrack_init();
    rge_hipobank calorimeter  = rge_reccalorimeter_init();
    rge_hipobank cherenkov    = rge_reccherenkov_init();
    rge_hipobank scintillator = rge_recscintillator_init();
    rge_hipobank fmt_tracks   = rge_fmttracks_init();

    // Link bank container to tree branches.
    rge_link_branches(&particle,     tree);
    rge_link_branches(&track,        tree);
    rge_link_branches(&calorimeter,  tree);
    rge_link_branches(&cherenkov,    tree);
    rge_link_branches(&scintillator, tree);
    if (use_fmt) rge_link_branches(&fmt_tracks, tree);

    // Open input file and get hipo schemas.
    hipo::reader reader;
    reader.open(in_filename);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::event event;
    hipo::bank particle_bank    (factory.getSchema(BANKRECPARTICLE));
    hipo::bank track_bank       (factory.getSchema(BANKRECTRACK));
    hipo::bank calorimeter_bank (factory.getSchema(BANKRECCALORIMETER));
    hipo::bank cherenkov_bank   (factory.getSchema(BANKRECCHERENKOV));
    hipo::bank scintillator_bank(factory.getSchema(BANKRECSCINTILLATOR));
    hipo::bank fmt_tracks_bank;
    if (use_fmt) fmt_tracks_bank = factory.getSchema(BANKFMTTRACKS);

    // Get stuff from hipo file and write to root file.
    if (event_max == -1 || event_max > reader.getEntries()) {
        event_max = reader.getEntries();
    }
    printf("Reading %ld events from %s.\n", event_max, in_filename);

    int event_no = 0;

    // Prepare fancy progress bar.
    rge_pbar_set_nentries(event_max);
    while (reader.next() && event_no < event_max) {
        // Print fancy progress bar.
        rge_pbar_update(event_no);
        ++event_no;

        // Read next event.
        reader.read(event);

        // Get bank structures from hipo event.
        event.getStructure(particle_bank);
        event.getStructure(track_bank);
        event.getStructure(calorimeter_bank);
        event.getStructure(cherenkov_bank);
        event.getStructure(scintillator_bank);
        if (use_fmt) event.getStructure(fmt_tracks_bank);

        // Fill banks from hipo event.
        rge_fill(&particle,     particle_bank);
        rge_fill(&track,        track_bank);
        rge_fill(&calorimeter,  calorimeter_bank);
        rge_fill(&cherenkov,    cherenkov_bank);
        rge_fill(&scintillator, scintillator_bank);
        if (use_fmt) rge_fill(&fmt_tracks, fmt_tracks_bank);

        // Write to tree *if* event is not empty.
        long unsigned int total_nrows = particle.nrows +
                                        track.nrows +
                                        calorimeter.nrows +
                                        cherenkov.nrows +
                                        scintillator.nrows;
        if (use_fmt) total_nrows += fmt_tracks.nrows;

        if (total_nrows > 0) tree->Fill();
    }
    printf("\33[2K\rRead %8d events... Done!\n", event_no);

    // Write to root tree and clean up after ourselves.
    tree->Write();
    out_file->Close();

    rge_errno = RGEERR_NOERR;
    return 0;
}

/**
 * Handle arguments for hipo2root using optarg. Error codes used are explained
 *     in the handle_err() function.
 */
static int handle_args(
        int argc, char **argv, char **in_filename, char **work_dir,
        bool *use_fmt, int *run_no, long int *event_max
) {
    // Handle arguments.
    int opt;
    while ((opt = getopt(argc, argv, "-hfn:w:")) != -1) {
        switch (opt) {
            case 'h':
                rge_errno = RGEERR_USAGE;
                return 1;
            case 'f':
                *use_fmt = true;
                break;
            case 'n':
                if (rge_process_nentries(event_max, optarg)) return 1;
                break;
            case 'w':
                *work_dir = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*work_dir, optarg);
                break;
            case 1:
                *in_filename = static_cast<char *>(malloc(strlen(optarg) + 1));
                strcpy(*in_filename, optarg);
                break;
            default:
                rge_errno = RGEERR_BADOPTARGS;
                return 1;
        }
    }

    // Define workdir if undefined.
    if (*work_dir == NULL) {
        *work_dir = static_cast<char *>(malloc(PATH_MAX));
        sprintf(*work_dir, "%s/../root_io", dirname(argv[0]));
    }

    // Check that a positional argument was given.
    if (*in_filename == NULL) {
        rge_errno = RGEERR_NOINPUTFILE;
        return 1;
    }

    if (rge_handle_hipo_filename(*in_filename, run_no)) return 1;

    return 0;
}

/** Entry point of hipo2root. Check usage() for details. */
int main(int argc, char **argv) {
    // Handle arguments.
    char *in_filename  = NULL;
    char *work_dir     = NULL;
    bool use_fmt       = false;
    int  run_no        = -1;
    long int event_max = -1;

    handle_args(
            argc, argv, &in_filename, &work_dir, &use_fmt, &run_no, &event_max
    );

    // Run.
    if (rge_errno == RGEERR_UNDEFINED) {
        run(in_filename, work_dir, use_fmt, run_no, event_max);
    }

    // Free up memory.
    if (in_filename != NULL) free(in_filename);
    if (work_dir    != NULL) free(work_dir);

    // Return errcode.
    return rge_print_usage(usage_message);
}
