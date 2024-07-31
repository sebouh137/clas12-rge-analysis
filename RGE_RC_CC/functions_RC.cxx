
#include "functions_RC.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <tuple>

void ImportRadcor(std::vector<double> &v1, std::vector<double> &v2, std::vector<double> &v3, std::vector<double> &v4, std::vector<double> &v5, std::vector<double> &v6, std::vector<double> &v7, std::vector<double> &v8, std::vector<double> &v9, std::vector<double> &v10, std::vector<double> &v11, std::vector<double> &v12, std::vector<double> &v13, const char * filename) {
  double i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13;
  std::ifstream infile(filename);

  if(infile.fail()){
    std::cout << "Cannot open the file: " << filename << std::endl;
    exit(1);
  } else {
    while(!infile.eof()){
      infile >> i1 >> i2 >> i3 >> i4 >> i5 >> i6 >> i7 >> i8 >> i9 >> i10 >> i11 >> i12 >> i13;

      v1.push_back (i1);
      v2.push_back (i2);
      v3.push_back (i3);
      v4.push_back (i4);
      v5.push_back (i5);
      v6.push_back (i6);
      v7.push_back (i7);
      v8.push_back (i8);
      v9.push_back (i9);
      v10.push_back(i10);
      v11.push_back(i11);
      v12.push_back(i12);
      v13.push_back(i13);
    }

    infile.close();
    v1.pop_back();
    v2.pop_back();
    v3.pop_back();
    v4.pop_back();
    v5.pop_back();
    v6.pop_back();
    v7.pop_back();
    v8.pop_back();
    v9.pop_back();
    v10.pop_back();
    v11.pop_back();
    v12.pop_back();
    v13.pop_back();
  }
}

double findNearestValue(double input, double angle) {
  std::vector<double> validValues;
  double minValue , maxValue;
  double angle_deviation=6.0; // adjust as needed
  minValue= angle - angle_deviation;
  maxValue= angle + angle_deviation;

  for (double val = minValue; val <= maxValue; val += 0.2) {
    validValues.push_back(val);
  }

  if (input < minValue) {
    std::cout << "Input value " << input << " is too small. Minimum allowed value is " << minValue << "." << std::endl;
    return minValue;
  }
  if (input > maxValue) {
    std::cout << "Input value " << input << " is too large. Maximum allowed value is " << maxValue << "." << std::endl;
    return maxValue;
  }

  double nearestValue = validValues[0];
  double minDifference = std::abs(input - validValues[0]);

  for (const auto& value : validValues) {
    double difference = std::abs(input - value);
    if (difference < minDifference) {
      minDifference = difference;
      nearestValue = value;
    }
  }

  return nearestValue;
}

double linearInterpolation(double x0, double y0, double x1, double y1, double x) {
  return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

bool areEqual(double a, double b, double tolerance) {
  return std::fabs(a - b) < tolerance;
}

double Interpolate1DFromGraph2DErrors(TGraph2DErrors* graph, double Eprime, double angle) {
  std::vector<double> x_values, y_values, z_values;
  for (int i = 0; i < graph->GetN(); ++i) {
    double x, y, z;
    graph->GetPoint(i, x, y, z);
    if (areEqual(y, angle, 1e-8)) { // Only consider points where y is equal to the angle
      x_values.push_back(x);
      y_values.push_back(y);
      z_values.push_back(z);
    }
  }

  if (x_values.size() < 2) {
    std::cout << "the angle is : " << angle << std::endl;
    std::cout << "Error: Not enough points for interpolation" << std::endl;
    return NAN;
  }

  std::vector<std::tuple<double, double>> points;
  for (size_t i = 0; i < x_values.size(); ++i) {
    points.emplace_back(x_values[i], z_values[i]);
  }
  std::sort(points.begin(), points.end());

  size_t index = 0;
  while (index < points.size() && std::get<0>(points[index]) < Eprime) {
    index++;
  }

  if (index == 0 || index == points.size()) {
    std::cout << "Error: Interpolation point is outside the range of the data" << std::endl;
    return NAN;
  }

  double x0 = std::get<0>(points[index - 1]);
  double x1 = std::get<0>(points[index]);
  double y0 = std::get<1>(points[index - 1]);
  double y1 = std::get<1>(points[index]);

  double interpolated_value = linearInterpolation(x0, y0, x1, y1, Eprime);

  return interpolated_value;
}

TGraph2DErrors* createGraph2D(const std::vector<double>& V2, const std::vector<double>& V3, const std::vector<double>& VData, int size) {
  TGraph2DErrors *graph = new TGraph2DErrors();
  for (Int_t j = 0; j < size; j++) {
    graph->SetPoint(j, V2[j], V3[j], VData[j]);
    graph->SetPointError(j, 0., 0., 0.);
  }
  return graph;
}

double extractRCfactor(TGraph2DErrors* gr2D_SigmaRad, double Eprime, double thetadeg) {
  double weight = Interpolate1DFromGraph2DErrors(gr2D_SigmaRad, Eprime, thetadeg);
  return weight;
}

double Get_CC_RATIO(TString target, double Eprime, double thetadeg) {
  std::vector<double> V1_st, V2_st, V3_st, V4_st, V5_st, V6_st, V7_st, V8_st, V9_st, V10_st, V11_st, V12_st, V13_st;
  ImportRadcor(V1_st, V2_st, V3_st, V4_st, V5_st, V6_st, V7_st, V8_st, V9_st, V10_st, V11_st, V12_st, V13_st, (const char*)Form("%s.out", (const char*)target));
  const int size_st = V1_st.size();
  TGraph2DErrors* gr2D_CoulombCorrection_ST = createGraph2D(V2_st, V3_st, V13_st, size_st);
  double factor = extractRCfactor(gr2D_CoulombCorrection_ST, Eprime, thetadeg);
  return factor;
}

double Get_RC_RATIO(TString target, double Eprime, double thetadeg) {
  std::vector<double> V1_lt, V2_lt, V3_lt, V4_lt, V5_lt, V6_lt, V7_lt, V8_lt, V9_lt, V10_lt, V11_lt, V12_lt, V13_lt;
  std::vector<double> V1_st, V2_st, V3_st, V4_st, V5_st, V6_st, V7_st, V8_st, V9_st, V10_st, V11_st, V12_st, V13_st;

  ImportRadcor(V1_lt, V2_lt, V3_lt, V4_lt, V5_lt, V6_lt, V7_lt, V8_lt, V9_lt, V10_lt, V11_lt, V12_lt, V13_lt, "cryo2.out");
  ImportRadcor(V1_st, V2_st, V3_st, V4_st, V5_st, V6_st, V7_st, V8_st, V9_st, V10_st, V11_st, V12_st, V13_st, (const char*)Form("%s.out", (const char*)target));

  const int size_st = V1_st.size();
  const int size_lt = V1_lt.size();

  TGraph2DErrors* gr2D_SigmaRad_ST = createGraph2D(V2_st, V3_st, V9_st, size_st);
  TGraph2DErrors* gr2D_SigmaBorn_ST = createGraph2D(V2_st, V3_st, V6_st, size_st);
  TGraph2DErrors* gr2D_SigmaRad_LT = createGraph2D(V2_lt, V3_lt, V9_lt, size_lt);
  TGraph2DErrors* gr2D_SigmaBorn_LT = createGraph2D(V2_lt, V3_lt, V6_lt, size_lt);
  TGraph2DErrors* gr2D_CoulombCorrection_ST = createGraph2D(V2_st, V3_st, V13_st, size_st);

  double sigmaRad_LT = extractRCfactor(gr2D_SigmaRad_LT, Eprime, thetadeg);
  double sigmaRad_ST = extractRCfactor(gr2D_SigmaRad_ST, Eprime, thetadeg);
  double sigmaBorn_LT = extractRCfactor(gr2D_SigmaBorn_LT, Eprime, thetadeg);
  double sigmaBorn_ST = extractRCfactor(gr2D_SigmaBorn_ST, Eprime, thetadeg);
  double factor = (sigmaRad_ST / sigmaBorn_ST) / (sigmaRad_LT / sigmaBorn_LT);
  return factor;
}
