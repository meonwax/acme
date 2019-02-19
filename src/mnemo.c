// ACME - a crossassembler for producing 6502/65c02/65816/65ce02 code.
// Copyright (C) 1998-2017 Marco Baye
// Have a look at "acme.c" for further info
//
// Mnemonics stuff
#include "mnemo.h"
#include "config.h"
#include "alu.h"
#include "cpu.h"
#include "dynabuf.h"
#include "global.h"
#include "input.h"
#include "output.h"
#include "tree.h"
#include "typesystem.h"


// Constants
#define s_ror	(s_error + 2)	// Yes, I know I'm sick
#define MNEMO_DYNABUF_INITIALSIZE	8	// 4 + terminator should suffice

// These values are needed to recognize addressing modes:
// indexing:
#define INDEX_NONE	0	// no index
#define INDEX_S		1	// stack-indexed (",s" or ",sp"), for 65816 and 65ce02
#define INDEX_X		2	// x-indexed (",x")
#define INDEX_Y		3	// y-indexed (",y")
#define INDEX_Z		4	// z-indexed (",z"), only for 65ce02
	// 5..7 are left for future expansion, 8 would need the AMB_INDEX macro below to be adjusted!
// adress mode bits:
#define AMB_IMPLIED		(1u << 0)	// no value given
#define AMB_IMMEDIATE		(1u << 1)	// '#' at start
#define AMB_INDIRECT		(1u << 2)	// value has at least one unnecessary pair of "()"
#define AMB_LONGINDIRECT	(1u << 3)	// value is given in []
#define AMB_PREINDEX(idx)	((idx) << 4)	// three bits for indexing inside ()
#define AMB_INDEX(idx)		((idx) << 7)	// three bits for external indexing

// end values (here, "absolute addressing" always includes zeropage addressing, because they look the same)
#define IMPLIED_ADDRESSING		AMB_IMPLIED
#define IMMEDIATE_ADDRESSING		AMB_IMMEDIATE
#define ABSOLUTE_ADDRESSING		0
#define X_INDEXED_ADDRESSING		AMB_INDEX(INDEX_X)
#define Y_INDEXED_ADDRESSING		AMB_INDEX(INDEX_Y)
#define INDIRECT_ADDRESSING		AMB_INDIRECT
#define X_INDEXED_INDIRECT_ADDRESSING	(AMB_PREINDEX(INDEX_X) | AMB_INDIRECT)
#define INDIRECT_Y_INDEXED_ADDRESSING	(AMB_INDIRECT | AMB_INDEX(INDEX_Y))
// only for 65ce02:
#define INDIRECT_Z_INDEXED_ADDRESSING	(AMB_INDIRECT | AMB_INDEX(INDEX_Z))
// for 65816 and 65ce02:
#define STACK_INDEXED_INDIRECT_Y_INDEXED_ADDRESSING	(AMB_PREINDEX(INDEX_S) | AMB_INDIRECT | AMB_INDEX(INDEX_Y))
// only for 65816:
#define STACK_INDEXED_ADDRESSING			AMB_INDEX(INDEX_S)
#define LONG_INDIRECT_ADDRESSING			AMB_LONGINDIRECT
#define LONG_INDIRECT_Y_INDEXED_ADDRESSING		(AMB_LONGINDIRECT | AMB_INDEX(INDEX_Y))

// Constant values, used to mark the possible parameter lengths of commands.
// Not all of the eight values are actually used, however (because of the
// supported CPUs).
#define MAYBE______	(0)
#define MAYBE_1____	(MVALUE_FORCE08)
#define MAYBE___2__	(MVALUE_FORCE16)
#define MAYBE_1_2__	(MVALUE_FORCE08 | MVALUE_FORCE16)
#define MAYBE_____3	(MVALUE_FORCE24)
#define MAYBE_1___3	(MVALUE_FORCE08 | MVALUE_FORCE24)
#define MAYBE___2_3	(MVALUE_FORCE16 | MVALUE_FORCE24)
#define MAYBE_1_2_3	(MVALUE_FORCE08 | MVALUE_FORCE16 | MVALUE_FORCE24)

// The mnemonics are split up into groups, each group has its own function to be dealt with:
enum mnemogroup {
	GROUP_ACCU,		// main accumulator stuff, plus PEI		Byte value = table index
	GROUP_MISC,		// read-modify-write and others			Byte value = table index
	GROUP_ALLJUMPS,		// the jump instructions			Byte value = table index
	GROUP_IMPLIEDONLY,	// mnemonics using only implied addressing	Byte value = opcode
	GROUP_RELATIVE8,	// short branch instructions			Byte value = opcode
	GROUP_BITBRANCH,	// bbr0..7 and bbs0..7				Byte value = opcode
	GROUP_REL16_2,		// 16bit relative to pc+2			Byte value = opcode
	GROUP_REL16_3,		// 16bit relative to pc+3			Byte value = opcode
	GROUP_BOTHMOVES,	// the "move" commands MVP and MVN		Byte value = opcode
	GROUP_ZPONLY		// rmb0..7 and smb0..7				Byte value = opcode	FIXME - use for IDX816COP,IDXeDEW,IDXeINW as well!
};

// save some space
#define SCB	static const unsigned char
#define SCS	static const unsigned short
#define SCL	static const unsigned long

// Code tables for group GROUP_ACCU:
// These tables are used for the main accumulator-related mnemonics. By reading
// the mnemonic's byte value (from the mnemotable), the assembler finds out the
// column to use here. The row depends on the used addressing mode. A zero
// entry in these tables means that the combination of mnemonic and addressing
// mode is illegal.
//                  |                                                                     6502/65c02/65816/65ce02                                                                                                                                                                                   |  65816  |                             6510 illegals                             |
enum {               IDX_ORA,IDXcORA,IDX816ORA,IDXeORA,IDX_AND,IDXcAND,IDX816AND,IDXeAND,IDX_EOR,IDXcEOR,IDX816EOR,IDXeEOR,IDX_ADC,IDXcADC,IDX816ADC,IDXeADC,IDX_STA,IDXcSTA,IDX816STA,IDXeSTA,IDX_LDA,IDXcLDA,IDX816LDA,IDXeLDA,IDX_CMP,IDXcCMP,IDX816CMP,IDXeCMP,IDX_SBC,IDXcSBC,IDX816SBC,IDXeSBC,IDX816PEI,IDX_SLO,IDX_RLA,IDX_SRE,IDX_RRA,IDX_SAX,IDX_LAX,IDX_DCP,IDX_ISC,IDX_SHA};
SCB accu_imm[]    = {   0x09,   0x09,     0x09,   0x09,   0x29,   0x29,     0x29,   0x29,   0x49,   0x49,     0x49,   0x49,   0x69,   0x69,     0x69,   0x69,      0,      0,        0,      0,   0xa9,   0xa9,     0xa9,   0xa9,   0xc9,   0xc9,     0xc9,   0xc9,   0xe9,   0xe9,     0xe9,   0xe9,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// #$ff     #$ffff
SCL accu_abs[]    = { 0x0d05, 0x0d05, 0x0f0d05, 0x0d05, 0x2d25, 0x2d25, 0x2f2d25, 0x2d25, 0x4d45, 0x4d45, 0x4f4d45, 0x4d45, 0x6d65, 0x6d65, 0x6f6d65, 0x6d65, 0x8d85, 0x8d85, 0x8f8d85, 0x8d85, 0xada5, 0xada5, 0xafada5, 0xada5, 0xcdc5, 0xcdc5, 0xcfcdc5, 0xcdc5, 0xede5, 0xede5, 0xefede5, 0xede5,        0, 0x0f07, 0x2f27, 0x4f47, 0x6f67, 0x8f87, 0xafa7, 0xcfc7, 0xefe7,      0};	// $ff      $ffff    $ffffff
SCL accu_xabs[]   = { 0x1d15, 0x1d15, 0x1f1d15, 0x1d15, 0x3d35, 0x3d35, 0x3f3d35, 0x3d35, 0x5d55, 0x5d55, 0x5f5d55, 0x5d55, 0x7d75, 0x7d75, 0x7f7d75, 0x7d75, 0x9d95, 0x9d95, 0x9f9d95, 0x9d95, 0xbdb5, 0xbdb5, 0xbfbdb5, 0xbdb5, 0xddd5, 0xddd5, 0xdfddd5, 0xddd5, 0xfdf5, 0xfdf5, 0xfffdf5, 0xfdf5,        0, 0x1f17, 0x3f37, 0x5f57, 0x7f77,      0,      0, 0xdfd7, 0xfff7,      0};	// $ff,x    $ffff,x  $ffffff,x
SCS accu_yabs[]   = { 0x1900, 0x1900,   0x1900, 0x1900, 0x3900, 0x3900,   0x3900, 0x3900, 0x5900, 0x5900,   0x5900, 0x5900, 0x7900, 0x7900,   0x7900, 0x7900, 0x9900, 0x9900,   0x9900, 0x9900, 0xb900, 0xb900,   0xb900, 0xb900, 0xd900, 0xd900,   0xd900, 0xd900, 0xf900, 0xf900,   0xf900, 0xf900,        0, 0x1b00, 0x3b00, 0x5b00, 0x7b00,   0x97, 0xbfb7, 0xdb00, 0xfb00, 0x9f00};	// $ff,y    $ffff,y
SCB accu_xind8[]  = {   0x01,   0x01,     0x01,   0x01,   0x21,   0x21,     0x21,   0x21,   0x41,   0x41,     0x41,   0x41,   0x61,   0x61,     0x61,   0x61,   0x81,   0x81,     0x81,   0x81,   0xa1,   0xa1,     0xa1,   0xa1,   0xc1,   0xc1,     0xc1,   0xc1,   0xe1,   0xe1,     0xe1,   0xe1,        0,   0x03,   0x23,   0x43,   0x63,   0x83,   0xa3,   0xc3,   0xe3,      0};	// ($ff,x)
SCB accu_indy8[]  = {   0x11,   0x11,     0x11,   0x11,   0x31,   0x31,     0x31,   0x31,   0x51,   0x51,     0x51,   0x51,   0x71,   0x71,     0x71,   0x71,   0x91,   0x91,     0x91,   0x91,   0xb1,   0xb1,     0xb1,   0xb1,   0xd1,   0xd1,     0xd1,   0xd1,   0xf1,   0xf1,     0xf1,   0xf1,        0,   0x13,   0x33,   0x53,   0x73,      0,   0xb3,   0xd3,   0xf3,   0x93};	// ($ff),y
SCB accu_ind8[]   = {      0,   0x12,     0x12,      0,      0,   0x32,     0x32,      0,      0,   0x52,     0x52,      0,      0,   0x72,     0x72,      0,      0,   0x92,     0x92,      0,      0,   0xb2,     0xb2,      0,      0,   0xd2,     0xd2,      0,      0,   0xf2,     0xf2,      0,     0xd4,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// ($ff)
SCB accu_sabs8[]  = {      0,      0,     0x03,      0,      0,      0,     0x23,      0,      0,      0,     0x43,      0,      0,      0,     0x63,      0,      0,      0,     0x83,      0,      0,      0,     0xa3,      0,      0,      0,     0xc3,      0,      0,      0,     0xe3,      0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// $ff,s
SCB accu_sindy8[] = {      0,      0,     0x13,      0,      0,      0,     0x33,      0,      0,      0,     0x53,      0,      0,      0,     0x73,      0,      0,      0,     0x93,   0x82,      0,      0,     0xb3,   0xe2,      0,      0,     0xd3,      0,      0,      0,     0xf3,      0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// ($ff,s),y
SCB accu_lind8[]  = {      0,      0,     0x07,      0,      0,      0,     0x27,      0,      0,      0,     0x47,      0,      0,      0,     0x67,      0,      0,      0,     0x87,      0,      0,      0,     0xa7,      0,      0,      0,     0xc7,      0,      0,      0,     0xe7,      0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// [$ff]
SCB accu_lindy8[] = {      0,      0,     0x17,      0,      0,      0,     0x37,      0,      0,      0,     0x57,      0,      0,      0,     0x77,      0,      0,      0,     0x97,      0,      0,      0,     0xb7,      0,      0,      0,     0xd7,      0,      0,      0,     0xf7,      0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// [$ff],y
SCB accu_indz8[]  = {      0,      0,        0,   0x12,      0,      0,        0,   0x32,      0,      0,        0,   0x52,      0,      0,        0,   0x72,      0,      0,        0,   0x92,      0,      0,        0,   0xb2,      0,      0,        0,   0xd2,      0,      0,        0,   0xf2,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// ($ff),z

// Code tables for group GROUP_MISC:
// These tables are needed for finding out the correct code in cases when
// there are no general rules. By reading the mnemonic's byte value (from the
// mnemotable), the assembler finds out the column to use here. The row
// depends on the used addressing mode. A zero entry in these tables means
// that the combination of mnemonic and addressing mode is illegal.
//                |                             6502                              |                             6502/65c02/65ce02                                 |         65c02         |                                         65ce02                        |                 65816                 |                                             6510 illegals                                             |    C64DTV2    |
enum {             IDX_ASL,IDX_ROL,IDX_LSR,IDX_ROR,IDX_LDY,IDX_LDX,IDX_CPY,IDX_CPX,IDX_BIT,IDXcBIT,IDX_STX,IDXeSTX,IDX_STY,IDXeSTY,IDX_DEC,IDXcDEC,IDX_INC,IDXcINC,IDXcTSB,IDXcTRB,IDXcSTZ,IDXeASR,IDXeASW,IDXeCPZ,IDXeDEW,IDXeINW,IDXeLDZ,IDXePHW,IDXeROW,IDXeRTN,IDX816COP,IDX816REP,IDX816SEP,IDX816PEA,IDX_ANC,IDX_ASR,IDX_ARR,IDX_SBX,IDX_DOP,IDX_TOP,IDX_JAM,IDX_LXA,IDX_ANE,IDX_LAS,IDX_TAS,IDX_SHX,IDX_SHY,IDX_SAC,IDX_SIR};
SCB misc_impl[] = {   0x0a,   0x2a,   0x4a,   0x6a,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,   0x3a,      0,   0x1a,      0,      0,      0,   0x43,      0,      0,      0,      0,      0,      0,      0,      0,        0,        0,        0,        0,      0,      0,      0,      0,   0x80,   0x0c,   0x02,      0,      0,      0,      0,      0,      0,      0,      0};	// implied/accu
SCB misc_imm[]  = {      0,      0,      0,      0,   0xa0,   0xa2,   0xc0,   0xe0,      0,   0x89,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,   0xc2,      0,      0,   0xa3,   0xf4,      0,   0x62,        0,     0xc2,     0xe2,        0,   0x0b,   0x4b,   0x6b,   0xcb,   0x80,      0,      0,   0xab,   0x8b,      0,      0,      0,      0,   0x32,   0x42};	// #$ff     #$ffff
SCS misc_abs[]  = { 0x0e06, 0x2e26, 0x4e46, 0x6e66, 0xaca4, 0xaea6, 0xccc4, 0xece4, 0x2c24, 0x2c24, 0x8e86, 0x8e86, 0x8c84, 0x8c84, 0xcec6, 0xcec6, 0xeee6, 0xeee6, 0x0c04, 0x1c14, 0x9c64,   0x44, 0xcb00, 0xdcd4,   0xc3,   0xe3, 0xab00, 0xfc00, 0xeb00,      0,     0x02,        0,        0,   0xf400,      0,      0,      0,      0,   0x04, 0x0c00,      0,      0,      0,      0,      0,      0,      0,      0,      0};	// $ff      $ffff
SCS misc_xabs[] = { 0x1e16, 0x3e36, 0x5e56, 0x7e76, 0xbcb4,      0,      0,      0,      0, 0x3c34,      0,      0,   0x94, 0x8b94, 0xded6, 0xded6, 0xfef6, 0xfef6,      0,      0, 0x9e74,   0x54,      0,      0,      0,      0, 0xbb00,      0,      0,      0,        0,        0,        0,        0,      0,      0,      0,      0,   0x14, 0x1c00,      0,      0,      0,      0,      0,      0, 0x9c00,      0,      0};	// $ff,x    $ffff,x
SCS misc_yabs[] = {      0,      0,      0,      0,      0, 0xbeb6,      0,      0,      0,      0,   0x96, 0x9b96,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,        0,        0,        0,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0, 0xbb00, 0x9b00, 0x9e00,      0,      0,      0};	// $ff,y    $ffff,y

// Code tables for group GROUP_ALLJUMPS:
// These tables are needed for finding out the correct code when the mnemonic
// is "jmp" or "jsr" (or the long versions "jml" and "jsl").
// By reading the mnemonic's byte value (from the mnemotable), the assembler
// finds out the column to use here. The row depends on the used addressing
// mode. A zero entry in these tables means that the combination of mnemonic
// and addressing mode is illegal.
//                 |              6502/65c02/65816/65ce02              |       65816       |
enum {              IDX_JMP,IDXcJMP,IDX816JMP,IDX_JSR,IDXeJSR,IDX816JSR,IDX816JML,IDX816JSL};
SCL jump_abs[]   = { 0x4c00, 0x4c00, 0x5c4c00, 0x2000, 0x2000, 0x222000, 0x5c0000, 0x220000};	// $ffff    $ffffff
SCS jump_ind[]   = { 0x6c00, 0x6c00,   0x6c00,      0, 0x2200,        0,        0,        0};	// ($ffff)
SCS jump_xind[]  = {      0, 0x7c00,   0x7c00,      0, 0x2300,   0xfc00,        0,        0};	// ($ffff,x)
SCS jump_lind[]  = {      0,      0,   0xdc00,      0,      0,        0,   0xdc00,        0};	// [$ffff]

#undef SCB
#undef SCS
#undef SCL

// error message strings
static const char	exception_illegal_combination[]	= "Illegal combination of command and addressing mode.";
static const char	exception_oversized_addrmode[]	= "Using oversized addressing mode.";


// Variables

static struct dynabuf	*mnemo_dyna_buf;	// dynamic buffer for mnemonics
// predefined stuff
static struct ronode	*mnemo_6502_tree	= NULL;	// 6502 mnemonics
static struct ronode	*mnemo_6502undoc1_tree	= NULL;	// 6502 undocumented ("illegal") opcodes supported by DTV2
static struct ronode	*mnemo_6502undoc2_tree	= NULL;	// remaining 6502 undocumented ("illegal") opcodes (currently ANC only, maybe more will get moved)
static struct ronode	*mnemo_c64dtv2_tree	= NULL;	// C64DTV2 extensions (BRA/SAC/SIR)
static struct ronode	*mnemo_65c02_tree	= NULL;	// 65c02 extensions
static struct ronode	*mnemo_bitmanips_tree	= NULL;	// Rockwell's bit manipulation extensions
static struct ronode	*mnemo_stp_wai_tree	= NULL;	// WDC's "stp" and "wai" instructions
static struct ronode	*mnemo_65816_tree	= NULL;	// WDC 65816 extensions
static struct ronode	*mnemo_65ce02_tree	= NULL;	// CSG 65ce02/4502 extensions
static struct ronode	*mnemo_aug_tree		= NULL;	// CSG 65ce02's "aug" instruction
static struct ronode	*mnemo_map_eom_tree	= NULL;	// CSG 4502's "map" and "eom" instructions

// Command's code and group values are stored together in a single integer.
// To extract the code, use "& CODEMASK".
// To extract the immediate mode, use "& IMMASK".
// To extract the group, use GROUP()
#define CODEMASK	0x0ff	// opcode or table index
// immediate mode:
#define IM_FORCE8	0x000	// immediate values are 8 bits (CAUTION - program relies on "no bits set" being the default!)
#define IM_FORCE16	0x100	// immediate value is 16 bits (for 65ce02's PHW#)
#define IM_ACCUMULATOR	0x200	// immediate value depends on accumulator length
#define IM_INDEXREGS	0x300	// immediate value depends on index register length
#define IMMASK		0x300	// mask for immediate modes
#define MERGE(g, v)	(((g) << 10) | (v))
#define GROUP(v)	((v) >> 10)

static struct ronode	mnemos_6502[]	= {
	PREDEFNODE("ora", MERGE(GROUP_ACCU, IDX_ORA)),
	PREDEFNODE(s_and, MERGE(GROUP_ACCU, IDX_AND)),
	PREDEFNODE(s_eor, MERGE(GROUP_ACCU, IDX_EOR)),
	PREDEFNODE("adc", MERGE(GROUP_ACCU, IDX_ADC)),
	PREDEFNODE("sta", MERGE(GROUP_ACCU, IDX_STA)),
	PREDEFNODE("lda", MERGE(GROUP_ACCU, IDX_LDA)),
	PREDEFNODE("cmp", MERGE(GROUP_ACCU, IDX_CMP)),
	PREDEFNODE("sbc", MERGE(GROUP_ACCU, IDX_SBC)),
	PREDEFNODE("bit", MERGE(GROUP_MISC, IDX_BIT)),
	PREDEFNODE(s_asl, MERGE(GROUP_MISC, IDX_ASL)),
	PREDEFNODE("rol", MERGE(GROUP_MISC, IDX_ROL)),
	PREDEFNODE(s_lsr, MERGE(GROUP_MISC, IDX_LSR)),
	PREDEFNODE(s_ror, MERGE(GROUP_MISC, IDX_ROR)),
	PREDEFNODE("sty", MERGE(GROUP_MISC, IDX_STY)),
	PREDEFNODE("stx", MERGE(GROUP_MISC, IDX_STX)),
	PREDEFNODE("ldy", MERGE(GROUP_MISC, IDX_LDY)),
	PREDEFNODE("ldx", MERGE(GROUP_MISC, IDX_LDX)),
	PREDEFNODE("cpy", MERGE(GROUP_MISC, IDX_CPY)),
	PREDEFNODE("cpx", MERGE(GROUP_MISC, IDX_CPX)),
	PREDEFNODE("dec", MERGE(GROUP_MISC, IDX_DEC)),
	PREDEFNODE("inc", MERGE(GROUP_MISC, IDX_INC)),
	PREDEFNODE("bpl", MERGE(GROUP_RELATIVE8, 0x10)),
	PREDEFNODE("bmi", MERGE(GROUP_RELATIVE8, 0x30)),
	PREDEFNODE("bvc", MERGE(GROUP_RELATIVE8, 0x50)),
	PREDEFNODE("bvs", MERGE(GROUP_RELATIVE8, 0x70)),
	PREDEFNODE("bcc", MERGE(GROUP_RELATIVE8, 0x90)),
	PREDEFNODE("bcs", MERGE(GROUP_RELATIVE8, 0xb0)),
	PREDEFNODE("bne", MERGE(GROUP_RELATIVE8, 0xd0)),
	PREDEFNODE("beq", MERGE(GROUP_RELATIVE8, 0xf0)),
	PREDEFNODE("jmp", MERGE(GROUP_ALLJUMPS, IDX_JMP)),
	PREDEFNODE("jsr", MERGE(GROUP_ALLJUMPS, IDX_JSR)),
	PREDEFNODE("brk", MERGE(GROUP_IMPLIEDONLY,   0)),
	PREDEFNODE("php", MERGE(GROUP_IMPLIEDONLY,   8)),
	PREDEFNODE("clc", MERGE(GROUP_IMPLIEDONLY,  24)),
	PREDEFNODE("plp", MERGE(GROUP_IMPLIEDONLY,  40)),
	PREDEFNODE("sec", MERGE(GROUP_IMPLIEDONLY,  56)),
	PREDEFNODE("rti", MERGE(GROUP_IMPLIEDONLY,  64)),
	PREDEFNODE("pha", MERGE(GROUP_IMPLIEDONLY,  72)),
	PREDEFNODE("cli", MERGE(GROUP_IMPLIEDONLY,  88)),
	PREDEFNODE("rts", MERGE(GROUP_IMPLIEDONLY,  96)),
	PREDEFNODE("pla", MERGE(GROUP_IMPLIEDONLY, 104)),
	PREDEFNODE("sei", MERGE(GROUP_IMPLIEDONLY, 120)),
	PREDEFNODE("dey", MERGE(GROUP_IMPLIEDONLY, 136)),
	PREDEFNODE("txa", MERGE(GROUP_IMPLIEDONLY, 138)),
	PREDEFNODE("tya", MERGE(GROUP_IMPLIEDONLY, 152)),
	PREDEFNODE("txs", MERGE(GROUP_IMPLIEDONLY, 154)),
	PREDEFNODE("tay", MERGE(GROUP_IMPLIEDONLY, 168)),
	PREDEFNODE("tax", MERGE(GROUP_IMPLIEDONLY, 170)),
	PREDEFNODE("clv", MERGE(GROUP_IMPLIEDONLY, 184)),
	PREDEFNODE("tsx", MERGE(GROUP_IMPLIEDONLY, 186)),
	PREDEFNODE("iny", MERGE(GROUP_IMPLIEDONLY, 200)),
	PREDEFNODE("dex", MERGE(GROUP_IMPLIEDONLY, 202)),
	PREDEFNODE("cld", MERGE(GROUP_IMPLIEDONLY, 216)),
	PREDEFNODE("inx", MERGE(GROUP_IMPLIEDONLY, 232)),
	PREDEFNODE("nop", MERGE(GROUP_IMPLIEDONLY, 234)),
	PREDEFLAST("sed", MERGE(GROUP_IMPLIEDONLY, 248)),
	//    ^^^^ this marks the last element
};

// undocumented opcodes of the NMOS 6502 that are also supported by c64dtv2:
static struct ronode	mnemos_6502undoc1[]	= {
	PREDEFNODE("slo", MERGE(GROUP_ACCU, IDX_SLO)),	// ASL + ORA (aka ASO)
	PREDEFNODE("rla", MERGE(GROUP_ACCU, IDX_RLA)),	// ROL + AND
	PREDEFNODE("sre", MERGE(GROUP_ACCU, IDX_SRE)),	// LSR + EOR (aka LSE)
	PREDEFNODE("rra", MERGE(GROUP_ACCU, IDX_RRA)),	// ROR + ADC
	PREDEFNODE("sax", MERGE(GROUP_ACCU, IDX_SAX)),	// STX + STA (aka AXS aka AAX)
	PREDEFNODE("lax", MERGE(GROUP_ACCU, IDX_LAX)),	// LDX + LDA
	PREDEFNODE("dcp", MERGE(GROUP_ACCU, IDX_DCP)),	// DEC + CMP (aka DCM)
	PREDEFNODE("isc", MERGE(GROUP_ACCU, IDX_ISC)),	// INC + SBC (aka ISB aka INS)
	PREDEFNODE("las", MERGE(GROUP_MISC, IDX_LAS)),	// A,X,S = {addr} & S (aka LAR aka LAE)
	PREDEFNODE("tas", MERGE(GROUP_MISC, IDX_TAS)),	// S = A & X	{addr} = A&X& {H+1} (aka SHS aka XAS)
	PREDEFNODE("sha", MERGE(GROUP_ACCU, IDX_SHA)),	// {addr} = A & X & {H+1} (aka AXA aka AHX)
	PREDEFNODE("shx", MERGE(GROUP_MISC, IDX_SHX)),	// {addr} = X & {H+1} (aka XAS aka SXA)
	PREDEFNODE("shy", MERGE(GROUP_MISC, IDX_SHY)),	// {addr} = Y & {H+1} (aka SAY aka SYA)
	PREDEFNODE(s_asr, MERGE(GROUP_MISC, IDX_ASR)),	// LSR + EOR (aka ALR)
	PREDEFNODE("arr", MERGE(GROUP_MISC, IDX_ARR)),	// ROR + ADC
	PREDEFNODE("sbx", MERGE(GROUP_MISC, IDX_SBX)),	// DEX + CMP (aka AXS aka SAX)
	PREDEFNODE("dop", MERGE(GROUP_MISC, IDX_DOP)),	// skip next byte
	PREDEFNODE("top", MERGE(GROUP_MISC, IDX_TOP)),	// skip next word
	PREDEFNODE("jam", MERGE(GROUP_MISC, IDX_JAM)),	// jam/crash/kill/halt-and-catch-fire
	PREDEFNODE("ane", MERGE(GROUP_MISC, IDX_ANE)),	// A = (A | ??) & X & arg (aka XAA)
	PREDEFLAST("lxa", MERGE(GROUP_MISC, IDX_LXA)),	// A,X = (A | ??) & arg (aka OAL aka ATX)
	//    ^^^^ this marks the last element
};

// undocumented opcodes of the NMOS 6502 that are _not_ supported by c64dtv2:
static struct ronode	mnemos_6502undoc2[]	= {
	PREDEFLAST("anc", MERGE(GROUP_MISC, IDX_ANC)),	// ROL + AND, ASL + ORA (aka AAC)
	//    ^^^^ this marks the last element
};

// additional opcodes of c64dtv2:
static struct ronode	mnemos_c64dtv2[]	= {
	PREDEFNODE(s_bra, MERGE(GROUP_RELATIVE8, 0x12)),	// branch always
	PREDEFNODE("sac", MERGE(GROUP_MISC, IDX_SAC)),	// set accumulator mapping
	PREDEFLAST("sir", MERGE(GROUP_MISC, IDX_SIR)),	// set index register mapping
	//    ^^^^ this marks the last element
};

// new stuff in CMOS re-design:
static struct ronode	mnemos_65c02[]	= {
	// more addressing modes for some mnemonics:
	PREDEFNODE("ora", MERGE(GROUP_ACCU,	IDXcORA)),
	PREDEFNODE(s_and, MERGE(GROUP_ACCU,	IDXcAND)),
	PREDEFNODE(s_eor, MERGE(GROUP_ACCU,	IDXcEOR)),
	PREDEFNODE("adc", MERGE(GROUP_ACCU,	IDXcADC)),
	PREDEFNODE("sta", MERGE(GROUP_ACCU,	IDXcSTA)),
	PREDEFNODE("lda", MERGE(GROUP_ACCU,	IDXcLDA)),
	PREDEFNODE("cmp", MERGE(GROUP_ACCU,	IDXcCMP)),
	PREDEFNODE("sbc", MERGE(GROUP_ACCU,	IDXcSBC)),
	PREDEFNODE("jmp", MERGE(GROUP_ALLJUMPS,	IDXcJMP)),
	PREDEFNODE("bit", MERGE(GROUP_MISC,	IDXcBIT)),
	PREDEFNODE("dec", MERGE(GROUP_MISC,	IDXcDEC)),
	PREDEFNODE("inc", MERGE(GROUP_MISC,	IDXcINC)),
	// and eight new mnemonics:
	PREDEFNODE(s_bra, MERGE(GROUP_RELATIVE8,	0x80)),
	PREDEFNODE("phy", MERGE(GROUP_IMPLIEDONLY,	 90)),
	PREDEFNODE("ply", MERGE(GROUP_IMPLIEDONLY,	122)),
	PREDEFNODE("phx", MERGE(GROUP_IMPLIEDONLY,	218)),
	PREDEFNODE("plx", MERGE(GROUP_IMPLIEDONLY,	250)),
	PREDEFNODE("tsb", MERGE(GROUP_MISC,	IDXcTSB)),
	PREDEFNODE("trb", MERGE(GROUP_MISC,	IDXcTRB)),
	PREDEFLAST("stz", MERGE(GROUP_MISC,	IDXcSTZ)),
	//    ^^^^ this marks the last element
};

// bit-manipulation extensions (by Rockwell?)
static struct ronode	mnemos_bitmanips[]	= {
	PREDEFNODE("rmb0", MERGE(GROUP_ZPONLY, 0x07)),
	PREDEFNODE("rmb1", MERGE(GROUP_ZPONLY, 0x17)),
	PREDEFNODE("rmb2", MERGE(GROUP_ZPONLY, 0x27)),
	PREDEFNODE("rmb3", MERGE(GROUP_ZPONLY, 0x37)),
	PREDEFNODE("rmb4", MERGE(GROUP_ZPONLY, 0x47)),
	PREDEFNODE("rmb5", MERGE(GROUP_ZPONLY, 0x57)),
	PREDEFNODE("rmb6", MERGE(GROUP_ZPONLY, 0x67)),
	PREDEFNODE("rmb7", MERGE(GROUP_ZPONLY, 0x77)),
	PREDEFNODE("smb0", MERGE(GROUP_ZPONLY, 0x87)),
	PREDEFNODE("smb1", MERGE(GROUP_ZPONLY, 0x97)),
	PREDEFNODE("smb2", MERGE(GROUP_ZPONLY, 0xa7)),
	PREDEFNODE("smb3", MERGE(GROUP_ZPONLY, 0xb7)),
	PREDEFNODE("smb4", MERGE(GROUP_ZPONLY, 0xc7)),
	PREDEFNODE("smb5", MERGE(GROUP_ZPONLY, 0xd7)),
	PREDEFNODE("smb6", MERGE(GROUP_ZPONLY, 0xe7)),
	PREDEFNODE("smb7", MERGE(GROUP_ZPONLY, 0xf7)),
	PREDEFNODE("bbr0", MERGE(GROUP_BITBRANCH, 0x0f)),
	PREDEFNODE("bbr1", MERGE(GROUP_BITBRANCH, 0x1f)),
	PREDEFNODE("bbr2", MERGE(GROUP_BITBRANCH, 0x2f)),
	PREDEFNODE("bbr3", MERGE(GROUP_BITBRANCH, 0x3f)),
	PREDEFNODE("bbr4", MERGE(GROUP_BITBRANCH, 0x4f)),
	PREDEFNODE("bbr5", MERGE(GROUP_BITBRANCH, 0x5f)),
	PREDEFNODE("bbr6", MERGE(GROUP_BITBRANCH, 0x6f)),
	PREDEFNODE("bbr7", MERGE(GROUP_BITBRANCH, 0x7f)),
	PREDEFNODE("bbs0", MERGE(GROUP_BITBRANCH, 0x8f)),
	PREDEFNODE("bbs1", MERGE(GROUP_BITBRANCH, 0x9f)),
	PREDEFNODE("bbs2", MERGE(GROUP_BITBRANCH, 0xaf)),
	PREDEFNODE("bbs3", MERGE(GROUP_BITBRANCH, 0xbf)),
	PREDEFNODE("bbs4", MERGE(GROUP_BITBRANCH, 0xcf)),
	PREDEFNODE("bbs5", MERGE(GROUP_BITBRANCH, 0xdf)),
	PREDEFNODE("bbs6", MERGE(GROUP_BITBRANCH, 0xef)),
	PREDEFLAST("bbs7", MERGE(GROUP_BITBRANCH, 0xff)),
	//    ^^^^ this marks the last element
};

// "stp" and "wai" extensions by WDC:
static struct ronode	mnemos_stp_wai[]	= {
	PREDEFNODE("stp", MERGE(GROUP_IMPLIEDONLY, 219)),
	PREDEFLAST("wai", MERGE(GROUP_IMPLIEDONLY, 203)),
	//    ^^^^ this marks the last element
};

// most of the 65816 stuff
static struct ronode	mnemos_65816[]	= {
	// CAUTION - these use 6502/65c02 indices, because the opcodes are the same - but I need flags for immediate mode!
	PREDEFNODE("ldy", MERGE(GROUP_MISC, IDX_LDY | IM_INDEXREGS)),
	PREDEFNODE("ldx", MERGE(GROUP_MISC, IDX_LDX | IM_INDEXREGS)),
	PREDEFNODE("cpy", MERGE(GROUP_MISC, IDX_CPY | IM_INDEXREGS)),
	PREDEFNODE("cpx", MERGE(GROUP_MISC, IDX_CPX | IM_INDEXREGS)),
	PREDEFNODE("bit", MERGE(GROUP_MISC, IDXcBIT | IM_ACCUMULATOR)),
	// more addressing modes for some mnemonics:
	PREDEFNODE("ora", MERGE(GROUP_ACCU,	IDX816ORA | IM_ACCUMULATOR)),
	PREDEFNODE(s_and, MERGE(GROUP_ACCU,	IDX816AND | IM_ACCUMULATOR)),
	PREDEFNODE(s_eor, MERGE(GROUP_ACCU,	IDX816EOR | IM_ACCUMULATOR)),
	PREDEFNODE("adc", MERGE(GROUP_ACCU,	IDX816ADC | IM_ACCUMULATOR)),
	PREDEFNODE("sta", MERGE(GROUP_ACCU,	IDX816STA)),
	PREDEFNODE("lda", MERGE(GROUP_ACCU,	IDX816LDA | IM_ACCUMULATOR)),
	PREDEFNODE("cmp", MERGE(GROUP_ACCU,	IDX816CMP | IM_ACCUMULATOR)),
	PREDEFNODE("sbc", MERGE(GROUP_ACCU,	IDX816SBC | IM_ACCUMULATOR)),
	PREDEFNODE("jmp", MERGE(GROUP_ALLJUMPS,	IDX816JMP)),
	PREDEFNODE("jsr", MERGE(GROUP_ALLJUMPS,	IDX816JSR)),
	// 
	PREDEFNODE("pei", MERGE(GROUP_ACCU,	IDX816PEI)),
	PREDEFNODE("jml", MERGE(GROUP_ALLJUMPS,	IDX816JML)),
	PREDEFNODE("jsl", MERGE(GROUP_ALLJUMPS,	IDX816JSL)),
	PREDEFNODE("mvp", MERGE(GROUP_BOTHMOVES,	0x44)),
	PREDEFNODE("mvn", MERGE(GROUP_BOTHMOVES,	0x54)),
	PREDEFNODE("per", MERGE(GROUP_REL16_3,	 98)),
	PREDEFNODE(s_brl, MERGE(GROUP_REL16_3,	130)),
	PREDEFNODE("cop", MERGE(GROUP_MISC,	IDX816COP)),
	PREDEFNODE("rep", MERGE(GROUP_MISC,	IDX816REP)),
	PREDEFNODE("sep", MERGE(GROUP_MISC,	IDX816SEP)),
	PREDEFNODE("pea", MERGE(GROUP_MISC,	IDX816PEA)),
	PREDEFNODE("phd", MERGE(GROUP_IMPLIEDONLY,	 11)),
	PREDEFNODE("tcs", MERGE(GROUP_IMPLIEDONLY,	 27)),
	PREDEFNODE("pld", MERGE(GROUP_IMPLIEDONLY,	 43)),
	PREDEFNODE("tsc", MERGE(GROUP_IMPLIEDONLY,	 59)),
	PREDEFNODE("wdm", MERGE(GROUP_IMPLIEDONLY,	 66)),
	PREDEFNODE("phk", MERGE(GROUP_IMPLIEDONLY,	 75)),
	PREDEFNODE("tcd", MERGE(GROUP_IMPLIEDONLY,	 91)),
	PREDEFNODE("rtl", MERGE(GROUP_IMPLIEDONLY,	107)),
	PREDEFNODE("tdc", MERGE(GROUP_IMPLIEDONLY,	123)),
	PREDEFNODE("phb", MERGE(GROUP_IMPLIEDONLY,	139)),
	PREDEFNODE("txy", MERGE(GROUP_IMPLIEDONLY,	155)),
	PREDEFNODE("plb", MERGE(GROUP_IMPLIEDONLY,	171)),
	PREDEFNODE("tyx", MERGE(GROUP_IMPLIEDONLY,	187)),
	PREDEFNODE("xba", MERGE(GROUP_IMPLIEDONLY,	235)),
	PREDEFLAST("xce", MERGE(GROUP_IMPLIEDONLY,	251)),
	//    ^^^^ this marks the last element
};

// 65ce02 has 46 new opcodes and a few changes:
static struct ronode	mnemos_65ce02[]	= {
	// 65ce02 changes (zp) addressing of 65c02 to (zp),z addressing:
	PREDEFNODE("ora", MERGE(GROUP_ACCU,	IDXeORA)),
	PREDEFNODE(s_and, MERGE(GROUP_ACCU,	IDXeAND)),
	PREDEFNODE(s_eor, MERGE(GROUP_ACCU,	IDXeEOR)),
	PREDEFNODE("adc", MERGE(GROUP_ACCU,	IDXeADC)),
	PREDEFNODE("sta", MERGE(GROUP_ACCU,	IDXeSTA)),	// +1 for (8,s),y
	PREDEFNODE("lda", MERGE(GROUP_ACCU,	IDXeLDA)),	// +1 for (8,s),y
	PREDEFNODE("cmp", MERGE(GROUP_ACCU,	IDXeCMP)),
	PREDEFNODE("sbc", MERGE(GROUP_ACCU,	IDXeSBC)),
	// more addressing modes:
	PREDEFNODE("jsr", MERGE(GROUP_ALLJUMPS,	IDXeJSR)),	// +2
	PREDEFNODE("stx", MERGE(GROUP_MISC,	IDXeSTX)),	// +1
	PREDEFNODE("sty", MERGE(GROUP_MISC,	IDXeSTY)),	// +1
	// +10 long branches (8 normal, 1 unconditional, BSR uncond to subroutine))
	PREDEFNODE("lbpl", MERGE(GROUP_REL16_2, 0x13)),
	PREDEFNODE("lbmi", MERGE(GROUP_REL16_2, 0x33)),
	PREDEFNODE("lbvc", MERGE(GROUP_REL16_2, 0x53)),
	PREDEFNODE("lbvs", MERGE(GROUP_REL16_2, 0x73)),
	PREDEFNODE("lbcc", MERGE(GROUP_REL16_2, 0x93)),
	PREDEFNODE("lbcs", MERGE(GROUP_REL16_2, 0xb3)),
	PREDEFNODE("lbne", MERGE(GROUP_REL16_2, 0xd3)),
	PREDEFNODE("lbeq", MERGE(GROUP_REL16_2, 0xf3)),
	PREDEFNODE("bru",  MERGE(GROUP_RELATIVE8, 0x80)),	// alias for 65c02's "bra"
	PREDEFNODE("bsr",  MERGE(GROUP_REL16_2, 0x63)),
	PREDEFNODE("lbru", MERGE(GROUP_REL16_2, 0x83)),
	PREDEFNODE("lbra", MERGE(GROUP_REL16_2, 0x83)),	// alias
	// new mnemonics:
	PREDEFNODE("asr", MERGE(GROUP_MISC,	IDXeASR)),
	PREDEFNODE("asw", MERGE(GROUP_MISC,	IDXeASW)),
	PREDEFNODE("cpz", MERGE(GROUP_MISC,	IDXeCPZ)),
	PREDEFNODE("dew", MERGE(GROUP_MISC,	IDXeDEW)),
	PREDEFNODE("inw", MERGE(GROUP_MISC,	IDXeINW)),
	PREDEFNODE("ldz", MERGE(GROUP_MISC,	IDXeLDZ)),
	PREDEFNODE("phw", MERGE(GROUP_MISC,	IDXePHW | IM_FORCE16)),
	PREDEFNODE("row", MERGE(GROUP_MISC,	IDXeROW)),
	PREDEFNODE("rtn", MERGE(GROUP_MISC,	IDXeRTN)),
	PREDEFNODE("cle", MERGE(GROUP_IMPLIEDONLY, 0x02)),
	PREDEFNODE("see", MERGE(GROUP_IMPLIEDONLY, 0x03)),
	PREDEFNODE("inz", MERGE(GROUP_IMPLIEDONLY, 0x1b)),
	PREDEFNODE("dez", MERGE(GROUP_IMPLIEDONLY, 0x3b)),
	PREDEFNODE("neg", MERGE(GROUP_IMPLIEDONLY, 0x42)),
	PREDEFNODE("tsy", MERGE(GROUP_IMPLIEDONLY, 0x0b)),
	PREDEFNODE("tys", MERGE(GROUP_IMPLIEDONLY, 0x2b)),
	PREDEFNODE("taz", MERGE(GROUP_IMPLIEDONLY, 0x4b)),
	PREDEFNODE("tab", MERGE(GROUP_IMPLIEDONLY, 0x5b)),
	PREDEFNODE("tza", MERGE(GROUP_IMPLIEDONLY, 0x6b)),
	PREDEFNODE("tba", MERGE(GROUP_IMPLIEDONLY, 0x7b)),
	PREDEFNODE("phz", MERGE(GROUP_IMPLIEDONLY, 0xdb)),
	PREDEFLAST("plz", MERGE(GROUP_IMPLIEDONLY, 0xfb)),
	//    ^^^^ this marks the last element
};

// 65ce02's "aug" opcode:
static struct ronode	mnemos_aug[]	= {
	PREDEFLAST("aug", MERGE(GROUP_IMPLIEDONLY, 0x5c)),	// actually a "4-byte NOP reserved for future expansion"
	//    ^^^^ this marks the last element
};

// 4502's "map" and "eom" opcodes:
static struct ronode	mnemos_map_eom[]	= {
	PREDEFNODE("map", MERGE(GROUP_IMPLIEDONLY, 0x5c)),	// change memory mapping
	PREDEFLAST("eom", MERGE(GROUP_IMPLIEDONLY, 0xea)),	// actually the NOP opcode
	//    ^^^^ this marks the last element
};

// Functions

// create dynamic buffer, build keyword trees
void Mnemo_init(void)
{
	mnemo_dyna_buf = DynaBuf_create(MNEMO_DYNABUF_INITIALSIZE);
	Tree_add_table(&mnemo_6502_tree, mnemos_6502);
	Tree_add_table(&mnemo_6502undoc1_tree, mnemos_6502undoc1);
	Tree_add_table(&mnemo_6502undoc2_tree, mnemos_6502undoc2);
	Tree_add_table(&mnemo_c64dtv2_tree, mnemos_c64dtv2);
	Tree_add_table(&mnemo_65c02_tree, mnemos_65c02);
	Tree_add_table(&mnemo_bitmanips_tree, mnemos_bitmanips);
	Tree_add_table(&mnemo_stp_wai_tree, mnemos_stp_wai);
	Tree_add_table(&mnemo_65816_tree, mnemos_65816);
	Tree_add_table(&mnemo_65ce02_tree, mnemos_65ce02);
	Tree_add_table(&mnemo_aug_tree, mnemos_aug);
	Tree_add_table(&mnemo_map_eom_tree, mnemos_map_eom);
}


// Address mode parsing

// utility function for parsing indices. result must be processed via AMB_PREINDEX() or AMB_INDEX() macro!
static int get_index(int next)
{
	if (next)
		GetByte();
	if (!Input_accept_comma())
		return INDEX_NONE;

	// there was a comma, so check next character (spaces will have been skipped):
	switch (GotByte) {
	case 's':
	case 'S':
		// if next character is 'p' or 'P', eat that as well, so ",sp" works just like ",s"
		GetByte();
		if ((GotByte == 'p') || (GotByte == 'P'))
			GetByte();
		SKIPSPACE();
		return INDEX_S;

	case 'x':
	case 'X':
		NEXTANDSKIPSPACE();
		return INDEX_X;

	case 'y':
	case 'Y':
		NEXTANDSKIPSPACE();
		return INDEX_Y;

	case 'z':
	case 'Z':
		NEXTANDSKIPSPACE();
		return INDEX_Z;
	}
	Throw_error(exception_syntax);
	return INDEX_NONE;
}

// This function stores the command's argument in the given result
// structure (using the valueparser). The addressing mode is returned.
static int get_argument(struct result *result)
{
	int	open_paren,
		address_mode_bits	= 0;

	SKIPSPACE();
	switch (GotByte) {
	case '[':
		GetByte();	// proceed with next char
		ALU_int_result(result);
		typesystem_want_addr(result);
		if (GotByte == ']')
			address_mode_bits |= AMB_LONGINDIRECT | AMB_INDEX(get_index(TRUE));
		else
			Throw_error(exception_syntax);
		break;
	case '#':
		GetByte();	// proceed with next char
		address_mode_bits |= AMB_IMMEDIATE;
		ALU_int_result(result);
		typesystem_want_imm(result);	// FIXME - this is wrong for 65ce02's PHW#
		break;
	default:
		// liberal, to allow for "(...,"
		open_paren = ALU_liberal_int(result);
		typesystem_want_addr(result);
		// check for implied addressing
		if ((result->flags & MVALUE_EXISTS) == 0)
			address_mode_bits |= AMB_IMPLIED;
		// check for indirect addressing
		if (result->flags & MVALUE_INDIRECT)
			address_mode_bits |= AMB_INDIRECT;
		// check for internal index (before closing parenthesis)
		if (open_paren) {
			// in case there are still open parentheses,
			// read internal index
			address_mode_bits |= AMB_PREINDEX(get_index(FALSE));
			if (GotByte == ')')
				GetByte();	// go on with next char
			else
				Throw_error(exception_syntax);
		}
		// check for external index (after closing parenthesis)
		address_mode_bits |= AMB_INDEX(get_index(FALSE));
	}
	// ensure end of line
	Input_ensure_EOS();
	//printf("AM: %x\n", addressing_mode);
	return address_mode_bits;
}

// Helper function for calc_arg_size()
// Only call with "size_bit = MVALUE_FORCE16" or "size_bit = MVALUE_FORCE24"
static int check_oversize(int size_bit, struct result *argument)
{
	// only check if value is *defined*
	if ((argument->flags & MVALUE_DEFINED) == 0)
		return size_bit;	// pass on result

	// value is defined, so check
	if (size_bit == MVALUE_FORCE16) {
		// check 16-bit argument for high byte zero
		if ((argument->val.intval <= 255) && (argument->val.intval >= -128))
			Throw_warning(exception_oversized_addrmode);
	} else {
		// check 24-bit argument for bank byte zero
		if ((argument->val.intval <= 65535) && (argument->val.intval >= -32768))
			Throw_warning(exception_oversized_addrmode);
	}
	return size_bit;	// pass on result
}

// Utility function for comparing force bits, argument value, argument size,
// "unsure"-flag and possible addressing modes. Returns force bit matching
// number of parameter bytes to send. If it returns zero, an error occurred
// (and has already been delivered).
// force_bit		none, 8b, 16b, 24b
// argument		value and flags of parameter
// addressing_modes	adressing modes (8b, 16b, 24b or any combination)
// Return value = force bit for number of parameter bytes to send (0 = error)
static int calc_arg_size(int force_bit, struct result *argument, int addressing_modes)
{
	// if there are no possible addressing modes, complain
	if (addressing_modes == MAYBE______) {
		Throw_error(exception_illegal_combination);
		return 0;
	}
	// if command has force bit, act upon it
	if (force_bit) {
		// if command allows this force bit, return it
		if (addressing_modes & force_bit)
			return force_bit;

		// if not, complain
		Throw_error("Illegal combination of command and postfix.");
		return 0;
	}

	// Command has no force bit. Check whether value has one
	// if value has force bit, act upon it
	if (argument->flags & MVALUE_FORCEBITS) {
		// Value has force bit set, so return this or bigger size
		if (MVALUE_FORCE08 & addressing_modes & argument->flags)
			return MVALUE_FORCE08;

		if (MVALUE_FORCE16 & addressing_modes & argument->flags)
			return MVALUE_FORCE16;

		if (MVALUE_FORCE24 & addressing_modes)
			return MVALUE_FORCE24;

		Throw_error(exception_number_out_of_range);	// else error
		return 0;
	}

	// Value has no force bit. Check whether there's only one addr mode
	// if only one addressing mode, use that
	if ((addressing_modes == MVALUE_FORCE08)
	|| (addressing_modes == MVALUE_FORCE16)
	|| (addressing_modes == MVALUE_FORCE24))
		return addressing_modes;	// There's only one, so use it

	// There's more than one addressing mode. Check whether value is sure
	// if value is unsure, use default size
	if (argument->flags & MVALUE_UNSURE) {
		// if there is an 8-bit addressing mode *and* the value
		// is sure to fit into 8 bits, use the 8-bit mode
		if ((addressing_modes & MVALUE_FORCE08) && (argument->flags & MVALUE_ISBYTE))
			return MVALUE_FORCE08;

		// if there is a 16-bit addressing, use that
		// call helper function for "oversized addr mode" warning
		if (MVALUE_FORCE16 & addressing_modes)
			return check_oversize(MVALUE_FORCE16, argument);

		// if there is a 24-bit addressing, use that
		// call helper function for "oversized addr mode" warning
		if (MVALUE_FORCE24 & addressing_modes)
			return check_oversize(MVALUE_FORCE24, argument);

		// otherwise, use 8-bit-addressing, which will raise an
		// error later on if the value won't fit
		return MVALUE_FORCE08;
	}

	// Value is sure, so use its own size
	// if value is negative, size cannot be chosen. Complain!
	if (argument->val.intval < 0) {
		Throw_error("Negative value - cannot choose addressing mode.");
		return 0;
	}

	// Value is positive or zero. Check size ranges
	// if there is an 8-bit addressing mode and value fits, use 8 bits
	if ((addressing_modes & MVALUE_FORCE08) && (argument->val.intval < 256))
		return MVALUE_FORCE08;

	// if there is a 16-bit addressing mode and value fits, use 16 bits
	if ((addressing_modes & MVALUE_FORCE16) && (argument->val.intval < 65536))
		return MVALUE_FORCE16;

	// if there is a 24-bit addressing mode, use that. In case the
	// value doesn't fit, the output function will do the complaining.
	if (addressing_modes & MVALUE_FORCE24)
		return MVALUE_FORCE24;

	// Value is too big for all possible addressing modes
	Throw_error(exception_number_out_of_range);
	return 0;
}

// Mnemonics using only implied addressing.
static void group_only_implied_addressing(int opcode)
{
	// for 65ce02 and 4502, warn about buggy decimal mode
	if ((opcode == 248) && (CPU_state.type->flags & CPUFLAG_DECIMALSUBTRACTBUGGY))
		Throw_first_pass_warning("Found SED instruction for CPU with known decimal SBC bug.");
	Output_byte(opcode);
	Input_ensure_EOS();
}

// helper function to output "Target not in bank" message
static void not_in_bank(intval_t target)
{
	char	buffer[60];	// 640K should be enough for anybody

	sprintf(buffer, "Target not in bank (0x%lx).", (long) target);
	Throw_error(buffer);
}

// helper function for branches with 8-bit offset (including bbr0..7/bbs0..7)
static void near_branch(int preoffset)
{
	struct result	target;
	intval_t	offset	= 0;	// dummy value, to not throw more errors than necessary

	ALU_int_result(&target);	// FIXME - check for outermost parentheses and raise error!
	typesystem_want_addr(&target);
	// FIXME - read pc via function call instead!
	if (CPU_state.pc.flags & target.flags & MVALUE_DEFINED) {
		if ((target.val.intval | 0xffff) != 0xffff) {
			not_in_bank(target.val.intval);
		} else {
			offset = (target.val.intval - (CPU_state.pc.val.intval + preoffset)) & 0xffff;	// clip to 16 bit offset
			// fix sign
			if (offset & 0x8000)
				offset -= 0x10000;
			// range check
			if ((offset < -128) || (offset > 127)) {
				char	buffer[60];	// 640K should be enough for anybody

				sprintf(buffer, "Target out of range (%ld; %ld too far).", (long) offset, (long) (offset < -128 ? -128 - offset : offset - 127));
				Throw_error(buffer);
			}
		}
	}
	// this fn has its own range check (see above).
	// No reason to irritate the user with another error message,
	// so use Output_byte() instead of output_8()
	//output_8(offset);
	Output_byte(offset);
	Input_ensure_EOS();
}

// helper function for relative addressing with 16-bit offset
static void far_branch(int preoffset)
{
	struct result	target;
	intval_t	offset	= 0;	// dummy value, to not throw more errors than necessary

	ALU_int_result(&target);	// FIXME - check for outermost parentheses and raise error!
	typesystem_want_addr(&target);
	// FIXME - read pc via function call instead!
	if (CPU_state.pc.flags & target.flags & MVALUE_DEFINED) {
		if ((target.val.intval | 0xffff) != 0xffff) {
			not_in_bank(target.val.intval);
		} else {
			offset = (target.val.intval - (CPU_state.pc.val.intval + preoffset)) & 0xffff;
			// no further checks necessary, 16-bit branches can access whole bank
		}
	}
	output_le16(offset);
	Input_ensure_EOS();
}

// set addressing mode bits depending on which opcodes exist, then calculate
// argument size and output both opcode and argument
static void make_command(int force_bit, struct result *result, unsigned long opcodes)
{
	int	addressing_modes	= MAYBE______;

	if (opcodes & 0x0000ff)
		addressing_modes |= MAYBE_1____;
	if (opcodes & 0x00ff00)
		addressing_modes |= MAYBE___2__;
	if (opcodes & 0xff0000)
		addressing_modes |= MAYBE_____3;
	switch (calc_arg_size(force_bit, result, addressing_modes)) {
	case MVALUE_FORCE08:
		Output_byte(opcodes & 255);
		output_8(result->val.intval);
		break;
	case MVALUE_FORCE16:
		Output_byte((opcodes >> 8) & 255);
		output_le16(result->val.intval);
		break;
	case MVALUE_FORCE24:
		Output_byte((opcodes >> 16) & 255);
		output_le24(result->val.intval);
	}
}

// check whether 16bit immediate addressing is allowed. If not, return given
// opcode. If it is allowed, set force bits according to CPU register length
// and return given opcode for both 8- and 16-bit mode.
static unsigned int imm_ops(int *force_bit, unsigned char opcode, int immediate_mode)
{
	int	long_register	= 0;

	switch (immediate_mode) {
	case IM_FORCE8:
		return opcode;	// result in bits 0..7 forces single-byte argument

	case IM_FORCE16:	// currently only for 65ce02's PHW#
		return ((unsigned int) opcode) << 8;	// opcode in bits8.15 forces two-byte argument

	case IM_ACCUMULATOR:	// for 65816
		long_register = CPU_state.a_is_long;
		break;
	case IM_INDEXREGS:	// for 65816
		long_register = CPU_state.xy_are_long;
		break;
	default:
		Bug_found("IllegalImmediateMode", immediate_mode);
	}
	// if the CPU does not support long registers...
	if ((CPU_state.type->flags & CPUFLAG_SUPPORTSLONGREGS) == 0)
		return opcode;	// result in bits 0..7 forces single-byte argument

	// check force bits - if no force bits given, use cpu state and convert to force bit
	if (*force_bit == 0)
		*force_bit = long_register ? MVALUE_FORCE16 : MVALUE_FORCE08;
	// return identical opcodes for single-byte and two-byte arguments:
	return (((unsigned int) opcode) << 8) | opcode;
}

// helper function to warn if zp pointer wraps around
static void check_zp_wraparound(struct result *result)
{
	if ((result->val.intval == 0xff)
	&& (result->flags & MVALUE_DEFINED))
		Throw_warning("Zeropage pointer wraps around from $ff to $00");
}

// The main accumulator stuff (ADC, AND, CMP, EOR, LDA, ORA, SBC, STA)
// plus PEI.
static void group_main(int index, int immediate_mode)
{
	unsigned long	immediate_opcodes;
	struct result	result;
	int		force_bit	= Input_get_force_bit();	// skips spaces after

	switch (get_argument(&result)) {
	case IMMEDIATE_ADDRESSING:	// #$ff or #$ffff (depending on accu length)
		immediate_opcodes = imm_ops(&force_bit, accu_imm[index], immediate_mode);
		// CAUTION - do not incorporate the line above into the line
		// below - "force_bit" might be undefined (depends on compiler).
		make_command(force_bit, &result, immediate_opcodes);
		break;
	case ABSOLUTE_ADDRESSING:	// $ff, $ffff, $ffffff
		make_command(force_bit, &result, accu_abs[index]);
		break;
	case X_INDEXED_ADDRESSING:	// $ff,x, $ffff,x, $ffffff,x
		make_command(force_bit, &result, accu_xabs[index]);
		break;
	case Y_INDEXED_ADDRESSING:	// $ffff,y (in theory, "$ff,y" as well)
		make_command(force_bit, &result, accu_yabs[index]);
		break;
	case STACK_INDEXED_ADDRESSING:	// $ff,s
		make_command(force_bit, &result, accu_sabs8[index]);
		break;
	case X_INDEXED_INDIRECT_ADDRESSING:	// ($ff,x)
		make_command(force_bit, &result, accu_xind8[index]);
		break;
	case INDIRECT_ADDRESSING:	// ($ff)
		make_command(force_bit, &result, accu_ind8[index]);
		check_zp_wraparound(&result);
		break;
	case INDIRECT_Y_INDEXED_ADDRESSING:	// ($ff),y
		make_command(force_bit, &result, accu_indy8[index]);
		check_zp_wraparound(&result);
		break;
	case INDIRECT_Z_INDEXED_ADDRESSING:	// ($ff),z
		make_command(force_bit, &result, accu_indz8[index]);
		check_zp_wraparound(&result);
		break;
	case LONG_INDIRECT_ADDRESSING:	// [$ff]
		make_command(force_bit, &result, accu_lind8[index]);
		break;
	case LONG_INDIRECT_Y_INDEXED_ADDRESSING:	// [$ff],y
		make_command(force_bit, &result, accu_lindy8[index]);
		break;
	case STACK_INDEXED_INDIRECT_Y_INDEXED_ADDRESSING:	// ($ff,s),y
		make_command(force_bit, &result, accu_sindy8[index]);
		break;
	default:	// other combinations are illegal
		Throw_error(exception_illegal_combination);
	}
}

// Various mnemonics with different addressing modes.
static void group_misc(int index, int immediate_mode)
{
	unsigned long	immediate_opcodes;
	struct result	result;
	int		force_bit	= Input_get_force_bit();	// skips spaces after

	switch (get_argument(&result)) {
	case IMPLIED_ADDRESSING:	// implied addressing
		if (misc_impl[index])
			Output_byte(misc_impl[index]);
		else
			Throw_error(exception_illegal_combination);
		break;
	case IMMEDIATE_ADDRESSING:	// #$ff or #$ffff (depending on index register length)
		immediate_opcodes = imm_ops(&force_bit, misc_imm[index], immediate_mode);
		// CAUTION - do not incorporate the line above into the line
		// below - "force_bit" might be undefined (depends on compiler).
		make_command(force_bit, &result, immediate_opcodes);
		// check whether to warn about 6510's unstable ANE/LXA
		if ((CPU_state.type->flags & CPUFLAG_8B_AND_AB_NEED_0_ARG)
		&& ((result.val.intval & 0xff) != 0x00)
		&& (result.flags & MVALUE_DEFINED)) {
			if (immediate_opcodes == 0x8b)
				Throw_warning("Assembling unstable ANE #NONZERO instruction");
			else if (immediate_opcodes == 0xab)
				Throw_warning("Assembling unstable LXA #NONZERO instruction");
		}
		break;
	case ABSOLUTE_ADDRESSING:	// $ff or  $ffff
		make_command(force_bit, &result, misc_abs[index]);
		break;
	case X_INDEXED_ADDRESSING:	// $ff,x  or  $ffff,x
		make_command(force_bit, &result, misc_xabs[index]);
		break;
	case Y_INDEXED_ADDRESSING:	// $ff,y  or  $ffff,y
		make_command(force_bit, &result, misc_yabs[index]);
		break;
	default:	// other combinations are illegal
		Throw_error(exception_illegal_combination);
	}
}

// mnemonics using only 8bit relative addressing (short branch instructions).
static void group_std_branches(int opcode)
{
	Output_byte(opcode);
	near_branch(2);
}

// "bbr0..7" and "bbs0..7"
static void group_bbr_bbs(int opcode)
{
	struct result	zpmem;

	ALU_int_result(&zpmem);	// FIXME - check for outermost parentheses and raise error!
	typesystem_want_addr(&zpmem);
	if (Input_accept_comma()) {
		Output_byte(opcode);
		Output_byte(zpmem.val.intval);
		near_branch(3);
	} else {
		Throw_error(exception_syntax);
	}
}

// mnemonics using only 16bit relative addressing (BRL and PER of 65816, and the long branches of 65ce02)
static void group_relative16(int opcode, int preoffset)
{
	Output_byte(opcode);
	far_branch(preoffset);
}

// "mvn" and "mvp"
static void group_mvn_mvp(int opcode)
{
	struct result	source,
			target;

	// assembler syntax: "mnemonic source, target"
	ALU_int_result(&source);	// FIXME - check for outermost parentheses and raise error!
	typesystem_want_imm(&source);
	if (Input_accept_comma()) {
		ALU_int_result(&target);	// FIXME - check for outermost parentheses and raise error!
		typesystem_want_imm(&target);
		// machine language order: "opcode target source"
		Output_byte(opcode);
		output_8(target.val.intval);
		output_8(source.val.intval);
		Input_ensure_EOS();
	} else {
		Throw_error(exception_syntax);
	}
}

// "rmb0..7" and "smb0..7"
static void group_only_zp(int opcode)
{
	struct result	target;

	ALU_int_result(&target);	// FIXME - check for outermost parentheses and raise error!
	typesystem_want_addr(&target);
	Output_byte(opcode);
	output_8(target.val.intval);
	Input_ensure_EOS();
}

// The jump instructions.
static void group_jump(int index)
{
	struct result	result;
	int		force_bit	= Input_get_force_bit();	// skips spaces after

	switch (get_argument(&result)) {
	case ABSOLUTE_ADDRESSING:	// absolute16 or absolute24
		make_command(force_bit, &result, jump_abs[index]);
		break;
	case INDIRECT_ADDRESSING:	// ($ffff)
		make_command(force_bit, &result, jump_ind[index]);
		// check whether to warn about 6502's JMP() bug
		if (((result.val.intval & 0xff) == 0xff)
		&& (result.flags & MVALUE_DEFINED)
		&& (CPU_state.type->flags & CPUFLAG_INDIRECTJMPBUGGY))
			Throw_warning("Assembling buggy JMP($xxff) instruction");
		break;
	case X_INDEXED_INDIRECT_ADDRESSING:	// ($ffff,x)
		make_command(force_bit, &result, jump_xind[index]);
		break;
	case LONG_INDIRECT_ADDRESSING:	// [$ffff]
		make_command(force_bit, &result, jump_lind[index]);
		break;
	default:	// other combinations are illegal
		Throw_error(exception_illegal_combination);
	}
}

// Work function
static int check_mnemo_tree(struct ronode *tree, struct dynabuf *dyna_buf)
{
	void	*node_body;
	int	code,
		immediate_mode;	// size of immediate argument

	// search for tree item
	if (!Tree_easy_scan(tree, &node_body, dyna_buf))
		return FALSE;

	code = ((int) node_body) & CODEMASK;	// get opcode or table index
	immediate_mode = ((int) node_body) & IMMASK;	// get immediate mode
	switch (GROUP((long) node_body)) {
	case GROUP_ACCU:	// main accumulator stuff
		group_main(code, immediate_mode);
		break;
	case GROUP_MISC:	// misc
		group_misc(code, immediate_mode);
		break;
	case GROUP_ALLJUMPS:	// the jump instructions
		group_jump(code);
		break;
	case GROUP_IMPLIEDONLY:	// mnemonics with only implied addressing
		group_only_implied_addressing(code);
		break;
	case GROUP_RELATIVE8:	// short relative
		group_std_branches(code);
		break;
	case GROUP_BITBRANCH:	// "bbr0..7" and "bbs0..7"
		group_bbr_bbs(code);
		break;
	case GROUP_REL16_2:	// long relative to pc+2
		group_relative16(code, 2);
		break;
	case GROUP_REL16_3:	// long relative to pc+3
		group_relative16(code, 3);
		break;
	case GROUP_BOTHMOVES:	// "mvp" and "mvn"
		group_mvn_mvp(code);
		break;
	case GROUP_ZPONLY:	// "rmb0..7" and "smb0..7"
		group_only_zp(code);
		break;
	default:	// others indicate bugs
		Bug_found("IllegalGroupIndex", code);
	}
	return TRUE;
}

// check whether mnemonic in GlobalDynaBuf is supported by 6502 cpu.
int keyword_is_6502_mnemo(int length)
{
	if (length != 3)
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	return check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by 6510 cpu.
int keyword_is_6510_mnemo(int length)
{
	if (length != 3)
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check undocumented ("illegal") opcodes...
	if (check_mnemo_tree(mnemo_6502undoc1_tree, mnemo_dyna_buf))
		return TRUE;

	// then check some more undocumented ("illegal") opcodes...
	if (check_mnemo_tree(mnemo_6502undoc2_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes
	return check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by C64DTV2 cpu.
int keyword_is_c64dtv2_mnemo(int length)
{
	if (length != 3)
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check C64DTV2 extensions...
	if (check_mnemo_tree(mnemo_c64dtv2_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check a few undocumented ("illegal") opcodes...
	if (check_mnemo_tree(mnemo_6502undoc1_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes
	return check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by 65c02 cpu.
int keyword_is_65c02_mnemo(int length)
{
	if (length != 3)
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes
	return check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by Rockwell 65c02 cpu.
int keyword_is_r65c02_mnemo(int length)
{
	if ((length != 3) && (length != 4))
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check 65c02 extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes...
	if (check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check Rockwell extensions (rmb, smb, bbr, bbs)
	return check_mnemo_tree(mnemo_bitmanips_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by WDC w65c02 cpu.
int keyword_is_w65c02_mnemo(int length)
{
	if ((length != 3) && (length != 4))
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check 65c02 extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes...
	if (check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check Rockwell extensions (rmb, smb, bbr, bbs)...
	if (check_mnemo_tree(mnemo_bitmanips_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check WDC extensions "stp" and "wai"
	return check_mnemo_tree(mnemo_stp_wai_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by CSG 65CE02 cpu.
int keyword_is_65ce02_mnemo(int length)
{
	if ((length != 3) && (length != 4))
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check 65ce02 extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65ce02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check 65c02 extensions because of the same reason...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes...
	if (check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check Rockwell extensions (rmb, smb, bbr, bbs)...
	if (check_mnemo_tree(mnemo_bitmanips_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check "aug"
	return check_mnemo_tree(mnemo_aug_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by CSG 4502 cpu.
int keyword_is_4502_mnemo(int length)
{
	if ((length != 3) && (length != 4))
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check 65ce02 extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65ce02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check 65c02 extensions because of the same reason...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes...
	if (check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check Rockwell extensions (rmb, smb, bbr, bbs)...
	if (check_mnemo_tree(mnemo_bitmanips_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check "map" and "eom"
	return check_mnemo_tree(mnemo_map_eom_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}

// check whether mnemonic in GlobalDynaBuf is supported by 65816 cpu.
int keyword_is_65816_mnemo(int length)
{
	if (length != 3)
		return FALSE;

	// make lower case version of mnemonic in local dynamic buffer
	DynaBuf_to_lower(mnemo_dyna_buf, GlobalDynaBuf);
	// first check 65816 extensions because some mnemonics gained new addressing modes...
	if (check_mnemo_tree(mnemo_65816_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check 65c02 extensions because of the same reason...
	if (check_mnemo_tree(mnemo_65c02_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check original opcodes...
	if (check_mnemo_tree(mnemo_6502_tree, mnemo_dyna_buf))
		return TRUE;

	// ...then check WDC extensions "stp" and "wai"
	return check_mnemo_tree(mnemo_stp_wai_tree, mnemo_dyna_buf) ? TRUE : FALSE;
}
