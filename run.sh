#!/bin/sh -f

# Setup
i="0"
args=""
for arg; do args="$args $arg"; done

# Run
run-groovy "acceptance.groovy$args"
