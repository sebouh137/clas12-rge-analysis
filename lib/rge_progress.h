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

#ifndef RGE_PROGRESS
#define RGE_PROGRESS

// --+ preamble +---------------------------------------------------------------
// C.
#include "stdio.h"

// typedefs.
typedef unsigned int uint;
typedef long unsigned int luint;
typedef long int lint;

// --+ internal +---------------------------------------------------------------
/** Length (in chars) of the progress bar. */
static const int PBARLENGTH = 50;
/** Total number of entries. */
static lint pbar_nentries = 0;
/** Counter to check in which part of the progress bar we are. */
static lint pbar_divcntr  = 0;
/** Counter to check if the bar needs to be updated or not. */
static lint pbar_splitter = 0;

// --+ library +----------------------------------------------------------------
/** Set the pbar_nentries state variable. */
int rge_pbar_set_nentries(lint in_nentries);

/** Reset the pbar_nentries and pbar_divcntr state variables to 0. */
int rge_pbar_reset();

/**
 * Update a progress bar counting the number of entries processed. This should
 *     be called every time entry changes.
 *
 * @param entry : number of the entry being processed.
 * @return      : 0 if no change was made, 1 otherwise, and 2 if bar is full.
 */
int rge_pbar_update(lint entry);

#endif
