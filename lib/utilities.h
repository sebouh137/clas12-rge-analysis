#ifndef UTILS
#define UTILS

#include <map>
#include <math.h>

#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>

#include "constants.h"

double to_deg(double radians);
double calc_magnitude(double x, double y, double z);
double calc_angle(double x1, double y1, double z1, double x2, double y2, double z2);
void rotate_y(double *x, double *z, double th);
void rotate_z(double *x, double *y, double th);
int catch_string(const char * list[], int size);
double catch_double();
long catch_long();
int find_ntuple(char ** tuplename, const char * list[], int x);
int insert_TH1F(std::map<const char *, TH1 *> *map, const char *k, const char *n, const char *xn,
               int bins, double min, double max);
int insert_TH2F(std::map<const char *, TH1 *> *map, const char *k,
                const char *n, const char *nx, const char *ny,
                int xbins, double xmin, double xmax, int ybins, double ymin, double ymax);

#endif
