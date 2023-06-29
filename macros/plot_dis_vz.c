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
// PID to process.
const int pid = 11;

// FILES.
const int NFILES = 3;
const char *IN_FILENAMES_VZ[NFILES] = {
    Form("../root_io/dis/pid%d/vz_dc.root",   pid),
    Form("../root_io/dis/pid%d/vz_fmt2.root", pid),
    Form("../root_io/dis/pid%d/vz_fmt3.root", pid)
};
const char *LEGEND_ENTRIES[NFILES] = {
    "DC", "FMT - 2 layers", "FMT - 3 layers"
};
const int COLORS[NFILES] = {kRed, kBlue, kGreen};
const char *OUTPUT_FILENAME = Form("../root_io/dis_vz_pid%d.root", pid);

// PLOTS.
const int NVARS = 5;
const std::map<int, const char *> VAR_NAMES {
    {0, "Q^{2} (GeV^{2})"},
    {1, "#nu (GeV)"},
    {2, "z_{h}"},
    {3, "p_{T}^{2} (GeV^{2})"},
    {4, "#phi_{PQ} (rad)"}
};
const std::map<int, const char *> CANVAS_NAMES {
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};
const int CANVAS_YSCALE[NVARS] = {1, 0, 1, 1, 1};

// BINS.
const double BIN_MIN  = -30.;
const double BIN_INT  =   5.;
const double BIN_MAX  =  20.;
const int    BIN_NX   =   5;    // number of columns of plots.
const int    BIN_NY   =   2;    // number of rows of plots.
const double BIN_SEPX =   0.01; // separation between columns.
const double BIN_SEPY =   0.01; // separation between rows.

// --- Macro code begins here ----------------------------------------------- //
/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/** Run the program. */
int plot_dis_vz() {
    printf("Plotting DIS variables in vz bins... ");
    fflush(stdout);

    // Open input files.
    TFile *in_files[NFILES];
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i] = TFile::Open(IN_FILENAMES_VZ[file_i], "READ");
        if (!in_files[file_i] || in_files[file_i]->IsZombie()) {
            fprintf(stderr, "File %s is not valid.\n", IN_FILENAMES_VZ[file_i]);
            return 1;
        }
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int i = 0; i < NVARS; ++i) {
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
        TDirectory *dirs[NFILES];
        for (int file_i = 0; file_i < NFILES; ++file_i) {
            dirs[file_i] = (TDirectory *)
                in_files[file_i]->Get(Form("v_{z} (cm) (%s)", bin_name));
        }

        // Draw plot on canvases.
        int canvas_idx = -1;
        for (TCanvas *canvas : canvases) {
            ++canvas_idx;
            canvas->cd(bin_idx);
            for (int pad_i = 1; pad_i <= BIN_NX*BIN_NY; ++pad_i) {
                canvas->GetPad(pad_i)->SetGrid();
            }

            // Get TH1s from files.
            TH1 *plots[NFILES];
            for (int file_i = 0; file_i < NFILES; ++file_i) {
                plots[file_i] = (TH1 *) dirs[file_i]->Get(
                    Form("%s (v_{z} (cm): %s)",
                    VAR_NAMES.at(canvas_idx), bin_name)
                );
            }

            // Create TGraphErrors from TH1s.
            TGraphErrors *graphs[NFILES];
            double y_min = 1e20;
            double y_max = 1e-20;
            for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
                // Create TGraphErrors from TH1 plots.
                graphs[plot_i] = new TGraphErrors(plots[plot_i]->GetNbinsX());

                // Write data.
                for (
                        int bin_i = 1;
                        bin_i <= plots[plot_i]->GetNbinsX();
                        ++bin_i
                ) {
                    double x = plots[plot_i]->GetBinCenter(bin_i);
                    double y = plots[plot_i]->GetBinContent(bin_i);
                    double xerr = plots[plot_i]->GetBinWidth(bin_i) / 2;
                    double yerr = plots[plot_i]->GetBinError(bin_i);
                    graphs[plot_i]->SetPoint(bin_i - 1, x, y);
                    graphs[plot_i]->SetPointError(bin_i - 1, xerr, yerr);
                }

                // Set the title and axis labels of the TGraphErrors object.
                if (plot_i == 0)
                    graphs[plot_i]->SetTitle(Form(
                        "%s (v_{z}: %s)", VAR_NAMES.at(canvas_idx), bin_name
                    ));
                else
                    graphs[plot_i]->SetTitle("");
                graphs[plot_i]->GetXaxis()->SetTitle(
                        plots[plot_i]->GetXaxis()->GetTitle()
                );
                graphs[plot_i]->GetYaxis()->SetTitle(
                        plots[plot_i]->GetYaxis()->GetTitle()
                );
                graphs[plot_i]->SetMarkerColor(COLORS[plot_i]);
                graphs[plot_i]->SetMarkerStyle(21);
                graphs[plot_i]->SetMarkerSize(0.7);

                // Draw graphs.
                if (plot_i == 0) graphs[plot_i]->Draw("AP");
                else             graphs[plot_i]->Draw("sameP");

                // Get minimum and maximum y values.
                if (
                        plots[plot_i]->GetMinimum() < y_min &&
                        plots[plot_i]->GetMinimum() > 1e-20
                ) {
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
                    min = y_min / sqrt(2);
                    max = y_max * sqrt(2);
                }
                graphs[plot_i]->GetYaxis()->SetRangeUser(min, max);
            }

            // Change y scale to log if necessary.
            if (CANVAS_YSCALE[canvas_idx] == 1) {
                for (int pad_i = 1; pad_i <= BIN_NX*BIN_NY; ++pad_i) {
                    canvas->GetPad(pad_i)->SetLogy();
                    // graph[0]->GetYaxis()->SetMoreLogLabels();
                }
            }

            // Add legend.
            if (bin_idx == 5) {
                TLegend* legend = new TLegend(0.7, 0.7, 0.886, 0.88);
                for (int file_i = 0; file_i < NFILES; ++file_i) {
                    legend->AddEntry(
                            graphs[file_i],
                            LEGEND_ENTRIES[file_i],
                            "lp"
                    );
                }
                legend->Draw();
            }

            canvas->Update();
        }
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUTPUT_FILENAME, "RECREATE");
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
