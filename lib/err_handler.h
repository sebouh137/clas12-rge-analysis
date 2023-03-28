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

#ifndef RGE_ANALYSIS_ERRHANDLER
#define RGE_ANALYSIS_ERRHANDLER

#include <stdio.h>

extern int rge_errno;
int handle_err();

// --+ ERROR CODES LIST +-------------------------------------------------------
//    0 -  199 general
int general_err();
#define ERR_NOERR 0
#define ERR_USAGE 1
#define ERR_INVALIDROOTFILE 2

//  200 -  299 acc_corr
int acc_corr_err();
#define ERR_ACCCORR_USAGE         200
#define ERR_ACCCORR_NOEDGE        201
#define ERR_ACCCORR_BADEDGES      202
#define ERR_ACCCORR_NOGENFILE     203
#define ERR_ACCCORR_NOSIMFILE     204
#define ERR_ACCCORR_WRONGGENFILE  205
#define ERR_ACCCORR_BADGENFILE    206
#define ERR_ACCCORR_WRONGSIMFILE  207
#define ERR_ACCCORR_BADSIMFILE    208
#define ERR_ACCCORR_OUTFILEEXISTS 209

//  300 -  399 bank_containers
//  400 -  499 constants
//  500 -  599 draw_plots
//  600 -  699 err_handler
//  700 -  799 extract_sf
//  800 -  899 file_handler
//  900 -  999 hipo2root
// 1000 - 1099 io_handler
int io_handler_err();
#define ERR_IOHANDLER_INVALIDROOTFILE 1001
#define ERR_IOHANDLER_NOINPUTFILE     1002

// 1100 - 1199 make_ntuples
// 1200 - 1299 particle
// 1300 - 1399 utilities
int utilities_err();
#define ERR_UTILITIES_ANGLEOUTOFRANGE 1301

#endif
