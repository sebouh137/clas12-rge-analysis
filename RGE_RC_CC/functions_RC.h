
#include <vector>
#include <iostream>
#include "TGraph2DErrors.h" 

void ImportRadcor(std::vector<double> &v1, std::vector<double> &v2, std::vector<double> &v3, std::vector<double> &v4, std::vector<double> &v5, std::vector<double> &v6, std::vector<double> &v7, std::vector<double> &v8, std::vector<double> &v9, std::vector<double> &v10, std::vector<double> &v11, std::vector<double> &v12, std::vector<double> &v13, const char * filename);
double findNearestValue(double input, double angle);
double Interpolate1DFromGraph2DErrors(TGraph2DErrors* graph, double Eprime, double angle);
double extractRCfactor(TGraph2DErrors* gr2D_SigmaRad ,  double Eprime, double thetadeg);
TGraph2DErrors* createGraph2D(const std::vector<double>& V2, const std::vector<double>& V3, const std::vector<double>& VData, int size);
bool areEqual(double a, double b, double tolerance);
double linearInterpolation(double x0, double y0, double x1, double y1, double x);
double Get_RC_RATIO(TString target , double Eprime, double thetadeg);
double Get_CC_RATIO(TString target , double Eprime, double thetadeg);
