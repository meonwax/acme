;ACME 0.96.4

!ifdef lib_cbm_c64_memcopy_a !eof
lib_cbm_c64_memcopy_a = 1

; this macro inserts code to copy a memory block.
; it calls a function from the basic interpreter, so:
;	- BASIC ROM must be banked in
;	- the source block must be readable (so no RAM hidden under BASIC, Kernal, or I/O)
;	- the target block must be writable (so no RAM hidden under I/O)
; higher addresses are copied first, so:
;	- moving data to higher addresses works even if areas overlap
;	- moving data to lower addresses only works if areas do not overlap
!macro basic_memcopy @src_start, @src_end, @target_start {
	!addr	@z_target_end	= $58
	!addr	@z_src_end	= $5a
	!addr	@z_src_start	= $5f
	!addr	@fn		= $a3bf

		lda #<@src_start
		ldx #>@src_start
		sta @z_src_start
		stx @z_src_start + 1
		lda #<@src_end
		ldx #>@src_end
		sta @z_src_end
		stx @z_src_end + 1
		lda #<(@target_start + @src_end - @src_start)
		ldx #>(@target_start + @src_end - @src_start)
		sta @z_target_end
		stx @z_target_end + 1
		jsr @fn
}
