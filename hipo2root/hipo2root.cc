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
    std::vector<Int_t>    vec_pid;     tree->Branch("pid",     &vec_pid);
    std::vector<Float_t>  vec_px;      tree->Branch("px",      &vec_px);
    std::vector<Float_t>  vec_py;      tree->Branch("py",      &vec_py);
    std::vector<Float_t>  vec_pz;      tree->Branch("pz",      &vec_pz);
    std::vector<Float_t>  vec_vx;      tree->Branch("vx",      &vec_vx);
    std::vector<Float_t>  vec_vy;      tree->Branch("vy",      &vec_vy);
    std::vector<Float_t>  vec_vz;      tree->Branch("vz",      &vec_vz);
    std::vector<Float_t>  vec_vt;      tree->Branch("vt",      &vec_vt);
    std::vector<Char_t>   vec_charge;  tree->Branch("charge",  &vec_charge);
    std::vector<Float_t>  vec_beta;    tree->Branch("beta",    &vec_beta);
    std::vector<Float_t>  vec_chi2pid; tree->Branch("chi2pid", &vec_chi2pid);
    std::vector<Short_t>  vec_status;  tree->Branch("status",  &vec_status);

    // REC::Track.
    std::vector<Int_t>    vec_index;   tree->Branch("index",   &vec_index);
    std::vector<Int_t>    vec_pindex;  tree->Branch("pindex",  &vec_pindex);
    std::vector<Int_t>    vec_sector;  tree->Branch("sector",  &vec_sector);
    std::vector<Int_t>    vec_ndf;     tree->Branch("NDF",     &vec_ndf);
    std::vector<Float_t>  vec_chi2;    tree->Branch("chi2",    &vec_chi2);

    // REC::Traj.

    // REC::Calorimeter.

    // REC::Scintillator.

    // FMT::rec_trk.


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
    writer.open(outfile_hipo);

    hipo::bank  rec_part(factory.getSchema("REC::Particle"));
    hipo::bank  rec_trk (factory.getSchema("REC::Track"));
    hipo::event event;

    while (reader.next()) {
        reader.read(event);

        // REC::Particle
        event.getStructure(rec_part);
        int part_nrows = rec_part.getRows();
        vec_pid.resize(part_nrows);
        vec_px.resize(part_nrows);
        vec_py.resize(part_nrows);
        vec_pz.resize(part_nrows);
        vec_vx.resize(part_nrows);
        vec_vy.resize(part_nrows);
        vec_vz.resize(part_nrows);
        vec_vt.resize(part_nrows);
        vec_beta.resize(part_nrows);
        vec_chi2pid.resize(part_nrows);
        vec_charge.resize(part_nrows);
        vec_status.resize(part_nrows);
        for (int row = 0; row < part_nrows; ++row) {
            vec_pid[row]     = rec_part.getInt  ("pid",     row);
            vec_px[row]      = rec_part.getFloat("px",      row);
            vec_py[row]      = rec_part.getFloat("py",      row);
            vec_pz[row]      = rec_part.getFloat("pz",      row);
            vec_vx[row]      = rec_part.getFloat("vx",      row);
            vec_vy[row]      = rec_part.getFloat("vy",      row);
            vec_vz[row]      = rec_part.getFloat("vz",      row);
            vec_vt[row]      = rec_part.getFloat("vt",      row);
            vec_beta[row]    = rec_part.getFloat("beta",    row);
            vec_chi2pid[row] = rec_part.getFloat("chi2pid", row);
            vec_charge[row]  = (int8_t)  rec_part.getByte ("charge", row);
            vec_status[row]  = (int16_t) rec_part.getShort("status", row);
        }

        // REC::Track
        event.getStructure(rec_trk);
        int trk_nrows = rec_trk.getRows();
        vec_index.resize(trk_nrows);
        vec_pindex.resize(trk_nrows);
        vec_sector.resize(trk_nrows);
        vec_ndf.resize(trk_nrows);
        vec_chi2.resize(trk_nrows);
        for (int row = 0; row < trk_nrows; ++row) {
            vec_index[row]  = (int16_t) rec_trk.getShort("index", row);
            vec_pindex[row] = (int16_t) rec_trk.getShort("pindex", row);
            vec_sector[row] = (int8_t)  rec_trk.getByte("sector", row);
            vec_ndf[row]    = (int16_t) rec_trk.getShort("NDF", row);
            vec_chi2[row]   = rec_trk.getFloat("chi2", row);
        }

        if (part_nrows > 0 || trk_nrows > 0) tree->Fill();
        writer.addEvent(event);
    }

    f->Close();
    writer.close();

    return 0;
}
