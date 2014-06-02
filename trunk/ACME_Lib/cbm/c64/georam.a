;ACME 0.95

!ifdef lib_cbm_c64_georam_a !eof
lib_cbm_c64_georam_a = 1

; This file contains definitions for accessing the "GeoRAM" RAM expansion and
; its clones (BBG, BBU, NeoRAM, ...)
; These units allow access to a single page of memory (256 bytes) visible at
; address $de00 in i/o space. Writing to registers allows to change which
; memory page is visible at that location.

!address {
	; memory page
	georam_page	= $de00
	; control registers (write-only, these registers can not be read)
	georam_track	= $dffe	; 0..63, i.e. the lower six bits are significant
	georam_sector	= $dfff	; 0..31, i.e. the lower five bits are significant
}
; these are the official addresses - actually the registers are accessible
; *everywhere* at $dfxx, but using these locations does not clash with the
; registers of a Commodore REU.

; Upgraded units and clones may have more memory, in those cases the registers
; will have more significant bits.

; I could have called the registers "row" and "column" instead of track and
; sector, but the fact that this device was designed with one six-bit register
; and one five-bit register (instead of one eight-bit register and one
; three-bit register) tells me that this was meant as an easily programmable
; RAM disk: A 1541 disk has 35 tracks with (at most) 21 sectors. Numbers in
; these ranges can be written to the GeoRAM registers without the need to
; shift bits around.
; Knowing this is a handy way of remembering the number of significant bits of
; GeoRAM registers.
