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

#ifndef RGE_HIPOBANK
#define RGE_HIPOBANK

#include <map>
#include <vector>
#include <TTree.h>
#include "reader.h"

/** internal variables to refer to different primitive types. */
// NOTE. These could be improved by adding a reference to the primitive itself.
static const unsigned int BYTE  = 0;
static const unsigned int SHORT = 1;
static const unsigned int INT   = 2;
static const unsigned int FLOAT = 3;

// --+ structs +----------------------------------------------------------------
/** Struct containing one entry of a particular hipo bank. */
typedef struct{
    const char *addr;          /** Address of entry (BANK::NAME::VAR). */
    std::vector<double> *data; /** Vector with data of the entry. */
    TBranch *branch;           /** Pointer to TBranch where to write data. */
    unsigned int type;         /** Int containing variable type in hypo bank. */
} rge_hipoentry;

/** Struct containing a map of all entries associated to a hipo bank. */
typedef struct{
    long unsigned int nrows;
    std::map<const char *, rge_hipoentry> entries;
} rge_hipobank;

// --+ internal +---------------------------------------------------------------
/** Internal iterator used to loop through entries. */
static std::map<const char *, rge_hipoentry>::const_iterator it;

/**
 * Initialize and return one rge_hipoentry defined to *write* to a TTree.
 *     Parameters addr and type are initialized to input, data is initialized to
 *     an empty vector to be read from hipo, and branch is initialized to a
 *     nullptr to be handled by root.
 */
static rge_hipoentry entry_writer_init(
        const char *in_addr, unsigned int in_type
);

/**
 * Initialize one rge_hipoentry defined to *read* from a TTree.
 */
// static rge_hipoentry entry_reader_init(const char *in_addr);

/** Set b.nrows to in_rows. */
static int set_nrows(rge_hipobank *b, long unsigned int in_nrows);

// --+ library +----------------------------------------------------------------
/** Initializers. TO BE UPDATED. */
rge_hipobank rge_recparticle_init();
rge_hipobank rge_rectrack_init();
rge_hipobank rge_reccalorimeter_init();
rge_hipobank rge_reccherenkov_init();
rge_hipobank rge_recscintillator_init();
rge_hipobank rge_fmttracks_init();
// int rge_set_branches(rge_hipobank b, TTree *t);

/** Link branches of t to entries of b. */
int rge_link_branches(rge_hipobank *b, TTree *t);

/** Fill entries in rb with data from hb. */
int rge_fill(rge_hipobank *rb, hipo::bank hb);

#endif
