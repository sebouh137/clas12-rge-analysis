#ifndef UTILS
#define UTILS

#include <map>
#include <math.h>

#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>

#include "constants.h"

double to_deg(double radians);
double calc_theta(double px, double py, double pz);
double calc_phi(double px, double py);
double calc_P(double px, double py, double pz);
double calc_Q2(double beam_E, double momentum, double theta);
double calc_nu(double beam_E, double momentum);
double calc_Xb(double beam_E, double momentum, double theta);
int insert_TH1F(std::map<const char *, TH1 *> * map, const char * k, const char * n, const char * xn,
               int bins, double min, double max);
int insert_TH2F(std::map<const char *, TH1 *> * map, const char * k,
                const char * n, const char * nx, const char * ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax);

#endif
