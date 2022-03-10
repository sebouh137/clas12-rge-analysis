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

    TFile *f = TFile::Open(outfile_root,"CREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);
    TTree *tree = new TTree("clas12", "CLAS12 ROOT Tree");

    // REC::Particle bank.
    std::vector<Int_t>    vec_pid;     tree->Branch("pid",     &vec_pid);
    std::vector<Float_t>  vec_px;      tree->Branch("px",      &vec_px);
    std::vector<Float_t>  vec_py;      tree->Branch("py",      &vec_py);
    std::vector<Float_t>  vec_pz;      tree->Branch("pz",      &vec_pz);
    std::vector<Float_t>  vec_vx;      tree->Branch("vx",      &vec_vx);
    std::vector<Float_t>  vec_vy;      tree->Branch("vy",      &vec_vy);
    std::vector<Float_t>  vec_vz;      tree->Branch("vz",      &vec_vz);
    std::vector<Float_t>  vec_vt;      tree->Branch("vt",      &vec_vt);
    std::vector<Char_t>   vec_charge;  tree->Branch("beta",    &vec_beta);
    std::vector<Float_t>  vec_beta;    tree->Branch("chi2pid", &vec_chi2pid);
    std::vector<Float_t>  vec_chi2pid; tree->Branch("charge",  &vec_charge);
    std::vector<Short_t>  vec_status;  tree->Branch("status",  &vec_status);

    // Prepare infile reader.
    hipo::reader reader;
    reader.open(infile);

    // Prepare factory.
    hipo::dictionary factory;
    reader.readDictionary(factory);
    factory.show();

    // Prepare outfile writer.
    hipo::writer writer;
    writer.getDictionary().addSchema(factory.getSchema("REC::Particle"));
    writer.open(outfile_hipo);

    hipo::bank  particles(factory.getSchema("REC::Particle"));
    hipo::event event;

    while (reader.next()) {
        reader.read(event);
        event.getStructure(particles);

        int nrows = particles.getRows();
        vec_pid.resize(nrows);
        vec_px.resize(nrows);
        vec_py.resize(nrows);
        vec_pz.resize(nrows);
        vec_vx.resize(nrows);
        vec_vy.resize(nrows);
        vec_vz.resize(nrows);
        vec_vt.resize(nrows);
        vec_beta.resize(nrows);
        vec_chi2pid.resize(nrows);
        vec_charge.resize(nrows);
        vec_status.resize(nrows);

        for (int row = 0; row < nrows; row++) {
            vec_pid[row]      = particles.getInt  ("pid",     row);
            vec_px[row]       = particles.getFloat("px",      row);
            vec_py[row]       = particles.getFloat("py",      row);
            vec_pz[row]       = particles.getFloat("pz",      row);
            vec_vx[row]       = particles.getFloat("vx",      row);
            vec_vy[row]       = particles.getFloat("vy",      row);
            vec_vz[row]       = particles.getFloat("vz",      row);
            vec_vt[row]       = particles.getFloat("vt",      row);
            vec_beta[row]     = particles.getFloat("beta",    row);
            vec_chi2pid[row]  = particles.getFloat("chi2pid", row);
            vec_charge[row]   = (int8_t)  particles.getByte ("charge", row);
            vec_status[row]   = (int16_t) particles.getShort("status", row);
        }

        if (vec_pid.size() > 0) tree->Fill();
        writer.addEvent(event);
    }

    f->Close();
    writer.close();

    return 0;
}
