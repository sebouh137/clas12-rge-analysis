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
const int NPIDS      = 3; // Number of PIDs.
const int PIDLIST[NPIDS] = {11, 211, -211};
const int NDETECTORS = 2; // Number of detectors.
const char *DETECTORLIST[NDETECTORS] = {"dc", "fmt2"};
const int NVERSIONS = 2; // Number of versions - i.e: not-corrected & corrected.
const char *VERSIONLIST[NVERSIONS] = {"raw", "gcut"};
const char *OUT_FILENAME = "../root_io/eff/study_012016.root";

// PLOTS.
const int NPLOTS = 4;
const char *PLOT_NAMES[NPLOTS]   = {
        "v_{z} (cm)", "#phi (rad)", "#theta (rad)", "p (GeV)"
};
const char *CANVAS_NAMES[NPLOTS] = {"vz", "phi", "theta", "p"};

// --- Macro code begins here --------------------------------------------------
/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/**
 * Obtain FMT efficiencies from in_filenames. Here, we're defining FMT
 *     efficiency as % of DC tracks accepted by FMT.
 */
int plot_eff() {
    printf("Plotting efficiencies... ");
    fflush(stdout);

    // Open input files.
    TFile *in_files[NPIDS][NDETECTORS][NVERSIONS];

    for (int pid_i = 0; pid_i < NPIDS; ++pid_i) {
        for (int det_i = 0; det_i < NDETECTORS; ++det_i) {
            for (int ver_i = 0; ver_i < NVERSIONS; ++ver_i) {
                in_files[pid_i][det_i][ver_i] = TFile::Open(Form(
                    "../root_io/eff/study/pid%d_%s_%s.root",
                    PIDLIST[pid_i], DETECTORLIST[det_i], VERSIONLIST[ver_i]
                ));
                if (
                    !in_files[pid_i][det_i][ver_i] ||
                    in_files[pid_i][det_i][ver_i]->IsZombie()
                ) {
                    fprintf(stderr, "File %s is not valid.\n", Form(
                        "../root_io/eff/study/pid%d_%s_%s.root",
                        PIDLIST[pid_i], DETECTORLIST[det_i], VERSIONLIST[ver_i]
                    ));
                    return 1;
                }
            }
        }
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int pid_i = 0; pid_i < NPLOTS; ++pid_i) {
        add_tcanvas(&canvases, CANVAS_NAMES[pid_i]);
    }

    // Divide TCanvases.
    for (TCanvas *canvas : canvases) {
        canvas->Divide(NPIDS, 1);
    }

    int canvas_idx = -1;
    for (TCanvas *canvas : canvases) {
        ++canvas_idx;

        // Draw each plot.
        for (int pid_i = 0; pid_i < NPIDS; ++pid_i) {
            TPad *pad = (TPad *) canvas->cd(pid_i + 1);
            canvas->SetGrid();

            // Adjust the pad margins.
            if (pid_i == 0) pad->SetMargin(0.05,   0.0025, 0.08, 0.0);
            else            pad->SetMargin(0.0025, 0.0025, 0.08, 0.0);

            // Get TH1Fs from files.
            TH1F *plots[NDETECTORS][NVERSIONS];
            for (int det_i = 0; det_i < NDETECTORS; ++det_i) {
                for (int ver_i = 0; ver_i < NVERSIONS; ++ver_i) {
                    plots[det_i][ver_i] = (TH1F *)
                        in_files[pid_i][det_i][ver_i]->Get(
                            PLOT_NAMES[canvas_idx]
                        );
                }
            }

            // Divide FMT plots by DC plots.
            for (int ver_i = 0; ver_i < NVERSIONS; ++ver_i) {
                plots[1][ver_i]->Divide(plots[0][ver_i]);
                plots[1][ver_i]->GetYaxis()->SetRangeUser(0, 1.05);
            }

            plots[1][0]->SetTitle(Form(
                "%s FMT efficiency (pid %d)",
                PLOT_NAMES[canvas_idx], PIDLIST[pid_i]
            ));

            // Set plots color.
            plots[1][0]->SetLineColor(kRed);
            plots[1][1]->SetLineColor(kBlue);

            // Remove stats.
            plots[1][0]->SetStats(0);
            plots[1][1]->SetStats(0);

            // Draw.
            plots[1][0]->Draw();
            plots[1][1]->Draw("SAME");

            // Add legend.
            if (pid_i == NPIDS - 1) {
                TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
                legend->AddEntry(plots[1][0], "Raw", "l");
                legend->AddEntry(plots[1][1], "Geometry-corrected", "l");
                legend->Draw();
            }

            canvas->Update();
        }
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) {
        canvas->Write();
    }

    // Clean up.
    for (int pid_i = 0; pid_i < NPIDS; ++pid_i) {
        for (int det_i = 0; det_i < NDETECTORS; ++det_i) {
            for (int ver_i = 0; ver_i < NVERSIONS; ++ver_i) {
                in_files[pid_i][det_i][ver_i]->Close();
            }
        }
    }
    file_out->Close();
    printf("Done!\n");

    return 0;
}
