;ACME 0.95

!ifdef lib_cbm_c64_reu_a !eof
lib_cbm_c64_reu_a = 1

; This file contains definitions for accessing a RAM Expansion Unit (REU) of
; type 1700, 1764, 1750 and compatible. These units contain a chip called REC
; (RAM Expansion Controller) capable of direct memory access (DMA).
; Standard base address of control registers is $df00 in i/o space.

!address {
	; status register
	rec_status	= $df00	; reading will clear IRQ, END and ERROR bits
}
rec_STATUS_IRQ		= %#.......
rec_STATUS_END		= %.#......
rec_STATUS_ERROR	= %..#.....	; for verify command
rec_STATUS_TYPE		= %...#....	; chip type (do not use to determine unit size!)
rec_STATUS_VERSION	= %....####
!address {
	; command register
	rec_command	= $df01
}
rec_COMMAND_EXECUTE	= %#.......
;reserved		= %.#......
rec_COMMAND_RELOAD	= %..#.....
rec_COMMAND_IMMEDIATELY	= %...#....	; do not wait for $ff00 write
;reserved		= %....##..
rec_COMMAND_MODE_MASK	= %......##	; bit mask for the four modes
rec_COMMAND_MODE_STASH	= %........	; computer-to-REU
rec_COMMAND_MODE_FETCH	= %.......#	; REU-to-computer
rec_COMMAND_MODE_SWAP	= %......#.	; exchange
rec_COMMAND_MODE_VERIFY	= %......##	; compare
rec_COMMAND_STASH	= %#.#.....	; these wait for $ff00 before
rec_COMMAND_FETCH	= %#.#....#	;	starting and then reload values.
!address {
	; internal address (computer RAM)
	rec_int_low	= $df02
	rec_int_high	= $df03
	; external address (expansion RAM)
	rec_ext_low	= $df04
	rec_ext_high	= $df05
	rec_ext_bank	= $df06
	; A stock 1700 unit has two banks (128 KiB).
	; A stock 1764 unit has four banks (256 KiB).
	; A stock 1750 unit has eight banks (512 KiB).
	; Upgraded units and clones may have more, but the REC chip will always
	; "wrap around" after eight banks if crossing bank borders!
	; amount of bytes to process
	rec_amount_low	= $df07
	rec_amount_high	= $df08
	; when to request interrupts
	rec_irqctrl	= $df09
}
rec_IRQCTRL_ENABLE	= %#.......
rec_IRQCTRL_ON_END	= %.#......
rec_IRQCTRL_ON_ERROR	= %..#.....	; for verify errors
!address {
	; address control (set to zero for normal operation)
	rec_addrctrl	= $df0a
}
rec_ADDRCTRL_FIX_INT	= %#.......
rec_ADDRCTRL_FIX_EXT	= %.#......
