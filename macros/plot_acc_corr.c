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
const int PID = 211;

// acc_corr.txt files produced by acc_corr.
const char *DC_FILENAME   = "../data/acc_corr_dc_raw.txt";
const char *FMT2_FILENAME = "../data/acc_corr_fmt2_raw.txt";
const char *FMT3_FILENAME = "../data/acc_corr_fmt3_raw.txt";

// Root file where we'll write the plots.
const char *OUTPUT_FILENAME = Form(
        "../root_io/simul/raw/acc_corr/pid%d.root", PID
);

// Map containing the variables we're working with.
const int NPLOTS = 5;
const std::map<int, const char *> PLOT_NAMES {
    {0, "Q2"}, {1, "#nu"}, {2, "z_{h}"}, {3, "Pt2"}, {4, "#phi_{PQ}"}
};
const int CANVAS_YSCALE[NPLOTS] = {1, 1, 1, 1, 1}; // 0 is linear, 1 is log.
// Colors.
const int color_thrown = kMagenta;
const int color_dc     = kRed;
const int color_fmt2   = kBlue;
const int color_fmt3   = kGreen;

// --- Macro code begins here ----------------------------------------------- //
/**
 * Read binning data from text file and fill binning sizes array, binnings
 *     array, and an array of PID list sizes. Copied from file_handler because
 *     tracking dependencies with ROOT is a bitch.
 */
int get_binnings(FILE *f_in, long int *b_sizes, double **binnings,
        long int *pids_size)
{
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
int get_acc_corr(FILE *f_in, long int pids_size, long int nbins,
        long int *pids, int **n_thrown, int **n_simul)
{
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
int read_acc_corr_file(char *acc_filename, long int b_sizes[5],
        double ***binnings, long int *pids_size, long int *nbins,
        long int **pids, int ***n_thrown, int ***n_simul)
{
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

/** Run the macro. */
int plot_acc_corr() {
    // printf("Running... ");
    // fflush(stdout);

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
    int **n_simul_dc;
    int **n_simul_fmt2;
    int **n_simul_fmt3;

    // Read DC acceptance correction.
    read_acc_corr_file(
            dc_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_simul_dc
    );

    // Free everything but n_simul_dc.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(n_thrown[pi]);
    free(n_thrown);

    // Read FMT 2 acceptance correction.
    read_acc_corr_file(
            fmt2_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_simul_fmt2
    );

    // Free everything but n_simul_fmt2.
    for (int bi = 0; bi < 5; ++bi) free(binnings[bi]);
    free(binnings);
    free(pids);
    for (int pi = 0; pi < pids_size; ++pi) free(n_thrown[pi]);
    free(n_thrown);

    // Read FMT 3 acceptance correction.
    read_acc_corr_file(
            fmt3_filename, bs, &binnings, &pids_size, &nbins, &pids, &n_thrown,
            &n_simul_fmt3
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
        int y_simul_dc[bin_size - 1];
        int y_simul_fmt2[bin_size - 1];
        int y_simul_fmt3[bin_size - 1];
        double y_err[bin_size - 1];
        for (int bii = 0; bii < bin_size - 1; ++bii) {
            x_pos[bii]    = (binnings[var_idx][bii+1]+binnings[var_idx][bii])/2;
            x_length[bii] = (binnings[var_idx][bii+1]-binnings[var_idx][bii])/2;
            y_thrown[bii] = 0;
            y_simul_dc[bii]   = 0;
            y_simul_fmt2[bii] = 0;
            y_simul_fmt3[bii] = 0;
            y_err[bii]        = 0.; // Dummy variable.
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
                            y_simul_dc[sel_idx] +=
                                    n_simul_dc[pid_pos][bin_pos];
                            y_simul_fmt2[sel_idx] +=
                                    n_simul_fmt2[pid_pos][bin_pos];
                            y_simul_fmt3[sel_idx] +=
                                    n_simul_fmt3[pid_pos][bin_pos];
                        }
                    }
                }
            }
        }

        // Create a copy of n_thrown, n_simul_dc, and n_simul_fmt2 as doubles.
        double y_thrown_dbl[bin_size - 1];
        double y_simul_dc_dbl[bin_size - 1];
        double y_simul_fmt2_dbl[bin_size - 1];
        double y_simul_fmt3_dbl[bin_size - 1];
        double y_min = 1e20;
        double y_max = 1e-20;
        for (int bii = 0; bii < bin_size - 1; ++bii) {
            y_thrown_dbl[bii]     = (double) y_thrown[bii];
            y_simul_dc_dbl[bii]   = (double) y_simul_dc[bii];
            y_simul_fmt2_dbl[bii] = (double) y_simul_fmt2[bii];
            y_simul_fmt3_dbl[bii] = (double) y_simul_fmt3[bii];

            // Check for new minimum.
            if (y_thrown_dbl[bii]     < y_min && y_thrown_dbl[bii]     > 1e-20)
                y_min = y_thrown_dbl[bii];
            if (y_simul_dc_dbl[bii]   < y_min && y_simul_dc_dbl[bii]   > 1e-20)
                y_min = y_simul_dc_dbl[bii];
            if (y_simul_fmt2_dbl[bii] < y_min && y_simul_fmt2_dbl[bii] > 1e-20)
                y_min = y_simul_fmt2_dbl[bii];
            if (y_simul_fmt3_dbl[bii] < y_min && y_simul_fmt3_dbl[bii] > 1e-20)
                y_min = y_simul_fmt3_dbl[bii];

            // Check for new maximum.
            if (y_thrown_dbl[bii]     > y_max) y_max = y_thrown_dbl[bii];
            if (y_simul_dc_dbl[bii]   > y_max) y_max = y_simul_dc_dbl[bii];
            if (y_simul_fmt2_dbl[bii] > y_max) y_max = y_simul_fmt2_dbl[bii];
            if (y_simul_fmt3_dbl[bii] > y_max) y_max = y_simul_fmt3_dbl[bii];
        }

        // Write results to plots.
        canvases.at(var_idx)->cd();
        canvases.at(var_idx)->SetGrid();
        gStyle->SetOptTitle(0);

        // Rescale y axis to fit both DC and FMT data.
        double min = 0;
        double max = 1.1 * y_max;
        if (CANVAS_YSCALE[var_idx] == 1) {
            min = y_min / sqrt(10);
            max = y_max * sqrt(10);
        }

        // Set y scale to log when needed.
        if (CANVAS_YSCALE[var_idx] == 1) {
            canvases.at(var_idx)->SetLogy();
        }

        // Write TGraphErrors for thrown events.
        TGraphErrors *graph_thrown = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_thrown_dbl, x_length, y_err
        );
        graph_thrown->SetTitle(Form("%s (thrown)", PLOT_NAMES.at(var_idx)));
        graph_thrown->SetMarkerColor(color_thrown);
        graph_thrown->SetMarkerStyle(21);
        graph_thrown->GetYaxis()->SetRangeUser(min, max);
        graph_thrown->GetXaxis()->SetTitle(PLOT_NAMES.at(var_idx));
        graph_thrown->Draw("AP");

        // Write TGraphErrors for DC events.
        TGraphErrors *graph_simul_dc = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_simul_dc_dbl, x_length, y_err
        );
        graph_simul_dc->SetTitle(
                Form("%s (simul - DC)", PLOT_NAMES.at(var_idx))
        );
        graph_simul_dc->SetMarkerColor(color_dc);
        graph_simul_dc->SetMarkerStyle(21);
        graph_simul_dc->GetYaxis()->SetRangeUser(min, max);
        graph_simul_dc->Draw("sameP");

        // Write TGraphErrors for FMT2 events.
        TGraphErrors *graph_simul_fmt2 = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_simul_fmt2_dbl, x_length, y_err
        );
        graph_simul_fmt2->SetTitle(
                Form("%s (simul - FMT2)", PLOT_NAMES.at(var_idx))
        );
        graph_simul_fmt2->SetMarkerColor(color_fmt2);
        graph_simul_fmt2->SetMarkerStyle(21);
        graph_simul_fmt2->GetYaxis()->SetRangeUser(min, max);
        graph_simul_fmt2->Draw("sameP");

        // Write TGraphErrors for FMT3 events.
        TGraphErrors *graph_simul_fmt3 = new TGraphErrors(
                bs[var_idx]-1, x_pos, y_simul_fmt3_dbl, x_length, y_err
        );
        graph_simul_fmt3->SetTitle(
                Form("%s (simul - FMT3)", PLOT_NAMES.at(var_idx))
        );
        graph_simul_fmt3->SetMarkerColor(color_fmt3);
        graph_simul_fmt3->SetMarkerStyle(21);
        graph_simul_fmt3->GetYaxis()->SetRangeUser(min, max);
        graph_simul_fmt3->Draw("sameP");

        // Add legend.
        TLegend* legend = new TLegend(0.7, 0.7, 0.886, 0.88);
        legend->AddEntry(graph_thrown, "thrown", "lp");
        legend->AddEntry(graph_simul_dc, "simul (DC)", "lp");
        legend->AddEntry(graph_simul_fmt2, "simul (FMT2)", "lp");
        legend->AddEntry(graph_simul_fmt3, "simul (FMT3)", "lp");
        legend->Draw();

        // Add title.
        TPaveLabel *pavel_label = new TPaveLabel(
                0.6, 0.9, 0.3, 1.0, PLOT_NAMES.at(var_idx), "brNDC");
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
        free(n_simul_dc[pi]);
        free(n_simul_fmt2[pi]);
    }
    free(n_thrown);
    free(n_simul_dc);
    free(n_simul_fmt2);
    file_out->Close();
    printf("Done!\n");

    return 0;
}
