// CLAS12 RG-E Analyser.
// Copyright (C) 2022 Bruno Benkel
//
// This program is free software: you can redistribute it and/or modify it under the terms of the
// GNU Lesser General Public License as published by the Free Software Foundation, either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You can see a copy of the GNU Lesser Public License under the LICENSE file.

#ifndef ERR_HANDLER
#define ERR_HANDLER

#include <stdio.h>
#include <stdlib.h>

int make_ntuples_usage();
int make_ntuples_handle_args_err(int errcode, char **in_filename, int run_no);
int make_ntuples_err(int errcode, char **in_filename);
int extractsf_usage();
int extractsf_handle_args_err(int errcode, char **in_filename);
int extractsf_err(int errcode, char **in_filename);
int hipo2root_usage();
int hipo2root_handle_args_err(int errcode, char **in_filename);

#endif
