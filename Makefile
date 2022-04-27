BIN         := ./bin
BLD         := ./build
SRC         := ./src

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

OBJS        := $(BLD)/bank_containers.o $(BLD)/constants.o $(BLD)/err_handler.o $(BLD)/file_handler.o $(BLD)/io_handler.o $(BLD)/particle.o $(BLD)/utilities.o

all: $(BIN)/hipo2root $(BIN)/extract_sf $(BIN)/acceptance

$(BIN)/acceptance: $(OBJS) $(SRC)/acceptance.c
	$(CXX) $(CFLAGS) $(OBJS) $(SRC)/acceptance.c -o $(BIN)/acceptance $(ROOTCFLAGS) $(HIPOCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BIN)/extract_sf: $(OBJS) $(SRC)/extract_sf.c
	$(CXX) $(CFLAGS) $(OBJS) $(SRC)/extract_sf.c -o $(BIN)/extract_sf $(ROOTCFLAGS) $(HIPOCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BIN)/hipo2root: $(OBJS) $(SRC)/hipo2root.c
	$(CXX) $(CFLAGS) $(OBJS) $(ROOTCFLAGS) $(HIPOCFLAGS) $(LZ4INCLUDES) $(SRC)/hipo2root.c -o $(BIN)/hipo2root $(ROOTCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(LZ4LIBS) $(ROOTLIBS)

$(BLD)/bank_containers.o: $(SRC)/bank_containers.c $(SRC)/bank_containers.h
	$(CXX) $(CFLAGS) -c $(SRC)/bank_containers.c -o $(BLD)/bank_containers.o $(ROOTCFLAGS) $(HIPOCFLAGS) $(ROOTLDFLAGS) $(HIPOLIBS) $(ROOTLIBS)

$(BLD)/constants.o: $(SRC)/constants.c $(SRC)/constants.h
	$(CXX) $(CFLAGS) -c $(SRC)/constants.c -o $(BLD)/constants.o

$(BLD)/err_handler.o: $(SRC)/err_handler.c $(SRC)/err_handler.h
	$(CXX) $(CFLAGS) -c $(SRC)/err_handler.c -o $(BLD)/err_handler.o

$(BLD)/file_handler.o: $(SRC)/file_handler.c
	$(CXX) $(CFLAGS) -c $(SRC)/file_handler.c -o $(BLD)/file_handler.o

$(BLD)/io_handler.o: $(SRC)/io_handler.c
	$(CXX) $(CFLAGS) -c $(SRC)/io_handler.c -o $(BLD)/io_handler.o

$(BLD)/particle.o: $(SRC)/particle.c
	$(CXX) $(CFLAGS) -c $(SRC)/particle.c -o $(BLD)/particle.o

$(BLD)/utilities.o: $(SRC)/utilities.c
	$(CXX) $(CFLAGS) -c $(SRC)/utilities.c -o $(BLD)/utilities.o $(ROOTCFLAGS) $(ROOTLDFLAGS) $(ROOTLIBS)

clean:
	@echo "Removing all build files and binaries."
	@rm $(BLD)/*.o
	@rm $(BIN)/*
