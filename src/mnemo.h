// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2016 Marco Baye
// Have a look at "acme.c" for further info
//
// mnemonic definitions
#ifndef mnemo_H
#define mnemo_H


// create dynamic buffer, build keyword trees
extern void Mnemo_init(void);
// check whether mnemonic in GlobalDynaBuf is supported by 6502 cpu.
extern int keyword_is_6502_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by 6510 cpu.
extern int keyword_is_6510_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by C64DTV2 cpu.
extern int keyword_is_c64dtv2_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by 65c02 cpu.
extern int keyword_is_65c02_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by Rockwell 65c02 cpu.
extern int keyword_is_r65c02_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by WDC 65c02 cpu.
extern int keyword_is_w65c02_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by 65816 cpu.
extern int keyword_is_65816_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by CSG 65ce02 cpu.
extern int keyword_is_65ce02_mnemo(int length);
// check whether mnemonic in GlobalDynaBuf is supported by CSG 4502 cpu.
extern int keyword_is_4502_mnemo(int length);


#endif
