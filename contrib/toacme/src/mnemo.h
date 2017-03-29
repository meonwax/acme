// ToACME - converts other source codes to ACME format.
// Copyright (C) 1999-2006 Marco Baye
// Have a look at "main.c" for further info
//
// 6502 mnemonics
#ifndef mnemo_H
#define mnemo_H


// mnemonics of legal 6502 instructions
extern const char	MnemonicADC[], MnemonicSBC[];
extern const char	MnemonicAND[], MnemonicEOR[], MnemonicORA[];
extern const char	MnemonicASL[], MnemonicLSR[];
extern const char	MnemonicBCC[], MnemonicBCS[];
extern const char	MnemonicBEQ[], MnemonicBNE[];
extern const char	MnemonicBMI[], MnemonicBPL[];
extern const char	MnemonicBRK[], MnemonicRTI[];
extern const char	MnemonicBVC[], MnemonicBVS[];
extern const char	MnemonicCLC[], MnemonicSEC[];
extern const char	MnemonicCLD[], MnemonicSED[];
extern const char	MnemonicCLI[], MnemonicSEI[];
extern const char	MnemonicBIT[], MnemonicCLV[], MnemonicNOP[];
extern const char	MnemonicCMP[], MnemonicCPX[], MnemonicCPY[];
extern const char	MnemonicDEC[], MnemonicDEX[], MnemonicDEY[];
extern const char	MnemonicINC[], MnemonicINX[], MnemonicINY[];
extern const char	MnemonicJMP[], MnemonicJSR[], MnemonicRTS[];
extern const char	MnemonicLDA[], MnemonicLDX[], MnemonicLDY[];
extern const char	MnemonicPHA[], MnemonicPLA[];
extern const char	MnemonicPHP[], MnemonicPLP[];
extern const char	MnemonicROL[], MnemonicROR[];
extern const char	MnemonicSTA[], MnemonicSTX[], MnemonicSTY[];
extern const char	MnemonicTSX[], MnemonicTXA[], MnemonicTAY[];
extern const char	MnemonicTYA[], MnemonicTAX[], MnemonicTXS[];

// mnemonics of undocumented ("illegal") 6502 instructions
extern const char	MnemonicANC[], MnemonicARR[], MnemonicASR[];
extern const char	MnemonicDCP[], MnemonicDOP[], MnemonicISC[];
extern const char	MnemonicJAM[], MnemonicLAX[], MnemonicRLA[];
extern const char	MnemonicRRA[], MnemonicSAX[], MnemonicSBX[];
extern const char	MnemonicSLO[], MnemonicSRE[], MnemonicTOP[];
extern const char	MnemonicSHX[];

// mnemonics of 65c02 instructions
extern const char	MnemonicBRA[];
extern const char	MnemonicPHX[], MnemonicPHY[];
extern const char	MnemonicPLX[], MnemonicPLY[];
extern const char	MnemonicSTZ[];
extern const char	MnemonicTRB[], MnemonicTSB[];

// mnemonics of 65816 instructions
extern const char	MnemonicJML[], MnemonicJSL[];
extern const char	MnemonicMVN[], MnemonicMVP[];
extern const char	MnemonicPEI[];
extern const char	MnemonicBRL[];
extern const char	MnemonicPER[];
extern const char	MnemonicCOP[];
extern const char	MnemonicPEA[];
extern const char	MnemonicREP[], MnemonicSEP[];
extern const char	MnemonicPHB[];
extern const char	MnemonicPHD[];
extern const char	MnemonicPHK[];
extern const char	MnemonicPLB[];
extern const char	MnemonicPLD[];
extern const char	MnemonicRTL[];
extern const char	MnemonicSTP[];
extern const char	MnemonicTCD[], MnemonicTCS[];
extern const char	MnemonicTDC[], MnemonicTSC[];
extern const char	MnemonicTXY[], MnemonicTYX[];
extern const char	MnemonicWAI[];
extern const char	MnemonicWDM[];
extern const char	MnemonicXBA[], MnemonicXCE[];


#endif
