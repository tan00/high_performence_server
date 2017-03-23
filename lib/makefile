CC = g++
INCL = -I ../include 
PROLIBDIR = ../obj/
OSSPEC = -m64

DEFINES = -Wall -O1 $(OSSPEC)  $(INCL)
.SUFFIXES:.c.o

.c.o:
	$(CC) $(DEFINES)  -c $*.c

objs=error.o\
     errorlog.o


all:makeall



makeall:$(objs)
	ar -rv  libapue.a  $(objs)


.PHONY:.clean
clean:
	rm *.o 

	
