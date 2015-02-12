// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2003 Marco Baye
// Have a look at "main.c" for further info
//
// assembler mnemonics
//

// Mnemonics of legal 6502 instructions
const char	MnemonicADC[]	= "adc";
const char	MnemonicAND[]	= "and";
const char	MnemonicASL[]	= "asl";
const char	MnemonicBCC[]	= "bcc";
const char	MnemonicBCS[]	= "bcs";
const char	MnemonicBEQ[]	= "beq";
const char	MnemonicBIT[]	= "bit";
const char	MnemonicBMI[]	= "bmi";
const char	MnemonicBNE[]	= "bne";
const char	MnemonicBPL[]	= "bpl";
const char	MnemonicBRK[]	= "brk";
const char	MnemonicBVC[]	= "bvc";
const char	MnemonicBVS[]	= "bvs";
const char	MnemonicCLC[]	= "clc";
const char	MnemonicCLD[]	= "cld";
const char	MnemonicCLI[]	= "cli";
const char	MnemonicCLV[]	= "clv";
const char	MnemonicCMP[]	= "cmp";
const char	MnemonicCPX[]	= "cpx";
const char	MnemonicCPY[]	= "cpy";
const char	MnemonicDEC[]	= "dec";
const char	MnemonicDEX[]	= "dex";
const char	MnemonicDEY[]	= "dey";
const char	MnemonicEOR[]	= "eor";
const char	MnemonicINC[]	= "inc";
const char	MnemonicINX[]	= "inx";
const char	MnemonicINY[]	= "iny";
const char	MnemonicJMP[]	= "jmp";
const char	MnemonicJSR[]	= "jsr";
const char	MnemonicLDA[]	= "lda";
const char	MnemonicLDX[]	= "ldx";
const char	MnemonicLDY[]	= "ldy";
const char	MnemonicLSR[]	= "lsr";
const char	MnemonicNOP[]	= "nop";
const char	MnemonicORA[]	= "ora";
const char	MnemonicPHA[]	= "pha";
const char	MnemonicPHP[]	= "php";
const char	MnemonicPLA[]	= "pla";
const char	MnemonicPLP[]	= "plp";
const char	MnemonicROL[]	= "rol";
const char	MnemonicROR[]	= "ror";
const char	MnemonicRTI[]	= "rti";
const char	MnemonicRTS[]	= "rts";
const char	MnemonicSBC[]	= "sbc";
const char	MnemonicSEC[]	= "sec";
const char	MnemonicSED[]	= "sed";
const char	MnemonicSEI[]	= "sei";
const char	MnemonicSTA[]	= "sta";
const char	MnemonicSTX[]	= "stx";
const char	MnemonicSTY[]	= "sty";
const char	MnemonicTAX[]	= "tax";
const char	MnemonicTAY[]	= "tay";
const char	MnemonicTSX[]	= "tsx";
const char	MnemonicTXA[]	= "txa";
const char	MnemonicTXS[]	= "txs";
const char	MnemonicTYA[]	= "tya";

// Mnemonics of undocumented ("illegal") 6502 instructions
const char	MnemonicSLO[]	= " Slo";
const char	MnemonicRLA[]	= " Rla";
const char	MnemonicSRE[]	= " Sre";
const char	MnemonicRRA[]	= " Rra";
const char	MnemonicSAX[]	= " Sax";
const char	MnemonicLAX[]	= " Lax";
const char	MnemonicDCP[]	= " Dcp";
const char	MnemonicISC[]	= " Isc";

// Mnemonics of 65c02 instructions
const char	MnemonicBRA[]	= "bra";
const char	MnemonicPHX[]	= "phx";
const char	MnemonicPHY[]	= "phy";
const char	MnemonicPLX[]	= "plx";
const char	MnemonicPLY[]	= "ply";
const char	MnemonicSTZ[]	= "stz";
const char	MnemonicTRB[]	= "trb";
const char	MnemonicTSB[]	= "tsb";

// Mnemonics of 65816 instructions
const char	MnemonicJML[]	= "jml";
const char	MnemonicJSL[]	= "jsl";
const char	MnemonicMVN[]	= "mvn";
const char	MnemonicMVP[]	= "mvp";
const char	MnemonicPEI[]	= "pei";
const char	MnemonicBRL[]	= "brl";
const char	MnemonicPER[]	= "per";
const char	MnemonicCOP[]	= "cop";
const char	MnemonicPEA[]	= "pea";
const char	MnemonicREP[]	= "rep";
const char	MnemonicSEP[]	= "sep";
const char	MnemonicPHB[]	= "phb";
const char	MnemonicPHD[]	= "phd";
const char	MnemonicPHK[]	= "phk";
const char	MnemonicPLB[]	= "plb";
const char	MnemonicPLD[]	= "pld";
const char	MnemonicRTL[]	= "rtl";
const char	MnemonicSTP[]	= "stp";
const char	MnemonicTCD[]	= "tcd";
const char	MnemonicTCS[]	= "tcs";
const char	MnemonicTDC[]	= "tdc";
const char	MnemonicTSC[]	= "tsc";
const char	MnemonicTXY[]	= "txy";
const char	MnemonicTYX[]	= "tyx";
const char	MnemonicWAI[]	= "wai";
const char	MnemonicWDM[]	= "wdm";
const char	MnemonicXBA[]	= "xba";
const char	MnemonicXCE[]	= "xce";
