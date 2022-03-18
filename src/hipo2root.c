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

    if (argc > 2) {
        nevents = atoi(argv[2]);
    }
    if (argc > 1) {
        // TODO. Remove extension.
        sprintf(infile, "%s", argv[1]);
        sprintf(outfile_root, "%s.root", argv[1]);
    }
    else {
        printf("Error. No file name provided. Exiting...\n");
        exit(0);
    }

    TFile *f = TFile::Open(outfile_root, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    TTree *tree = new TTree("Tree", "Tree");
    int nevent = 0;
    tree->Branch("event", &nevent);

    // REC::Particle.
    std::vector<Int_t>    rp_pid;     tree->Branch("REC::Particle::pid",     &rp_pid);
    std::vector<Float_t>  rp_px;      tree->Branch("REC::Particle::px",      &rp_px);
    std::vector<Float_t>  rp_py;      tree->Branch("REC::Particle::py",      &rp_py);
    std::vector<Float_t>  rp_pz;      tree->Branch("REC::Particle::pz",      &rp_pz);
    std::vector<Float_t>  rp_vx;      tree->Branch("REC::Particle::vx",      &rp_vx);
    std::vector<Float_t>  rp_vy;      tree->Branch("REC::Particle::vy",      &rp_vy);
    std::vector<Float_t>  rp_vz;      tree->Branch("REC::Particle::vz",      &rp_vz);
    std::vector<Float_t>  rp_vt;      tree->Branch("REC::Particle::vt",      &rp_vt);
    std::vector<Char_t>   rp_charge;  tree->Branch("REC::Particle::charge",  &rp_charge);
    std::vector<Float_t>  rp_beta;    tree->Branch("REC::Particle::beta",    &rp_beta);
    std::vector<Float_t>  rp_chi2pid; tree->Branch("REC::Particle::chi2pid", &rp_chi2pid);
    std::vector<Short_t>  rp_status;  tree->Branch("REC::Particle::status",  &rp_status);

    // REC::Track.
    std::vector<Short_t>  rt_index;   tree->Branch("REC::Track::index",  &rt_index);
    std::vector<Short_t>  rt_pindex;  tree->Branch("REC::Track::pindex", &rt_pindex);
    std::vector<Short_t>  rt_sector;  tree->Branch("REC::Track::sector", &rt_sector);
    std::vector<Short_t>  rt_ndf;     tree->Branch("REC::Track::ndf",    &rt_ndf);
    std::vector<Float_t>  rt_chi2;    tree->Branch("REC::Track::chi2",   &rt_chi2);

    // REC::Calorimeter.
    std::vector<Short_t>  rc_pindex;  tree->Branch("REC::Calorimeter::pindex", &rc_pindex);
    std::vector<Char_t>   rc_layer;   tree->Branch("REC::Calorimeter::layer",  &rc_layer);
    std::vector<Float_t>  rc_energy;  tree->Branch("REC::Calorimeter::energy", &rc_energy);

    // REC::Scintillator.
    std::vector<Short_t>  rs_pindex;  tree->Branch("REC::Scintillator::pindex", &rs_pindex);
    std::vector<Float_t>  rs_time;    tree->Branch("REC::Scintillator::time",   &rs_time);

    // FMT::Track.
    std::vector<Float_t>  ft_vx;      tree->Branch("FMT::Tracks::vx", &ft_vx);
    std::vector<Float_t>  ft_vy;      tree->Branch("FMT::Tracks::vy", &ft_vy);
    std::vector<Float_t>  ft_vz;      tree->Branch("FMT::Tracks::vz", &ft_vz);
    std::vector<Float_t>  ft_px;      tree->Branch("FMT::Tracks::px", &ft_px);
    std::vector<Float_t>  ft_py;      tree->Branch("FMT::Tracks::py", &ft_py);
    std::vector<Float_t>  ft_pz;      tree->Branch("FMT::Tracks::pz", &ft_pz);

    // Prepare infile reader.
    hipo::reader reader;
    reader.open(infile);

    // Prepare factory.
    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank  rp_b(factory.getSchema("REC::Particle"));
    hipo::bank  rt_b(factory.getSchema("REC::Track"));
    hipo::bank  rc_b(factory.getSchema("REC::Calorimeter"));
    hipo::bank  rs_b(factory.getSchema("REC::Scintillator"));
    hipo::bank  ft_b(factory.getSchema("FMT::Tracks"));
    hipo::event event;

    while (reader.next()) {
        if (nevent++ >= nevents && nevents != 0) break;
        reader.read(event);

        // REC::Particle
        event.getStructure(rp_b);
        int rp_nrows = rp_b.getRows();
        rp_pid    .resize(rp_nrows);
        rp_px     .resize(rp_nrows);
        rp_py     .resize(rp_nrows);
        rp_pz     .resize(rp_nrows);
        rp_vx     .resize(rp_nrows);
        rp_vy     .resize(rp_nrows);
        rp_vz     .resize(rp_nrows);
        rp_vt     .resize(rp_nrows);
        rp_beta   .resize(rp_nrows);
        rp_chi2pid.resize(rp_nrows);
        rp_charge .resize(rp_nrows);
        rp_status .resize(rp_nrows);
        for (int row = 0; row < rp_nrows; ++row) {
            rp_pid[row]     = rp_b.getInt  ("pid",     row);
            rp_px[row]      = rp_b.getFloat("px",      row);
            rp_py[row]      = rp_b.getFloat("py",      row);
            rp_pz[row]      = rp_b.getFloat("pz",      row);
            rp_vx[row]      = rp_b.getFloat("vx",      row);
            rp_vy[row]      = rp_b.getFloat("vy",      row);
            rp_vz[row]      = rp_b.getFloat("vz",      row);
            rp_vt[row]      = rp_b.getFloat("vt",      row);
            rp_beta[row]    = rp_b.getFloat("beta",    row);
            rp_chi2pid[row] = rp_b.getFloat("chi2pid", row);
            rp_charge[row]  = (int8_t)  rp_b.getByte ("charge", row);
            rp_status[row]  = (int16_t) rp_b.getShort("status", row);
        }

        // REC::Track
        event.getStructure(rt_b);
        int rt_nrows = rt_b.getRows();
        rt_index .resize(rt_nrows);
        rt_pindex.resize(rt_nrows);
        rt_sector.resize(rt_nrows);
        rt_ndf   .resize(rt_nrows);
        rt_chi2  .resize(rt_nrows);
        for (int row = 0; row < rt_nrows; ++row) {
            rt_index[row]  = (int16_t) rt_b.getShort("index", row);
            rt_pindex[row] = (int16_t) rt_b.getShort("pindex", row);
            rt_sector[row] = (int8_t)  rt_b.getByte("sector", row);
            rt_ndf[row]    = (int16_t) rt_b.getShort("NDF", row);
            rt_chi2[row]   = rt_b.getFloat("chi2", row);
        }

        // REC::Calorimeter
        event.getStructure(rc_b);
        int rc_nrows = rc_b.getRows();
        rc_pindex.resize(rc_nrows);
        rc_layer.resize(rc_nrows);
        rc_energy.resize(rc_nrows);
        for (int row = 0; row < rc_nrows; ++row) {
            rc_pindex[row] = (int16_t) rc_b.getShort("pindex", row);
            rc_layer[row]  = (int8_t)  rc_b.getByte("layer", row);
            rc_energy[row] = rc_b.getFloat("energy", row);
        }

        // REC::Scintillator
        event.getStructure(rs_b);
        int rs_nrows = rs_b.getRows();
        rs_pindex.resize(rs_nrows);
        rs_time.resize(rs_nrows);
        for (int row = 0; row < rs_nrows; ++row) {
            rs_pindex[row] = (int16_t) rs_b.getShort("pindex", row);
            rs_time[row]   = rs_b.getFloat("time", row);
        }

        // FMT::Tracks
        event.getStructure(ft_b);
        int ft_nrows = ft_b.getRows();
        ft_vx.resize(ft_nrows);
        ft_vy.resize(ft_nrows);
        ft_vz.resize(ft_nrows);
        ft_px.resize(ft_nrows);
        ft_py.resize(ft_nrows);
        ft_pz.resize(ft_nrows);
        for (int row = 0; row < ft_nrows; ++row) {
            ft_vx[row] = ft_b.getFloat("Vtx0_x", row);
            ft_vy[row] = ft_b.getFloat("Vtx0_y", row);
            ft_vz[row] = ft_b.getFloat("Vtx0_z", row);
            ft_px[row] = ft_b.getFloat("p0_x", row);
            ft_py[row] = ft_b.getFloat("p0_y", row);
            ft_pz[row] = ft_b.getFloat("p0_z", row);
        }

        if (rp_nrows > 0 || rt_nrows > 0 || rs_nrows > 0 || rc_nrows > 0 || ft_nrows > 0)
            tree->Fill();
    }

    f->Close();

    return 0;
}
