CFLAGS		= -O3 -Wall -Wstrict-prototypes
#LIBS		= -lm
CC		= gcc
RM		= rm

#SRC		=

PROGS		= toacme
BINDIR		= /usr/local/bin
USERBIN		= $(HOME)/bin

all: $(PROGS)

vis.o: config.h acme.h io.h mnemo.h scr2iso.h vis.c

ab3.o: config.h ab.h acme.h io.h mnemo.h scr2iso.h ab3.c

ab.o: config.h ab.h acme.h io.h scr2iso.h ab.c

f8ab.o: config.h ab.h acme.h io.h mnemo.h scr2iso.h f8ab.c

giga.o: config.h acme.h gighyp.h io.h mnemo.h pet2iso.h giga.c

gighyp.o: config.h acme.h io.h pet2iso.h gighyp.h gighyp.c

hypra.o: config.h acme.h gighyp.h io.h pet2iso.h hypra.c

obj.o: config.h acme.h io.h mnemo.h obj.c

acme.o: config.h acme.h acme.c

main.o: config.h version.h main.c

mnemo.o: config.h mnemo.c

pet2iso.o: config.h pet2iso.h pet2iso.c

platform.o: config.h platform.h platform.c

prof.o: config.h prof.c

scr2iso.o: config.h scr2iso.h scr2iso.c

version.o: config.h version.c

toacme: vis.o ab.o ab3.o acme.o f8ab.o giga.o gighyp.o hypra.o io.o main.o mnemo.o obj.o pet2iso.o platform.o prof.o scr2iso.o version.o
	$(CC) $(LIBS) $(CFLAGS) -o toacme vis.o ab.o ab3.o acme.o f8ab.o giga.o gighyp.o hypra.o io.o main.o mnemo.o obj.o pet2iso.o platform.o prof.o scr2iso.o version.o
	strip toacme

clean:
	-$(RM) -f *.o $(PROGS) *~ core

install: all
	install -d $(BINDIR)
	install $(PROGS) $(BINDIR)

userinstall: all
	install -d $(USERBIN)
	install $(PROGS) $(USERBIN)

# DO NOT DELETE
