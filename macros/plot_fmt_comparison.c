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
// run to process.
const int run = 12016;
// const int run = 12933;

// FILES.
const int NFILES = 4; // Number of input files.
const char *IN_FILENAMES[NFILES] = {
        Form("../root_io/eff/vz_comparison/0%d/dc_raw.root",   run),
        Form("../root_io/eff/vz_comparison/0%d/fmt2_raw.root", run),
        Form("../root_io/eff/vz_comparison/0%d/dc_geomcut.root",   run),
        Form("../root_io/eff/vz_comparison/0%d/fmt2_geomcut.root", run),
};
const char *LEGEND_ENTRIES[NFILES] = {
        "DC (raw)", "FMT (raw)", "DC (w/ cut)", "FMT (w/ cut)"
};
const int COLORS[NFILES] = {kRed, kBlue, kRed+2, kBlue+2};
const char *OUT_FILENAME = Form(
        "../root_io/eff/vz_comparison/dc_vs_fmt_geomcut_0%d.root", run
);

// PLOTS.
const char *VAR = "v_{z}";
const int YSCALE = 0; // 0 for linear, 1 for log.

// --- Macro code begins here --------------------------------------------------
/** Draw two plots (same variable, same range, etc.) together. */
int plot_fmt_comparison() {
    printf("Plotting vz... ");
    fflush(stdout);

    // Open input files.
    TFile *in_files[NFILES];
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i] = TFile::Open(IN_FILENAMES[file_i]);
        if (!in_files[file_i] || in_files[file_i]->IsZombie()) {
            fprintf(stderr, "File %s is not valid.\n", IN_FILENAMES[file_i]);
            return 1;
        }
    }

    // Create TCanvas.
    TCanvas *canvas = new TCanvas(VAR, VAR, 1600, 900);

    canvas->cd();
    canvas->SetGrid();

    // Get TH1Fs from files.
    TH1F *plots[NFILES];
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        plots[file_i] = (TH1F *) in_files[file_i]->Get(VAR);
    }

    // Create TGraphErrors from TH1Fs.
    double y_min = 1e20;
    double y_max = 1e-20;
    for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
        // Set the title and axis labels of the TGraphErrors object.
        if (plot_i == 0) plots[plot_i]->SetTitle(VAR);
        else             plots[plot_i]->SetTitle("");
        plots[plot_i]->GetXaxis()->SetTitle(
            Form("%s [cm]", plots[plot_i]->GetXaxis()->GetTitle())
        );
        plots[plot_i]->GetYaxis()->SetTitle(
            plots[plot_i]->GetYaxis()->GetTitle()
        );
        plots[plot_i]->SetLineColor(COLORS[plot_i]);

        // Draw plots.
        if (plot_i == 0) plots[plot_i]->Draw("");
        else             plots[plot_i]->Draw("same");

        // Get minimum and maximum y values.
        if (plots[plot_i]->GetMinimum() < y_min) {
            y_min = plots[plot_i]->GetMinimum();
        }
        if (plots[plot_i]->GetMaximum() > y_max) {
            y_max = plots[plot_i]->GetMaximum();
        }
    }

    // Remove stats.
    for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
        plots[plot_i]->SetStats(0);
    }

    // Rescale y axis to fit both DC and FMT data.
    for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
        double min = 0;
        double max = 1.1 * y_max;
        if (YSCALE == 1) {
            min = y_min / sqrt(10);
            max = y_max * sqrt(10);
        }
        plots[plot_i]->GetYaxis()->SetRangeUser(min, max);
    }

    // Draw plots.
    for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
        if (plot_i == 0) plots[plot_i]->Draw("");
        else             plots[plot_i]->Draw("same");
    }

    // Change y scale to log if necessary.
    if (YSCALE == 1) canvas->SetLogy();

    // Add legend.
    TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        legend->AddEntry(plots[file_i], LEGEND_ENTRIES[file_i], "l");
    }
    legend->Draw();
    canvas->Update();

    // Write to file.
    TFile *file_out = TFile::Open(OUT_FILENAME, "RECREATE");
    canvas->Write();

    // Clean up.
    for (int file_i = 0; file_i < NFILES; ++file_i) in_files[file_i]->Close();
    file_out->Close();
    printf("Done!\n");

    return 0;
}
