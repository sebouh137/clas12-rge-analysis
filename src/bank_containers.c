#include "bank_containers.h"

// TODO. This file could use a lot of improvement using interfaces and smart array handling.
// TODO. All strings here should be handled by `constants.h`.
REC_Particle::REC_Particle() {
    nrows   = 0;
    pid     = {};
    px      = {};
    py      = {};
    pz      = {};
    vx      = {};
    vy      = {};
    vz      = {};
    vt      = {};
    charge  = {};
    beta    = {};
    chi2pid = {};
    status  = {};
}
REC_Particle::REC_Particle(TTree *t) {
    pid     = nullptr; b_pid     = nullptr; t->SetBranchAddress("pid",     &pid,     &b_pid);
    px      = nullptr; b_px      = nullptr; t->SetBranchAddress("px",      &px,      &b_px);
    py      = nullptr; b_py      = nullptr; t->SetBranchAddress("py",      &py,      &b_py);
    pz      = nullptr; b_pz      = nullptr; t->SetBranchAddress("pz",      &pz,      &b_pz);
    vx      = nullptr; b_vx      = nullptr; t->SetBranchAddress("vx",      &vx,      &b_vx);
    vy      = nullptr; b_vy      = nullptr; t->SetBranchAddress("vy",      &vy,      &b_vy);
    vz      = nullptr; b_vz      = nullptr; t->SetBranchAddress("vz",      &vz,      &b_vz);
    vt      = nullptr; b_vt      = nullptr; t->SetBranchAddress("vt",      &vt,      &b_vt);
    charge  = nullptr; b_charge  = nullptr; t->SetBranchAddress("charge",  &charge,  &b_charge);
    beta    = nullptr; b_beta    = nullptr; t->SetBranchAddress("beta",    &beta,    &b_beta);
    chi2pid = nullptr; b_chi2pid = nullptr; t->SetBranchAddress("chi2pid", &chi2pid, &b_chi2pid);
    status  = nullptr; b_status  = nullptr; t->SetBranchAddress("status",  &status,  &b_status);
}
int REC_Particle::get_nrows() {return nrows;}
int REC_Particle::link_branches(TTree *t) {
    t->Branch("REC::Particle::pid",     &pid);
    t->Branch("REC::Particle::px",      &px);
    t->Branch("REC::Particle::py",      &py);
    t->Branch("REC::Particle::pz",      &pz);
    t->Branch("REC::Particle::vx",      &vx);
    t->Branch("REC::Particle::vy",      &vy);
    t->Branch("REC::Particle::vz",      &vz);
    t->Branch("REC::Particle::vt",      &vt);
    t->Branch("REC::Particle::charge",  &charge);
    t->Branch("REC::Particle::beta",    &beta);
    t->Branch("REC::Particle::chi2pid", &chi2pid);
    t->Branch("REC::Particle::status",  &status);
    return 0;
}
int REC_Particle::resize() {
    pid    ->resize(nrows);
    px     ->resize(nrows);
    py     ->resize(nrows);
    pz     ->resize(nrows);
    vx     ->resize(nrows);
    vy     ->resize(nrows);
    vz     ->resize(nrows);
    vt     ->resize(nrows);
    beta   ->resize(nrows);
    chi2pid->resize(nrows);
    charge ->resize(nrows);
    status ->resize(nrows);
    return 0;
}
int REC_Particle::fill(hipo::bank b) {
    nrows = b.getRows();
    resize();
    for (int row = 0; row < nrows; ++row) {
        pid    ->at(row) = b.getInt  ("pid",     row);
        px     ->at(row) = b.getFloat("px",      row);
        py     ->at(row) = b.getFloat("py",      row);
        pz     ->at(row) = b.getFloat("pz",      row);
        vx     ->at(row) = b.getFloat("vx",      row);
        vy     ->at(row) = b.getFloat("vy",      row);
        vz     ->at(row) = b.getFloat("vz",      row);
        vt     ->at(row) = b.getFloat("vt",      row);
        beta   ->at(row) = b.getFloat("beta",    row);
        chi2pid->at(row) = b.getFloat("chi2pid", row);
        charge ->at(row) = (int8_t)  b.getByte ("charge", row);
        status ->at(row) = (int16_t) b.getShort("status", row);
    }
    return 0;
}
int REC_Particle::get_entries(TTree *t, int idx) {
    b_pid    ->GetEntry(t->LoadTree(idx));
    b_px     ->GetEntry(t->LoadTree(idx));
    b_py     ->GetEntry(t->LoadTree(idx));
    b_pz     ->GetEntry(t->LoadTree(idx));
    b_vx     ->GetEntry(t->LoadTree(idx));
    b_vy     ->GetEntry(t->LoadTree(idx));
    b_vz     ->GetEntry(t->LoadTree(idx));
    b_vt     ->GetEntry(t->LoadTree(idx));
    b_charge ->GetEntry(t->LoadTree(idx));
    b_beta   ->GetEntry(t->LoadTree(idx));
    b_chi2pid->GetEntry(t->LoadTree(idx));
    b_status ->GetEntry(t->LoadTree(idx));
    return 0;
}

REC_Track::REC_Track() {
    nrows  = 0;
    index  = {};
    pindex = {};
    sector = {};
    ndf    = {};
    chi2   = {};
}
REC_Track::REC_Track(TTree *t) {
    index  = nullptr; b_index  = nullptr; t->SetBranchAddress("index",  &index,  &b_index);
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    sector = nullptr; b_sector = nullptr; t->SetBranchAddress("sector", &sector, &b_sector);
    ndf    = nullptr; b_ndf    = nullptr; t->SetBranchAddress("NDF",    &ndf,    &b_ndf);
    chi2   = nullptr; b_chi2   = nullptr; t->SetBranchAddress("chi2",   &chi2,   &b_chi2);
}
int REC_Track::get_nrows() {return nrows;}
int REC_Track::link_branches(TTree *t) {
    t->Branch("REC::Track::index",  &index);
    t->Branch("REC::Track::pindex", &pindex);
    t->Branch("REC::Track::sector", &sector);
    t->Branch("REC::Track::ndf",    &ndf);
    t->Branch("REC::Track::chi2",   &chi2);
    return 0;
}
int REC_Track::resize() {
    index ->resize(nrows);
    pindex->resize(nrows);
    sector->resize(nrows);
    ndf   ->resize(nrows);
    chi2  ->resize(nrows);
    return 0;
}
int REC_Track::fill(hipo::bank b) {
    nrows = b.getRows();
    resize();
    for (int row = 0; row < nrows; ++row) {
        index ->at(row) = (int16_t) b.getShort("index",  row);
        pindex->at(row) = (int16_t) b.getShort("pindex", row);
        sector->at(row) = (int8_t)  b.getByte ("sector", row);
        ndf   ->at(row) = (int16_t) b.getShort("NDF",    row);
        chi2  ->at(row) = b.getFloat("chi2", row);
    }
    return 0;
}
int REC_Track::get_entries(TTree *t, int idx) {
    b_index ->GetEntry(t->LoadTree(idx));
    b_pindex->GetEntry(t->LoadTree(idx));
    b_sector->GetEntry(t->LoadTree(idx));
    b_ndf   ->GetEntry(t->LoadTree(idx));
    b_chi2  ->GetEntry(t->LoadTree(idx));
    return 0;
}

REC_Calorimeter::REC_Calorimeter() {
    nrows = 0;
    pindex = {};
    layer  = {};
    energy = {};
}
REC_Calorimeter::REC_Calorimeter(TTree *t) {
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    layer  = nullptr; b_layer  = nullptr; t->SetBranchAddress("layer",  &layer,  &b_layer);
    energy = nullptr; b_energy = nullptr; t->SetBranchAddress("energy", &energy, &b_energy);
}
int REC_Calorimeter::get_nrows() {return nrows;}
int REC_Calorimeter::link_branches(TTree *t) {
    t->Branch("REC::Calorimeter::pindex", &pindex);
    t->Branch("REC::Calorimeter::layer",  &layer);
    t->Branch("REC::Calorimeter::energy", &energy);
    return 0;
}
int REC_Calorimeter::resize() {
    pindex->resize(nrows);
    layer ->resize(nrows);
    energy->resize(nrows);
    return 0;
}
int REC_Calorimeter::fill(hipo::bank b) {
    nrows = b.getRows();
    resize();
    for (int row = 0; row < nrows; ++row) {
        pindex->at(row) = (int16_t) b.getShort("pindex", row);
        layer ->at(row) = (int8_t)  b.getByte ("layer",  row);
        energy->at(row) = b.getFloat("energy", row);
    }
    return 0;
}
int REC_Calorimeter::get_entries(TTree *t, int idx) {
    b_pindex->GetEntry(t->LoadTree(idx));
    b_layer ->GetEntry(t->LoadTree(idx));
    b_energy->GetEntry(t->LoadTree(idx));
    return 0;
}

REC_Scintillator::REC_Scintillator() {
    nrows  = 0;
    pindex = {};
    time   = {};
}
REC_Scintillator::REC_Scintillator(TTree *t) {
    pindex = nullptr; pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    time   = nullptr; time   = nullptr; t->SetBranchAddress("time",   &time,   &b_time);
}
int REC_Scintillator::get_nrows() {return nrows;}
int REC_Scintillator::link_branches(TTree *t) {
    t->Branch("REC::Scintillator::pindex", &pindex);
    t->Branch("REC::Scintillator::time",   &time);
    return 0;
}
int REC_Scintillator::resize() {
    pindex->resize(nrows);
    time  ->resize(nrows);
    return 0;
}
int REC_Scintillator::fill(hipo::bank b) {
    nrows = b.getRows();
    resize();
    for (int row = 0; row < nrows; ++row) {
        pindex->at(row) = (int16_t) b.getShort("pindex", row);
        time  ->at(row) = b.getFloat("time", row);
    }
    return 0;
}
int REC_Scintillator::get_entries(TTree *t, int idx) {
    b_pindex->GetEntry(t->LoadTree(idx));
    b_time  ->GetEntry(t->LoadTree(idx));
    return 0;
}

FMT_Tracks::FMT_Tracks() {
    vx = {};
    vy = {};
    vz = {};
    px = {};
    py = {};
    pz = {};
}
FMT_Tracks::FMT_Tracks(TTree *t) {
    vx = nullptr; b_vx = nullptr; t->SetBranchAddress("vx", &vx, &b_vx);
    vy = nullptr; b_vy = nullptr; t->SetBranchAddress("vy", &vy, &b_vy);
    vz = nullptr; b_vz = nullptr; t->SetBranchAddress("vz", &vz, &b_vz);
    px = nullptr; b_px = nullptr; t->SetBranchAddress("px", &px, &b_px);
    py = nullptr; b_py = nullptr; t->SetBranchAddress("py", &py, &b_py);
    pz = nullptr; b_pz = nullptr; t->SetBranchAddress("pz", &pz, &b_pz);
}
int FMT_Tracks::get_nrows() {return nrows;}
int FMT_Tracks::link_branches(TTree *t) {
    t->Branch("FMT::Tracks::vx",    &vx);
    t->Branch("FMT::Tracks::vy",    &vy);
    t->Branch("FMT::Tracks::vz",    &vz);
    t->Branch("FMT::Tracks::px",    &px);
    t->Branch("FMT::Tracks::py",    &py);
    t->Branch("FMT::Tracks::pz",    &pz);
    return 0;
}
int FMT_Tracks::resize() {
    vx->resize(nrows);
    vy->resize(nrows);
    vz->resize(nrows);
    px->resize(nrows);
    py->resize(nrows);
    pz->resize(nrows);
    return 0;
}
int FMT_Tracks::fill(hipo::bank b) {
    nrows = b.getRows();
    resize();
    for (int row = 0; row < nrows; ++row) {
        vx->at(row) = b.getFloat("Vtx0_x", row);
        vy->at(row) = b.getFloat("Vtx0_y", row);
        vz->at(row) = b.getFloat("Vtx0_z", row);
        px->at(row) = b.getFloat("p0_x",   row);
        py->at(row) = b.getFloat("p0_y",   row);
        pz->at(row) = b.getFloat("p0_z",   row);
    }
    return 0;
}
int FMT_Tracks::get_entries(TTree *t, int idx) {
    b_vx->GetEntry(t->LoadTree(idx));
    b_vy->GetEntry(t->LoadTree(idx));
    b_vz->GetEntry(t->LoadTree(idx));
    b_px->GetEntry(t->LoadTree(idx));
    b_py->GetEntry(t->LoadTree(idx));
    b_pz->GetEntry(t->LoadTree(idx));
    return 0;
}
