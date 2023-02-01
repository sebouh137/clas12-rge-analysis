#!/bin/bash
# CLAS12 RG-E Analyser.
# Copyright (C) 2022 Bruno Benkel
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You can see a copy of the GNU Lesser Public License under the LICENSE file.

# Compile.
g++ -c -h -Wall `root-config --cflags` draw_acc_corr.c
g++ `root-config --glibs` ../build/file_handler.o draw_acc_corr.o

# Run.
./a.out

# Clean up.
rm draw_acc_corr.o a.out
