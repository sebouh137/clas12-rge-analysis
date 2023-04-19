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

#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>

// --- Define macro constants here ---------------------------------------------
const char *DC_FILENAME   = "../root_io/vz_resolution/dc_all.root";
const char *FMT2_FILENAME = "../root_io/vz_resolution/fmt_2lyrs_all.root";
const char *FMT3_FILENAME = "../root_io/vz_resolution/fmt_3lyrs_all.root";

// --- Macro code begins here --------------------------------------------------
int vz_plots() {
    TFile *file1 = TFile::Open(DC_FILENAME);
    TFile *file2 = TFile::Open(FMT2_FILENAME);
    TFile *file3 = TFile::Open(FMT3_FILENAME);

    TCanvas *c = new TCanvas("c", "c", 800, 600);

    // DC plot.
    TH1F *hist1 = (TH1F *) file1->Get("vz");
    hist1->SetTitle("DC");
    hist1->SetLineColor(kBlue);
    hist1->Draw();

    // FMT 2 layers plot.
    TH1F *hist2 = (TH1F *) file2->Get("vz");
    hist2->SetTitle("FMT (2 layers)");
    hist2->SetLineColor(kRed);
    hist2->Draw("SAME");

    // FMT 3 layers plot.
    TH1F *hist3 = (TH1F *) file3->Get("vz");
    hist3->SetTitle("FMT (3 layers)");
    hist3->SetLineColor(kGreen);
    hist3->Draw("SAME");

    // Add legend.
    gPad->BuildLegend();

    c->Draw();

    return 0;
}
