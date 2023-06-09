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
// PID to process.
const int pid = 11;

// FILES.
const int NFILES = 3; // Number of input files.
const char *IN_FILENAMES[NFILES] = {
        Form("../root_io/dis/pid%d/integrated_dc.root",   pid),
        Form("../root_io/dis/pid%d/integrated_fmt2.root", pid),
        Form("../root_io/dis/pid%d/integrated_fmt3.root", pid)
};
const char *LEGEND_ENTRIES[NFILES] = {
        "DC", "FMT - 2 layers", "FMT - 3 layers"
};
const int COLORS[NFILES] = {kRed, kBlue, kGreen};
const char *OUT_FILENAME = Form("../root_io/dis/integrated_pid%d.root", pid);

// PLOTS.
const int NVARS = 5;
const std::map<int, const char *> VAR_NAMES {
    {0, "Q^{2}"}, {1, "#nu"}, {2, "z_{h}"}, {3, "P_{T}^{2}"}, {4, "#phi_{PQ}"}
};
const std::map<int, const char *> CANVAS_NAMES {
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};
const int CANVAS_YSCALE[NVARS] = {1, 0, 0, 1, 0}; // 0 is linear, 1 is log.

// --- Macro code begins here --------------------------------------------------
/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/** Obtain DIS plots from IN_FILENAMES. */
int plot_dis() {
    printf("Plotting unbinned DIS variables... ");
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

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int i = 0; i < NVARS; ++i) {
        add_tcanvas(&canvases, CANVAS_NAMES.at(i));
    }

    int canvas_idx = -1;
    for (TCanvas *canvas : canvases) {
        ++canvas_idx;
        canvas->cd();
        canvas->SetGrid();

        // Get TH1Fs from files.
        TH1F *plots[NFILES];
        for (int file_i = 0; file_i < NFILES; ++file_i) {
            plots[file_i] =
                    (TH1F *) in_files[file_i]->Get(VAR_NAMES.at(canvas_idx));
        }

        // Create TGraphErrors from TH1Fs.
        TGraphErrors *graphs[NFILES];
        double y_min = 1e20;
        double y_max = 1e-20;
        for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
            // Create TGraphErrors with the same number of points as TH1F plots.
            graphs[plot_i] = new TGraphErrors(plots[plot_i]->GetNbinsX());

            // Write data.
            for (int bin_i = 1; bin_i <= plots[plot_i]->GetNbinsX(); ++bin_i) {
                double x = plots[plot_i]->GetBinCenter(bin_i);
                double y = plots[plot_i]->GetBinContent(bin_i);
                double xerr = plots[plot_i]->GetBinWidth(bin_i) / 2;
                double yerr = plots[plot_i]->GetBinError(bin_i);
                graphs[plot_i]->SetPoint(bin_i - 1, x, y);
                graphs[plot_i]->SetPointError(bin_i - 1, xerr, yerr);
            }

            // Set the title and axis labels of the TGraphErrors object.
            if (plot_i == 0) graphs[plot_i]->SetTitle(VAR_NAMES.at(canvas_idx));
            else             graphs[plot_i]->SetTitle("");
            graphs[plot_i]->GetXaxis()->SetTitle(
                    plots[plot_i]->GetXaxis()->GetTitle()
            );
            graphs[plot_i]->GetYaxis()->SetTitle(
                    plots[plot_i]->GetYaxis()->GetTitle()
            );
            graphs[plot_i]->SetMarkerColor(COLORS[plot_i]);
            graphs[plot_i]->SetMarkerStyle(21);

            // Draw graphs.
            if (plot_i == 0) graphs[plot_i]->Draw("AP");
            else             graphs[plot_i]->Draw("sameP");

            // Get minimum and maximum y values.
            if (plots[plot_i]->GetMinimum() < y_min) {
                y_min = plots[plot_i]->GetMinimum();
            }
            if (plots[plot_i]->GetMaximum() > y_max) {
                y_max = plots[plot_i]->GetMaximum();
            }
        }

        // Rescale y axis to fit both DC and FMT data.
        for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
            double min = 0;
            double max = 1.1 * y_max;
            if (CANVAS_YSCALE[canvas_idx] == 1) {
                min = y_min / sqrt(10);
                max = y_max * sqrt(10);
            }
            graphs[plot_i]->GetYaxis()->SetRangeUser(min, max);
        }

        // Change y scale to log if necessary.
        if (CANVAS_YSCALE[canvas_idx] == 1) {
            canvas->SetLogy();
            // graphs[0]->GetYaxis()->SetMoreLogLabels();
        }

        // Add legend.
        TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
        for (int file_i = 0; file_i < NFILES; ++file_i) {
            legend->AddEntry(graphs[file_i], LEGEND_ENTRIES[file_i], "lp");
        }
        legend->Draw();

        canvas->Update();
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) {
        canvas->Write();
    }

    // Clean up.
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i]->Close();
    }
    file_out->Close();
    printf("Done!\n");

    return 0;
}
