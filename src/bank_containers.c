#include "bank_containers.h"

REC_Particle::REC_Particle(TTree *t) {
    pid     = nullptr, b_pid     = nullptr; t->SetBranchAddress("pid",     &pid,     &b_pid);
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

REC_Track::REC_Track(TTree *t) {
    index  = nullptr; b_index  = nullptr; t->SetBranchAddress("index",  &index,  &b_index);
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    sector = nullptr; b_sector = nullptr; t->SetBranchAddress("sector", &sector, &b_sector);
    ndf    = nullptr; b_ndf    = nullptr; t->SetBranchAddress("NDF",    &ndf,    &b_ndf);
    chi2   = nullptr; b_chi2   = nullptr; t->SetBranchAddress("chi2",   &chi2,   &b_chi2);
}

int REC_Track::get_entries(TTree *t, int idx) {
    b_index ->GetEntry(t->LoadTree(idx));
    b_pindex->GetEntry(t->LoadTree(idx));
    b_sector->GetEntry(t->LoadTree(idx));
    b_ndf   ->GetEntry(t->LoadTree(idx));
    b_chi2  ->GetEntry(t->LoadTree(idx));
    return 0;
}

REC_Calorimeter::REC_Calorimeter(TTree *t) {
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    layer  = nullptr; b_layer  = nullptr; t->SetBranchAddress("layer",  &layer,  &b_layer);
    energy = nullptr; b_energy = nullptr; t->SetBranchAddress("energy", &energy, &b_energy);
}

int REC_Calorimeter::get_entries(TTree *t, int idx) {
    b_pindex->GetEntry(t->LoadTree(idx));
    b_layer ->GetEntry(t->LoadTree(idx));
    b_energy->GetEntry(t->LoadTree(idx));
    return 0;
}

REC_Scintillator::REC_Scintillator(TTree *t) {
    pindex = nullptr; pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    time   = nullptr; time   = nullptr; t->SetBranchAddress("time",   &time,   &b_time);
}

int REC_Scintillator::get_entries(TTree *t, int idx) {
    b_pindex->GetEntry(t->LoadTree(idx));
    b_time  ->GetEntry(t->LoadTree(idx));
    return 0;
}

FMT_Tracks::FMT_Tracks(TTree *t) {
    vx = nullptr; b_vx = nullptr; t->SetBranchAddress("vx", &vx, &b_vx);
    vy = nullptr; b_vy = nullptr; t->SetBranchAddress("vy", &vy, &b_vy);
    vz = nullptr; b_vz = nullptr; t->SetBranchAddress("vz", &vz, &b_vz);
    px = nullptr; b_px = nullptr; t->SetBranchAddress("px", &px, &b_px);
    py = nullptr; b_py = nullptr; t->SetBranchAddress("py", &py, &b_py);
    pz = nullptr; b_pz = nullptr; t->SetBranchAddress("pz", &pz, &b_pz);
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
