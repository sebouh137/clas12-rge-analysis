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

#include <vector>
#include <map>
#include <TCanvas.h>
#include <TFile.h>

// --- Define macro constants here. ----------------------------------------- //
const char *INPUT_FILENAME  = "../root_io/plots_012933_dc_pid-211_bins_vz.root";
const char *OUTPUT_FILENAME = "../root_io/vz_acceptance.root";
const int NPLOTS = 5;
const std::map<int, const char *> PLOT_NAMES {
    // {0, "Q2"}, {1, "#nu"}, {2, "z_{h}"}, {3, "Pt2"}, {4, "#phi_{PQ}"}
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};

const double BIN_MIN = -40.;
const double BIN_INT =   4.;
const double BIN_MAX =  40.;

// --- Macro code begins here ----------------------------------------------- //
/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/** Run the program. */
int draw_vz_canvas() {
    printf("Running... ");
    fflush(stdout);

    // Open input file.
    TFile *file_in = TFile::Open(INPUT_FILENAME, "READ");
    if (!file_in || file_in->IsZombie()) {
        fprintf(stderr, "\nInput file is not a valid root file.\n\n");
        return 1;
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int i = 0; i < NPLOTS; ++i) {
        add_tcanvas(&canvases, PLOT_NAMES.at(i));
    }

    // Divide TCanvases.
    for (TCanvas *canvas : canvases) {
        //             nx ny sep_x sep_y
        canvas->Divide( 5, 4, 0.01, 0.01, 0);
    }

    // Add plots from input file to TCanvases.
    int bin_idx = 0;
    for (double bin = BIN_MIN; bin < BIN_MAX - BIN_INT/2; bin = bin + BIN_INT) {
        ++bin_idx;
        char bin_name[128];
        sprintf(bin_name, "%6.2f, %6.2f", bin, bin + BIN_INT);

        // Get bin dir.
        TDirectory *dir =
                (TDirectory *) file_in->Get(Form("vz (%s)", bin_name));

        // Draw plot on canvases.
        int canvas_idx = -1;
        for (TCanvas *canvas : canvases) {
            ++canvas_idx;
            TH1 *plot = (TH1 *) dir->Get(
                    Form("%s (vz: %s)", PLOT_NAMES.at(canvas_idx), bin_name)
            );

            canvas->cd(bin_idx);
            plot->Draw("apl");
        }
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUTPUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) {
        canvas->Write();
    }

    // Clean up.
    file_in ->Close();
    file_out->Close();
    printf("Done!\n");

    return 0;
}
