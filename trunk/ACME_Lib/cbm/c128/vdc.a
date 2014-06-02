;ACME 0.95

!ifdef lib_cbm_c128_vdc_a !eof
lib_cbm_c128_vdc_a = 1

; there are three different versions of the C128's Video Display Controller:
;	v0: VDC 8563 R7A	in C128 and C128-D
;	v1: VDC 8563 R8/R9	in C128 and C128-D
;	v2: VDC 8568		in C128-DCR
; 8563 and 8568 have different pinouts, so do not try to use one of them on a
; board intended for the other!


; hardware differences:

; the 8563 uses external circuitry to convert RGBIHV to monochrome/luminance.
; the 8568 does a part of this internally and therefore needs fewer external
; components.

; the 8568 has an additional register (r37) to change sync polarities.


; software differences:

; the horizontal scroll bits of r25 differ between v0 and v1/v2: in v0, set these
; bits to zero. in v1/v2, set these bits to the same value as bits4-7 of r22.
; the additional register r37 can be ignored (as the C128 kernal does).


; access macros:

!macro vdc_lda {
-			bit vdc_state
			bpl -
		lda vdc_data
}

!macro vdc_sta {
-			bit vdc_state
			bpl -
		sta vdc_data
}

!macro vdc_ldx {
-			bit vdc_state
			bpl -
		ldx vdc_data
}

!macro vdc_stx {
-			bit vdc_state
			bpl -
		stx vdc_data
}

!macro vdc_ldy {
-			bit vdc_state
			bpl -
		ldy vdc_data
}

!macro vdc_sty {
-			bit vdc_state
			bpl -
		sty vdc_data
}


; color codes:

; These are the colors officially available on the C128 - the same names as if
; using a C64's VIC, but different codes of course.

; color name	   RGBI	  VIC equivalent
vdccolor_BLACK	= %....	; 0
vdccolor_WHITE	= %####	; 15
vdccolor_RED	= %#...	; 8
vdccolor_CYAN	= %.###	; 7
vdccolor_PURPLE	= %#.##	; 11
vdccolor_GREEN	= %.#..	; 4
vdccolor_BLUE	= %..#.	; 2
vdccolor_YELLOW	= %##.#	; 13
vdccolor_ORANGE	= %#.#.	; 10 (on VDC, this is in fact a dark shade of purple)
vdccolor_BROWN	= %##..	; 12
vdccolor_LRED	= %#..#	; 9
vdccolor_GRAY1	= %.##.	; 6 (on VDC, this is in fact a dark shade of cyan)
vdccolor_GRAY2	= %...#	; 1 (this is almost, but not quite, entirely black)
vdccolor_LGREEN	= %.#.#	; 5
vdccolor_LBLUE	= %..##	; 3
vdccolor_GRAY3	= %###.	; 14

; The following alternative names are much easier to remember when you're used
; to writing programs for the VDC: There are eight main colors, and each one is
; available in a light and a dark shade - even black and white!

; primary colors   RGBI code
vdccolor_DRED	= %#...
vdccolor_DGREEN	= %.#..
vdccolor_DBLUE	= %..#.
vdccolor_LRED	= %#..#
vdccolor_LGREEN	= %.#.#
vdccolor_LBLUE	= %..##

; secondary colors	   RGBI code
vdccolor_LCYAN		= %.###
vdccolor_LMAGENTA	= %#.##
vdccolor_LYELLOW	= %##.#
vdccolor_DCYAN		= %.##.
vdccolor_DMAGENTA	= %#.#.
vdccolor_DYELLOW	= %##..

; black & white    RGBI code
vdccolor_DBLACK	= %....
vdccolor_LBLACK	= %...#
vdccolor_LWHITE	= %####
vdccolor_DWHITE	= %###.

; if you don't like the concept of shaded black/white, then use these:

; gray level       RGBI code
vdccolor_BLACK	= %....	; "dark black"  => "black"
vdccolor_DGRAY	= %...#	; "light black" => "dark grey"
vdccolor_LGRAY	= %###.	; "dark white"  => "light grey"
vdccolor_WHITE	= %####	; "light white" => "white"


; attribute flags (2rufRGBI)
vdcattr_2ND	= %#.......	; second character set
vdcattr_REVS	= %.#......	; reverse mode
vdcattr_UL	= %..#.....	; underline
vdcattr_FLASH	= %...#....	; flash
vdcattr_R	= %....#...	; red
vdcattr_G	= %.....#..	; green
vdcattr_B	= %......#.	; blue
vdcattr_I	= %.......#	; intensity


!address {
	; direct registers
	vdc_state	= $d600	; READING this location yields status flags, see below
	vdc_reg		= $d600	; WRITING this location selects an indirect register
	vdc_data	= $d601	; data of selected indirect register
}
; status flags in vdc_state:
vdcstate_READY		= %#.......	; RAM access is finished
vdcstate_LIGHTPEN	= %.#......	; light pen has been activated
vdcstate_IN_BORDER	= %..#.....	; electron beam is in upper or lower border
vdcstate_VERSIONMASK	= %...#####	; vdc version (0, 1 or 2)


; indirect registers		(default value, see $e179 in C128 kernal)
vdcr_htotal		= $00	; 126 (127 for PAL, depends on kernal version)	characters per line, minus one
vdcr_columns		= $01	; 80	characters per line, actually displayed
 vdcr_hdisp		= $01
vdcr_hsync_pos		= $02	; 102	character position to send horizontal sync in
vdcr_syncwidth		= $03	; $49	4b vertical, 4b horizontal
vdcr_vtotal		= $04	; 32 (39 or 38 for PAL, depends on kernal version)	character lines per screen, minus one
vdcr_vadjust		= $05	; 0	additional scan lines per screen (to fix timings)
vdcr_lines		= $06	; 25	character lines per screen, actually displayed
 vdcr_vdisp		= $06
vdcr_vsync_pos		= $07	; 29 (32 for PAL)	character line to send vertical sync in
vdcr_interlace		= $08	; 0	interlace mode (0=2=std, 1=jitter, 3=interlace)
vdcr_charheight_total	= $09	; 7	5b total, minus one
vdcr_crsr_start		= $0a	; $20	2b mode, 5b scanline
	vdcr_CRSRSTART_MODE_MASK	= %.##.....
;	vdcr_CRSRSTART_MODE_FIXED	= %........	; fixed cursor
	vdcr_CRSRSTART_MODE_OFF		= %..#.....	; invisible
	vdcr_CRSRSTART_MODE_FAST	= %.#......	; flashing with 1/16 of refresh freq
	vdcr_CRSRSTART_MODE_SLOW	= %.##.....	; flashing with 1/32 of refresh freq
vdcr_crsr_end		= $0b	; 7		5b scanline
vdcr_display_hi		= $0c	; 0	RAM address of display buffer
vdcr_display_lo		= $0d	; 0
vdcr_crsr_hi		= $0e	; 0	RAM address of cursor
vdcr_crsr_lo		= $0f	; 0
vdcr_lp_y		= $10	; --	y position of light pen (lines, plus 1)
vdcr_lp_x		= $11	; --	x position if light pen (characters, plus 8)
vdcr_ram_hi		= $12	; --	RAM address of register $1f
vdcr_ram_lo		= $13	; --
vdcr_attr_hi		= $14	; $08	RAM address of attribute buffer
vdcr_attr_lo		= $15	; $00
vdcr_charwidth		= $16	; $78	4b total minus one, 4b displayed minus one
vdcr_charheight_disp	= $17	; 8	5b displayed, minus one
vdcr_control_v		= $18	; $20	vertical scroll and much other stuff
	vdcr_CONTROLV_BLOCKMODE_MASK	= %#.......
;	vdcr_CONTROLV_BLOCKMODE_WRITE	= %........
	vdcr_CONTROLV_BLOCKMODE_COPY	= %#.......
	vdcr_CONTROLV_REVERSESCREEN	= %.#......
	vdcr_CONTROLV_FLASHFREQ_MASK	= %..#.....
;	vdcr_CONTROLV_FLASHFREQ_FAST	= %........
	vdcr_CONTROLV_FLASHFREQ_SLOW	= %..#.....
	vdcr_CONTROLV_UPSCROLL_MASK	= %...#####
vdcr_control_h		= $19	; $47 ($40 for vdc version 0)	horizontal scroll and much other stuff
	vdcr_CONTROLH_MODE_MASK		= %#.......
;	vdcr_CONTROLH_MODE_TEXT		= %........
	vdcr_CONTROLH_MODE_BITMAP	= %#.......
	vdcr_CONTROLH_ATTRIBUTES_MASK	= %.#......
;	vdcr_CONTROLH_ATTRIBUTES_OFF	= %........
	vdcr_CONTROLH_ATTRIBUTES_ON	= %.#......
	vdcr_CONTROLH_SEMIGRAPHICS	= %..#.....
	vdcr_CONTROLH_PIXELWIDTH_MASK	= %...#....
;	vdcr_CONTROLH_PIXELWIDTH_NARROW	= %........
	vdcr_CONTROLH_PIXELWIDTH_WIDE	= %...#....
	vdcr_CONTROLH_LEFTSCROLL_MASK	= %....####
vdcr_color		= $1a	; $f0	4b foreground, 4b background
	vdcr_COLOR_FOREGROUND_MASK	= %####....	; only used if attributes are off
	vdcr_COLOR_BACKGROUND_MASK	= %....####
vdcr_row_inc		= $1b	; 0	address increment per row
vdcr_charset		= $1c	; $20	3b charset pointer, 1b RAM type, 4b unused
	vdcr_CHARSET_ADRESS_MASK	= %###.....
	vdcr_CHARSET_RAMTYPE_MASK	= %...#....
;	vdcr_CHARSET_RAMTYPE_16K	= %........
	vdcr_CHARSET_RAMTYPE_64K	= %...#....
vdcr_underline		= $1d	; 7	5b scanline
vdcr_cycles		= $1e	; --	number of write- or copy-cycles. 0 means 256.
vdcr_access		= $1f	; --	RAM content of address r$12/13
vdcr_source_hi		= $20	; --	RAM address of cycle start
vdcr_source_lo		= $21	; --
vdcr_enable_start	= $22	; 125	column to enable display in
vdcr_enable_end		= $23	; 100	column to disable display in
vdcr_dram_refresh	= $24	; 5	RAM refresh rate (lower 4 bits)
vdcr_sync_polarity	= $25	; --	only in VDC 8568
	vdcr_HSYNC_POLARITY	= %#.......
	vdcr_VSYNC_POLARITY	= %.#......

!address {
	; the c128 ROMs contain look-up tables to convert vic color values to vdc color
	; values and vice-versa:
	; rom4_* needs "low rom area" enabled ($4000..$7fff)
	; rom8_* needs "middle rom area" enabled ($8000..$bfff)
	; romc_* needs "high rom area" enabled ($c000..$ffff)
	rom4_vic_to_vdc_color_table	= $6a4c ; 00 0f 08 07 0b 04 02 0d 0a 0c 09 06 01 05 03 0e
	rom8_vdc_to_vic_color_table	= $81f3 ; 00 0c 06 0e 05 0d 0b 03 02 0a 08 04 09 07 0f 01
	romc_vic_to_vdc_color_table	= $ce5c ; 00 0f 08 07 0b 04 02 0d 0a 0c 09 06 01 05 03 0e
}
