// CLAS12 RG-E Analyser.
// Copyright (C) 2022-2023 Bruno Benkel
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

#ifndef BANK_CONTAINERS
#define BANK_CONTAINERS

#include <TTree.h>
#include "reader.h"

/** Reconstructed particle "final" information. */
class Particle {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // particle id in LUND conventions.
    std::vector<Int_t>    *pid;     TBranch *b_pid;
    // x component of the momentum (GeV).
    std::vector<Float_t>  *px;      TBranch *b_px;
    // y component of the momentum (GeV).
    std::vector<Float_t>  *py;      TBranch *b_py;
    // z component of the momentum (GeV).
    std::vector<Float_t>  *pz;      TBranch *b_pz;
    // x component of the vertex (cm).
    std::vector<Float_t>  *vx;      TBranch *b_vx;
    // y component of the vertex (cm).
    std::vector<Float_t>  *vy;      TBranch *b_vy;
    // z component of the vertex (cm).
    std::vector<Float_t>  *vz;      TBranch *b_vz;
    // RF and z corrected vertex time (ns).
    std::vector<Float_t>  *vt;      TBranch *b_vt;
    // particle charge.
    std::vector<Char_t>   *charge;  TBranch *b_charge;
    // particle beta measured by TOF.
    std::vector<Float_t>  *beta;    TBranch *b_beta;
    // Chi2 of assigned PID.
    std::vector<Float_t>  *chi2pid; TBranch *b_chi2pid;
    // Detector collection particle passed.
    std::vector<Short_t>  *status;  TBranch *b_status;
    Particle();
    Particle(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class Track {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // Index of the track in the specific detector bank.
    std::vector<Short_t>  *index;   TBranch *b_index;
    // Row number in the particle bank track is associated with.
    std::vector<Short_t>  *pindex;  TBranch *b_pindex;
    // Sector of the track.
    std::vector<Short_t>  *sector;  TBranch *b_sector;
    // Number of degrees of freedom in track fitting.
    std::vector<Short_t>  *ndf;     TBranch *b_ndf;
    // Chi2 (or quality) of the track fitting.
    std::vector<Float_t>  *chi2;    TBranch *b_chi2;
    Track();
    Track(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class Calorimeter {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // Row number in the particle bank.
    std::vector<Short_t> *pindex; TBranch *b_pindex;
    // Layer ID, as defined in org.jlab.detector.base.DetectorLayer.
    std::vector<Char_t>  *layer;  TBranch *b_layer;
    // Sector of the detector hit.
    std::vector<Char_t>  *sector; TBranch *b_sector;
    // Energy (GeV) associated with the hit.
    std::vector<Float_t> *energy; TBranch *b_energy;
    // Time (ns) associated with the hit.
    std::vector<Float_t> *time;   TBranch *b_time;
    Calorimeter();
    Calorimeter(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class Scintillator {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // Row number in the particle bank.
    std::vector<Short_t> *pindex;   TBranch *b_pindex;
    // Time (ns) associated with the hit.
    std::vector<Float_t> *time;     TBranch *b_time;
    // Detector ID, as defined in org.jlab.detector.base.DetectorType.
    std::vector<Byte_t>  *detector; TBranch *b_detector;
    // Layer ID, as defined in org.jlab.detector.base.DetectorLayer.
    std::vector<Byte_t>  *layer;    TBranch *b_layer;
    Scintillator();
    Scintillator(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class Cherenkov {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // Row number in the particle bank associated with the hit.
    std::vector<Short_t> *pindex;   TBranch *b_pindex;
    // Detector ID, as defined in org.jlab.detector.base.DetectorType.
    std::vector<Byte_t>  *detector; TBranch *b_detector;
    // Number of photoelectrons from Cherenkov radiation.
    std::vector<Float_t> *nphe;     TBranch *b_nphe;
    Cherenkov();
    Cherenkov(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class FMT_Tracks {
private:
    long unsigned int nrows;
    int set_nrows(long unsigned int in_nrows);
public:
    // index of the track in the DC bank.
    std::vector<Short_t> *index; TBranch *b_index;
    // number of degrees of freedom of the fit.
    std::vector<Int_t>   *ndf;   TBranch *b_ndf;
    // Vertex x-position to the DOCA to the beam.
    std::vector<Float_t> *vx;    TBranch *b_vx;
    // Vertex y-position of the DOCA to the beam.
    std::vector<Float_t> *vy;    TBranch *b_vy;
    // Vertex z-position of the DOCA to the beam.
    std::vector<Float_t> *vz;    TBranch *b_vz;
    // 3-momentum x-coordinate to the DOCA.
    std::vector<Float_t> *px;    TBranch *b_px;
    // 3-momentum y-coordinate of the DOCA.
    std::vector<Float_t> *py;    TBranch *b_py;
    // 3-momentum z-coordinate of the DOCA.
    std::vector<Float_t> *pz;    TBranch *b_pz;
    FMT_Tracks();
    FMT_Tracks(TTree *t);
    int link_tree(TTree *t);
    long unsigned int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};
#endif
