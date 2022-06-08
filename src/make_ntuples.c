#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <TFile.h>
#include <TNtuple.h>
#include <TTree.h>

#include "../lib/bank_containers.h"
#include "../lib/constants.h"
#include "../lib/err_handler.h"
#include "../lib/file_handler.h"
#include "../lib/io_handler.h"
#include "../lib/particle.h"
#include "../lib/utilities.h"

// TODO. Make this program write using both dc and fmt data.
int run(char * in_filename, bool use_fmt, bool debug, int nevn, int run_no, double beam_E) {
    // Access input file. TODO. Make this input file*s*, as in multiple files.
    TFile *f_in  = TFile::Open(in_filename, "READ");
    TFile *f_out = TFile::Open("../root_io/ntuples.root", "RECREATE"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;

    // Generate lists of variables.
    TString vars("");
    for (int vi = 0; vi < VAR_LIST_SIZE; ++vi) {
        vars.Append(Form("%s", S_VAR_LIST[vi]));
        if (vi != VAR_LIST_SIZE-1) vars.Append(":");
    }

    // Create TTree and TNTuples.
    TTree * t_in    = f_in->Get<TTree>("Tree");
    TNtuple * t_out = new TNtuple(S_PARTICLE, S_PARTICLE, vars);
    REC_Particle     rp(t_in);
    REC_Track        rt(t_in);
    REC_Scintillator rs(t_in);
    REC_Calorimeter  rc(t_in);
    FMT_Tracks       ft(t_in);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    // Iterate through input file. Each TTree entry is one event.
    printf("Reading %lld events from %s.\n", nevn == -1 ? t_in->GetEntries() : nevn, in_filename);
    for (int evn = 0; (evn < t_in->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
        if (!debug && evn >= evnsplitter) {
            if (evn != 0) {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            }
            printf("[");
            for (int i = 0; i <= 50; ++i) {
                if (i <= divcntr/2) printf("=");
                else                printf(" ");
            }
            printf("] %2d%%", divcntr);
            fflush(stdout);
            divcntr++;
            evnsplitter = nevn == -1 ? (t_in->GetEntries() / 100) * divcntr : (nevn/100) * divcntr;
        }

        rp.get_entries(t_in, evn);
        rt.get_entries(t_in, evn);
        rs.get_entries(t_in, evn);
        rc.get_entries(t_in, evn);
        ft.get_entries(t_in, evn);

        // Filter events without the necessary banks.
        if (rp.vz->size() == 0 || rt.pindex->size() == 0) continue;

        // Find trigger electron's TOF.
        int tre_pindex = rt.pindex->at(0);
        double tre_tof = INFINITY;
        for (UInt_t i = 0; i < rs.pindex->size(); ++i) {
            if (rs.pindex->at(i) == tre_pindex && rs.time->at(i) < tre_tof) tre_tof = rs.time->at(i);
        }

        // Process DIS event.
        for (UInt_t pos = 0; pos < rt.index->size(); ++pos) {
            int pindex = rt.pindex->at(pos); // pindex is always equal to pos!

            // Get reconstructed particle from either FMT or DC.
            particle p = use_fmt ? particle_init(&rp, &rt, &ft, pos) : particle_init(&rp, &rt, pos);
            if (!p.is_valid) continue;

            // Get calorimeters data.
            double pcal_E = 0; // PCAL total deposited energy.
            double ecin_E = 0; // EC inner total deposited energy.
            double ecou_E = 0; // EC outer total deposited energy.
            for (UInt_t i = 0; i < rc.pindex->size(); ++i) {
                if (rc.pindex->at(i) == pindex) {
                    int lyr = (int) rc.layer->at(i);
                    // TODO. Add correction via sampling fraction.

                    if      (lyr == PCAL_LYR) pcal_E += rc.energy->at(i);
                    else if (lyr == ECIN_LYR) ecin_E += rc.energy->at(i);
                    else if (lyr == ECOU_LYR) ecou_E += rc.energy->at(i);
                    else return 2;
                }
            }
            double tot_E = pcal_E + ecin_E + ecou_E;

            // Get scintillators data.
            double tof = INFINITY;
            for (UInt_t i = 0; i < rs.pindex->size(); ++i)
                if (rs.pindex->at(i) == pindex && rs.time->at(i) < tof) tof = rs.time->at(i);

            // Get miscellaneous data.
            int status  = rp.status->at(pindex);
            double chi2 = rt.chi2  ->at(pos);
            double ndf  = rt.ndf   ->at(pos);

            // Fill TNtuples. TODO. This probably could be implemented more elegantly.
            // NOTE. If adding new variables, check their order in S_VAR_LIST.
            Float_t v[VAR_LIST_SIZE] = {
                (Float_t) run_no, (Float_t) evn, (Float_t) beam_E,
                (Float_t) p.pid, (Float_t) status, (Float_t) p.q, (Float_t) p.mass, (Float_t) p.vx,
                        (Float_t) p.vy, (Float_t) p.vz, (Float_t) p.px, (Float_t) p.py,
                        (Float_t) p.pz, (Float_t) P(p), (Float_t) theta_lab(p),
                        (Float_t) phi_lab(p), (Float_t) p.beta,
                (Float_t) chi2, (Float_t) ndf,
                (Float_t) pcal_E, (Float_t) ecin_E, (Float_t) ecou_E, (Float_t) tot_E,
                (Float_t) tof,
                (Float_t) Q2(p, beam_E), (Float_t) nu(p, beam_E), (Float_t) Xb(p, beam_E),
                        (Float_t) W2(p, beam_E)
            };
            t_out->Fill(v);
        }
    }
    if (!debug) {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
        printf("[==================================================] 100%% \n");
    }

    // Write to output file.
    f_out->Write();

    // Clean up after ourselves.
    f_in ->Close();
    f_out->Close();
    free(in_filename);

    return 0;
}

// Call program from terminal, C-style.
int main(int argc, char ** argv) {
    bool use_fmt       = false;
    bool debug         = false;
    int nevn           = -1;
    int run_no         = -1;
    double beam_E      = -1;
    char * in_filename = NULL;

    if (make_ntuples_handle_args_err(make_ntuples_handle_args(argc, argv, &use_fmt, &debug, &nevn,
            &in_filename, &run_no, &beam_E), &in_filename, run_no))
        return 1;
    return make_ntuples_err(run(in_filename, use_fmt, debug, nevn, run_no, beam_E), &in_filename);
}
