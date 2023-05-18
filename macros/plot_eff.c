// CLAS12 RG-E Analyser.
// Copyright (C) 2023 Bruno Benkel
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

// --- Define macro constants here ---------------------------------------------
// FILES.
const int NVERSIONS = 2; // Number of versions - i.e: not-corrected & corrected.
const int NFILES    = 2; // Number of input files.
const char *IN_FILENAMES[NVERSIONS][NFILES] = {
        {
                "../root_io/data_nc/pid11/dc_effstudy.root",
                "../root_io/data_nc/pid11/fmt2_effstudy.root"
        },
        {
                "../root_io/data_c/pid11/dc_effstudy.root",
                "../root_io/data_c/pid11/fmt2_effstudy.root"
        }
};
const char *OUT_FILENAME = "../root_io/effstudy_pid11.root";

// PLOTS.
const int NPLOTS = 4;
const std::map<int, const char *> PLOT_NAMES {
        {0, "v_{z}"}, {1, "#phi"}, {2, "#theta"}, {3, "p"}
};
const std::map<int, const char *> CANVAS_NAMES {
        {0, "vz"}, {1, "phi"}, {2, "theta"}, {3, "p"}
};

// --- Macro code begins here --------------------------------------------------
/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/**
 * Obtain FMT efficiencies from IN_FILENAMES. Here, we're defining FMT
 *     efficiency as % of DC tracks accepted by FMT.
 */
int plot_eff() {
    printf("Plotting efficiencies... ");
    fflush(stdout);

    // Open input files.
    TFile *in_files[NVERSIONS][NFILES];
    for (int vi = 0; vi < NVERSIONS; ++vi) {
        for (int fi = 0; fi < NFILES; ++fi) {
            in_files[vi][fi] = TFile::Open(IN_FILENAMES[vi][fi]);
            if (!in_files[vi][fi] || in_files[vi][fi]->IsZombie()) {
                fprintf(
                        stderr,
                        "File %s is not valid.\n", IN_FILENAMES[vi][fi]
                );
                return 1;
            }
        }
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int pi = 0; pi < NPLOTS; ++pi) {
        add_tcanvas(&canvases, CANVAS_NAMES.at(pi));
    }

    int canvas_idx = -1;
    for (TCanvas *canvas : canvases) {
        ++canvas_idx;
        canvas->cd();
        canvas->SetGrid();

        // Get TH1Fs from files.
        TH1F *plots[NVERSIONS][NFILES];
        for (int vi = 0; vi < NVERSIONS; ++vi) {
            for (int fi = 0; fi < NFILES; ++fi) {
                plots[vi][fi] = (TH1F *)
                        in_files[vi][fi]->Get(PLOT_NAMES.at(canvas_idx));
            }
        }

        // Divide FMT plots by DC plots.
        for (int vi = 0; vi < NVERSIONS; ++vi) {
            plots[vi][1]->Divide(plots[vi][0]);
            plots[vi][1]->GetYaxis()->SetRangeUser(0, 1);
        }

        plots[0][1]->SetTitle(Form(
                "%s  FMT (2 layers) efficiency", PLOT_NAMES.at(canvas_idx)
        ));

        // Set plots color.
        plots[0][1]->SetLineColor(kRed);
        plots[1][1]->SetLineColor(kBlue);

        // Remove stats.
        plots[0][1]->SetStats(0);
        plots[1][1]->SetStats(0);

        // Draw.
        plots[0][1]->Draw();
        plots[1][1]->Draw("SAME");

        // Add legend.
        TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
        legend->AddEntry(plots[0][1], "Raw", "l");
        legend->AddEntry(plots[1][1], "Geometry-corrected", "l");
        legend->Draw();

        canvas->Update();
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) {
        canvas->Write();
    }

    // Clean up.
    for (int vi = 0; vi < NVERSIONS; ++vi) {
        for (int fi = 0; fi < NFILES; ++fi) {
            in_files[vi][fi]->Close();
        }
    }
    file_out->Close();
    printf("Done!\n");

    return 0;
}
