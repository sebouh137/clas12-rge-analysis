#ifndef CONSTANTS
#define CONSTANTS

// Masses.
#define PRTMASS 0.938272 // Proton mass.
#define KMASS   0.493677 // Kaon mass.
#define PIMASS  0.139570 // Pion mass.
#define DMASS   1.875    // Deuterium mass.
#define NTRMASS 0.939565 // Neutron mass.
#define EMASS   0.000051 // Electron mass.

// Particle map keys.
#define S_PALL "All particles"
// #define PPOS "Positive particles"
// #define PNEG "Negative particles"
// #define PNEU "Neutral particles"
// #define PPIP "Pi+"
// #define PPIM "Pi-"
// #define PELC "e-"
// #define PTRE "Trigger e-"

// Plot types.
#define PLOT_LIST_SIZE 2
extern const char * PLOT_LIST[PLOT_LIST_SIZE];
extern const char * DIM_LIST[PLOT_LIST_SIZE];
#define R_PLOT1D "1d"
#define R_PLOT2D "2d"
#define S_DIM1   "x"
#define S_DIM2   "y"

// Miscellaneous.
extern const char * RAN_LIST[2];
#define S_LOWER "lower"
#define S_UPPER "upper"

// All variables.
#define VAR_LIST_SIZE 25 // METADATA + PARTICLE + CALORIMETER + SCINTILLATOR + SIDIS.
extern const char * R_VAR_LIST[VAR_LIST_SIZE];
extern const char * S_VAR_LIST[VAR_LIST_SIZE];

// Metadata.
#define S_METADATA "metadata"
#define METADATA_LIST_SIZE 3
extern const char * METADATA_LIST[METADATA_LIST_SIZE];

#define S_RUNNO   "run_no"
#define R_RUNNO   "run_no"
#define S_EVENTNO "event_no"
#define R_EVENTNO "event_no"
#define S_BEAME   "beam_energy"
#define R_BEAME   "beam_energy"

// Particle.
#define S_PARTICLE "particle"
#define PARTICLE_LIST_SIZE 13
extern const char * PARTICLE_LIST[PARTICLE_LIST_SIZE];

#define S_PID    "pid"
#define R_PID    "pid"
#define S_CHARGE "charge"
#define R_CHARGE "charge"
#define S_MASS   "mass"   // GeV.
#define R_MASS   "mass"
#define S_VX     "vx"     // cm.
#define R_VX     "vx"
#define S_VY     "vy"     // cm.
#define R_VY     "vy"
#define S_VZ     "vz"     // cm.
#define R_VZ     "vz"
#define S_PX     "p_{x}"  // GeV.
#define R_PX     "px"
#define S_PY     "p_{y}"  // GeV.
#define R_PY     "py"
#define S_PZ     "p_{z}"  // GeV.
#define R_PZ     "pz"
#define S_P      "p"      // GeV.
#define R_P      "p"
#define S_THETA  "#theta" // #degree.
#define R_THETA  "theta"
#define S_PHI    "#phi"   // #degree.
#define R_PHI    "phi"
#define S_BETA   "#beta"  // adimensional.
#define R_BETA   "beta"

// Calorimeter.
#define S_CALORIMETER "calorimeter"
#define CALORIMETER_LIST_SIZE 4
extern const char * CALORIMETER_LIST[CALORIMETER_LIST_SIZE];

#define S_PCAL_E "E_{pcal}"  // GeV.
#define R_PCAL_E "e_pcal"
#define S_ECIN_E "E_{ecin}"  // GeV.
#define R_ECIN_E "e_ecin"
#define S_ECOU_E "E_{ecou}"  // GeV.
#define R_ECOU_E "e_ecou"
#define S_TOT_E  "E_{total}" // GeV.
#define R_TOT_E  "e_total"

// Scintillator.
#define S_SCINTILLATOR "scintillator"
#define SCINTILLATOR_LIST_SIZE 1
extern const char * SCINTILLATOR_LIST[SCINTILLATOR_LIST_SIZE];

#define S_DTOF "#DeltaTOF" // ns.
#define R_DTOF "dtof"

// SIDIS.
#define S_SIDIS "sidis"
#define SIDIS_LIST_SIZE 4
extern const char * SIDIS_LIST[SIDIS_LIST_SIZE];

#define S_Q2 "Q2"          // GeV^2.
#define R_Q2 "q2"
#define S_NU "#nu"         // GeV.
#define R_NU "nu"
#define S_XB "X_{bjorken}" // adimensional.
#define R_XB "x_bjorken"
#define S_W2 "W2"          // GeV^2.
#define R_W2 "w2"
// #define PHOTONTHETA "virtual photon #theta (lab frame #degree)"
// #define PHOTONPHI   "virtual photon #phi (lab frame #degree)"

// Sampling fraction constants.
extern const char * CALNAME[4]; // Calorimeters names.
extern const char * SFARR2D[4]; // Sampling Fraction (SF) 2D arr names.
extern const char * SFARR1D[4]; // SF 1D arr names.
extern const double PLIMITSARR[4][2]; // Momentum limits for 1D SF fits.
#define S_EDIVP   "E/Vp"
#define PCAL_IDX  0 // PCAL idx in Sampling fraction arrays.
#define ECIN_IDX  1 // ECIN idx in Sampling fraction arrays.
#define ECOU_IDX  2 // ECOU idx in Sampling fraction arrays.
#define CALS_IDX  3 // CALs idx in Sampling fraction arrays.
#define SF_PMIN   1.0 // GeV
#define SF_PMAX   9.0 // GeV
#define SF_PSTEP  0.4 // GeV
#define SF_CHI2CONFORMITY 2 // NOTE. This is a source of systematic error!

// Run constants (TODO. these should be in a map or taken from clas12mon.)
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
