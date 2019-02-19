;ACME 0.95

!ifdef lib_cbm_c128_kernal_a !eof
lib_cbm_c128_kernal_a = 1

; Taken from the web.
; Sorry, I can't give credit because I don't have the URL anymore.

!address {
	k_spin_spout	= $ff47
	k_close_all	= $ff4a
	k_c64mode	= $ff4d	; enter c64 mode (so does not return)
	k_dma_call	= $ff50
	k_boot_call	= $ff53
	k_phoenix	= $ff56
	k_lkupla	= $ff59	; find channel with file number A
	k_lkupsa	= $ff5c	; find channel with secondary address Y
	k_swapper	= $ff5f
	k_dlchr		= $ff62
	k_pfkey		= $ff65
	k_setbnk	= $ff68	; set banks for file name and load/save calls
	k_getcfg	= $ff6b
	k_jsrfar	= $ff6e
	k_jmpfar	= $ff71
	k_indfet	= $ff74
	k_indsta	= $ff77
	k_indcmp	= $ff7a
	k_primm		= $ff7d	; "print immediate" - output string without having to setup a pointer:
		; string must follow JSR $ff7d in memory, code execution will resume after terminating zero.
		; A/X/Y are preserved
	k_release_number	= $ff80
}
!source <cbm/kernal.a>	; $ff81-$fff5 is mostly compatible to older machines
; $fff6/$fff7 are unused (ff ff)
!address {
	k_indirect128mode	= $fff8	; indirect vector, without JMP command!
}
; $fffa through $ffff are cpu hardware vectors (see <6502/std.a>)
