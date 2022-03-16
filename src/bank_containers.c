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

REC_Track::REC_Track(TTree *t) {
    index  = nullptr; b_index  = nullptr; t->SetBranchAddress("index",  &index,  &b_index);
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    sector = nullptr; b_sector = nullptr; t->SetBranchAddress("sector", &sector, &b_sector);
    ndf    = nullptr; b_ndf    = nullptr; t->SetBranchAddress("NDF",    &ndf,    &b_ndf);
    chi2   = nullptr; b_chi2   = nullptr; t->SetBranchAddress("chi2",   &chi2,   &b_chi2);
}

REC_Calorimeter::REC_Calorimeter(TTree *t) {
    pindex = nullptr; b_pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    layer  = nullptr; b_layer  = nullptr; t->SetBranchAddress("layer",  &layer,  &b_layer);
    energy = nullptr; b_energy = nullptr; t->SetBranchAddress("energy", &energy, &b_energy);
}

REC_Scintillator::REC_Scintillator(TTree *t) {
    pindex = nullptr; pindex = nullptr; t->SetBranchAddress("pindex", &pindex, &b_pindex);
    time   = nullptr; time   = nullptr; t->SetBranchAddress("time",   &time,   &b_time);
}

FMT_Tracks::FMT_Tracks(TTree *t) {
    vx = nullptr; b_vx = nullptr; t->SetBranchAddress("vx", &vx, &b_vx);
    vy = nullptr; b_vy = nullptr; t->SetBranchAddress("vy", &vy, &b_vy);
    vz = nullptr; b_vz = nullptr; t->SetBranchAddress("vz", &vz, &b_vz);
    px = nullptr; b_px = nullptr; t->SetBranchAddress("px", &px, &b_px);
    py = nullptr; b_py = nullptr; t->SetBranchAddress("py", &py, &b_py);
    pz = nullptr; b_pz = nullptr; t->SetBranchAddress("pz", &pz, &b_pz);
}
