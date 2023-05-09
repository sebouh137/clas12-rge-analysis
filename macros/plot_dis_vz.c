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
const char *INPUT_FILENAME_DC  = "../root_io/plots_dc_vz.root";
const char *INPUT_FILENAME_FMT = "../root_io/plots_fmt2_vz.root";
const char *OUTPUT_FILENAME    = "../root_io/dis_plots_vz.root";

// PLOTS.
const int NPLOTS = 2;
const int NVARS = 5;
const std::map<int, const char *> PLOT_NAMES {
    {0, "Q2"}, {1, "#nu"}, {2, "z_{h}"}, {3, "Pt2"}, {4, "#phi_{PQ}"}
};
const std::map<int, const char *> CANVAS_NAMES {
    {0, "Q2"}, {1, "nu"}, {2, "zh"}, {3, "Pt2"}, {4, "phiPQ"}
};

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
    for (int i = 0; i < NVARS; ++i) {
        add_tcanvas(&canvases, CANVAS_NAMES.at(i));
    }

    // Divide TCanvases.
    for (TCanvas *canvas : canvases) {
        canvas->SetTitle("My Canvas Title");
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
                (TDirectory *) file_in_dc->Get(Form("v_{z} (%s)", bin_name));
        TDirectory *dir_fmt =
                (TDirectory *) file_in_fmt->Get(Form("v_{z} (%s)", bin_name));

        // Draw plot on canvases.
        int canvas_idx = -1;
        for (TCanvas *canvas : canvases) {
            ++canvas_idx;
            canvas->cd(bin_idx);

            // Get TH1s from files.
            TH1 *plots[NPLOTS];
            plots[0] = (TH1 *) dir_dc->Get(
                    Form("%s (v_{z}: %s)", PLOT_NAMES.at(canvas_idx), bin_name)
            );
            plots[1] = (TH1 *) dir_fmt->Get(
                    Form("%s (v_{z}: %s)", PLOT_NAMES.at(canvas_idx), bin_name)
            );

            // Create TGraphErrors from TH1s.
            TGraphErrors *graphs[NPLOTS];
            double y_max = 0;
            for (int plot_i = 0; plot_i < NPLOTS; ++plot_i) {
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
                    double yerr = 0.0;
                    graphs[plot_i]->SetPoint(bin_i - 1, x, y);
                    graphs[plot_i]->SetPointError(bin_i - 1, xerr, yerr);
                }

                // Set the title and axis labels of the TGraphErrors object.
                if (plot_i == 0)
                    graphs[plot_i]->SetTitle(Form("v_{z} : [%s]", bin_name));
                else
                    graphs[plot_i]->SetTitle("");
                graphs[plot_i]->GetXaxis()->SetTitle(
                        plots[plot_i]->GetXaxis()->GetTitle()
                );
                graphs[plot_i]->GetYaxis()->SetTitle(
                        plots[plot_i]->GetYaxis()->GetTitle()
                );
                if (plot_i == 0) graphs[plot_i]->SetMarkerColor(kBlue);
                else             graphs[plot_i]->SetMarkerColor(kRed);
                graphs[plot_i]->SetMarkerStyle(21);
                graphs[plot_i]->SetMarkerSize(0.7);

                // Draw graphs.
                if (plot_i == 0) graphs[plot_i]->Draw("AP");
                else             graphs[plot_i]->Draw("sameP");

                // Get maximum y value.
                if (graphs[plot_i]->GetMaximum() > y_max) {
                    y_max = graphs[plot_i]->GetMaximum();
                }
            }

            // Rescale y axis to fit both DC and FMT data.
            for (int plot_i = 0; plot_i < NPLOTS; ++plot_i) {
                plots[plot_i]->GetYaxis()->SetRangeUser(0, 1.1*y_max);
            }

            // Add legend.
            if (bin_idx == 5) {
                TLegend* legend = new TLegend(0.7, 0.7, 0.886, 0.88);
                legend->AddEntry(graphs[0],  "DC",   "lp");
                legend->AddEntry(graphs[1], "FMT2", "lp");
                legend->Draw();
            }

            canvas->Update();
        }
    }

    // Add title to canvases.
    // int plot_i = 0;
    // for (TCanvas *canvas : canvases) {
    //     canvas->cd();
    //     TPad *padtitle = new TPad("padtitle", "padtitle",0.3,0.9,0.7,0.99);
    //     padtitle->Draw();
    //     padtitle->cd();
    //     padtitle->SetFillStyle(0);
    //
    //     TLatex *tex = new TLatex(0.5, 0.5, PLOT_NAMES.at(plot_i));
    //     tex->SetTextAlign(22);
    //     tex->SetTextSize(0.5);
    //     tex->Draw();
    //
    //     ++plot_i;
    // }

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
