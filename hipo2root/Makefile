#=====================================================================
# MAKEFILE for building ROOT CONVERTER
#=====================================================================
ROOTCFLAGS  := $(shell root-config --cflags)
ROOTLDFLAGS := $(shell root-config --ldflags)
ROOTLIBS    := $(shell root-config --libs) -lEG
ROOTGLIBS   := $(shell root-config --glibs)

HIPOCFLAGS  := -I$(HIPO)/hipo4
HIPOLIBS    := -L$(HIPO)/lib -lhipo4

LZ4LIBS     := -L$(HIPO)/lz4/lib -llz4
LZ4INCLUDES := -I$(HIPO)/lz4/lib

F77        = gfortran
CXX       := g++

all: hipo2root

hipo2root: hipo2root.o
	$(CXX) -o hipo2root $< $(ROOTCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(LZ4LIBS) $(ROOTLIBS)

clean:
	@echo 'Removing all build files'
	@rm hipo2root.o hipo2root

#=====================================================================
# COMPILE RULES
#=====================================================================
%.o: %.cc
	$(CXX) -c $<  -O3 $(ROOTCFLAGS) $(HIPOCFLAGS) $(LZ4INCLUDES)
