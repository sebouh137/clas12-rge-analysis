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
// --+   0 - 199 general +------------------------------------------------------
#define ERR_NOERR                     0
#define ERR_USAGE                     1
#define ERR_BADINPUTFILE              2
#define ERR_OUTFILEEXISTS             3
#define ERR_OUTPUTROOTFAILED          4
#define ERR_INVALIDROOTFILE           5
#define ERR_NOINPUTFILE               6
#define ERR_NOSAMPFRACFILE            7
#define ERR_NOACCCORRFILE             8
#define ERR_ANGLEOUTOFRANGE           9
#define ERR_NODOTFILENAME            10
#define ERR_BADFILENAMEFORMAT        11
#define ERR_UNIMPLEMENTEDBEAMENERGY  12
#define ERR_BADOPTARGS               13
#define ERR_INVALIDENTRIES           14
#define ERR_NENTRIESLARGE            15
#define ERR_NENTRIESNEGATIVE         16
#define ERR_UNDEFINED                17
#define ERR_OUTPUTTEXTFAILED         18
#define ERR_INVALIDHIPOFILE          19
// --+ 200 - 299 acc_corr +-----------------------------------------------------
#define ERR_NOEDGE                  201
#define ERR_BADEDGES                202
#define ERR_NOGENFILE               203
#define ERR_NOSIMFILE               204
#define ERR_WRONGGENFILE            205
#define ERR_BADGENFILE              206
#define ERR_WRONGSIMFILE            207
#define ERR_BADSIMFILE              208
// --+ 300 - 399 draw_plots +---------------------------------------------------
#define ERR_2DACCEPTANCEPLOT        301
#define ERR_INVALIDACCEPTANCEOPT    302
#define ERR_WRONGACCVARS            303
#define ERR_NOACCDATA               304
// --+ 400 - 499 extract_sf +---------------------------------------------------
#define ERR_INVALIDCALSECTOR        401
#define ERR_INVALIDCALLAYER         402
// --+ 500 - 599 hipo2root +----------------------------------------------------
// --+ 600 - 699 make_ntuples +-------------------------------------------------

#endif
