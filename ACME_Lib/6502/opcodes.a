;ACME 0.94.4

!ifdef lib_6502_opcodes_a !eof
lib_6502_opcodes_a = 1

; if your self-modifying code not only changes its arguments, but even its opcodes, use these constants.
; unused slots in this table indicate "illegal" instructions.

; addressing mode modifiers:
;	<nothing>	for implied addressing (BRK/CLC/NOP/TAX/...) and branches
;	_A		for "accumulator addressing" (ASL/ROL/LSR/ROR without argument)
;			(...using "_A" instead of nothing so user does not forget to specify addressing mode)
;	_IMM		8-bit constant (immediate addressing)
;	_8		8-bit address (zero page addressing)
;	_8X, _8Y	8-bit address with X/Y indexing
;	_8XI, _8IY	8-bit address, indirect: TLA ($12, x) and TLA ($12), y
;	_16		16-bit address (absolute addressing)
;	_16X, _16Y	16-bit address with X/Y indexing
;	_16I		16-bit address, indirect: JMP ($1234)

;pattern %......00	 pattern %......01	 pattern %......10	 pattern %......11 (all illegal)
opcode_BRK	= 0x00:	opcode_ORA_8XI	= 0x01
			opcode_ORA_8	= 0x05:	opcode_ASL_8	= 0x06
opcode_PHP	= 0x08:	opcode_ORA_IMM	= 0x09:	opcode_ASL_A	= 0x0a
			opcode_ORA_16	= 0x0d:	opcode_ASL_16	= 0x0e
opcode_BPL	= 0x10:	opcode_ORA_8IY	= 0x11
			opcode_ORA_8X	= 0x15:	opcode_ASL_8X	= 0x16
opcode_CLC	= 0x18:	opcode_ORA_16Y	= 0x19
			opcode_ORA_16X	= 0x1d:	opcode_ASL_16X	= 0x1e
opcode_JSR_16	= 0x20:	opcode_AND_8XI	= 0x21
opcode_BIT_8	= 0x24:	opcode_AND_8	= 0x25: opcode_ROL_8	= 0x26
opcode_PLP	= 0x28:	opcode_AND_IMM	= 0x29:	opcode_ROL_A	= 0x2a
opcode_BIT_16	= 0x2c:	opcode_AND_16	= 0x2d:	opcode_ROL_16	= 0x2e
opcode_BMI	= 0x30:	opcode_AND_8IY	= 0x31
			opcode_AND_8X	= 0x35:	opcode_ROL_8X	= 0x36
opcode_SEC	= 0x38:	opcode_AND_16Y	= 0x39
			opcode_AND_16X	= 0x3d:	opcode_ROL_16X	= 0x3e
opcode_RTI	= 0x40:	opcode_EOR_8XI	= 0x41
			opcode_EOR_8	= 0x45:	opcode_LSR_8	= 0x46
opcode_PHA	= 0x48:	opcode_EOR_IMM	= 0x49:	opcode_LSR_A	= 0x4a
opcode_JMP_16	= 0x4c:	opcode_EOR_16	= 0x4d:	opcode_LSR_16	= 0x4e
opcode_BVC	= 0x50:	opcode_EOR_8IY	= 0x51
			opcode_EOR_8X	= 0x55:	opcode_LSR_8X	= 0x56
opcode_CLI	= 0x58:	opcode_EOR_16Y	= 0x59
			opcode_EOR_16X	= 0x5d:	opcode_LSR_16X	= 0x5e
opcode_RTS	= 0x60:	opcode_ADC_8XI	= 0x61
			opcode_ADC_8	= 0x65:	opcode_ROR_8	= 0x66
opcode_PLA	= 0x68:	opcode_ADC_IMM	= 0x69:	opcode_ROR_A	= 0x6a
opcode_JMP_16I	= 0x6c:	opcode_ADC_16	= 0x6d:	opcode_ROR_16	= 0x6e
opcode_BVS	= 0x70:	opcode_ADC_8IY	= 0x71
			opcode_ADC_8X	= 0x75:	opcode_ROR_8X	= 0x76
opcode_SEI	= 0x78:	opcode_ADC_16Y	= 0x79
			opcode_ADC_16X	= 0x7d:	opcode_ROR_16X	= 0x7e
			opcode_STA_8XI	= 0x81
opcode_STY_8	= 0x84:	opcode_STA_8	= 0x85:	opcode_STX_8	= 0x86
opcode_DEY	= 0x88:				opcode_TXA	= 0x8a
opcode_STY_16	= 0x8c:	opcode_STA_16	= 0x8d:	opcode_STX_16	= 0x8e
opcode_BCC	= 0x90:	opcode_STA_8IY	= 0x91
opcode_STY_8X	= 0x94:	opcode_STA_8X	= 0x95:	opcode_STX_8Y	= 0x96
opcode_TYA	= 0x98:	opcode_STA_16Y	= 0x99:	opcode_TXS	= 0x9a
			opcode_STA_16X	= 0x9d
opcode_LDY_IMM	= 0xa0:	opcode_LDA_8XI	= 0xa1:	opcode_LDX_IMM	= 0xa2
opcode_LDY_8	= 0xa4:	opcode_LDA_8	= 0xa5:	opcode_LDX_8	= 0xa6
opcode_TAY	= 0xa8:	opcode_LDA_IMM	= 0xa9:	opcode_TAX	= 0xaa
opcode_LDY_16	= 0xac:	opcode_LDA_16	= 0xad:	opcode_LDX_16	= 0xae
opcode_BCS	= 0xb0:	opcode_LDA_8IY	= 0xb1
opcode_LDY_8X	= 0xb4:	opcode_LDA_8X	= 0xb5:	opcode_LDX_8Y	= 0xb6
opcode_CLV	= 0xb8:	opcode_LDA_16Y	= 0xb9:	opcode_TSX	= 0xba
opcode_LDY_16X	= 0xbc:	opcode_LDA_16X	= 0xbd:	opcode_LDX_16Y	= 0xbe
opcode_CPY_IMM	= 0xc0:	opcode_CMP_8XI	= 0xc1
opcode_CPY_8	= 0xc4:	opcode_CMP_8	= 0xc5:	opcode_DEC_8	= 0xc6
opcode_INY	= 0xc8:	opcode_CMP_IMM	= 0xc9:	opcode_DEX	= 0xca
opcode_CPY_16	= 0xcc:	opcode_CMP_16	= 0xcd:	opcode_DEC_16	= 0xce
opcode_BNE	= 0xd0:	opcode_CMP_8IY	= 0xd1
			opcode_CMP_8X	= 0xd5:	opcode_DEC_8X	= 0xd6
opcode_CLD	= 0xd8:	opcode_CMP_16Y	= 0xd9
			opcode_CMP_16X	= 0xdd:	opcode_DEC_16X	= 0xde
opcode_CPX_IMM	= 0xe0:	opcode_SBC_8XI	= 0xe1
opcode_CPX_8	= 0xe4:	opcode_SBC_8	= 0xe5:	opcode_INC_8	= 0xe6
opcode_INX	= 0xe8:	opcode_SBC_IMM	= 0xe9:	opcode_NOP	= 0xea
opcode_CPX_16	= 0xec:	opcode_SBC_16	= 0xed:	opcode_INC_16	= 0xee
opcode_BEQ	= 0xf0:	opcode_SBC_8IY	= 0xf1
			opcode_SBC_8X	= 0xf5:	opcode_INC_8X	= 0xf6
opcode_SED	= 0xf8:	opcode_SBC_16Y	= 0xf9
			opcode_SBC_16X	= 0xfd:	opcode_INC_16X	= 0xfe
