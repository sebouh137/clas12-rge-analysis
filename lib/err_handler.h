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

// --+ ERRCODES +---------------------------------------------------------------
// --+   0 -   9 basic functionalities +----------------------------------------
#define ERR_NOERR                     0
#define ERR_USAGE                     1
#define ERR_UNDEFINED                 2
// --+  10 -  49 argument errors +----------------------------------------------
#define ERR_BADOPTARGS               10
#define ERR_INVALIDENTRIES           11
#define ERR_NENTRIESLARGE            12
#define ERR_NENTRIESNEGATIVE         13
#define ERR_NOEDGE                   14
#define ERR_BADEDGES                 15
#define ERR_INVALIDFMTNLAYERS        16
#define ERR_INVALIDACCEPTANCEOPT     17
// --+  50 -  99 file errors +--------------------------------------------------
#define ERR_NOINPUTFILE              50
#define ERR_NOSAMPFRACFILE           51
#define ERR_NOACCCORRFILE            52
#define ERR_NOGENFILE                53
#define ERR_NOSIMFILE                54
#define ERR_NODOTFILENAME            55
#define ERR_BADFILENAMEFORMAT        56
#define ERR_INVALIDROOTFILE          57
#define ERR_INVALIDHIPOFILE          58
#define ERR_BADINPUTFILE             59
#define ERR_BADGENFILE               60
#define ERR_BADSIMFILE               61
#define ERR_BADROOTFILE              62
#define ERR_WRONGGENFILE             63
#define ERR_WRONGSIMFILE             64
#define ERR_OUTFILEEXISTS            65
#define ERR_OUTPUTROOTFAILED         66
#define ERR_OUTPUTTEXTFAILED         67
// --+ 100 - 149 detector errors +----------------------------------------------
#define ERR_INVALIDCALLAYER         100
#define ERR_INVALIDCALSECTOR        101
#define ERR_INVALIDCHERENKOVID      102
// --+ 150 - 199 program errors +-----------------------------------------------
#define ERR_UNIMPLEMENTEDBEAMENERGY 150
#define ERR_2DACCEPTANCEPLOT        151
#define ERR_WRONGACCVARS            152
// --+ 900 - 999 miscellaneous +------------------------------------------------
#define ERR_ANGLEOUTOFRANGE         900
#define ERR_NOACCDATA               901
// -----------------------------------------------------------------------------

#endif
