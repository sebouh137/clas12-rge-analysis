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

// --- Define macro constants here. ----------------------------------------- //
// --+ FILES +------------------------------------------------------------------
const int pid = -211;
const char *IN_FILENAME  = Form("../root_io/fiducial_cuts_pid%d_in.root", pid);
const char *OUT_FILENAME = Form("../root_io/fiducial_cuts_pid%d_out.root", pid);

// --+ BINS +-------------------------------------------------------------------
const double BIN_MIN  = 0.00;
const double BIN_INT  = 0.60;
const double BIN_MAX  = 9.00;
const int    BIN_NX   = 5;    // number of columns of plots.
const int    BIN_NY   = 3;    // number of rows of plots.
const double BIN_SEPX = 0.01; // separation between columns (ignored by root).
const double BIN_SEPY = 0.01; // separation between rows (ignored by root).

// --- Macro code begins here ----------------------------------------------- //
/** Run the program. */
int plot_fiducial_cuts() {
    // Open input files.
    TFile *in_file = TFile::Open(IN_FILENAME, "READ");

    // Create TCanvas.
    TCanvas *canvas = new TCanvas("fiducial cuts", "fiducial cuts", 1522, 1051);
    canvas->Divide(BIN_NX, BIN_NY, BIN_SEPX, BIN_SEPY, 0);

    // Add plots from input file to TCanvases.
    int bin_idx = 0;
    for (double bin = BIN_MIN; bin < BIN_MAX - BIN_INT/2; bin = bin + BIN_INT) {
        ++bin_idx;
        char bin_name[128];
        sprintf(bin_name, "%6.2f, %6.2f", bin, bin + BIN_INT);

        // Get bin dir.
        TDirectory *dirs = (TDirectory *)
            in_file->Get(Form("p (GeV) (%s)", bin_name));

        // Draw plot on canvas.
        canvas->cd(bin_idx);
        for (int pad_i = 1; pad_i <= BIN_NX*BIN_NY; ++pad_i) {
            canvas->GetPad(pad_i)->SetGrid();
        }

        // Get TH1 from file.
        TH2 *plot = (TH2 *) dirs->Get(
            Form("#phi (rad) vs #theta (rad) (p (GeV): %s)", bin_name)
        );

        plot->SetTitle(Form("p: %s", bin_name));
        plot->SetStats(0);
        plot->Draw("COLZ");
        canvas->Update();
    }

    // Write to file.
    TFile *out_file = TFile::Open(OUT_FILENAME, "RECREATE");
    canvas->Write();

    // Clean up.
    in_file->Close();
    out_file->Close();

    return 0;
}
