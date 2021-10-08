#!/bin/sh -f

# Setup
CLASGROOVY="/home/twig/data/code/jsw/coatjava-5.6.8/bin/run-groovy"

i="0"
args=""
for arg; do args="$args $arg"; done

# Run
$CLASGROOVY "acceptance.groovy$args"
