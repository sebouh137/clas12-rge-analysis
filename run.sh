#!/bin/sh -f

# Setup
CLASGROOVY="/home/twig/data/code/jsw/coatjava-5.6.8/bin/run-groovy"
FILE="/home/twig/data/code/jsw/recon_data/out_clas_012016.hipo"

# Run
$CLASGROOVY acceptance.groovy $FILE
