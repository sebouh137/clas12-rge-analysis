// CLAS12 RG-E Analyser.
// Copyright (C) 2022-2023 Bruno Benkel
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

#ifndef RGE_EXTRACTSF
#define RGE_EXTRACTSF

// --+ preamble +---------------------------------------------------------------
// C.
#include "libgen.h"

// C++.
#include <map>
#include <utility>

// ROOT.
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>

// rge-analysis.
#include "../lib/rge_constants.h"
#include "../lib/rge_err_handler.h"
#include "../lib/rge_filename_handler.h"
#include "../lib/rge_hipo_bank.h"
#include "../lib/rge_io_handler.h"
#include "../lib/rge_math_utils.h"
#include "../lib/rge_progress.h"

// typedefs.
typedef unsigned int uint;
typedef long unsigned int luint;
typedef long int lint;

// --+ internal +---------------------------------------------------------------
/** Histogram name. */
static const char *R_EDIVP = "E/p";

/** Sampling fraction array pre-defined integers. */
static const int NCALS    = 4; /** Number of calorimeters. */
static const int PCAL_IDX = 0; /** PCAL idx in arrays. */
static const int ECIN_IDX = 1; /** ECIN idx in arrays. */
static const int ECOU_IDX = 2; /** ECOU idx in arrays. */
static const int ECAL_IDX = 3; /** ECAL idx in arrays. */

/** Momentum bin parameters, in GeV. */
static const double SF_PMIN  = 1.0; /** Minimum value. */
static const double SF_PMAX  = 9.0; /** Maximum value. */
static const double SF_PSTEP = 0.4; /** Step size to separate each bin. */

/** Chi2 conformity for sampling fraction fits. */
static const int SF_CHI2CONFORMITY = 2;

/**
 * Insert a 1-dimensional histogram of floats into a map.
 *
 * @param map  : Map onto which we'll insert the histogram.
 * @param k    : Name of the particle (or set of particles) in the histogram.
 * @param n    : Name of the histogram to be inserted.
 * @param xn   : Name of the variable in the histogram's x axis.
 * @param bins : Number of bins in the histogram.
 * @param min  : Minimum value for the x axis of the histogram.
 * @param max  : Maximum value for the x axis of the histogram.
 * @return     : Success code (0).
 */
static int insert_TH1F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *xn, int bins, double min, double max
);

/**
 * Insert a 2-dimensional histogram of floats into a map.
 *
 * @param map   : Map onto which we'll insert the histogram.
 * @param k     : Name of the particle (or set of particles) in the histogram.
 * @param n     : Name of the histogram to be inserted.
 * @param nx    : Name of the variable in the histogram's x axis.
 * @param ny    : Name of the variable in the histogram's y axis.
 * @param xbins : Number of bins in the x axis of the histogram.
 * @param ybins : Number of bins in the y axis of the histogram.
 * @param xmin  : Minimum value for the x axis of the histogram.
 * @param xmax  : Maximum value for the x axis of the histogram.
 * @param ymin  : Minimum value for the y axis of the histogram.
 * @param ymax  : Maximum value for the y axis of the histogram.
 * @return      : Success code (0).
 */
static int insert_TH2F(
        std::map<const char *, TH1 *> *map, const char *k, const char *n,
        const char *nx, const char *ny, int xbins, double xmin, double xmax,
        int ybins, double ymin, double ymax
);

// --+ library +----------------------------------------------------------------
/**
 * run() function of the extract_sf program. Check USAGE_MESSAGE from extract_sf
 *     for details.
 */
int rge_extract_sf(
        char *in_filename, char *work_dir, char *data_dir, lint nevn, int run_no
);

#endif
