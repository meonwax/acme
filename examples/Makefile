ASSEMBLER6502	= acme
AS_FLAGS	= -v9 -Wtype-mismatch
RM		= rm

PROGS		= ddrv128.prg ddrv64.prg macedit.o trigono.o

all: $(PROGS)

ddrv128.prg: ddrv.a
	$(ASSEMBLER6502) $(AS_FLAGS) --outfile ddrv128.prg --format cbm -DSYSTEM=128 ddrv.a

ddrv64.prg: ddrv.a
	$(ASSEMBLER6502) $(AS_FLAGS) --outfile ddrv64.prg --format cbm -DSYSTEM=64 ddrv.a

macedit.o: macedit.a
	$(ASSEMBLER6502) $(AS_FLAGS) --outfile macedit.o --format cbm macedit.a

trigono.o: trigono.a
	$(ASSEMBLER6502) $(AS_FLAGS) --outfile trigono.o --format plain trigono.a

clean:
	-$(RM) -f *.o *.tmp $(PROGS) *~ core
