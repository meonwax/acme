;ACME 0.95

!ifdef lib_cbm_c64_cia2_a !eof
lib_cbm_c64_cia2_a = 1

!address {
	cia2_pra	= $dd00
	; %7.......	DATA in (0 means GND)
	; %.6......	CLK in (0 means GND)
	; %..5.....	DATA out (inverted, 1 means GND)
	; %...4....	CLK out	(inverted, 1 means GND)
	; %....3...	ATN out (inverted, 1 means GND)
	; %.....2..	PA2 (pin M at user port, 0 means GND)
	; %......10	VIC bank (inverted, so value $3 means address $0000)
	cia2_prb	= $dd01
	cia2_ddra	= $dd02
	cia2_ddrb	= $dd03
	cia2_ta_lo	= $dd04
	cia2_ta_hi	= $dd05
	cia2_tb_lo	= $dd06
	cia2_tb_hi	= $dd07
	cia2_tod10ths	= $dd08
	cia2_todsec	= $dd09
	cia2_todmin	= $dd0a
	cia2_todhr	= $dd0b
	cia2_sdr	= $dd0c
	cia2_icr	= $dd0d
	cia2_cra	= $dd0e
	cia2_crb	= $dd0f
}
