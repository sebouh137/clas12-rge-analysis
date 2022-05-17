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

int run(char *in_filename, bool use_fmt, bool debug, int nevn, int run_no, double beam_E) {
    // Access input file. TODO. Make this input file*s*, as in multiple files.
    TFile *f_in  = TFile::Open(in_filename, "READ");
    TFile *f_out = TFile::Open("../root_io/out.root", "RECREATE"); // NOTE. This path sucks.
    if (!f_in || f_in->IsZombie()) return 1;

    // Generate lists of variables.
    const char * metadata_vars = Form("%s:%s:%s", RUNNO_STR, EVENTNO_STR, BEAME_STR);
    const char * particle_vars = Form("%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s:%s",
            PID_STR, CHARGE_STR, MASS_STR, VX_STR, VY_STR, VZ_STR, PX_STR, PY_STR, PZ_STR, P_STR,
            THETA_STR, PHI_STR, BETA_STR);
    const char * cal_vars  = Form("%s:%s:%s:%s", PCAL_E_STR, ECIN_E_STR, ECOU_E_STR, TOT_E_STR);
    const char * scin_vars = Form("%s", DTOF_STR);
    const char * sidis_vars = Form("%s:%s:%s:%s", Q2_STR, NU_STR, XB_STR, W2_STR);

    // Create tuples.
    TNtuple * metadata_tuple = new TNtuple(METADATA_STR,     METADATA_STR,     metadata_vars);
    TNtuple * particle_tuple = new TNtuple(PARTICLE_STR,     PARTICLE_STR,     particle_vars);
    TNtuple * cal_tuple      = new TNtuple(CALORIMETER_STR,  CALORIMETER_STR,  cal_vars);
    TNtuple * scin_tuple     = new TNtuple(SCINTILLATOR_STR, SCINTILLATOR_STR, scin_vars);
    TNtuple * sidis_tuple    = new TNtuple(SIDIS_STR,        SIDIS_STR,        sidis_vars);

    // Create TTree and link bank_containers.
    TTree *t = f_in->Get<TTree>("Tree");
    REC_Particle     rp(t);
    REC_Track        rt(t);
    REC_Scintillator rs(t);
    REC_Calorimeter  rc(t);
    FMT_Tracks       ft(t);

    // Counters for fancy progress bar.
    int divcntr     = 0;
    int evnsplitter = 0;

    // Iterate through input file. Each TTree entry is one event.
    printf("Reading %lld events from %s.\n", nevn == -1 ? t->GetEntries() : nevn, in_filename);
    for (int evn = 0; (evn < t->GetEntries()) && (nevn == -1 || evn < nevn); ++evn) {
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
            evnsplitter = nevn == -1 ? (t->GetEntries() / 100) * divcntr : (nevn/100) * divcntr;
        }

        rp.get_entries(t, evn);
        rt.get_entries(t, evn);
        rs.get_entries(t, evn);
        rc.get_entries(t, evn);
        ft.get_entries(t, evn);

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

            // Fill TNtuples.
            metadata_tuple->Fill(run_no, evn, beam_E);
            particle_tuple->Fill(p.pid, p.q, p.mass, p.vx, p.vy, p.vz, p.px, p.py, p.pz,
                                 P(p), theta_lab(p), phi_lab(p), p.beta);
            cal_tuple->Fill(pcal_E, ecin_E, ecou_E, tot_E);
            scin_tuple->Fill(tof);
            sidis_tuple->Fill(Q2(p, beam_E), nu(p, beam_E), Xb(p, beam_E), W2(p, beam_E));
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
