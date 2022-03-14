#include <cstdlib>
#include <iostream>

#include "reader.h"
#include "writer.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

int main(int argc, char** argv) {
    char infile[512];
    char outfile_root[512];
    char outfile_hipo[512];

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
    TTree *tree = new TTree("clas12", "CLAS12 ROOT Tree");

    // REC::Particle.
    std::vector<Int_t>    part_pid;     tree->Branch("pid",     /*"REC::Particle",*/ &part_pid);
    std::vector<Float_t>  part_px;      tree->Branch("px",      /*"REC::Particle",*/ &part_px);
    std::vector<Float_t>  part_py;      tree->Branch("py",      /*"REC::Particle",*/ &part_py);
    std::vector<Float_t>  part_pz;      tree->Branch("pz",      /*"REC::Particle",*/ &part_pz);
    std::vector<Float_t>  part_vx;      tree->Branch("vx",      /*"REC::Particle",*/ &part_vx);
    std::vector<Float_t>  part_vy;      tree->Branch("vy",      /*"REC::Particle",*/ &part_vy);
    std::vector<Float_t>  part_vz;      tree->Branch("vz",      /*"REC::Particle",*/ &part_vz);
    std::vector<Float_t>  part_vt;      tree->Branch("vt",      /*"REC::Particle",*/ &part_vt);
    std::vector<Char_t>   part_charge;  tree->Branch("charge",  /*"REC::Particle",*/ &part_charge);
    std::vector<Float_t>  part_beta;    tree->Branch("beta",    /*"REC::Particle",*/ &part_beta);
    std::vector<Float_t>  part_chi2pid; tree->Branch("chi2pid", /*"REC::Particle",*/ &part_chi2pid);
    std::vector<Short_t>  part_status;  tree->Branch("status",  /*"REC::Particle",*/ &part_status);

    // REC::Track.
    std::vector<Short_t>  trk_index;   tree->Branch("index",  /*"REC::Track",*/ &trk_index);
    std::vector<Short_t>  trk_pindex;  tree->Branch("pindex", /*"REC::Track",*/ &trk_pindex);
    std::vector<Short_t>  trk_sector;  tree->Branch("sector", /*"REC::Track",*/ &trk_sector);
    std::vector<Short_t>  trk_ndf;     tree->Branch("NDF",    /*"REC::Track",*/ &trk_ndf);
    std::vector<Float_t>  trk_chi2;    tree->Branch("chi2",   /*"REC::Track",*/ &trk_chi2);

    // REC::Calorimeter.
    std::vector<Short_t> cal_pindex;   tree->Branch("pindex", /*"REC::Calorimeter",*/ &cal_pindex);
    std::vector<Char_t>  cal_layer;    tree->Branch("layer",  /*"REC::Calorimeter",*/ &cal_layer);
    std::vector<Float_t> cal_energy;   tree->Branch("energy", /*"REC::Calorimeter",*/ &cal_energy);

    // REC::Scintillator.

    // FMT::Track.

    // Prepare infile reader.
    hipo::reader reader;
    reader.open(infile);

    // Prepare factory.
    hipo::dictionary factory;
    reader.readDictionary(factory);

    // Prepare outfile writer.
    hipo::writer writer;
    writer.getDictionary().addSchema(factory.getSchema("REC::Particle"));
    writer.getDictionary().addSchema(factory.getSchema("REC::Track"));
    writer.getDictionary().addSchema(factory.getSchema("REC::Calorimeter"));
    writer.open(outfile_hipo);

    hipo::bank  rec_part(factory.getSchema("REC::Particle"));
    hipo::bank  rec_trk (factory.getSchema("REC::Track"));
    hipo::bank  rec_cal (factory.getSchema("REC::Calorimeter"));
    hipo::event event;

    while (reader.next()) {
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

        if (part_nrows > 0 || trk_nrows > 0 || cal_nrows > 0) tree->Fill();
        writer.addEvent(event);
    }

    f->Close();
    writer.close();

    return 0;
}
