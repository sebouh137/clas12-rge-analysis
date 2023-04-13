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

static const unsigned int BYTE  = 0;
static const unsigned int SHORT = 1;
static const unsigned int INT   = 2;
static const unsigned int FLOAT = 3;

typedef struct{
    const char *addr;
    std::vector<double> *data;
    TBranch *branch;
    unsigned int type;
} rge_hipoentry;

typedef struct{
    long unsigned int nrows;
    std::map<const char *, rge_hipoentry> entries;
} rge_hipobank;

static std::map<const char *, rge_hipoentry>::const_iterator it;
rge_hipoentry entry_writer_init(const char *in_addr, unsigned int in_type);
// static rge_hipoentry entry_reader_init(const char *in_addr);

int rge_set_nrows(rge_hipobank *b, long unsigned int in_nrows);
rge_hipobank rge_recparticle_init();
// int rge_set_branches(rge_hipobank b, TTree *t);
int rge_link_branches(rge_hipobank *b, TTree *t);
int rge_fill(rge_hipobank *rb, hipo::bank hb);

#endif
