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
#include <map>

extern const std::map<unsigned int, const char *> ERRMAP;
extern unsigned int rge_errno;

int handle_err();

// List of error codes.
//    0 -  199 general
#define ERR_NOERR                    0
#define ERR_USAGE                    1
#define ERR_BADINPUTFILE             2
#define ERR_OUTFILEEXISTS            3
#define ERR_OUTPUTFAILED             4
#define ERR_INVALIDROOTFILE          5
#define ERR_NOINPUTFILE              6
#define ERR_NOSAMPFRACFILE           7
#define ERR_NOACCCORRFILE            8
#define ERR_ANGLEOUTOFRANGE          9
#define ERR_NODOTFILENAME           10
#define ERR_BADFILENAMEFORMAT       11
#define ERR_UNIMPLEMENTEDBEAMENERGY 12

//  200 -  299 acc_corr
#define ERR_ACCCORR_NOEDGE       201
#define ERR_ACCCORR_BADEDGES     202
#define ERR_ACCCORR_NOGENFILE    203
#define ERR_ACCCORR_NOSIMFILE    204
#define ERR_ACCCORR_WRONGGENFILE 205
#define ERR_ACCCORR_BADGENFILE   206
#define ERR_ACCCORR_WRONGSIMFILE 207
#define ERR_ACCCORR_BADSIMFILE   208

//  300 -  399 bank_containers
//  400 -  499 constants
//  500 -  599 draw_plots
#define ERR_DRAWPLOTS_BADOPTARGS           501
#define ERR_DRAWPLOTS_NOINPUTFILE          502
#define ERR_DRAWPLOTS_2DACCEPTANCEPLOT     503
#define ERR_DRAWPLOTS_INVALIDNENTRIES      504
#define ERR_DRAWPLOTS_NENTRIESLARGE        505
#define ERR_DRAWPLOTS_NENTRIESNEGATIVE     506
#define ERR_DRAWPLOTS_INVALIDACCEPTANCEOPT 507
#define ERR_DRAWPLOTS_WRONGACCVARS         508
#define ERR_DRAWPLOTS_NOACCDATA            509

//  600 -  699 err_handler
//  700 -  799 extract_sf
//  800 -  899 file_handler
//  900 -  999 hipo2root
// 1000 - 1099 io_handler

// 1100 - 1199 make_ntuples
// 1200 - 1299 particle
// 1300 - 1399 utilities

#endif
