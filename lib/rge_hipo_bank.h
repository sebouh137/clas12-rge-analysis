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

// --+ preamble +---------------------------------------------------------------
// C.
#include "string.h"

// C++.
#include <map>
#include <vector>

// ROOT.
#include <TTree.h>

// HIPO.
#include "bank.h"
#include "reader.h"

// rge-analysis.
#include "rge_err_handler.h"

/** Definitions to refer to different hipo banks using strings. */
#define RGE_RECPARTICLE     "REC::Particle"
#define RGE_RECTRACK        "REC::Track"
#define RGE_RECCALORIMETER  "REC::Calorimeter"
#define RGE_RECCHERENKOV    "REC::Cherenkov"
#define RGE_RECSCINTILLATOR "REC::Scintillator"
#define RGE_FMTTRACKS       "FMT::Tracks"

// --+ structs +----------------------------------------------------------------
/** String comparer to use const char * as std::map keys. */
struct cmp_str {
    bool operator() (char const *a, char const *b) const {
        return std::strcmp(a, b) < 0;
    }
};

/** Struct containing one entry of a particular hipo bank. */
typedef struct {
    const char *addr;          /** Address of entry (BANK::NAME::VAR). */
    std::vector<double> *data; /** Vector with data of the entry. */
    TBranch *branch;           /** Pointer to TBranch where to write data. */
    unsigned int type;         /** Int containing variable type in hypo bank. */
} rge_hipoentry;

/** Struct containing a map of all entries associated to a hipo bank. */
typedef struct {
    long unsigned int nrows;
    std::map<const char *, rge_hipoentry, cmp_str> entries;
} rge_hipobank;

// --+ internal +---------------------------------------------------------------
/** internal variables to refer to different primitive types. */
// NOTE. These could be improved by adding a reference to the primitive itself.
static const unsigned int BYTE  = 0;
static const unsigned int SHORT = 1;
static const unsigned int INT   = 2;
static const unsigned int FLOAT = 3;

/** Internal iterator used to loop through entries. */
static std::map<const char *, rge_hipoentry, cmp_str>::const_iterator it;

/**
 * Initialize and return one rge_hipoentry defined to *write* to a TTree.
 *     Parameters addr and type are initialized to input, data is initialized to
 *     an empty vector to be read from hipo, and branch is initialized to a
 *     nullptr to be handled by root.
 */
static rge_hipoentry entry_writer_init(
        const char *in_addr, unsigned int in_type
);

/** Set b.nrows to in_rows. */
static int set_nrows(rge_hipobank *b, long unsigned int in_nrows);

// --+ library +----------------------------------------------------------------
/** Initialize rge_hipobank based on static map related to bank_version. */
rge_hipobank rge_hipobank_init(const char *bank_version);

/** Initialize rge_hipobank and set branch addresses to t's branches. */
rge_hipobank rge_hipobank_init(const char *bank_version, TTree *t);

/** Link branches of t to entries of b. */
int rge_link_branches(rge_hipobank *b, TTree *t);

/** Fill entries in rb with data from hb. */
int rge_fill(rge_hipobank *rb, hipo::bank hb);

/** Read entries from t into b. */
int rge_get_entries(rge_hipobank *b, TTree *t, int idx);

#endif
