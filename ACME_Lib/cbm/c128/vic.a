;ACME 0.95

!ifdef lib_cbm_c128_vic_a !eof
lib_cbm_c128_vic_a = 1

!source <cbm/c64/vic.a>	; registers 0..2e

!address {
	; registers only present in the C128 variant of this chip:
	vic_keyboard	= $d02f
	vic_clock	= $d030

	; the c128 ROMs contain two copies of a look-up table to convert vic color
	; values to their corresponding petscii color codes:
	; rom4_* needs "low rom area" enabled ($4000..$7fff)
	; romc_* needs "high rom area" enabled ($c000..$ffff)
	rom4_vic_to_petscii_color_table	= $76b5 ; 90 05 1c 9f 9c 1e 1f 9e 81 95 96 97 98 99 9a 9b
	romc_vic_to_petscii_color_table	= $ce4c ; 90 05 1c 9f 9c 1e 1f 9e 81 95 96 97 98 99 9a 9b
}
