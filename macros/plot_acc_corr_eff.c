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
// Set to the PID to plot acceptance correction from.
const int PID = -211;

// acc_corr.txt files produced by acc_corr.
const char *DC_FILENAME   = "../data/acc_corr_dc.txt";
const char *FMT2_FILENAME = "../data/acc_corr_fmt2.txt";
const char *FMT3_FILENAME = "../data/acc_corr_fmt3.txt";

// Root file where we'll write the plots.
const char *OUTPUT_FILENAME = Form(
    "../root_io/simul/geomcut/acc_corr/pid%d.root", PID
);

// Map containing the variables we're working with.
const int NPLOTS = 5;
const std::map<int, const char *> PLOT_NAMES {
    {0, "Q^{2} (GeV^{2})"},
    {1, "#nu"},
    {2, "z_{h}"},
    {3, "P_{T}^{2} (GeV^{2})"},
    {4, "#phi_{PQ} (rad)"}
};

// Colors.
const int color_dc   = kRed;
const int color_fmt2 = kBlue;
const int color_fmt3 = kGreen;

// --- Macro code begins here ----------------------------------------------- //
/**
 * Read binning data from text file and fill binning sizes array, binnings
 *     array, and an array of PID list sizes. Copied from file_handler because
 *     tracking dependencies with ROOT is a bitch.
 */
int get_binnings(
        FILE *f_in, long int *b_sizes, double **binnings, long int *pids_size
) {
    // Get binning sizes.
    for (int bi = 0; bi < 5; ++bi) fscanf(f_in, "%ld ", &(b_sizes[bi]));

    // Get binnings.
    for (int bi = 0; bi < 5; ++bi) {
        binnings[bi] = (double *) malloc(b_sizes[bi] * sizeof(*binnings[bi]));
        for (int bii = 0; bii < b_sizes[bi]; ++bii)
            fscanf(f_in, "%lf ", &(binnings[bi][bii]));
    }

    // Get # of pids.
    fscanf(f_in, "%ld", pids_size);

    return 0;
}

/**
 * Read acceptance correction data from text file and fill PIDs list and
 *     accceptance correction array. Copied from file_handler because tracking
 *     dependencies with ROOT is a bitch.
 */
int get_acc_corr(
        FILE *f_in, long int pids_size, long int nbins, long int *pids,
        int **n_thrown, int **n_simul
) {
    // Get PIDs.
    for (int pi = 0; pi < pids_size; ++pi) fscanf(f_in, "%ld ", &(pids[pi]));

    // Get acceptance correction.
    for (int pi = 0; pi < pids_size; ++pi) {
        // Get number of thrown events.
        n_thrown[pi] = (int *) malloc(nbins * sizeof(*n_thrown[pi]));
        for (int bii = 0; bii < nbins; ++bii)
            fscanf(f_in, "%d ", &(n_thrown[pi][bii]));

        // Get number of simulated events.
        n_simul[pi]  = (int *) malloc(nbins * sizeof(*n_simul[pi]));
        for (int bii = 0; bii < nbins; ++bii)
            fscanf(f_in, "%d ", &(n_simul[pi][bii]));
    }

    return 0;
}

/**
 * Read acc_corr.txt file to get the acceptance correction for each bin for each
 *     PID. Copied from file_handler because tracking dependencies with ROOT is
 *     a bitch.
 */
int read_acc_corr_file(
        char *acc_filename, long int b_sizes[5], double ***binnings,
        long int *pids_size, long int *nbins, long int **pids, int ***n_thrown,
        int ***n_simul
) {
    // Access file.
    if (access(acc_filename, F_OK) != 0) return 1;
    FILE *acc_file = fopen(acc_filename, "r");

    // Get b_sizes, binnings, and pids_size.
    *binnings = (double **) malloc(5 * sizeof(**binnings));
    get_binnings(acc_file, b_sizes, *binnings, pids_size);

    // Compute total number of bins.
    *nbins = 1;
    for (int bi = 0; bi < 5; ++bi) *nbins *= b_sizes[bi] - 1;

    // Malloc list of pids and first dimension of pids and events.
    *pids = (long int *) malloc(*pids_size * sizeof(**pids));
    *n_thrown = (int **) malloc(*pids_size * sizeof(**n_thrown));
    *n_simul  = (int **) malloc(*pids_size * sizeof(**n_simul));

    // Get pids and acc_corr from acceptance correction file.
    get_acc_corr(acc_file, *pids_size, *nbins, *pids, *n_thrown, *n_simul);

    // Clean up.
    fclose(acc_file);

    return 0;
}

/** Add TCanvas with name n to std::vector<TCanvas *> c. */
int add_tcanvas(std::vector<TCanvas *> *c, const char *n) {
    c->push_back(new TCanvas(n, n, 1600, 900));
    return 0;
}

/** Copy const char * to char *. */
int copy_filename(char *tgt, const char *src) {
    for (int i = 0; i <= strlen(src); ++i) {
        if (i == strlen(src)) tgt[i] = '\0';
        else                  tgt[i] = src[i];
    }

    return 0;
}

/** Divide two TGraphErrors assuming a simple error propagation. */
TGraphAsymmErrors *divide_TGraphErrors(
        TGraphErrors *graph1, TGraphErrors *graph2, double offset
) {
    int graph_size = graph1->GetN();

    // Create new arrays to store the divided values.
    double x_divided[graph_size];
    double y_divided[graph_size];
    double ex_divided[graph_size];
    double ey_divided[graph_size];

    // Divide the values of the corresponding points from graph1 and graph2.
    for (int gi = 0; gi < graph_size; ++gi) {
        double x1, y1, x2, y2;
        graph1->GetPoint(gi, x1, y1);
        graph2->GetPoint(gi, x2, y2);

        double ex1 = graph1->GetErrorX(gi);
        double ey1 = graph1->GetErrorY(gi);
        double ex2 = graph2->GetErrorX(gi);
        double ey2 = graph2->GetErrorY(gi);

        // Divide the values.
        x_divided [gi] = x1;
        ex_divided[gi] = ex1;

        // If y1 or y2 == 0, its better to set everything to 0.
        if (y1 < 1e-20 || y2 < 1e-20) {
            y_divided[gi]  = 0;
            ey_divided[gi] = 0;
            continue;
        }

        y_divided[gi] = y1 / y2;
        ey_divided[gi] = (y1 / y2) *
                sqrt((ey1 / y1) * (ey1 / y1) + (ey2 / y2) * (ey2 / y2));
    }

    // Create a new TGraphErrors with the divided values.
    TGraphAsymmErrors *graph_divided = new TGraphAsymmErrors(
            graph_size, x_divided, y_divided,
            ex_divided, ex_divided,
            ey_divided, ey_divided
    );

    // Adjust the x error value to move the marker within the bin
    for (int i = 0; i < graph_size; ++i) {
        graph_divided->SetPoint(i, x_divided[i] + offset, y_divided[i]);

        // Adjust the x error by an offset within the bin
        graph_divided->SetPointError(
                i, ex_divided[i] + offset, ex_divided[i] - offset,
                ey_divided[i], ey_divided[i]
        );
    }

    return graph_divided;
}

/** Get the maximum y value from a TGraphErrors. */
double get_max(TGraphAsymmErrors *graph) {
    double y_max = 1e-20;
    int graph_size = graph->GetN(); // Get the number of points in the graph.

    // Iterate over each point and find the maximum y value
    for (int gi = 0; gi < graph_size; ++gi) {
        double x, y, ey;
        graph->GetPoint(gi, x, y);
        ey = graph->GetErrorY(gi);
        if (y + ey > y_max) y_max = y + ey;
    }

    return y_max;
}

void drawtext() {
    TLatex text;

    text.SetTextSize(0.025);
    text.SetTextFont(42);
    text.SetTextAlign(21);
    // text.SetTextColor(kBlue);

    TGraph *graph = (TGraph *) gPad->GetListOfPrimitives()->FindObject("Graph");
    int graph_size = graph->GetN();

    for (int gi = 0; gi < graph_size; gi++) {
        double x, y;
        graph->GetPoint(gi, x, y);
        text.PaintText(x, y+0.02, Form("%4.2f", y));
    }
}

/** Run the macro. */
int plot_acc_corr_eff() {
    // Create an executable for drawtext().
    // TExec *ex = new TExec("ex", "drawtext();");

    // Copy filenames to char *.
    char dc_filename[128];
    char fmt2_filename[128];
    char fmt3_filename[128];
    copy_filename(dc_filename,   DC_FILENAME);
    copy_filename(fmt2_filename, FMT2_FILENAME);
    copy_filename(fmt3_filename, FMT3_FILENAME);

    long int bs[5];
    double **binnings;
    long int pids_size;
    long int nbins;
    long int *pids;
    int **n_thrown;
    int **n_dc;
    int **n_fmt2;
    int **n_fmt3;

    // Read DC acceptance correction.
    read_acc_corr_file(
            dc_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_dc
    );

    // Free everything but n_dc.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(n_thrown[pi]);
    free(n_thrown);

    // Read FMT 2 acceptance correction.
    read_acc_corr_file(
            fmt2_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_fmt2
    );

    // Free everything but n_fmt2.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(n_thrown[pi]);
    free(n_thrown);

    // Read FMT 3 acceptance correction.
    read_acc_corr_file(
            fmt3_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_fmt3
    );

    // Get place of PID in *pids.
    int pid_pos = -1;
    for (int pi = 0; pi < pids_size; ++pi) if (PID == pids[pi]) pid_pos = pi;
    if (pid_pos == -1) {
        printf("\nPID %d not found in %s! Exiting...\n", PID, dc_filename);
        return 1;
    }

    // Create TCanvases.
    std::vector<TCanvas *> canvases;
    for (int i = 0; i < NPLOTS; ++i) add_tcanvas(&canvases, PLOT_NAMES.at(i));

    // Count variables and add them to the corresponding TGraphErrors.
    for (int var_idx = 0; var_idx < NPLOTS; ++var_idx) {
        // var_idx represents the variable we're processing, in the order
        //     defined in PLOT_NAMES[].
        long int bin_size = bs[var_idx];

        // Define x and y.
        double x_pos[bin_size - 1];
        double x_length[bin_size - 1];
        int y_thrown[bin_size - 1];
        int y_dc[bin_size - 1];
        int y_fmt2[bin_size - 1];
        int y_fmt3[bin_size - 1];
        double y_err[bin_size - 1];
        for (int bii = 0; bii < bin_size - 1; ++bii) {
            x_pos[bii]    = (binnings[var_idx][bii+1]+binnings[var_idx][bii])/2;
            x_length[bii] = (binnings[var_idx][bii+1]-binnings[var_idx][bii])/2;
            y_thrown[bii] = 0;
            y_dc[bii]   = 0;
            y_fmt2[bii] = 0;
            y_fmt3[bii] = 0;
        }

        // Fill y.
        for (int i0 = 0; i0 < bs[0]-1; ++i0) {
            for (int i1 = 0; i1 < bs[1]-1; ++i1) {
                for (int i2 = 0; i2 < bs[2]-1; ++i2) {
                    for (int i3 = 0; i3 < bs[3]-1; ++i3) {
                        for (int i4 = 0; i4 < bs[4]-1; ++i4) {
                            // Find 1D bin position from 5 indices.
                            int bin_pos =
                                i0 * ((bs[1]-1)*(bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                                i1 * ((bs[2]-1)*(bs[3]-1)*(bs[4]-1)) +
                                i2 * ((bs[3]-1)*(bs[4]-1)) +
                                i3 * ((bs[4]-1)) +
                                i4;

                            // Find which ID should be updated.
                            int sel_idx;
                            switch(var_idx) {
                                case 0: sel_idx = i0; break;
                                case 1: sel_idx = i1; break;
                                case 2: sel_idx = i2; break;
                                case 3: sel_idx = i3; break;
                                case 4: sel_idx = i4; break;
                            }

                            // Increment appropriate counters.
                            y_thrown[sel_idx] += n_thrown[pid_pos][bin_pos];
                            y_dc[sel_idx] +=
                                    n_dc[pid_pos][bin_pos];
                            y_fmt2[sel_idx] +=
                                    n_fmt2[pid_pos][bin_pos];
                            y_fmt3[sel_idx] +=
                                    n_fmt3[pid_pos][bin_pos];
                        }
                    }
                }
            }
        }

        // Create a copy of data as doubles.
        double y_thrown_dbl[bin_size - 1];
        double y_dc_dbl[bin_size - 1];
        double y_fmt2_dbl[bin_size - 1];
        double y_fmt3_dbl[bin_size - 1];

        // Store the errors of each.
        double y_thrown_err[bin_size - 1];
        double y_dc_err[bin_size - 1];
        double y_fmt2_err[bin_size - 1];
        double y_fmt3_err[bin_size - 1];

        for (int bii = 0; bii < bin_size - 1; ++bii) {
            y_thrown_dbl[bii] = (double) y_thrown[bii];
            y_dc_dbl[bii]     = (double) y_dc[bii];
            y_fmt2_dbl[bii]   = (double) y_fmt2[bii];
            y_fmt3_dbl[bii]   = (double) y_fmt3[bii];

            y_thrown_err[bii] = sqrt(y_thrown_dbl[bii]);
            y_dc_err[bii]     = sqrt(y_dc_dbl[bii]);
            y_fmt2_err[bii]   = sqrt(y_fmt2_dbl[bii]);
            y_fmt3_err[bii]   = sqrt(y_fmt3_dbl[bii]);
        }

        // Store maximum y value.
        double y_max = 1e-20;

        // Store x offset for assymetric error graph.
        double offset = x_length[0]/4;

        // Setup plot.
        canvases.at(var_idx)->cd();
        canvases.at(var_idx)->SetGrid();
        gStyle->SetOptTitle(0);

        // Write TGraphErrors for thrown events.
        TGraphErrors *graph_thrown = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_thrown_dbl, x_length, y_thrown_err
        );

        // Write TGraphErrors for DC events.
        TGraphErrors *graph_dc = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_dc_dbl, x_length, y_dc_err
        );
        TGraphAsymmErrors *graph_eff_dc =
                divide_TGraphErrors(graph_dc, graph_thrown, -offset);
        graph_eff_dc->SetMarkerColor(color_dc);
        graph_eff_dc->SetMarkerStyle(21);
        double dc_max = get_max(graph_eff_dc);
        y_max = dc_max > y_max ? dc_max : y_max;

        // Write TGraphErrors for FMT2 events.
        TGraphErrors *graph_fmt2 = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_fmt2_dbl, x_length, y_fmt2_err
        );
        TGraphAsymmErrors *graph_eff_fmt2 =
                divide_TGraphErrors(graph_fmt2, graph_thrown, 0);
        graph_eff_fmt2->SetMarkerColor(color_fmt2);
        graph_eff_fmt2->SetMarkerStyle(21);
        double fmt2_max = get_max(graph_eff_fmt2);
        y_max = fmt2_max > y_max ? fmt2_max : y_max;

        // Write TGraphErrors for FMT3 events.
        TGraphErrors *graph_fmt3 = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_fmt3_dbl, x_length, y_fmt3_err
        );
        TGraphAsymmErrors *graph_eff_fmt3 =
                divide_TGraphErrors(graph_fmt3, graph_thrown, offset);
        graph_eff_fmt3->SetMarkerColor(color_fmt3);
        graph_eff_fmt3->SetMarkerStyle(21);
        double fmt3_max = get_max(graph_eff_fmt3);
        y_max = fmt3_max > y_max ? fmt3_max : y_max;

        // Set conditions for plot.
        graph_eff_dc->GetYaxis()->SetRangeUser(0, 1.1*y_max);
        graph_eff_dc->GetXaxis()->SetTitle(PLOT_NAMES.at(var_idx));

        // Draw labels on top of each bin.
        // graph_eff_dc->GetListOfFunctions()->Add(ex);

        // Draw.
        graph_eff_dc  ->Draw("AP");
        graph_eff_fmt2->Draw("sameP");
        graph_eff_fmt3->Draw("sameP");

        // Add legend.
        TLegend* legend = new TLegend(0.7, 0.7, 0.886, 0.88);
        legend->AddEntry(graph_eff_dc,   "DC",             "lp");
        legend->AddEntry(graph_eff_fmt2, "FMT - 2 layers", "lp");
        legend->AddEntry(graph_eff_fmt3, "FMT - 3 layers", "lp");
        legend->Draw();

        // Add title.
        TPaveLabel *pavel_label = new TPaveLabel(
                0.6, 0.9, 0.3, 1.0,
                Form("%s efficiency", PLOT_NAMES.at(var_idx)), "brNDC"
        );
        pavel_label->SetBorderSize(0);
        pavel_label->SetFillColor(gStyle->GetTitleFillColor());
        pavel_label->Draw();

        canvases.at(var_idx)->Update();
    }

    // Write to file.
    TFile *file_out = TFile::Open(OUTPUT_FILENAME, "RECREATE");
    for (TCanvas *canvas : canvases) canvas->Write();

    // Clean up after ourselves.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) {
        free(n_thrown[pi]);
        free(n_dc[pi]);
        free(n_fmt2[pi]);
        free(n_fmt3[pi]);
    }
    free(n_thrown);
    free(n_dc);
    free(n_fmt2);
    free(n_fmt3);
    file_out->Close();
    printf("Done!\n");

    return 0;
}
