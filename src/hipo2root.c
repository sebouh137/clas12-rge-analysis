#include <cstdlib>
#include <iostream>

#include "reader.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

#include "bank_containers.h"

int main(int argc, char** argv) {
    int nevents = 0;
    char infile[512];
    char outfile_root[512];

    if (argc > 2) {
        nevents = atoi(argv[2]);
    }
    if (argc > 1) {
        // TODO. Remove extension.
        sprintf(infile, "%s", argv[1]);
        sprintf(outfile_root, "%s.root", argv[1]);
    }
    else {
        printf("Error. No file name provided. Exiting...\n");
        exit(0);
    }

    TFile *f = TFile::Open(outfile_root, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    TTree *tree = new TTree("Tree", "Tree");
    int nevent = 0;
    tree->Branch("event", &nevent);

    // REC::Particle.
    REC_Particle rp;
    rp.link_branches(tree);

    // REC::Track.
    REC_Track rt;
    rt.link_branches(tree);

    // REC::Calorimeter.
    REC_Calorimeter rc;
    rc.link_branches(tree);

    // REC::Scintillator.
    REC_Scintillator rs;
    rs.link_branches(tree);

    // FMT::Tracks.
    FMT_Tracks ft;
    ft.link_branches(tree);

    // Prepare infile reader.
    hipo::reader reader;
    reader.open(infile);

    // Prepare factory.
    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank  rp_b(factory.getSchema("REC::Particle"));
    hipo::bank  rt_b(factory.getSchema("REC::Track"));
    hipo::bank  rc_b(factory.getSchema("REC::Calorimeter"));
    hipo::bank  rs_b(factory.getSchema("REC::Scintillator"));
    hipo::bank  ft_b(factory.getSchema("FMT::Tracks"));
    hipo::event event;

    while (reader.next()) {
        if (nevent++ >= nevents && nevents != 0) break;
        reader.read(event);

        // REC::Particle
        event.getStructure(rp_b);
        rp.resize(rp_b.getRows());
        rp.fill(rp_b, rp_b.getRows());

        // REC::Track
        event.getStructure(rt_b);
        rt.resize(rt_b.getRows());
        rt.fill(rt_b, rt_b.getRows());

        // REC::Calorimeter
        event.getStructure(rc_b);
        rc.resize(rc_b.getRows());
        rc.fill(rc_b, rc_b.getRows());

        // REC::Scintillator
        event.getStructure(rs_b);
        rs.resize(rs_b.getRows());
        rs.fill(rs_b, rs_b.getRows());

        // FMT::Tracks
        event.getStructure(ft_b);
        ft.resize(ft_b.getRows());
        ft.fill(ft_b, ft_b.getRows());

        if (rp_b.getRows() > 0 || rt_b.getRows() > 0 || rs_b.getRows() > 0 || rc_b.getRows() > 0
                || ft_b.getRows() > 0)
            tree->Fill();
    }

    f->Close();

    return 0;
}
