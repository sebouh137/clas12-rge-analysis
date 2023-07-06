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
const char *IN_FILENAMES_VZ[NFILES] = {
    Form("../root_io/dis/acc_corr/pid%d/vz_dc.root",   pid),
    Form("../root_io/dis/acc_corr/pid%d/vz_fmt2.root", pid) // ,
    // Form("../root_io/dis/pid%d/vz_fmt3.root", pid)
};
const char *LEGEND_ENTRIES[NFILES] = {
    "DC", "FMT" // , "FMT - 3 layers"
};
const int COLORS[NFILES] = {
    kRed, kBlue // , kGreen
};
const char *OUTPUT_FILENAME = Form(
    "../root_io/dis/acc_corr/pid%d/vz.root", pid
);

// --+ PLOTS +------------------------------------------------------------------
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
const int CANVAS_YSCALE[NVARS] = {1, 1, 1, 1, 1};

// --+ BINS +-------------------------------------------------------------------
const double BIN_MIN  = -30.;
const double BIN_INT  =   5.;
const double BIN_MAX  =  20.;
const int    BIN_NX   =   5;    // number of columns of plots.
const int    BIN_NY   =   2;    // number of rows of plots.
const double BIN_SEPX =   0.01; // separation between columns (ignored by root).
const double BIN_SEPY =   0.01; // separation between rows (ignored by root).

// --+ ACC CORR ERRORS +--------------------------------------------------------
//                       VAR PARTICLE DETECTOR IDX
const double acccorr_err[  5][      3][      2][13] = {
    { // Q2.
        { // e-.
            { // DC.
                0.00012908, 0.00039971, 0.00068614, 0.00108265, 0.00168686,
                0.00254135, 0.00382909, 0.00536583, 0.00771245, 0.01151561,
                0., 0., 0.
            },
            { // FMT.
                0.00012442, 0.00039436, 0.00069525, 0.00111247, 0.00168235,
                0.00234473, 0.00310210, 0.00362824, 0.00438493, 0.00406226,
                0., 0., 0.
            }
        },
        { // pi+.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        },
        { // pi-.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        }
    },
    { // nu.
        { // e-.
            { // DC.
                0.00010852, 0.00019020, 0.00028107, 0.00037366, 0.00046048,
                0.00051410, 0.00055791,
                0., 0., 0., 0., 0., 0.
            },
            { // FMT.
                0.00010855, 0.00018651, 0.00027407, 0.00036419, 0.00045136,
                0.00050481, 0.00054759,
                0., 0., 0., 0., 0., 0.
            }
        },
        { // pi+.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        },
        { // pi-.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        }
    },
    { // zh.
        { // e-.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        },
        { // pi+.
            { // DC.
                0.00007019, 0.00014539, 0.00020205, 0.00024355, 0.00028487,
                0.00031971, 0.00035326, 0.00040572, 0.00052147, 0.00050985,
                0., 0., 0.
            },
            { // FMT.
                0.00005094, 0.00012542, 0.00018579, 0.00022840, 0.00027029,
                0.00030666, 0.00034098, 0.00039445, 0.00050593, 0.00049740,
                0., 0., 0.
            }
        },
        { // pi-.
            { // DC.
                0.00003178, 0.00007974, 0.00013615, 0.00018274, 0.00023378,
                0.00028402, 0.00034045, 0.00039971, 0.00051660, 0.00053644,
                0., 0., 0.
            },
            { // FMT.
                0.00001335, 0.00005463, 0.00011300, 0.00016161, 0.00021383,
                0.00026328, 0.00031954, 0.00037908, 0.00049705, 0.00052889,
                0., 0., 0.
            }
        }
    },
    { // pt2.
        { // e-.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        },
        { // pi+.
            { // DC.
                0.00007044, 0.00019256, 0.00030165, 0.00036507, 0.00050514,
                0.00076448, 0.00196878, 0.00698582, 0.02519470, 0.05842727,
                0., 0., 0.
            },
            { // FMT.
                0.00006252, 0.00017128, 0.00026502, 0.00031973, 0.00043270,
                0.00060436, 0.00152017, 0.00543610, 0.01587725, 0.01904496,
                0., 0., 0.
            }
        },
        { // pi-.
            { // DC.
                0.00003673, 0.00017637, 0.00028907, 0.00038762, 0.00053033,
                0.00080103, 0.00147620, 0.00413169, 0.01819378, 0.03345943,
                0., 0., 0.
            },
            { // FMT.
                0.00002828, 0.00014467, 0.00023933, 0.00031454, 0.00043790,
                0.00064270, 0.00118605, 0.00315004, 0.01389030, 0.02774309,
                0., 0., 0.
            }
        }
    },
    { // phipq.
        { // e-.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // DC.
            {0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.}, // FMT.
        },
        { // pi+.
            { // DC.
                0.00023735, 0.00024303, 0.00024260, 0.00023726, 0.00021399,
                0.00017222, 0.00017125, 0.00019609, 0.00023373, 0.00024192,
                0.00025099, 0.00025309, 0.00024343
            },
            { // FMT.
                0.00020549, 0.00021172, 0.00021434, 0.00021074, 0.00019147,
                0.00015437, 0.00015386, 0.00017407, 0.00021076, 0.00021656,
                0.00022536, 0.00022580, 0.00021165
            }
        },
        { // pi-.
            { // DC.
                0.00018391, 0.00017122, 0.00015650, 0.00013928, 0.00010835,
                0.00008388, 0.00006758, 0.00007270, 0.00010176, 0.00013158,
                0.00015546, 0.00016455, 0.00017614
            },
            { // FMT.
                0.00015022, 0.00013620, 0.00012361, 0.00010577, 0.00007964,
                0.00005923, 0.00004797, 0.00005397, 0.00007825, 0.00010304,
                0.00012521, 0.00013008, 0.00014416
            }
        }
    }
};

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
            TGraphAsymmErrors *graphs[NFILES];
            double y_min = 1e20;
            double y_max = 1e-20;
            for (int plot_i = 0; plot_i < NFILES; ++plot_i) {
                // Create TGraphErrors from TH1 plots.
                graphs[plot_i] =
                    new TGraphAsymmErrors(plots[plot_i]->GetNbinsX());

                // Write data.
                for (
                        int bin_i = 1;
                        bin_i <= plots[plot_i]->GetNbinsX();
                        ++bin_i
                ) {
                    double x      = plots[plot_i]->GetBinCenter(bin_i);
                    double y      = plots[plot_i]->GetBinContent(bin_i);
                    double xerr   = plots[plot_i]->GetBinWidth(bin_i) / 2;
                    double offset = xerr / 4;
                    if (plot_i == 0) offset = -offset;

                    // Compute yerr.
                    int part_i = -1;
                    if (pid ==   11) part_i = 0;
                    if (pid ==  211) part_i = 1;
                    if (pid == -211) part_i = 2;

                    double yerr_meas = plots[plot_i]->GetBinError(bin_i);
                    double yerr_acc =
                        y * acccorr_err[canvas_idx][part_i][plot_i][bin_i-1];
                    double yerr = sqrt(yerr_meas*yerr_meas + yerr_acc*yerr_acc);

                    graphs[plot_i]->SetPoint(bin_i - 1, x + offset, y);
                    graphs[plot_i]->SetPointError(
                        bin_i - 1, xerr + offset, xerr - offset, yerr, yerr
                    );
                }

                // Set the title and axis labels of the TGraphErrors object.
                if (plot_i == 0)
                    graphs[plot_i]->SetTitle(Form(
                        "v_{z}: %s", bin_name
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
                graphs[plot_i]->SetMarkerSize(0.5);

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
                    graphs[0]->GetYaxis()->SetMoreLogLabels();
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

    // // Write to file.
    // TFile *file_out = TFile::Open(OUTPUT_FILENAME, "RECREATE");
    // for (TCanvas *canvas : canvases) {
    //     canvas->Write();
    // }

    // Clean up.
    for (int file_i = 0; file_i < NFILES; ++file_i) {
        in_files[file_i]->Close();
    }
    // file_out->Close();
    printf("Done!\n");

    return 0;
}
