// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
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

#include "../lib/bank_containers.h"

// TODO. This file could use a lot of improvement using interfaces and smart
//       array handling.
// TODO. All strings here should be handled by `constants.h`.
Particle::Particle() {
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
Particle::Particle(TTree *t) {
    pid     = nullptr; b_pid     = nullptr;
    px      = nullptr; b_px      = nullptr;
    py      = nullptr; b_py      = nullptr;
    pz      = nullptr; b_pz      = nullptr;
    vx      = nullptr; b_vx      = nullptr;
    vy      = nullptr; b_vy      = nullptr;
    vz      = nullptr; b_vz      = nullptr;
    vt      = nullptr; b_vt      = nullptr;
    charge  = nullptr; b_charge  = nullptr;
    beta    = nullptr; b_beta    = nullptr;
    chi2pid = nullptr; b_chi2pid = nullptr;
    status  = nullptr; b_status  = nullptr;
    t->SetBranchAddress("REC::Particle::pid",     &pid,     &b_pid);
    t->SetBranchAddress("REC::Particle::px",      &px,      &b_px);
    t->SetBranchAddress("REC::Particle::py",      &py,      &b_py);
    t->SetBranchAddress("REC::Particle::pz",      &pz,      &b_pz);
    t->SetBranchAddress("REC::Particle::vx",      &vx,      &b_vx);
    t->SetBranchAddress("REC::Particle::vy",      &vy,      &b_vy);
    t->SetBranchAddress("REC::Particle::vz",      &vz,      &b_vz);
    t->SetBranchAddress("REC::Particle::vt",      &vt,      &b_vt);
    t->SetBranchAddress("REC::Particle::charge",  &charge,  &b_charge);
    t->SetBranchAddress("REC::Particle::beta",    &beta,    &b_beta);
    t->SetBranchAddress("REC::Particle::chi2pid", &chi2pid, &b_chi2pid);
    t->SetBranchAddress("REC::Particle::status",  &status,  &b_status);
}
int Particle::link_branches(TTree *t) {
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
int Particle::set_nrows(int in_nrows) {
    nrows = in_nrows;
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
int Particle::get_nrows() {return nrows;}
int Particle::fill(hipo::bank b) {
    set_nrows(b.getRows());
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
int Particle::get_entries(TTree *t, int idx) {
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

Track::Track() {
    nrows  = 0;
    index  = {};
    pindex = {};
    sector = {};
    ndf    = {};
    chi2   = {};
}
Track::Track(TTree *t) {
    index  = nullptr; b_index  = nullptr;
    pindex = nullptr; b_pindex = nullptr;
    sector = nullptr; b_sector = nullptr;
    ndf    = nullptr; b_ndf    = nullptr;
    chi2   = nullptr; b_chi2   = nullptr;
    t->SetBranchAddress("REC::Track::index",  &index,  &b_index);
    t->SetBranchAddress("REC::Track::pindex", &pindex, &b_pindex);
    t->SetBranchAddress("REC::Track::sector", &sector, &b_sector);
    t->SetBranchAddress("REC::Track::ndf",    &ndf,    &b_ndf);
    t->SetBranchAddress("REC::Track::chi2",   &chi2,   &b_chi2);
}
int Track::link_branches(TTree *t) {
    t->Branch("REC::Track::index",  &index);
    t->Branch("REC::Track::pindex", &pindex);
    t->Branch("REC::Track::sector", &sector);
    t->Branch("REC::Track::ndf",    &ndf);
    t->Branch("REC::Track::chi2",   &chi2);
    return 0;
}
int Track::set_nrows(int in_nrows) {
    nrows = in_nrows;
    index ->resize(nrows);
    pindex->resize(nrows);
    sector->resize(nrows);
    ndf   ->resize(nrows);
    chi2  ->resize(nrows);
    return 0;
}
int Track::get_nrows() {return nrows;}
int Track::fill(hipo::bank b) {
    set_nrows(b.getRows());
    for (int row = 0; row < nrows; ++row) {
        index ->at(row) = (int16_t) b.getShort("index",  row);
        pindex->at(row) = (int16_t) b.getShort("pindex", row);
        sector->at(row) = (int8_t)  b.getByte ("sector", row);
        ndf   ->at(row) = (int16_t) b.getShort("NDF",    row);
        chi2  ->at(row) = b.getFloat("chi2", row);
    }
    return 0;
}
int Track::get_entries(TTree *t, int idx) {
    b_index ->GetEntry(t->LoadTree(idx));
    b_pindex->GetEntry(t->LoadTree(idx));
    b_sector->GetEntry(t->LoadTree(idx));
    b_ndf   ->GetEntry(t->LoadTree(idx));
    b_chi2  ->GetEntry(t->LoadTree(idx));
    return 0;
}

Calorimeter::Calorimeter() {
    nrows = 0;
    pindex = {};
    layer  = {};
    sector = {};
    energy = {};
    time   = {};
}
Calorimeter::Calorimeter(TTree *t) {
    pindex = nullptr; b_pindex = nullptr;
    layer  = nullptr; b_layer  = nullptr;
    sector = nullptr; b_sector = nullptr;
    energy = nullptr; b_energy = nullptr;
    time   = nullptr; b_time   = nullptr;
    t->SetBranchAddress("REC::Calorimeter::pindex", &pindex, &b_pindex);
    t->SetBranchAddress("REC::Calorimeter::layer",  &layer,  &b_layer);
    t->SetBranchAddress("REC::Calorimeter::sector", &sector, &b_sector);
    t->SetBranchAddress("REC::Calorimeter::energy", &energy, &b_energy);
    t->SetBranchAddress("REC::Calorimeter::time",   &time,   &b_time);
}
int Calorimeter::link_branches(TTree *t) {
    t->Branch("REC::Calorimeter::pindex", &pindex);
    t->Branch("REC::Calorimeter::layer",  &layer);
    t->Branch("REC::Calorimeter::sector", &sector);
    t->Branch("REC::Calorimeter::energy", &energy);
    t->Branch("REC::Calorimeter::time",   &time);
    return 0;
}
int Calorimeter::set_nrows(int in_nrows) {
    nrows = in_nrows;
    pindex->resize(nrows);
    layer ->resize(nrows);
    sector->resize(nrows);
    energy->resize(nrows);
    time  ->resize(nrows);
    return 0;
}
int Calorimeter::get_nrows() {return nrows;}
int Calorimeter::fill(hipo::bank b) {
    set_nrows(b.getRows());
    for (int row = 0; row < nrows; ++row) {
        pindex->at(row) = (int16_t) b.getShort("pindex", row);
        layer ->at(row) = (int8_t)  b.getByte ("layer",  row);
        sector->at(row) = (int8_t)  b.getByte ("sector", row);
        energy->at(row) = b.getFloat("energy", row);
        time  ->at(row) = b.getFloat("time", row);
    }
    return 0;
}
int Calorimeter::get_entries(TTree *t, int idx) {
    b_pindex->GetEntry(t->LoadTree(idx));
    b_layer ->GetEntry(t->LoadTree(idx));
    b_sector->GetEntry(t->LoadTree(idx));
    b_energy->GetEntry(t->LoadTree(idx));
    b_time  ->GetEntry(t->LoadTree(idx));
    return 0;
}

Scintillator::Scintillator() {
    nrows    = 0;
    pindex   = {};
    time     = {};
    detector = {};
    layer    = {};
}
Scintillator::Scintillator(TTree *t) {
    pindex   = nullptr; b_pindex   = nullptr;
    time     = nullptr; b_time     = nullptr;
    detector = nullptr; b_detector = nullptr;
    layer    = nullptr; b_layer    = nullptr;
    t->SetBranchAddress("REC::Scintillator::pindex",   &pindex,   &b_pindex);
    t->SetBranchAddress("REC::Scintillator::time",     &time,     &b_time);
    t->SetBranchAddress("REC::Scintillator::detector", &detector, &b_detector);
    t->SetBranchAddress("REC::Scintillator::layer",    &layer,    &b_layer);
}
int Scintillator::link_branches(TTree *t) {
    t->Branch("REC::Scintillator::pindex",   &pindex);
    t->Branch("REC::Scintillator::time",     &time);
    t->Branch("REC::Scintillator::detector", &detector);
    t->Branch("REC::Scintillator::layer",    &layer);
    return 0;
}
int Scintillator::set_nrows(int in_nrows) {
    nrows = in_nrows;
    pindex  ->resize(nrows);
    time    ->resize(nrows);
    detector->resize(nrows);
    layer   ->resize(nrows);
    return 0;
}
int Scintillator::get_nrows() {return nrows;}
int Scintillator::fill(hipo::bank b) {
    set_nrows(b.getRows());
    for (int row = 0; row < nrows; ++row) {
        pindex  ->at(row) = (int16_t) b.getShort("pindex", row);
        time    ->at(row) = b.getFloat("time", row);
        detector->at(row) = (int8_t) b.getByte("detector", row);
        layer   ->at(row) = (int8_t) b.getByte("layer", row);
    }
    return 0;
}
int Scintillator::get_entries(TTree *t, int idx) {
    b_pindex  ->GetEntry(t->LoadTree(idx));
    b_time    ->GetEntry(t->LoadTree(idx));
    b_detector->GetEntry(t->LoadTree(idx));
    b_layer   ->GetEntry(t->LoadTree(idx));
    return 0;
}

Cherenkov::Cherenkov() {
    nrows    = 0;
    pindex   = {};
    detector = {};
    nphe     = {};
}
Cherenkov::Cherenkov(TTree *t) {
    pindex   = nullptr; b_pindex   = nullptr;
    detector = nullptr; b_detector = nullptr;
    nphe     = nullptr; b_nphe     = nullptr;
    t->SetBranchAddress("REC::Cherenkov::pindex",   &pindex,   &b_pindex);
    t->SetBranchAddress("REC::Cherenkov::detector", &detector, &b_detector);
    t->SetBranchAddress("REC::Cherenkov::nphe",     &nphe,     &b_nphe);
}
int Cherenkov::link_branches(TTree *t) {
    t->Branch("REC::Cherenkov::pindex",   &pindex);
    t->Branch("REC::Cherenkov::detector", &detector);
    t->Branch("REC::Cherenkov::nphe",     &nphe);
    return 0;
}
int Cherenkov::set_nrows(int in_nrows) {
    nrows = in_nrows;
    pindex  ->resize(nrows);
    detector->resize(nrows);
    nphe    ->resize(nrows);
    return 0;
}
int Cherenkov::get_nrows() {return nrows;}
int Cherenkov::fill(hipo::bank b) {
    set_nrows(b.getRows());
    for (int row = 0; row < nrows; ++row) {
        pindex  ->at(row) = (int16_t) b.getShort("pindex", row);
        detector->at(row) = (int8_t)  b.getByte("detector", row);
        nphe    ->at(row) = b.getFloat("nphe", row);
    }
    return 0;
}
int Cherenkov::get_entries(TTree *t, int idx) {
    b_pindex  ->GetEntry(t->LoadTree(idx));
    b_detector->GetEntry(t->LoadTree(idx));
    b_nphe    ->GetEntry(t->LoadTree(idx));
    return 0;
}

FMT_Tracks::FMT_Tracks() {
    index = {};
    ndf   = {};
    vx    = {};
    vy    = {};
    vz    = {};
    px    = {};
    py    = {};
    pz    = {};
}
FMT_Tracks::FMT_Tracks(TTree *t) {
    index = nullptr; b_index = nullptr;
    ndf   = nullptr; b_ndf   = nullptr;
    vx    = nullptr; b_vx    = nullptr;
    vy    = nullptr; b_vy    = nullptr;
    vz    = nullptr; b_vz    = nullptr;
    px    = nullptr; b_px    = nullptr;
    py    = nullptr; b_py    = nullptr;
    pz    = nullptr; b_pz    = nullptr;
    t->SetBranchAddress("FMT::Tracks::index", &index, &b_index);
    t->SetBranchAddress("FMT::Tracks::ndf",   &ndf,   &b_ndf);
    t->SetBranchAddress("FMT::Tracks::vx",    &vx,    &b_vx);
    t->SetBranchAddress("FMT::Tracks::vy",    &vy,    &b_vy);
    t->SetBranchAddress("FMT::Tracks::vz",    &vz,    &b_vz);
    t->SetBranchAddress("FMT::Tracks::px",    &px,    &b_px);
    t->SetBranchAddress("FMT::Tracks::py",    &py,    &b_py);
    t->SetBranchAddress("FMT::Tracks::pz",    &pz,    &b_pz);
}
int FMT_Tracks::link_tree(TTree *t) {
    index = nullptr; b_index = nullptr;
    ndf   = nullptr; b_ndf   = nullptr;
    vx    = nullptr; b_vx    = nullptr;
    vy    = nullptr; b_vy    = nullptr;
    vz    = nullptr; b_vz    = nullptr;
    px    = nullptr; b_px    = nullptr;
    py    = nullptr; b_py    = nullptr;
    pz    = nullptr; b_pz    = nullptr;
    t->SetBranchAddress("FMT::Tracks::index", &index, &b_index);
    t->SetBranchAddress("FMT::Tracks::ndf",   &ndf,   &b_ndf);
    t->SetBranchAddress("FMT::Tracks::vx",    &vx,    &b_vx);
    t->SetBranchAddress("FMT::Tracks::vy",    &vy,    &b_vy);
    t->SetBranchAddress("FMT::Tracks::vz",    &vz,    &b_vz);
    t->SetBranchAddress("FMT::Tracks::px",    &px,    &b_px);
    t->SetBranchAddress("FMT::Tracks::py",    &py,    &b_py);
    t->SetBranchAddress("FMT::Tracks::pz",    &pz,    &b_pz);
    return 0;
}
int FMT_Tracks::link_branches(TTree *t) {
    t->Branch("FMT::Tracks::index", &index);
    t->Branch("FMT::Tracks::ndf",   &ndf);
    t->Branch("FMT::Tracks::vx",    &vx);
    t->Branch("FMT::Tracks::vy",    &vy);
    t->Branch("FMT::Tracks::vz",    &vz);
    t->Branch("FMT::Tracks::px",    &px);
    t->Branch("FMT::Tracks::py",    &py);
    t->Branch("FMT::Tracks::pz",    &pz);
    return 0;
}
int FMT_Tracks::set_nrows(int in_nrows) {
    nrows = in_nrows;
    index->resize(nrows);
    ndf  ->resize(nrows);
    vx   ->resize(nrows);
    vy   ->resize(nrows);
    vz   ->resize(nrows);
    px   ->resize(nrows);
    py   ->resize(nrows);
    pz   ->resize(nrows);
    return 0;
}
int FMT_Tracks::get_nrows() {return nrows;}
int FMT_Tracks::fill(hipo::bank b) {
    set_nrows(b.getRows());
    for (int row = 0; row < nrows; ++row) {
        index->at(row) = (int16_t) b.getShort("index", row);
        ndf  ->at(row) = b.getInt("NDF", row);
        vx   ->at(row) = b.getFloat("Vtx0_x", row);
        vy   ->at(row) = b.getFloat("Vtx0_y", row);
        vz   ->at(row) = b.getFloat("Vtx0_z", row);
        px   ->at(row) = b.getFloat("p0_x",   row);
        py   ->at(row) = b.getFloat("p0_y",   row);
        pz   ->at(row) = b.getFloat("p0_z",   row);
    }
    return 0;
}
int FMT_Tracks::get_entries(TTree *t, int idx) {
    b_index->GetEntry(t->LoadTree(idx));
    b_ndf  ->GetEntry(t->LoadTree(idx));
    b_vx   ->GetEntry(t->LoadTree(idx));
    b_vy   ->GetEntry(t->LoadTree(idx));
    b_vz   ->GetEntry(t->LoadTree(idx));
    b_px   ->GetEntry(t->LoadTree(idx));
    b_py   ->GetEntry(t->LoadTree(idx));
    b_pz   ->GetEntry(t->LoadTree(idx));
    return 0;
}
