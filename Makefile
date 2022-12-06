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

# Locations.
BIN         := ./bin
BLD         := ./build
SRC         := ./src
LIB         := ./lib

# C++ compiler + flags.
CXX         := g++
CFLAGS      := -Wall -g # debug.
# CFLAGS      := -Wall -Werror -O3 # production.
CXX         := $(CXX) $(CFLAGS)

# ROOT.
ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLDFLAGS := $(shell root-config --ldflags)
RLIBS       := $(shell root-config --libs) -lEG
RXX         := $(CXX) $(ROOTCFLAGS)

# HIPO.
HIPOCFLAGS  := -I$(HIPO)/hipo4
HLIBS       := $(RLIBS) -L$(HIPO)/lib -lhipo4
HXX         := $(RXX) $(HIPOCFLAGS)

# Object lists.
O_HIPO2ROOT   := $(BLD)/bank_containers.o $(BLD)/file_handler.o \
				 $(BLD)/io_handler.o
O_EXTRACTSF   := $(BLD)/bank_containers.o $(BLD)/constants.o \
				 $(BLD)/file_handler.o $(BLD)/io_handler.o $(BLD)/utilities.o
O_ACCCORR     := $(BLD)/file_handler.o $(BLD)/io_handler.o $(BLD)/utilities.o
O_MKNTUPLES   := $(BLD)/bank_containers.o $(BLD)/constants.o \
				 $(BLD)/file_handler.o $(BLD)/io_handler.o \
				 $(BLD)/particle.o $(BLD)/utilities.o
O_DRAWPLOTS   := $(BLD)/constants.o $(BLD)/file_handler.o \
				 $(BLD)/io_handler.o $(BLD)/utilities.o

all: $(BIN)/hipo2root $(BIN)/extract_sf $(BIN)/acc_corr \
	 $(BIN)/make_ntuples $(BIN)/draw_plots

$(BIN)/hipo2root: $(O_HIPO2ROOT) $(SRC)/hipo2root.c
	$(HXX) $(O_HIPO2ROOT) $(SRC)/hipo2root.c -o $(BIN)/hipo2root $(HLIBS)

$(BIN)/extract_sf: $(O_EXTRACTSF) $(SRC)/extract_sf.c
	$(HXX) $(O_EXTRACTSF) $(SRC)/extract_sf.c -o $(BIN)/extract_sf $(HLIBS)

$(BIN)/acc_corr: $(O_ACCCORR) $(SRC)/acc_corr.c
	$(RXX) $(O_ACCCORR) $(SRC)/acc_corr.c -o $(BIN)/acc_corr $(RLIBS)

$(BIN)/make_ntuples: $(O_MKNTUPLES) $(SRC)/make_ntuples.c
	$(HXX) $(O_MKNTUPLES) $(SRC)/make_ntuples.c -o $(BIN)/make_ntuples $(HLIBS)

$(BIN)/draw_plots: $(O_DRAWPLOTS) $(SRC)/draw_plots.c
	$(RXX) $(O_DRAWPLOTS) $(SRC)/draw_plots.c -o $(BIN)/draw_plots $(RLIBS)

$(BLD)/bank_containers.o: $(SRC)/bank_containers.c $(LIB)/bank_containers.h
	$(HXX) -c $(SRC)/bank_containers.c -o $(BLD)/bank_containers.o

$(BLD)/constants.o: $(SRC)/constants.c $(LIB)/constants.h
	$(CXX) -c $(SRC)/constants.c -o $(BLD)/constants.o

$(BLD)/io_handler.o: $(SRC)/io_handler.c $(LIB)/io_handler.h
	$(CXX) -c $(SRC)/io_handler.c -o $(BLD)/io_handler.o

$(BLD)/file_handler.o: $(SRC)/file_handler.c $(LIB)/file_handler.h
	$(CXX) -c $(SRC)/file_handler.c -o $(BLD)/file_handler.o

$(BLD)/particle.o: $(SRC)/particle.c $(LIB)/particle.h
	$(HXX) -c $(SRC)/particle.c -o $(BLD)/particle.o

$(BLD)/utilities.o: $(SRC)/utilities.c $(LIB)/utilities.h
	$(RXX) -c $(SRC)/utilities.c -o $(BLD)/utilities.o

clean:
	@echo "Removing all build files and binaries."
	@rm $(BLD)/*.o
	@rm $(BIN)/*
