int theta_gcut_effect() {
    // Open input files.
    TFile *files[2][2];
    files[0][0] = TFile::Open("../root_io/eff/study/pid11_dc_raw.root");
    files[0][1] = TFile::Open("../root_io/eff/study/pid11_dc_gcut.root");
    files[1][0] = TFile::Open("../root_io/eff/study/pid11_fmt2_raw.root");
    files[1][1] = TFile::Open("../root_io/eff/study/pid11_fmt2_gcut.root");

    // Create and setup canvas.
    TCanvas *canvas = new TCanvas("gcut effect", "gcut effect", 1600, 900);

    canvas->Divide(2, 1);

    // Get DC and FMT2 plots.
    for (int det_i = 0; det_i < 2; ++det_i) {
        TPad *pad = (TPad *) canvas->cd(det_i + 1);
        canvas->SetGrid();

        // Adjust the pad margins.
        if (det_i == 0) pad->SetMargin(0.05,   0.0025, 0.08, 0.0);
        else            pad->SetMargin(0.0025, 0.0025, 0.08, 0.0);

        // Get TH1Fs from files.
        TH1F *plots[2];
        for (int ver_i = 0; ver_i < 2; ++ver_i) {
            plots[ver_i] = (TH1F *) files[det_i][ver_i]->Get("#phi (rad)");
            plots[ver_i]->SetStats(0);
        }
        plots[0]->SetLineColor(kRed);
        plots[1]->SetLineColor(kBlue);

        plots[0]->SetTitle(Form("e- detected by %s", det_i == 0 ? "DC" : "FMT"));

        plots[0]->Draw();
        plots[1]->Draw("SAME");

        TLegend *legend = new TLegend(0.7, 0.7, 0.886, 0.88);
        legend->AddEntry(plots[0], "Raw", "l");
        legend->AddEntry(plots[1], "Geometry-corrected", "l");
        legend->Draw();

        canvas->Update();
    }

    // Write to file.
    TFile *file_out = TFile::Open("../root_io/eff/theta_gcut_effect.root", "RECREATE");
    canvas->Write();

    for (int det_i = 0; det_i < 2; ++det_i) {
        for (int ver_i = 0; ver_i < 2; ++ver_i) {
            files[det_i][ver_i]->Close();
        }
    }
    file_out->Close();

    return 0;
}
