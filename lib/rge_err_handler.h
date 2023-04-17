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

#ifndef RGE_ERRHANDLER
#define RGE_ERRHANDLER

// --+ preamble +---------------------------------------------------------------
// C.
#include <stdio.h>

// C++.
#include <map>

// typedefs.
typedef unsigned int uint;
typedef long unsigned int luint;
typedef long int lint;

// --+ internal +---------------------------------------------------------------
/**
 * Entry point to all error handling. Decides how to react to rge_errno.
 *
 * @return : error code, handled by rge_print_usage().
 *   * 0 : no error was found.
 *   * 1 : a user error was found.
 *   * 2 : a programmer error was found.
 */
static int handle_err();

// --+ library +----------------------------------------------------------------
/**
 * Error number. Initially defined to RGEERR_UNDEFINED to check if the program
 *     ends abruptly without setting an error number.
 *
 * NOTE. To check for undefined errors, all run() functions in the code should
 *       have a line with `rge_errno = RGEERR_NOERR;` before returning 0.
 */
extern uint rge_errno;

/**
 * Print usage and exit.
 *
 * @param msg : usage message.
 * @return    : err received from handle_err().
 */
int rge_print_usage(const char *msg);

// --+ error numbers +----------------------------------------------------------
// --+   0 -   9 basic functionalities +----------------------------------------
#define RGEERR_NOERR                     0
#define RGEERR_USAGE                     1
#define RGEERR_UNDEFINED                 2
// --+  10 -  49 argument errors +----------------------------------------------
#define RGEERR_BADOPTARGS               10
#define RGEERR_INVALIDENTRIES           11
#define RGEERR_NENTRIESLARGE            12
#define RGEERR_NENTRIESNEGATIVE         13
#define RGEERR_NOEDGE                   14
#define RGEERR_BADEDGES                 15
#define RGEERR_INVALIDFMTNLAYERS        16
#define RGEERR_INVALIDACCEPTANCEOPT     17
// --+  50 -  99 file errors +--------------------------------------------------
#define RGEERR_NOINPUTFILE              50
#define RGEERR_NOSAMPFRACFILE           51
#define RGEERR_NOACCCORRFILE            52
#define RGEERR_NOGENFILE                53
#define RGEERR_NOSIMFILE                54
#define RGEERR_NODOTFILENAME            55
#define RGEERR_BADFILENAMEFORMAT        56
#define RGEERR_INVALIDROOTFILE          57
#define RGEERR_INVALIDHIPOFILE          58
#define RGEERR_BADINPUTFILE             59
#define RGEERR_BADGENFILE               60
#define RGEERR_BADSIMFILE               61
#define RGEERR_BADROOTFILE              62
#define RGEERR_WRONGGENFILE             63
#define RGEERR_WRONGSIMFILE             64
#define RGEERR_OUTFILEEXISTS            65
#define RGEERR_OUTPUTROOTFAILED         66
#define RGEERR_OUTPUTTEXTFAILED         67
// --+ 100 - 149 detector errors +----------------------------------------------
#define RGEERR_INVALIDCALLAYER         100
#define RGEERR_INVALIDCALSECTOR        101
#define RGEERR_INVALIDCHERENKOVID      102
#define RGEERR_NOFMTBANK               103
// --+ 150 - 199 program errors +-----------------------------------------------
#define RGEERR_UNIMPLEMENTEDBEAMENERGY 150
#define RGEERR_2DACCEPTANCEPLOT        151
#define RGEERR_WRONGACCVARS            152
#define RGEERR_INVALIDBANKID           153
// --+ 200 - 249 particle errors +----------------------------------------------
#define RGEERR_PIDNOTFOUND             201
#define RGEERR_UNSUPPORTEDPID          202
// --+ 900 - 999 miscellaneous +------------------------------------------------
#define RGEERR_ANGLEOUTOFRANGE         900
#define RGEERR_NOACCDATA               901
// -----------------------------------------------------------------------------

#endif
