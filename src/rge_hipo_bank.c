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

#include "../lib/rge_hipo_bank.h"

// --+ internal +---------------------------------------------------------------
rge_hipoentry entry_writer_init(const char *in_addr, unsigned int in_type) {
    return __extension__ (rge_hipoentry) {
            .addr = in_addr, .data = {}, .branch = nullptr, .type = in_type
    };
}

// rge_hipoentry entry_reader_init(const char *in_addr) {
//     return __extension__ (rge_hipoentry) {
//             .addr = in_addr, .data = nullptr, .branch = nullptr
//     };
// }

int set_nrows(rge_hipobank *b, long unsigned int in_nrows) {
    // Set internal variable.
    b->nrows = in_nrows;

    // Resize vectors.
    for (it = b->entries.begin(); it != b->entries.end(); ++it) {
        const char *key = it->first;
        b->entries.at(key).data->resize(b->nrows);
    }

    return 0;
}

/** Static map containing all entry lists. */
static std::map<
        const char *, std::map<const char *, rge_hipoentry, cmp_str>, cmp_str
> ENTRYMAP = {
    {RGE_RECPARTICLE, {
        {"pid",     entry_writer_init("REC::Particle::pid",     INT)},
        {"vx",      entry_writer_init("REC::Particle::vx",      FLOAT)},
        {"vy",      entry_writer_init("REC::Particle::vy",      FLOAT)},
        {"vz",      entry_writer_init("REC::Particle::vz",      FLOAT)},
        {"px",      entry_writer_init("REC::Particle::px",      FLOAT)},
        {"py",      entry_writer_init("REC::Particle::py",      FLOAT)},
        {"pz",      entry_writer_init("REC::Particle::pz",      FLOAT)},
        {"vt",      entry_writer_init("REC::Particle::vt",      FLOAT)},
        {"charge",  entry_writer_init("REC::Particle::charge",  BYTE)},
        {"beta",    entry_writer_init("REC::Particle::beta",    FLOAT)},
        {"chi2pid", entry_writer_init("REC::Particle::chi2pid", FLOAT)},
        {"status",  entry_writer_init("REC::Particle::status",  SHORT)}
    }},
    {RGE_RECTRACK, {
        {"index",  entry_writer_init("REC::Track::index",  SHORT)},
        {"pindex", entry_writer_init("REC::Track::pindex", SHORT)},
        {"sector", entry_writer_init("REC::Track::sector", BYTE)},
        {"NDF",    entry_writer_init("REC::Track::ndf",    SHORT)},
        {"chi2",   entry_writer_init("REC::Track::chi2",   FLOAT)}
    }},
    {RGE_RECCALORIMETER, {
        {"pindex", entry_writer_init("REC::Calorimeter::pindex", SHORT)},
        {"layer",  entry_writer_init("REC::Calorimeter::layer",  BYTE)},
        {"sector", entry_writer_init("REC::Calorimeter::sector", BYTE)},
        {"energy", entry_writer_init("REC::Calorimeter::energy", FLOAT)},
        {"time",   entry_writer_init("REC::Calorimeter::time",   FLOAT)}
    }},
    {RGE_RECCHERENKOV, {
        {"pindex",   entry_writer_init("REC::Cherenkov::pindex",   SHORT)},
        {"detector", entry_writer_init("REC::Cherenkov::detector", BYTE)},
        {"nphe",     entry_writer_init("REC::Cherenkov::nphe",     FLOAT)}
    }},
    {RGE_RECSCINTILLATOR, {
        {"pindex",   entry_writer_init("REC::Scintillator::pindex",   SHORT)},
        {"time",     entry_writer_init("REC::Scintillator::time",     FLOAT)},
        {"detector", entry_writer_init("REC::Scintillator::detector", BYTE)},
        {"layer",    entry_writer_init("REC::Scintillator::layer",    BYTE)}
    }},
    {RGE_FMTTRACKS, {
        {"index",  entry_writer_init("FMT::Tracks::index", SHORT)},
        {"NDF",    entry_writer_init("FMT::Tracks::ndf",   INT)},
        {"Vtx0_x", entry_writer_init("FMT::Tracks::vx",    FLOAT)},
        {"Vtx0_y", entry_writer_init("FMT::Tracks::vy",    FLOAT)},
        {"Vtx0_z", entry_writer_init("FMT::Tracks::vz",    FLOAT)},
        {"p0_x",   entry_writer_init("FMT::Tracks::px",    FLOAT)},
        {"p0_y",   entry_writer_init("FMT::Tracks::py",    FLOAT)},
        {"p0_z",   entry_writer_init("FMT::Tracks::pz",    FLOAT)}
    }}
};

// --+ library +----------------------------------------------------------------
rge_hipobank rge_hipobank_init(const char *bank_version) {
    rge_hipobank b;
    b.nrows = 0;

    try {b.entries = ENTRYMAP.at(bank_version);}
    catch (...) {rge_errno = RGEERR_INVALIDBANKID;}

    return b;
}

rge_hipobank rge_hipobank_init(const char *bank_version, TTree *t) {
    rge_hipobank b = rge_hipobank_init(bank_version);

    for (it = b.entries.begin(); it != b.entries.end(); ++it) {
        const char *key = it->first;
        t->SetBranchAddress(
                b.entries.at(key).addr,
                &(b.entries.at(key).data),
                &(b.entries.at(key).branch)
        );
    }

    return b;
}

int rge_link_branches(rge_hipobank *b, TTree *t) {
    for (it = b->entries.begin(); it != b->entries.end(); ++it) {
        const char *key = it->first;
        t->Branch(b->entries.at(key).addr, &(b->entries.at(key).data));
    }

    return 0;
}

int rge_fill(rge_hipobank *rb, hipo::bank hb) {
    set_nrows(rb, static_cast<long unsigned int>(hb.getRows()));

    for (long unsigned int row = 0; row < rb->nrows; ++row) {
        for (it = rb->entries.begin(); it != rb->entries.end(); ++it) {
            const char *key = it->first;
            double bank_data = 0;
            switch (rb->entries.at(key).type) {
                case BYTE:
                    bank_data = static_cast<double>(hb.getByte(key, row));
                    break;
                case SHORT:
                    bank_data = static_cast<double>(hb.getShort(key, row));
                    break;
                case INT:
                    bank_data = static_cast<double>(hb.getInt(key, row));
                    break;
                case FLOAT:
                    bank_data = static_cast<double>(hb.getFloat(key, row));
                    break;
                default:
                    // TODO. Set rge_errno.
                    return 1;
            }
            rb->entries.at(key).data->at(row) = bank_data;
        }
    }

    return 0;
}

int rge_get_entries(rge_hipobank *b, TTree *t, int idx) {
    for (it = b->entries.begin(); it != b->entries.end(); ++it) {
        const char *key = it->first;
        b->entries.at(key).branch->GetEntry(t->LoadTree(idx));
    }

    return 0;
}
