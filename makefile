CC = g++
INCL = -I ./include 
PROLIBDIR = ../obj/
OSSPEC = -m64

LIBDIR = ./lib/

DEFINES = -Wall -O1 $(OSSPEC)  $(INCL)
.SUFFIXES:.cpp.o

.cpp.o:
	$(CC) $(DEFINES)  -c $*.cpp

objs=demo.o



all:makeall



makeall:$(objs)
	$(CC) -o  main  $(objs)  $(LIBDIR)libapue.a  -lpthread 

     
