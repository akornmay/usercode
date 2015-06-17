# define compile command

CC = g++

#CFLAGS  = -g -Wno-deprecated
CFLAGS  = -O3 -Wno-deprecated -funroll-loops 
LDFLAGS = -L/usr/local/lib
LDFLAGS+=-L/usr/lib

SRC 	= DoubleColumn.cpp ROC.cpp DataFlow.cpp Event.cpp EventReader.cpp Testboard.cpp
OBJ 	= DoubleColumn.o ROC.o DataFlow.o Event.o EventReader.o Testboard.o
HEAD    = DoubleColumn.h CommonDefs.h ROC.h Statistics.h DataFlow.h Event.h EventReader.h Testboard.h

ROOTCFLAGS    =$(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      =$(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     =$(shell $(ROOTSYS)/bin/root-config --glibs) 


CFLAGS       += $(ROOTCFLAGS)

all: $(OBJ) makefile
	$(CC) $(OBJ) $(LDFLAGS) $(ROOTGLIBS) $(ROOTLIBS) -o DataFlow

%.o: %.cpp %.h 
	$(CC) $(ROOTCFLAGS) $(CFLAGS) -c -o $@ $<

clean:	
	rm -f *.o DataFlow *~ Analyse

