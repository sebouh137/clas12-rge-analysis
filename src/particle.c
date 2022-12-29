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

#include "../lib/particle.h"

// Lists of positive, negative, and neutral particles.
const int PID_POSITIVE[PID_POSITIVE_SIZE] = {-11,  211,  321,  2212, 45};
const int PID_NEGATIVE[PID_NEGATIVE_SIZE] = { 11, -211, -321, -2212};
const int PID_NEUTRAL [PID_NEUTRAL_SIZE]  = { 22, 2112};

/** Initialize an empty particle. */
particle particle_init() {
    particle p;
    p.is_valid            = false;
    p.is_hadron           = false;
    p.is_trigger_electron = false;

    return p;
}

// Initialize a new particle from the particle and track banks.
particle particle_init(REC_Particle *rp, REC_Track *rt, int pos) {
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!
    return particle_init(rp->charge->at(pindex), rp->beta->at(pindex),
            rt->sector->at(pos), rp->vx->at(pindex), rp->vy->at(pindex),
            rp->vz->at(pindex),  rp->px->at(pindex), rp->py->at(pindex),
            rp->pz->at(pindex));
}

// Initialize a new particle from the particle, tracks, and FMT banks.
particle particle_init(REC_Particle *rp, REC_Track *rt, FMT_Tracks *ft,
        int pos)
{
    int index  = rt->index->at(pos);
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!

    // Apply FMT cuts.
    // Track reconstructed by FMT.
    if (ft->vz->size() < 1)               return particle_init();
    // Track crossed 3 FMT layers.
    if (ft->ndf->at(index) < FMTNLYRSCUT) return particle_init();

    return particle_init(rp->charge->at(pindex), rp->beta->at(pindex),
            rt->sector->at(pos), ft->vx->at(index), ft->vy->at(index),
            ft->vz->at(index), ft->px->at(index), ft->py->at(index),
            ft->pz->at(index));
}

// Initialize a new particle.
particle particle_init(int charge, double beta, int sector, double vx,
        double vy, double vz, double px, double py, double pz)
{
    particle p;

    // Inherent vars.
    p.is_valid            = true;
    p.is_trigger_electron = false;
    p.is_hadron           = false;

    p.pid    = 0;
    p.q      = charge;
    p.beta   = beta;
    p.sector = sector;

    // Tracking vars.
    p.vx = vx;
    p.vy = vy;
    p.vz = vz;
    p.px = px;
    p.py = py;
    p.pz = pz;

    // NOTE. If programs gets slow, I should cache values of Q2, nu, etc here.
    return p;
}

// Set PID from all available information. This function mimics PIDMatch from
//         the EB engine.
int set_pid(particle * p, int recon_pid, int status, double tot_E,
        double pcal_E, int htcc_nphe, int ltcc_nphe,
        double sf_params[SF_NPARAMS][2])
{
    // Assign PID for neutrals and store PID from reconstruction for charge
    //         particles.
    int rpid = p->q == 0 ? assign_neutral_pid(tot_E, p->beta) : recon_pid;

    // Create PID list.
    int hypotheses_size;
    if      (p->q >  0) hypotheses_size = PID_POSITIVE_SIZE;
    else if (p->q == 0) hypotheses_size = PID_NEUTRAL_SIZE;
    else                hypotheses_size = PID_NEGATIVE_SIZE;

    int hypotheses[hypotheses_size];
    for (int pi = 0; p->q >  0 && pi < hypotheses_size; ++pi)
        hypotheses[pi] = PID_POSITIVE[pi];
    for (int pi = 0; p->q == 0 && pi < hypotheses_size; ++pi)
        hypotheses[pi] = PID_NEUTRAL [pi];
    for (int pi = 0; p->q <  0 && pi < hypotheses_size; ++pi)
        hypotheses[pi] = PID_NEGATIVE[pi];

    // Perform checks.
    bool e_check = is_electron(tot_E, pcal_E, htcc_nphe, P(*p), sf_params);

    bool htcc_signal_check = htcc_nphe > HTCC_NPHE_CUT;
    bool htcc_pion_threshold = P(*p) > HTCC_PION_THRESHOLD;

    // NOTE. LTCC signals are used in recon to veto back from kaon and proton to
    //       pion, but we don't do that here since we're using the PID from
    //       reconstrution anyway.
    // bool ltcc_signal_check = ltcc_nphe > LTCC_NPHE_CUT;
    // bool ltcc_pion_threshold = P(*p) > LTCC_PION_THRESHOLD;

    // NOTE. LTCC kaon threshold is defined in reconstruction, but never
    //       actually used.
    // bool ltcc_kaon_threshold = P(*p) > LTCC_KAON_THRESHOLD;

    // Match PID.
    for (int pi = 0; p->pid == 0 && pi < hypotheses_size; ++pi) {
        p->pid = match_pid(hypotheses[pi], hypotheses[pi] == rpid, p->q,
                e_check, htcc_signal_check, htcc_pion_threshold);
    }

    // Check if particle is trigger electron and define mass from PID.
    p->is_trigger_electron = (p->pid == 11 && status < 0);
    p->mass = MASS.at(abs(p->pid));

    // If particle is not a lepton, check if it is a valid hadron.
    if (p->pid >= 100 || p->pid <= 100) p->is_hadron = true;

    return 0;
}

// Check if a particle satisfies all requirements to be considered an electron
//         or positron.
bool is_electron(double tot_E, double pcal_E, double htcc_nphe, double p,
        double pars[SF_NPARAMS][2])
{
    // Require ECAL.
    if (tot_E < 1e-9)              return false;
    // Momentum must be greater than 0.
    if (p < 1e-9)                  return false;
    // Require HTCC photoelectrons.
    if (htcc_nphe < HTCC_NPHE_CUT) return false;
    // Require PCAL.
    if (pcal_E < MIN_PCAL_ENERGY)  return false;

    // Require ECAL sampling fraction to be below threshold.
    double mean  = pars[0][0]*(pars[1][0] + pars[2][0]/tot_E
            + pars[3][0]/(tot_E*tot_E));
    double sigma = pars[0][1]*(pars[1][1] + pars[2][1]/tot_E
            + pars[3][1]/(tot_E*tot_E));
    if (abs((tot_E/p - mean)/sigma) > E_SF_NSIGMA) return false;

    return true;
}

int assign_neutral_pid(double tot_E, double beta) {
    return beta < NEUTRON_MAXBETA ? 2212 : (tot_E > 1e-9 ? 22 : 0);
}

// Compare momentum-computed beta with tof-computed beta.
int best_pid_from_momentum(double p, double beta, int hypotheses[],
        int hypotheses_size)
{
    int min_pid = 0;
    double min_diff = DBL_MAX;
    for (int pi = 0; pi < hypotheses_size; ++pi) {
        if (abs(hypotheses[pi]) == 45 || hypotheses[pi] == 0 ||
                abs(hypotheses[pi]) == 11)
            continue;
        double mass = MASS.at(abs(hypotheses[pi]));
        double p_beta = p/(sqrt(mass*mass + p*p));
        double diff = abs(p_beta - beta);
        if (diff < min_diff) {
            min_pid  = hypotheses[pi];
            min_diff = diff;
        }
    }
    return min_pid;
}

// Match PID hypothesis with available checks.
int match_pid(int hyp, bool r_match, int q, bool e, bool htcc_s, bool htcc_p) {
    switch(abs(hyp)) {
        case 11:
            if (r_match || e) return hyp;
            break;
        case 211:
            if (r_match || (!e && htcc_s && htcc_p)) return hyp;
            break;
        case 321: case 2212: case 45: case 2112: case 22:
            if (r_match) return hyp;
            break;
    }
    return 0;
}

// Fill array to be stored in ntuples_%06d.root file. Array is of constant size
// VAR_LIST_SIZE, and the order of variables can be seen in constants.h.
int fill_ntuples_arr(Float_t *arr, particle p, particle e, int run_no, int evn,
        int status, double beam_E, float chi2, float ndf, double pcal_E,
        double ecin_E, double ecou_E, double tof, double tre_tof)
{
    // Metadata.
    arr[A_RUNNO]   = (Float_t) run_no;
    arr[A_EVENTNO] = (Float_t) evn;
    arr[A_BEAME]   = beam_E;

    // Particle.
    arr[A_PID]    = (Float_t) p.pid;
    arr[A_CHARGE] = p.q;
    arr[A_STATUS] = (Float_t) status;
    arr[A_MASS]   = p.mass;
    arr[A_VX]     = p.vx;
    arr[A_VY]     = p.vy;
    arr[A_VZ]     = p.vz;
    arr[A_PX]     = p.px;
    arr[A_PY]     = p.py;
    arr[A_PZ]     = p.pz;
    arr[A_P]      = P(p);
    arr[A_THETA]  = theta_lab(p);
    arr[A_PHI]    = phi_lab(p);
    arr[A_BETA]   = p.beta;

    // Tracking.
    arr[A_CHI2] = chi2;
    arr[A_NDF]  = ndf;

    // Calorimeter.
    arr[A_PCAL_E] = pcal_E;
    arr[A_ECIN_E] = ecin_E;
    arr[A_ECOU_E] = ecou_E;
    arr[A_TOT_E]  = pcal_E + ecin_E + ecou_E;

    // Scintillator.
    arr[A_DTOF] = tof - tre_tof;

    // DIS -- For hadrons, just use e- data.
    arr[A_Q2] = Q2(e, beam_E);
    arr[A_NU] = nu(e, beam_E);
    arr[A_XB] = Xb(e, beam_E);
    arr[A_W2] = W2(e, beam_E);

    // SIDIS -- if p is trigger electron, all will be 0 by default.
    arr[A_ZH]      = zh(p, e, beam_E);
    arr[A_PT2]     = Pt2(p, e, beam_E);
    arr[A_PL2]     = Pl2(p, e, beam_E);
    arr[A_PHIPQ]   = phi_pq(p, e, beam_E);
    arr[A_THETAPQ] = theta_pq(p, e, beam_E);

    return 0;
}

// === PARTICLE FUNCTIONS ======================================================
// Get distance from beamline.
float d_from_beamline(particle p) {
    return (sqrt(p.vx*p.vx + p.vy+p.vy));
}

// Calculate theta angle in the lab frame from momentum components of particle.
float theta_lab(particle p) {
    return p.px == 0.0 && p.py == 0.0 && p.pz == 0.0 ? 0.0 :
            atan2(sqrt(p.px*p.px + p.py*p.py), p.pz);
}

// Calculate phi angle in the lab frame from momentum components of particle.
float phi_lab(particle p) {
    return atan2(p.py, p.px);
}

// Calculate momentum magnitude from its components.
float P(particle p) {
    return calc_magnitude(p.px, p.py, p.pz);
}

// Calculate squared mass from momentum and beta.
float mass2(particle p) {
    return (P(p)*P(p)) / (p.beta*p.beta);
}

// === DIS e- FUNCTIONS ========================================================
// Calculate nu from beam energy and total momentum.
float nu(particle p, double bE) {
    // TODO. I need an invalid return value, not zero!
    if (!p.is_trigger_electron) return 0;
    return bE - P(p);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
float Q2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return 4 * bE * P(p) * pow(sin(theta_lab(p)/2), 2);
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
float Xb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return Q2(p, bE) / (2*MASS.at(2212)*nu(p, bE));
}

// Calculate y_bjorken from beam energy and nu.
float Yb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return nu(p, bE) / bE;
}

// Calculate the invariant mass of the electron-nucleon interaction.
float W(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return sqrt(abs(W2(p, bE)));
}

// TODO. Plot W without any Q2 cuts.
// TODO. Read a bit about resonances.

// Calculate the squared invariant mass of the electron-nucleon interaction.
float W2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return MASS.at(2212)*MASS.at(2212) + 2*MASS.at(2212)*nu(p, bE) - Q2(p, bE);
}

// NOTE. double s(particle p) ?

// Calculate the virtual photon's theta angle in the lab frame.
float theta_photon_lab(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return acos((bE - P(p)*cos(theta_lab(p))) /
            (sqrt(Q2(p, bE)+nu(p, bE)*nu(p, bE))));
}

// Calculate the virtual photon's phi angle in the lab frame.
float phi_photon_lab(particle p) {
    if (!p.is_trigger_electron) return 0;
    return M_PI + phi_lab(p);
}

// === SIDIS PRODUCED PARTICLE FUNCTIONS =======================================
// TODO. Many of the methods hereafter assume the particle is a pion. This
//       probably needs to be fixed for CLAS12.

// Compute the polar angle of a produced particle p with respect to the virtual
//         photon direction.
// `p` is the produced particle while `e` is the trigger electron.

float theta_pq(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return calc_angle(-e.px, -e.py, bE-e.pz, p.px, p.py, p.pz);
}

// Compute the azimuthal angle of a produced particle p with respect to the
//         virtual photon direction.
float phi_pq(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;

    double gpx = -e.px, gpy = -e.py, gpz = bE-e.pz;
    double ppx = p.px,  ppy = p.py,  ppz = p.pz;

    // Analyser uses gp isntead of p, i.e. the momentum of virtual photon.
    double phi_z = M_PI - atan2(gpy, gpx);

    rotate_z(&gpx, &gpy, phi_z);
    rotate_z(&ppx, &ppy, phi_z);

    double phi_y = calc_angle(gpx, gpy, gpz, 0, 0, 1);

    rotate_y(&ppx, &ppz, phi_y);

    return atan2(ppy, ppx);
}

// Compute the cosine of the polar angle with respect to the virtual photon
//         direction.
float cos_theta_pq(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return (p.pz*(bE-e.pz) - p.px*e.px - p.py*e.py) /
            (sqrt(nu(e,bE)*nu(e,bE) + Q2(e,bE)) * P(p));
}

// Return the squared momentum transverse to the virtual photon.
float Pt2(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return P(p) * P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the squared momentum longitudinal to the virtual photon.
float Pl2(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return P(p) * P(p) * cos_theta_pq(p,e,bE) * cos_theta_pq(p,e,bE);
}

// Obtain the fraction of the virtual photon energy taken by the produced
//         particle in the lab frame.
float zh(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return sqrt(p.mass*p.mass + P(p)*P(p)) / nu(e,bE);
}

// Return the longitudinal momentum in the center of mass frame.
float PlCM(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return (nu(e,bE) + MASS.at(2212)) * (sqrt(Pl2(p,e,bE)) - sqrt(Q2(e,bE) +
            nu(e,bE)*nu(e,bE))
            * zh(p,e,bE)*nu(e,bE) / (nu(e,bE) + MASS.at(2212))) / W(e,bE);
}

// Obtain the maximum possible value that the momentum could've had in the
//         center of mass frame.
float PmaxCM(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return sqrt(pow(W(e,bE)*W(e,bE) - MASS.at(2112)*MASS.at(2112) +
            MASS.at(211)*MASS.at(211), 2)
            - 4*MASS.at(211)*MASS.at(211)*W(e,bE)*W(e,bE)) / (2*W(e,bE));
}

// Return the momentum transverse component squared of the produced particle wrt
//        the virtual photon direction.
float PTrans2PQ(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return P(p)*P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the momentum longitudinal component squared of the produced particle
//         wrt the virtual photon direction.
float PLong2PQ(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return P(p)*P(p) * cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE);
}

// Calculate X_f (X Feynman).
float Xf(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return PlCM(p,e,bE) / PmaxCM(p,e,bE);
}

// Compute the missing mass
float Mx2(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return W(e,bE)*W(e,bE) - 2*nu(e,bE)*zh(p,e,bE) * (nu(e,bE) +
            MASS.at(2212)) + MASS.at(211)*MASS.at(211) + 2*sqrt((Q2(e,bE) +
            nu(e,bE)*nu(e,bE)) * Pl2(p,e,bE));
}

// Compute Mandelstam t. TODO. Make sure that that is what this is!
float t_mandelstam(particle p, particle e, double bE) {
    if (!(p.is_hadron && e.is_trigger_electron)) return 0;
    return 2*sqrt((nu(e,bE)*nu(e,bE) + Q2(e,bE)) * Pl2(p,e,bE)) +
            MASS.at(211)*MASS.at(211) - Q2(e,bE) -
            2*nu(e,bE)*nu(e,bE)*zh(p,e,bE);
}
