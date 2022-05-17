#ifndef CONSTANTS
#define CONSTANTS

// Masses.
#define PRTMASS 0.938272 // Proton mass.
#define KMASS   0.493677 // Kaon mass.
#define PIMASS  0.139570 // Pion mass.
#define DMASS   1.875    // Deuterium mass.
#define NTRMASS 0.939565 // Neutron mass.
#define EMASS   0.000051 // Electron mass.

// PTUPLES MAP DATA CATEGORIES.
#define METADATA_STR     "metadata"
#define PARTICLE_STR     "particle"
#define CALORIMETER_STR  "calorimeter"
#define SCINTILLATOR_STR "scintillator"
#define SIDIS_STR        "sidis"

// Particle map keys.
#define PALL_STR "All particles"
// #define PPOS "Positive particles"
// #define PNEG "Negative particles"
// #define PNEU "Neutral particles"
// #define PPIP "Pi+"
// #define PPIM "Pi-"
// #define PELC "e-"
// #define PTRE "Trigger e-"

// Variable names (& titles).
#define RUNNO_STR   "run_no"
#define EVENTNO_STR "event_no"
#define BEAME_STR   "beam_energy"

#define PID_STR    "pid"
#define CHARGE_STR "charge"
#define MASS_STR   "mass"   // GeV.
#define VX_STR     "vx"     // cm.
#define VY_STR     "vy"     // cm.
#define VZ_STR     "vz"     // cm.
#define PX_STR     "p_{x}"  // GeV.
#define PY_STR     "p_{y}"  // GeV.
#define PZ_STR     "p_{z}"  // GeV.
#define P_STR      "p"      // GeV.
#define THETA_STR  "#theta" // #degree.
#define PHI_STR    "#phi"   // #degree.
#define BETA_STR   "#beta"  // adimensional.

#define PCAL_E_STR "E_{pcal}"  // GeV.
#define ECIN_E_STR "E_{ecin}"  // GeV.
#define ECOU_E_STR "E_{ecou}"  // GeV.
#define TOT_E_STR  "E_{total}" // GeV.

#define DTOF_STR   "#DeltaTOF" // ns.

#define Q2_STR      "Q2"          // GeV^2.
#define NU_STR      "#nu"         // GeV.
#define XB_STR      "X_{bjorken}" // adimensional.
#define W2_STR      "W2"          // GeV^2.
// #define PHOTONTHETA "virtual photon #theta (lab frame #degree)"
// #define PHOTONPHI   "virtual photon #phi (lab frame #degree)"

// Regular expression constants.
#define R_PLOT1D "plot1d"
#define R_PLOT2D "plot2d"

#define R_RUNNO   "v_run_no"
#define R_EVENTNO "v_event_no"
#define R_BEAME   "v_beam_energy"
#define R_PID     "v_pid"
#define R_CHARGE  "v_charge"
#define R_MASS    "v_mass"
#define R_VX      "v_vx"
#define R_VY      "v_vy"
#define R_VZ      "v_vz"
#define R_PX      "v_px"
#define R_PY      "v_py"
#define R_PZ      "v_pz"
#define R_P       "v_p"
#define R_THETA   "v_theta"
#define R_PHI     "v_phi"
#define R_BETA    "v_beta"
#define R_PCAL_E  "v_e_pcal"
#define R_ECIN_E  "v_e_ecin"
#define R_ECOU_E  "v_e_ecou"
#define R_TOT_E   "v_e_total"
#define R_DTOF    "v_dtof"
#define R_Q2      "v_q2"
#define R_NU      "v_nu"
#define R_XB      "v_x_bjorken"
#define R_W2      "v_w2"

// Sampling fraction constants.
extern const char * CALNAME[4]; // Calorimeters names.
extern const char * SFARR2D[4]; // Sampling Fraction (SF) 2D arr names.
extern const char * SFARR1D[4]; // SF 1D arr names.
extern const double PLIMITSARR[4][2]; // Momentum limits for 1D SF fits.
#define EDIVP_STR "E/Vp"
#define PCAL_IDX  0 // PCAL idx in Sampling fraction arrays.
#define ECIN_IDX  1 // ECIN idx in Sampling fraction arrays.
#define ECOU_IDX  2 // ECOU idx in Sampling fraction arrays.
#define CALS_IDX  3 // CALs idx in Sampling fraction arrays.
#define SF_PMIN   1.0 // GeV
#define SF_PMAX   9.0 // GeV
#define SF_PSTEP  0.4 // GeV
#define SF_CHI2CONFORMITY 2 // NOTE. This is a source of systematic error!

// Run constants (TODO. these should be in a map.)
#define BE11983 10.3894 //  50 nA.
#define BE12016 10.3894 // 250 nA.
#define BE12439  2.1864 //  15 nA.

// Detector constants.
#define NSECTORS 6 // # of CLAS12 sectors.
#define PCAL_LYR 1 // PCAL's layer id.
#define ECIN_LYR 4 // EC inner layer id.
#define ECOU_LYR 7 // EC outer layer id.

// Cuts.
#define FMTNLYRSCUT   3 // # of FMT layers required to accept track.
#define Q2CUT         1 // Q2 of particle must be over this value.
#define WCUT          2 // W of particle must be over this value.
#define CHI2NDFCUT   15 // Chi2/NDF must be below this value.
#define VXVYCUT       4 // sqrt(vx^2 + vy^2) must be below this value.
#define VZLOWCUT    -40 // vz must be above this value.
#define VZHIGHCUT    40 // vz must be below this.

#endif
