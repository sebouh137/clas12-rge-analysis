#include <cstdlib>
#include <iostream>

#include "reader.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

int main(int argc, char** argv) {
    int nevents = 0;
    char infile[512];
    char outfile_root[512];
    char outfile_hipo[512];

    if (argc > 2) {
        nevents = atoi(argv[2]);
    }
    if (argc > 1) {
        sprintf(infile, "%s", argv[1]);
        sprintf(outfile_root, "%s.root", argv[1]);
        sprintf(outfile_hipo, "%s_writer.hipo", argv[1]);
    }
    else {
        printf("Error. No file name provided. Exiting...\n");
        exit(0);
    }

    TFile *f = TFile::Open(outfile_root, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    // REC::Particle.
    TTree *part_tree = new TTree("REC::Particle", "REC::Particle");
    std::vector<Int_t>    part_pid;     part_tree->Branch("pid",     &part_pid);
    std::vector<Float_t>  part_px;      part_tree->Branch("px",      &part_px);
    std::vector<Float_t>  part_py;      part_tree->Branch("py",      &part_py);
    std::vector<Float_t>  part_pz;      part_tree->Branch("pz",      &part_pz);
    std::vector<Float_t>  part_vx;      part_tree->Branch("vx",      &part_vx);
    std::vector<Float_t>  part_vy;      part_tree->Branch("vy",      &part_vy);
    std::vector<Float_t>  part_vz;      part_tree->Branch("vz",      &part_vz);
    std::vector<Float_t>  part_vt;      part_tree->Branch("vt",      &part_vt);
    std::vector<Char_t>   part_charge;  part_tree->Branch("charge",  &part_charge);
    std::vector<Float_t>  part_beta;    part_tree->Branch("beta",    &part_beta);
    std::vector<Float_t>  part_chi2pid; part_tree->Branch("chi2pid", &part_chi2pid);
    std::vector<Short_t>  part_status;  part_tree->Branch("status",  &part_status);

    // REC::Track.
    TTree *trk_tree = new TTree("REC::Track", "REC::Track");
    std::vector<Short_t>  trk_index;   trk_tree->Branch("index",  &trk_index);
    std::vector<Short_t>  trk_pindex;  trk_tree->Branch("pindex", &trk_pindex);
    std::vector<Short_t>  trk_sector;  trk_tree->Branch("sector", &trk_sector);
    std::vector<Short_t>  trk_ndf;     trk_tree->Branch("NDF",    &trk_ndf);
    std::vector<Float_t>  trk_chi2;    trk_tree->Branch("chi2",   &trk_chi2);

    // REC::Calorimeter.
    TTree *cal_tree = new TTree("REC::Calorimeter", "REC::Calorimeter");
    std::vector<Short_t> cal_pindex;   cal_tree->Branch("pindex", &cal_pindex);
    std::vector<Char_t>  cal_layer;    cal_tree->Branch("layer",  &cal_layer);
    std::vector<Float_t> cal_energy;   cal_tree->Branch("energy", &cal_energy);

    // REC::Scintillator.
    TTree *sci_tree = new TTree("REC::Scintillator", "REC::Scintillator");
    std::vector<Short_t> tof_pindex;   sci_tree->Branch("pindex", &tof_pindex);
    std::vector<Float_t> tof_time;     sci_tree->Branch("time",   &tof_time);

    // FMT::Track.
    TTree *fmt_tree = new TTree("FMT::Track", "FMT::Track");
    std::vector<Float_t> fmt_vx;       fmt_tree->Branch("vx", &fmt_vx);
    std::vector<Float_t> fmt_vy;       fmt_tree->Branch("vy", &fmt_vy);
    std::vector<Float_t> fmt_vz;       fmt_tree->Branch("vz", &fmt_vz);
    std::vector<Float_t> fmt_px;       fmt_tree->Branch("px", &fmt_px);
    std::vector<Float_t> fmt_py;       fmt_tree->Branch("py", &fmt_py);
    std::vector<Float_t> fmt_pz;       fmt_tree->Branch("pz", &fmt_pz);

    // Prepare infile reader.
    hipo::reader reader;
    reader.open(infile);

    // Prepare factory.
    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank  rec_part(factory.getSchema("REC::Particle"));
    hipo::bank  rec_trk (factory.getSchema("REC::Track"));
    hipo::bank  rec_cal (factory.getSchema("REC::Calorimeter"));
    hipo::bank  rec_tof (factory.getSchema("REC::Scintillator"));
    hipo::bank  fmt_trk (factory.getSchema("FMT::Tracks"));
    hipo::event event;

    int cnt = 0;
    while (reader.next()) {
        if (nevents != 0 && cnt++ >= nevents) break;
        reader.read(event);

        // REC::Particle
        event.getStructure(rec_part);
        int part_nrows = rec_part.getRows();
        part_pid.resize(part_nrows);
        part_px.resize(part_nrows);
        part_py.resize(part_nrows);
        part_pz.resize(part_nrows);
        part_vx.resize(part_nrows);
        part_vy.resize(part_nrows);
        part_vz.resize(part_nrows);
        part_vt.resize(part_nrows);
        part_beta.resize(part_nrows);
        part_chi2pid.resize(part_nrows);
        part_charge.resize(part_nrows);
        part_status.resize(part_nrows);
        for (int row = 0; row < part_nrows; ++row) {
            part_pid[row]     = rec_part.getInt  ("pid",     row);
            part_px[row]      = rec_part.getFloat("px",      row);
            part_py[row]      = rec_part.getFloat("py",      row);
            part_pz[row]      = rec_part.getFloat("pz",      row);
            part_vx[row]      = rec_part.getFloat("vx",      row);
            part_vy[row]      = rec_part.getFloat("vy",      row);
            part_vz[row]      = rec_part.getFloat("vz",      row);
            part_vt[row]      = rec_part.getFloat("vt",      row);
            part_beta[row]    = rec_part.getFloat("beta",    row);
            part_chi2pid[row] = rec_part.getFloat("chi2pid", row);
            part_charge[row]  = (int8_t)  rec_part.getByte ("charge", row);
            part_status[row]  = (int16_t) rec_part.getShort("status", row);
        }
        if (part_nrows > 0) part_tree->Fill();

        // REC::Track
        event.getStructure(rec_trk);
        int trk_nrows = rec_trk.getRows();
        trk_index.resize(trk_nrows);
        trk_pindex.resize(trk_nrows);
        trk_sector.resize(trk_nrows);
        trk_ndf.resize(trk_nrows);
        trk_chi2.resize(trk_nrows);
        for (int row = 0; row < trk_nrows; ++row) {
            trk_index[row]  = (int16_t) rec_trk.getShort("index", row);
            trk_pindex[row] = (int16_t) rec_trk.getShort("pindex", row);
            trk_sector[row] = (int8_t)  rec_trk.getByte("sector", row);
            trk_ndf[row]    = (int16_t) rec_trk.getShort("NDF", row);
            trk_chi2[row]   = rec_trk.getFloat("chi2", row);
        }
        if (trk_nrows > 0) trk_tree->Fill();

        // REC::Calorimeter
        event.getStructure(rec_cal);
        int cal_nrows = rec_cal.getRows();
        cal_pindex.resize(cal_nrows);
        cal_layer.resize(cal_nrows);
        cal_energy.resize(cal_nrows);
        for (int row = 0; row < cal_nrows; ++row) {
            cal_pindex[row] = (int16_t) rec_cal.getShort("pindex", row);
            cal_layer[row]  = (int8_t)  rec_cal.getByte("layer", row);
            cal_energy[row] = rec_cal.getFloat("energy", row);
        }
        if (cal_nrows > 0) cal_tree->Fill();

        // REC::Scintillator
        event.getStructure(rec_tof);
        int tof_nrows = rec_tof.getRows();
        tof_pindex.resize(tof_nrows);
        tof_time.resize(tof_nrows);
        for (int row = 0; row < tof_nrows; ++row) {
            tof_pindex[row] = (int16_t) rec_tof.getShort("pindex", row);
            tof_time[row]   = rec_tof.getFloat("time", row);
        }
        if (tof_nrows > 0) sci_tree->Fill();

        // FMT::Tracks
        event.getStructure(fmt_trk);
        int fmt_nrows = fmt_trk.getRows();
        fmt_vx.resize(fmt_nrows);
        fmt_vy.resize(fmt_nrows);
        fmt_vz.resize(fmt_nrows);
        fmt_px.resize(fmt_nrows);
        fmt_py.resize(fmt_nrows);
        fmt_pz.resize(fmt_nrows);
        for (int row = 0; row < fmt_nrows; ++row) {
            fmt_vx[row] = fmt_trk.getFloat("Vtx0_x", row);
            fmt_vy[row] = fmt_trk.getFloat("Vtx0_y", row);
            fmt_vz[row] = fmt_trk.getFloat("Vtx0_z", row);
            fmt_px[row] = fmt_trk.getFloat("p0_x", row);
            fmt_py[row] = fmt_trk.getFloat("p0_y", row);
            fmt_pz[row] = fmt_trk.getFloat("p0_z", row);
        }
        if (fmt_nrows > 0) fmt_tree->Fill();
    }

    f->Close();

    return 0;
}
