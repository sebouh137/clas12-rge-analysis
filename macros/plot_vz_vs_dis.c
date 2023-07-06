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
// --+ PID to process +---------------------------------------------------------
const int pid = -211;

// --+ FILES +------------------------------------------------------------------
const int NFILES = 2;
const char *IN_FILENAMES[NFILES] = {
    Form("../root_io/dis/statistics_study/dc_%d.root",   pid),
    Form("../root_io/dis/statistics_study/fmt2_%d.root", pid)
};
const char *LEGEND_ENTRIES[NFILES] = {
    "DC", "FMT"
};
const int COLORS[NFILES] = {
    kRed, kBlue
};
const char *OUT_FILENAME = Form(
    "../root_io/dis/statistics_study/study_%d.root", pid
);

// --+ PLOTS +------------------------------------------------------------------
const int NVARS = 5;

// --+ BINS +-------------------------------------------------------------------
const double BIN_MIN  =  -5.;
const double BIN_MAX  =  10.;
const double BIN_INT  =   1.;
const int NBINS       =  15;

// --- Macro code begins here ----------------------------------------------- //
/** Run the program. */
int plot_vz_vs_dis() {
    printf("Plotting vz vs DIS variables... ");
    fflush(stdout);

    // Open input files.
    TFile *in_files[NFILES];
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i] = TFile::Open(IN_FILENAMES[file_i], "READ");
        if (!in_files[file_i] || in_files[file_i]->IsZombie()) {
            fprintf(stderr, "File %s is not valid.\n", IN_FILENAMES[file_i]);
            return 1;
        }
    }

    // Create TCanvases.
    TCanvas *canvas = new TCanvas("Statistics", "Statistics", 1600, 900);
    canvas->cd();
    canvas->SetGrid();

    // Create TGraphErrors.
    TGraphErrors *graphs[NFILES];
    double ymax = 1e-20;

    for (int file_i = 0; file_i < NFILES; ++file_i) {
        graphs[file_i] = new TGraphErrors(NBINS);

        for (int bin_i = 0; bin_i < NBINS; ++bin_i) {
            double x    = BIN_MIN + bin_i*BIN_INT + 0.5*BIN_INT;
            double xerr = 0.5*BIN_INT;

            // Fill arrays with vz bins data.
            char bin_name[128];
            sprintf(
                bin_name, "%6.2f, %6.2f",
                BIN_MIN + bin_i*BIN_INT, BIN_MIN + (bin_i+1)*BIN_INT
            );

            // Get bin dir.
            TDirectory *dirs = (TDirectory *)
            in_files[file_i]->Get(Form("v_{z} (cm) (%s)", bin_name));

            // Get TH1 from files.
            TH1 *plot = (TH1 *) dirs->Get(
                Form("Q^{2} (GeV^{2}) (v_{z} (cm): %s)", bin_name)
            );

            // Rescue data from TH1.
            double y = plot->GetBinContent(1);
            double yerr = plot->GetBinError(1);
            if (ymax < y + yerr) ymax = y + yerr;

            // Fill graph.
            graphs[file_i]->SetPoint(bin_i, x, y);
            graphs[file_i]->SetPointError(bin_i, xerr, yerr);
        }

        // Set range.
        graphs[file_i]->GetYaxis()->SetRangeUser(0, 1.1*ymax);

        // Set title and axis label.
        if (file_i == 0) {
            graphs[file_i]->SetTitle(Form("v_{z} statistics - PID %d", pid));
        }
        else {
            graphs[file_i]->SetTitle("");
        }

        graphs[file_i]->GetXaxis()->SetTitle("v_{z}");
        graphs[file_i]->SetMarkerColor(COLORS[file_i]);
        graphs[file_i]->SetMarkerStyle(21);
        graphs[file_i]->SetMarkerSize(1);
    }

    // Set graph range and draw.
    TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        // Set graph range.
        graphs[file_i]->GetYaxis()->SetRangeUser(0, 1.1*ymax);

        // Draw graphs.
        if (file_i == 0) graphs[file_i]->Draw("AP");
        else             graphs[file_i]->Draw("sameP");

        // Add legend entry.
        legend->AddEntry(graphs[file_i], LEGEND_ENTRIES[file_i], "lp");
    }

    // Draw legend.
    legend->Draw();
    canvas->Update();

    // Clean up.
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i]->Close();
    }
    printf("Done!\n");

    return 0;
}
