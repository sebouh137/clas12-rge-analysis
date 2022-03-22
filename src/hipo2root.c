#include <cstdlib>
#include <iostream>

#include "reader.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

#include "err_handler.h"
#include "io_handler.h"
#include "bank_containers.h"

int main(int argc, char** argv) {
    char *in_filename = NULL;
    int  run_no = -1;

    if (hipo2root_handle_args_err(hipo2root_handle_args(argc, argv, &in_filename, &run_no),
                                  &in_filename));

    char *out_filename = (char*) malloc(22 * sizeof(char));
    sprintf(out_filename, "../root_io/%d.root", run_no);

    TFile *f = TFile::Open(out_filename, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    TTree *tree = new TTree("Tree", "Tree");
    REC_Particle rp;     rp.link_branches(tree);
    REC_Track rt;        rt.link_branches(tree);
    REC_Calorimeter rc;  rc.link_branches(tree);
    REC_Scintillator rs; rs.link_branches(tree);
    FMT_Tracks ft;       ft.link_branches(tree);

    // Setup.
    hipo::reader reader;
    reader.open(in_filename);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank rp_b(factory.getSchema("REC::Particle"));
    hipo::bank rt_b(factory.getSchema("REC::Track"));
    hipo::bank rc_b(factory.getSchema("REC::Calorimeter"));
    hipo::bank rs_b(factory.getSchema("REC::Scintillator"));
    hipo::bank ft_b(factory.getSchema("FMT::Tracks"));
    hipo::event event;

    int c = 0;
    while (reader.next()) {
        c++;
        if (c == 10000) {
            printf("Read %8d events...", c);
            fflush(stdout);
        }
        else if (c % 10000 == 0) {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            printf("Read %8d events...", c);
            fflush(stdout);
        }
        reader.read(event);

        event.getStructure(rp_b); rp.fill(rp_b);
        event.getStructure(rt_b); rt.fill(rt_b);
        event.getStructure(rc_b); rc.fill(rc_b);
        event.getStructure(rs_b); rs.fill(rs_b);
        event.getStructure(ft_b); ft.fill(ft_b);
        if (rp.get_nrows() + rt.get_nrows() + rs.get_nrows() + rc.get_nrows() + ft.get_nrows() > 0)
            tree->Fill();
    }
    printf(" Done!\n");

    // Clean up.
    f->Close();
    free(in_filename);
    free(out_filename);
    return 0;
}
