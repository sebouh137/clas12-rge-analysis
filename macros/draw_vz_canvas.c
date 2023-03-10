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
// FILES.
const char *INPUT_FILENAME_DC  =  "../root_io/vz_acceptance/src/dc_211_corr.root";
const char *INPUT_FILENAME_FMT = "../root_io/vz_acceptance/src/fmt_211_corr.root";
const char *OUTPUT_FILENAME    =         "../root_io/vz_acceptance/211_corr.root";

// PLOTS.
const int NPLOTS = 5;
const std::map<int, const char *> PLOT_NAMES {
    {0, "Q2"}, {1, "#nu"}, {2, "z_{h}"}, {3, "Pt2"}, {4, "#phi_{PQ}"}
};
const std::map<int, const char *> CANVAS_NAMES {
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};

// BINS.
const double BIN_MIN  = -40.;
const double BIN_INT  =   4.;
const double BIN_MAX  =  40.;
const int    BIN_NX   =   5;    // number of columns of plots.
const int    BIN_NY   =   4;    // number of rows of plots.
const double BIN_SEPX =   0.01; // separation between columns.
const double BIN_SEPY =   0.01; // separation between rows.

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

    // Open input files.
    TFile *file_in_dc  = TFile::Open(INPUT_FILENAME_DC, "READ");
    TFile *file_in_fmt = TFile::Open(INPUT_FILENAME_FMT, "READ");
    if (
            !file_in_dc  || file_in_dc->IsZombie() ||
            !file_in_fmt || file_in_fmt->IsZombie()
    ) {
        fprintf(stderr, "\nInput files are not a valid root files.\n\n");
        return 1;
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int i = 0; i < NPLOTS; ++i) {
        add_tcanvas(&canvases, CANVAS_NAMES.at(i));
    }

    // Divide TCanvases.
    for (TCanvas *canvas : canvases) {
        canvas->Divide(BIN_NX, BIN_NY, BIN_SEPX, BIN_SEPY, 0);
    }

    // Add plots from input file to TCanvases.
    int bin_idx = 0;
    for (double bin = BIN_MIN; bin < BIN_MAX - BIN_INT/2; bin = bin + BIN_INT) {
        ++bin_idx;
        char bin_name[128];
        sprintf(bin_name, "%6.2f, %6.2f", bin, bin + BIN_INT);

        // Get bin dir.
        TDirectory *dir_dc =
                (TDirectory *) file_in_dc->Get(Form("vz (%s)", bin_name));
        TDirectory *dir_fmt =
                (TDirectory *) file_in_fmt->Get(Form("vz (%s)", bin_name));

        // Draw plot on canvases.
        int canvas_idx = -1;
        for (TCanvas *canvas : canvases) {
            ++canvas_idx;
            TH1 *plot_dc  = (TH1 *) dir_dc->Get(
                    Form("%s (vz: %s)", PLOT_NAMES.at(canvas_idx), bin_name)
            );
            TH1 *plot_fmt = (TH1 *) dir_fmt->Get(
                    Form("%s (vz: %s)", PLOT_NAMES.at(canvas_idx), bin_name)
            );

            canvas->cd(bin_idx);

            // Write DC data.
            plot_dc->SetTitle("DC");
            plot_dc->SetLineColor(kBlue);
            // plot_dc->SetMarkerStyle(21);
            plot_dc->Draw("apl");

            // Write FMT data.
            plot_fmt->SetTitle("FMT");
            plot_fmt->SetLineColor(kGreen);
            // plot_fmt->SetMarkerStyle(21);
            plot_fmt->Draw("samepl");

            // Add legend.
            gPad->BuildLegend();
        }
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUTPUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) {
        canvas->Write();
    }

    // Clean up.
    file_in_dc ->Close();
    file_in_fmt->Close();
    file_out->Close();
    printf("Done!\n");

    return 0;
}
