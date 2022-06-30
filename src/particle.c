#include "../lib/particle.h"

const int PID_POSITIVE[PID_POSITIVE_SIZE] = {-11,  211,  321,  2212, 45};
const int PID_NEGATIVE[PID_NEGATIVE_SIZE] = { 11, -211, -321, -2212};
const int PID_NEUTRAL [PID_NEUTRAL_SIZE]  = { 22, 2112};

// TODO. Essentially all methods in this file require testing. Get to that.

// Initialize an empty particle.
particle particle_init() {
    particle p;
    p.is_valid = false;
    return p;
}

// Initialize a new particle from the particle and track banks.
particle particle_init(REC_Particle * rp, REC_Track * rt, int pos) {
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!
    return particle_init(rp->charge->at(pindex), rp->beta->at(pindex), rt->sector->at(pos),
                         rp->vx->at(pindex), rp->vy->at(pindex), rp->vz->at(pindex),
                         rp->px->at(pindex), rp->py->at(pindex), rp->pz->at(pindex));
}

// Initialize a new particle from the particle, tracks, and FMT banks.
particle particle_init(REC_Particle * rp, REC_Track * rt, FMT_Tracks * ft, int pos) {
    int index  = rt->index->at(pos);
    int pindex = rt->pindex->at(pos); // pindex is always equal to pos!

    // Apply FMT cuts.
    if (ft->vz->size() < 1)               return particle_init(); // Track reconstructed by FMT.
    if (ft->ndf->at(index) < FMTNLYRSCUT) return particle_init(); // Track crossed 3 FMT layers.

    return particle_init(rp->charge->at(pindex), rp->beta->at(pindex), rt->sector->at(pos),
                         ft->vx->at(index), ft->vy->at(index), ft->vz->at(index),
                         ft->px->at(index), ft->py->at(index), ft->pz->at(index));
}

// Initialize a new particle.
particle particle_init(int charge, double beta, int sector,
                       double vx, double vy, double vz, double px, double py, double pz) {
    particle p;

    // Inherent vars.
    p.is_valid = true;
    p.pid    = 0; // If particle identification later fails, PID will be 0.
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

// Set PID from all available information. This function mimics PIDMatch from the EB engine.
int set_pid(particle * p, int status, double tot_E, double pcal_E, int htcc_nphe, int ltcc_nphe,
            double sf_params[SF_NPARAMS][2]) {
    // Create PID list.
    int pid_list_size;
    if      (p->q >  0) pid_list_size = PID_POSITIVE_SIZE;
    else if (p->q == 0) pid_list_size = PID_NEUTRAL_SIZE;
    else                pid_list_size = PID_NEGATIVE_SIZE;

    int pid_list[pid_list_size];
    for (int pi = 0; p->q >  0 && pi < pid_list_size; ++pi) pid_list[pi] = PID_POSITIVE[pi];
    for (int pi = 0; p->q == 0 && pi < pid_list_size; ++pi) pid_list[pi] = PID_NEUTRAL [pi];
    for (int pi = 0; p->q <  0 && pi < pid_list_size; ++pi) pid_list[pi] = PID_NEGATIVE[pi];

    // Perform checks.
    bool e_check = is_electron(tot_E, pcal_E, htcc_nphe, P(*p), sf_params);

    int timing_pid = p->q == 0 ?
            assign_neutral_pid(tot_E, p->beta) :
            best_pid_from_momentum(P(*p), p->beta, pid_list, pid_list_size);

    bool htcc_signal_check = htcc_nphe > HTCC_NPHE_CUT;
    bool ltcc_signal_check = ltcc_nphe > LTCC_NPHE_CUT;

    bool htcc_pion_threshold = P(*p) > HTCC_PION_THRESHOLD;
    bool ltcc_pion_threshold = P(*p) > LTCC_PION_THRESHOLD;
    // bool ltcc_kaon_threshold = P(*p) > LTCC_KAON_THRESHOLD; // NOTE. Unused in recon...

    // Match PID.
    for (int pi = 0; p->pid == 0 && pi < pid_list_size; ++pi) {
        bool tpid_check = pid_list[pi] == timing_pid;
        p->pid = match_pid(pid_list[pi], p->q, e_check, tpid_check, htcc_signal_check,
                           ltcc_signal_check, htcc_pion_threshold, ltcc_pion_threshold);
    }

    // Check if particle is trigger electron.
    p->is_trigger_electron = (p->pid == 11 && status < 0);

    // Define mass from PID.
    p->mass = MASS.at(abs(p->pid));

    return timing_pid;
    // return 0;
}

// Check if a particle satisfies all requirements to be considered an electron or positron.
bool is_electron(double tot_E, double pcal_E, double htcc_nphe, double p,
                 double pars[SF_NPARAMS][2]) {
    if (tot_E < 1e-9)              return false; // Require ECAL.
    if (p < 1e-9)                  return false; // Momentum must be greater than 0.
    if (htcc_nphe < HTCC_NPHE_CUT) return false; // Require HTCC photoelectrons.
    if (pcal_E < MIN_PCAL_ENERGY)  return false; // Require PCAL.

    // Require ECAL sampling fraction to be below threshold.
    double mean  = pars[0][0]*(pars[1][0] + pars[2][0]/tot_E + pars[3][0]/(tot_E*tot_E));
    double sigma = pars[0][1]*(pars[1][1] + pars[2][1]/tot_E + pars[3][1]/(tot_E*tot_E));
    if (abs((tot_E/p - mean)/sigma) > E_SF_NSIGMA) return false;

    return true;
}

int assign_neutral_pid(double tot_E, double beta) {
    return beta < NEUTRON_MAXBETA ? 2212 : (tot_E > 1e-9 ? 22 : 0);
}

// Compare momentum-computed beta with tof-computed beta.
int best_pid_from_momentum(double p, double beta, int pid_list[], int pid_list_size) {
    int min_pid = 0;
    double min_diff = DBL_MAX;
    for (int pi = 0; pi < pid_list_size; ++pi) {
        if (abs(pid_list[pi]) == 45 || pid_list[pi] == 0 || abs(pid_list[pi]) == 11) continue;
        double mass = MASS.at(abs(pid_list[pi]));
        double p_beta = p/(sqrt(mass*mass + p*p));
        double diff = abs(p_beta - beta);
        if (diff < min_diff) {
            min_pid  = pid_list[pi];
            min_diff = diff;
        }
    }
    return min_pid;
}

int best_pid_from_timing(int pid_list[], int pid_list_size) {
    int min_pid = 0;
    double min_diff = DBL_MAX;
    bool chk = false;
    for (int pi = 0; pi < pid_list_size; ++pi) {
        if (abs(pid_list[pi]) == 11) continue;

    }

    return min_pid;

    // // recon code:
    // for (int ii=0; ii<hypotheses.length; ii++) {
    //     for (Entry<DetectorType,List<Integer>> bd : chargedBetaDetectors.entrySet()) {
    //         for (Integer layer : bd.getValue()) {
    //             if (p.hasHit(bd.getKey(),layer)==true) {
    //                 dt = p.getVertexTime(bd.getKey(),layer,hypotheses[ii])-p.getStartTime();
    //                 found=true;
    //                 break;
    //             }
    //         }
    //         if (found) break;
    //     }
    //     if ( abs(dt) < minTimeDiff ) {
    //         minTimeDiff=abs(dt);
    //         bestPid=hypotheses[ii];
    //     }
    // }
    // return bestPid;
}

// Match PID hypothesis with available checks.
int match_pid(int hyp, int q, bool e, bool tpid, bool htcc_s, bool ltcc_s, bool htcc_p,
              bool ltcc_p) {
    switch(abs(hyp)) {
        case 11:
            if (e) return hyp;
            break;
        case 211:
            if (!e && (tpid || (htcc_s && ltcc_p)))
                return hyp;
            break;
        case 321:
            if (!e && tpid) {
                if (ltcc_s && ltcc_p) return q * 211; // veto back to pion.
                else return hyp;
            }
            break;
        case 2212:
            if (!e && tpid) {
                if (ltcc_s && ltcc_p) return q * 211; // veto back to pion.
                else return hyp;
            }
            break;
        case 45:
            if (!e && tpid) return hyp;
            break;
        case 2112:
            if (tpid) return hyp;
            break;
        case 22:
            if (tpid) return hyp;
            break;
    }
    return 0;
}

// === PARTICLE FUNCTIONS ==========================================================================
// Get distance from beamline.
double d_from_beamline(particle p) {
    return (sqrt(p.vx*p.vx + p.vy+p.vy));
}

// Calculate theta angle in the lab frame from momentum components of particle.
double theta_lab(particle p) {
    return p.px == 0.0 && p.py == 0.0 && p.pz == 0.0 ? 0.0 : atan2(sqrt(p.px*p.px + p.py*p.py), p.pz);
}

// Calculate phi angle in the lab frame from momentum components of particle.
double phi_lab(particle p) {
    return atan2(p.py, p.px);
}

// Calculate momentum magnitude from its components.
double P(particle p) {
    return calc_magnitude(p.px, p.py, p.pz);
}

// Calculate squared mass from momentum and beta.
double mass2(particle p) {
    return (P(p)*P(p)) / (p.beta*p.beta);
}

// === SIDIS e- FUNCTIONS ==========================================================================
// Calculate nu from beam energy and total momentum.
double nu(particle p, double bE) {
    if (!p.is_trigger_electron) return 0; // TODO. I need an invalid return value, not zero!
    return bE - P(p);
}

// Calculate Q^2 from beam energy, particle momentum, and theta angle.
double Q2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return 4 * bE * P(p) * pow(sin(theta_lab(p)/2), 2);
}

// Calculate x_bjorken from beam energy, particle momentum, and theta angle.
double Xb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return Q2(p, bE) / (2*MASS.at(2212)*nu(p, bE));
}

// Calculate y_bjorken from beam energy and nu.
double Yb(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return nu(p, bE) / bE;
}

// Calculate the invariant mass of the electron-nucleon interaction.
double W(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return sqrt(abs(W2(p, bE)));
}
// TODO. Plot W without any Q2 cuts.
// TODO. Read a bit about resonances.

// Calculate the squared invariant mass of the electron-nucleon interaction.
double W2(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return MASS.at(2212)*MASS.at(2212) + 2*MASS.at(2212)*nu(p, bE) - Q2(p, bE);
}

// NOTE. double s(particle p) ?

// Calculate the virtual photon's theta angle in the lab frame.
double theta_photon_lab(particle p, double bE) {
    if (!p.is_trigger_electron) return 0;
    return acos((bE - P(p)*cos(theta_lab(p))) / (sqrt(Q2(p, bE)+nu(p, bE)*nu(p, bE))));
}

// Calculate the virtual photon's phi angle in the lab frame.
double phi_photon_lab(particle p) {
    if (!p.is_trigger_electron) return 0;
    return M_PI + phi_lab(p);
}

// === SIDIS PRODUCED PARTICLE FUNCTIONS ===========================================================
// TODO. Many of the methods hereafter assume the particle is a pion. This probably needs to be
//       fixed for CLAS12.

// Compute the polar angle of a produced particle p with respect to the virtual photon direction.
// `p` is the produced particle while `e` is the trigger electron.
double theta_pq(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return calc_angle(-e.px, -e.py, bE-e.pz, p.px, p.py, p.pz);
}

// Compute the azimuthal angle of a produced particle p with respect to the virtual photon direction.
double phi_pq(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;

    double gpx = -e.px, gpy = -e.py, gpz = bE-e.pz;
    double ppx = p.px,  ppy = p.py,  ppz = p.pz;

    double phi_z = M_PI - phi_lab(p);
    rotate_z(&gpx, &gpy, phi_z);
    rotate_z(&ppx, &ppy, phi_z);

    double phi_y = calc_angle(gpx, gpy, gpz, 0, 0, 1);
    rotate_y(&ppx, &ppz, phi_y);

    return atan2(ppy, ppx);
}

// Compute the cosine of the polar angle with respect to the virtual photon direction.
double cos_theta_pq(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return (p.pz*(bE-e.pz) - p.px*e.px - p.py*e.py) / (sqrt(nu(e,bE)*nu(e,bE) + Q2(e,bE)) * P(p));
}

// Return the squared momentum transverse to the virtual photon.
double Pt2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p) * P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the squared momentum longitudinal to the virtual photon.
double Pl2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p) * P(p) * cos_theta_pq(p,e,bE) * cos_theta_pq(p,e,bE);
}

// Obtain the fraction of the virtual photon energy taken by the produced particle in the lab frame.
double zh(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return sqrt(p.mass*p.mass + P(p)*P(p)) / nu(e,bE);
}

// Return the longitudinal momentum in the center of mass frame.
double PlCM(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return (nu(e,bE) + MASS.at(2212)) * (sqrt(Pl2(p,e,bE)) - sqrt(Q2(e,bE) + nu(e,bE)*nu(e,bE))
            * zh(p,e,bE)*nu(e,bE) / (nu(e,bE) + MASS.at(2212))) / W(e,bE);
}

// Obtain the maximum possible value that the momentum could've had in the center of mass frame.
double PmaxCM(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return sqrt(pow(W(e,bE)*W(e,bE) - MASS.at(2112)*MASS.at(2112) + MASS.at(211)*MASS.at(211), 2)
            - 4*MASS.at(211)*MASS.at(211)*W(e,bE)*W(e,bE)) / (2*W(e,bE));
}

// Return the momentum transverse component squared of the produced particle wrt the virtual photon
//     direction.
double PTrans2PQ(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p)*P(p) * (1 - cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE));
}

// Return the momentum longitudinal component squared of the produced particle wrt the virtual
//     photon direction.
double PLong2PQ(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return P(p)*P(p) * cos_theta_pq(p,e,bE)*cos_theta_pq(p,e,bE);
}

// Calculate X_f (X Feynmann).
double Xf(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return PlCM(p,e,bE) / PmaxCM(p,e,bE);
}

// Compute the missing mass
double Mx2(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return W(e,bE)*W(e,bE) - 2*nu(e,bE)*zh(p,e,bE) * (nu(e,bE) + MASS.at(2212))
            + MASS.at(211)*MASS.at(211) + 2*sqrt((Q2(e,bE) + nu(e,bE)*nu(e,bE)) * Pl2(p,e,bE));
}

// Compute Mandelstam t. TODO. Make sure that that is what this is!
double t_mandelstam(particle p, particle e, double bE) {
    if (!e.is_trigger_electron || p.is_trigger_electron) return 0;
    return 2*sqrt((nu(e,bE)*nu(e,bE) + Q2(e,bE)) * Pl2(p,e,bE)) + MASS.at(211)*MASS.at(211)
            - Q2(e,bE) - 2*nu(e,bE)*nu(e,bE)*zh(p,e,bE);
}
