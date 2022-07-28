# CLAS12 RG-E Analyser.
# Copyright (C) 2022 Bruno Benkel
#
# This program is free software: you can redistribute it and/or modify it under the terms of the
# GNU Lesser General Public License as published by the Free Software Foundation, either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You can see a copy of the GNU Lesser Public License under the LICENSE file.

BIN         := ./bin
BLD         := ./build
SRC         := ./src
LIB         := ./lib

CXX         := g++
CFLAGS      := -Wall -g
# CFLAGS      := -Wall -Werror -O3

ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLDFLAGS := $(shell root-config --ldflags)
ROOTLIBS    := $(shell root-config --libs) -lEG
ROOTGLIBS   := $(shell root-config --glibs)

HIPOCFLAGS  := -I$(HIPO)/hipo4
HIPOLIBS    := -L$(HIPO)/lib -lhipo4

LZ4LIBS     := -L$(HIPO)/lz4/lib -llz4
LZ4INCLUDES := -I$(HIPO)/lz4/lib

OBJS        := $(BLD)/bank_containers.o $(BLD)/constants.o $(BLD)/err_handler.o \
			   $(BLD)/file_handler.o $(BLD)/io_handler.o $(BLD)/particle.o $(BLD)/utilities.o

all: $(BIN)/hipo2root $(BIN)/extract_sf $(BIN)/make_ntuples $(BIN)/draw_plots

$(BIN)/draw_plots: $(OBJS) $(SRC)/draw_plots.c
	$(CXX) $(CFLAGS) $(OBJS) $(SRC)/draw_plots.c -o $(BIN)/draw_plots $(ROOTCFLAGS) \
	$(ROOTLDFLAGS) $(ROOTLIBS)

$(BIN)/make_ntuples: $(OBJS) $(SRC)/make_ntuples.c
	$(CXX) $(CFLAGS) $(OBJS) $(SRC)/make_ntuples.c -o $(BIN)/make_ntuples $(ROOTCFLAGS) \
	$(HIPOCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BIN)/extract_sf: $(OBJS) $(SRC)/extract_sf.c
	$(CXX) $(CFLAGS) $(OBJS) $(SRC)/extract_sf.c -o $(BIN)/extract_sf $(ROOTCFLAGS) $(HIPOCFLAGS) \
	$(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BIN)/hipo2root: $(OBJS) $(SRC)/hipo2root.c
	$(CXX) $(CFLAGS) $(OBJS) $(ROOTCFLAGS) $(HIPOCFLAGS) $(LZ4INCLUDES) $(SRC)/hipo2root.c \
	-o $(BIN)/hipo2root $(ROOTCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(LZ4LIBS) $(ROOTLIBS)

$(BLD)/bank_containers.o: $(SRC)/bank_containers.c $(LIB)/bank_containers.h
	$(CXX) $(CFLAGS) -c $(SRC)/bank_containers.c -o $(BLD)/bank_containers.o $(ROOTCFLAGS) \
	$(HIPOCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BLD)/constants.o: $(SRC)/constants.c $(LIB)/constants.h
	$(CXX) $(CFLAGS) -std=c++11 -c $(SRC)/constants.c -o $(BLD)/constants.o

$(BLD)/err_handler.o: $(SRC)/err_handler.c $(LIB)/err_handler.h
	$(CXX) $(CFLAGS) -c $(SRC)/err_handler.c -o $(BLD)/err_handler.o

$(BLD)/file_handler.o: $(SRC)/file_handler.c $(LIB)/file_handler.h
	$(CXX) $(CFLAGS) -c $(SRC)/file_handler.c -o $(BLD)/file_handler.o

$(BLD)/io_handler.o: $(SRC)/io_handler.c $(LIB)/io_handler.h
	$(CXX) $(CFLAGS) -c $(SRC)/io_handler.c -o $(BLD)/io_handler.o

$(BLD)/particle.o: $(SRC)/particle.c $(LIB)/particle.h
	$(CXX) $(CFLAGS) -c $(SRC)/particle.c -o $(BLD)/particle.o  $(ROOTCFLAGS) $(HIPOCFLAGS) \
	$(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BLD)/utilities.o: $(SRC)/utilities.c $(LIB)/utilities.h
	$(CXX) $(CFLAGS) -c $(SRC)/utilities.c -o $(BLD)/utilities.o $(ROOTCFLAGS) $(ROOTLDFLAGS) \
	$(ROOTLIBS)

clean:
	@echo "Removing all build files and binaries."
	@rm $(BLD)/*.o
	@rm $(BIN)/*
