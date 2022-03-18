#ifndef BANK_CONTAINERS
#define BANK_CONTAINERS

#include <TBranch.h>
#include <TTree.h>
#include "reader.h"

class REC_Particle {
private:
    int nrows;
    std::vector<Int_t>    *pid;     TBranch *b_pid;
    std::vector<Float_t>  *px;      TBranch *b_px;
    std::vector<Float_t>  *py;      TBranch *b_py;
    std::vector<Float_t>  *pz;      TBranch *b_pz;
    std::vector<Float_t>  *vx;      TBranch *b_vx;
    std::vector<Float_t>  *vy;      TBranch *b_vy;
    std::vector<Float_t>  *vz;      TBranch *b_vz;
    std::vector<Float_t>  *vt;      TBranch *b_vt;
    std::vector<Char_t>   *charge;  TBranch *b_charge;
    std::vector<Float_t>  *beta;    TBranch *b_beta;
    std::vector<Float_t>  *chi2pid; TBranch *b_chi2pid;
    std::vector<Short_t>  *status;  TBranch *b_status;
    int resize();
public:
    REC_Particle();
    REC_Particle(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Track {
private:
    int nrows;
    std::vector<Short_t>  *index;   TBranch *b_index;
    std::vector<Short_t>  *pindex;  TBranch *b_pindex;
    std::vector<Short_t>  *sector;  TBranch *b_sector;
    std::vector<Short_t>  *ndf;     TBranch *b_ndf;
    std::vector<Float_t>  *chi2;    TBranch *b_chi2;
    int resize();
public:
    REC_Track();
    REC_Track(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Calorimeter {
private:
    int nrows;
    std::vector<Short_t> *pindex; TBranch *b_pindex;
    std::vector<Char_t>  *layer;  TBranch *b_layer;
    std::vector<Float_t> *energy; TBranch *b_energy;
    int resize();
public:
    REC_Calorimeter();
    REC_Calorimeter(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class REC_Scintillator {
private:
    int nrows;
    std::vector<Short_t> *pindex; TBranch *b_pindex;
    std::vector<Float_t> *time;   TBranch *b_time;
    int resize();
public:
    REC_Scintillator();
    REC_Scintillator(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

class FMT_Tracks {
private:
    int nrows;
    std::vector<Float_t> *vx; TBranch *b_vx;
    std::vector<Float_t> *vy; TBranch *b_vy;
    std::vector<Float_t> *vz; TBranch *b_vz;
    std::vector<Float_t> *px; TBranch *b_px;
    std::vector<Float_t> *py; TBranch *b_py;
    std::vector<Float_t> *pz; TBranch *b_pz;
    int resize();
public:
    FMT_Tracks();
    FMT_Tracks(TTree *t);
    int get_nrows();
    int link_branches(TTree *t);
    int fill(hipo::bank b);
    int get_entries(TTree *t, int idx);
};

#endif
