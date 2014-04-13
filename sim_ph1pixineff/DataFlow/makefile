# define compile command

CC = g++

CFLAGS  = -g -Wno-deprecated
#CFLAGS  = -O3 -Wno-deprecated -funroll-loops 
LDFLAGS = -L/usr/local/lib

OBJ 	= DoubleColumn.cpp ROC.cpp DataFlow.cpp Module.cpp Event.cpp EventReader.cpp TBM.cpp	
HEAD    = DoubleColumn.h CommonDefs.h ROC.h Module.h Statistics.h DataFlow.h Event.h EventReader.h TBM.h

ROOTCFLAGS    =$(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      =$(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     =$(shell $(ROOTSYS)/bin/root-config --glibs) 


CFLAGS       += $(ROOTCFLAGS)

all: $(OBJ) $(HEAD) makefile
	$(CC) $(OBJ) -o DataFlow $(CFLAGS) $(LDFLAGS) $(ROOTGLIBS)

clean:	
	rm -f *.o DataFlow *~ Analyse

