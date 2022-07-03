#include <cstdlib>
#include <iostream>

#include "reader.h"
#include "utils.h"
#include "TFile.h"
#include "TTree.h"
#include "Compression.h"

#include "../lib/err_handler.h"
#include "../lib/io_handler.h"
#include "../lib/bank_containers.h"

int main(int argc, char **argv) {
    char *in_filename = NULL;
    int  run_no = -1;

    if (hipo2root_handle_args_err(hipo2root_handle_args(argc, argv, &in_filename, &run_no),
                                  &in_filename))
        return 1;

    char *out_filename = (char *) malloc(128 * sizeof(char));
    sprintf(out_filename, "../root_io/banks_%06d.root", run_no);

    TFile *f = TFile::Open(out_filename, "RECREATE");
    f->SetCompressionAlgorithm(ROOT::kLZ4);

    TTree *tree = new TTree("Tree", "Tree");
    REC_Particle     rpart; rpart.link_branches(tree);
    REC_Track        rtrk;  rtrk .link_branches(tree);
    REC_Calorimeter  rcal;  rcal .link_branches(tree);
    REC_Cherenkov    rche;  rche .link_branches(tree);
    REC_Scintillator rsci;  rsci .link_branches(tree);
    FMT_Tracks       ftrk;  ftrk .link_branches(tree);

    // Setup.
    hipo::reader reader;
    reader.open(in_filename);

    hipo::dictionary factory;
    reader.readDictionary(factory);

    hipo::bank rpart_b(factory.getSchema("REC::Particle"));
    hipo::bank rtrk_b( factory.getSchema("REC::Track"));
    hipo::bank rcal_b( factory.getSchema("REC::Calorimeter"));
    hipo::bank rche_b( factory.getSchema("REC::Cherenkov"));
    hipo::bank rsci_b( factory.getSchema("REC::Scintillator"));
    hipo::bank ftrk_b( factory.getSchema("FMT::Tracks"));
    hipo::event event;

    int c = 0;
    while (reader.next()) {
        c++;
        if (c % 10000 == 0) {
            if (c != 10000) printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            printf("Read %8d events...", c);
            fflush(stdout);
        }
        reader.read(event);

        event.getStructure(rpart_b); rpart.fill(rpart_b);
        event.getStructure(rtrk_b);  rtrk .fill(rtrk_b);
        event.getStructure(rcal_b);  rcal .fill(rcal_b);
        event.getStructure(rche_b);  rche .fill(rche_b);
        event.getStructure(rsci_b);  rsci .fill(rsci_b);
        event.getStructure(ftrk_b);  ftrk .fill(ftrk_b);
        if (rpart.get_nrows() + rtrk.get_nrows() + rcal.get_nrows()
                + rche.get_nrows()  + rsci.get_nrows() + ftrk.get_nrows() > 0)
            tree->Fill();
    }
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    printf("Read %8d events... Done!\n", c);

    // Clean up.
    f->Close();
    free(in_filename);
    free(out_filename);
    return 0;
}
