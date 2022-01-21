#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <TMath.h>
#include <TH1.h>
#include <TF1.h>

#include "err_handler.h"
#include "file_handler.h"
#include "io_handler.h"

// TODO. TEMP CODE === === === ===
auto pi = TMath::Pi();

// function code in C
double single(double *x, double *par) {
    return pow(sin(pi*par[0]*x[0])/(pi*par[0]*x[0]),2);
}

double nslit0(double *x,double *par){
    return pow(sin(pi*par[1]*x[0])/sin(pi*x[0]),2);
}

double nslit(double *x, double *par){
    return single(x,par) * nslit0(x,par);
}

// This is the main program
void slits() {
    float r, ns;

    // request user input
    printf("slit width / g ?");
    scanf("%f", &r);
    printf("# of slits?");
    scanf("%f", &ns);
    printf("interference pattern for %f slits, width/distance: %f\n", ns, r);

    // define function and set options
    TF1 *Fnslit  = new TF1("Fnslit",nslit,-5.001,5.,2);
    Fnslit->SetNpx(500);

    // set parameters, as read in above
    Fnslit->SetParameter(0,r);
    Fnslit->SetParameter(1,ns);

    // draw the interference pattern for a grid with n slits
    Fnslit->Draw();
}
// === === === === === === === ===

// Call program from terminal, C-style.
int main(int argc, char **argv) {
    bool   use_fmt     = false;
    int    nevents     = -1;
    char   *input_file = NULL;
    int    run_no      = -1;
    double beam_energy = -1;

    if (handle_args_err(
            handle_args(argc, argv, &use_fmt, &nevents, &input_file, &run_no, &beam_energy),
            &input_file, run_no)
        ) return 1;

    printf("use_fmt:    %d\n", use_fmt);
    printf("nevents:    %d\n", nevents);
    printf("input_file: %s\n", input_file);
    printf("run_no:     %d\n", run_no);

    // NOTE. Program goes here.
    slits();

    free(input_file);
    return 0;
}
