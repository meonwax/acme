;ACME 0.96.4

!ifdef lib_65816_std_a !eof
lib_65816_std_a = 2

; Labels and macros for Western Digital's 65c816 processor

!address {
	cpu_e_cop   = $fff4
	cpu_e_abort = $fff8
	cpu_e_nmi   = $fffa
	cpu_e_reset = $fffc
	cpu_e_irq   = $fffe

	cpu_n_cop   = $ffe4
	cpu_n_brk   = $ffe6
	cpu_n_abort = $ffe8
	cpu_n_nmi   = $ffea
	; no reset vector for native mode because reset always enters emulation mode
	cpu_n_irq   = $ffee
}

!macro cpu_emu {; switch to emulation mode
	sec
	xce
}

!macro cpu_native {; switch to native mode
	clc
	xce
}

!macro a8 {; switch A to 8 bit
	sep #%..#.....
	!as
}

!macro a16 {; switch A to 16 bit
	rep #%..#.....
	!al
}

!macro i8 {; switch X/Y to 8 bit
	sep #%...#....
	!rs
}

!macro i16 {; switch X/Y to 16 bit
	rep #%...#....
	!rl
}

!macro ai8 {; switch A/X/Y to 8 bit
	sep #%..##....
	!as
	!rs
}

!macro ai16 {; switch A/X/Y to 16 bit
	rep #%..##....
	!al
	!rl
}

!macro a8i16 {; switch A to 8, X/Y to 16 bit
	+a8
	+i16
}

!macro a16i8 {; switch A to 16, X/Y to 8 bit
	+a16
	+i8
}

!macro inc24 @t {; increase 24-bit counter
	inc @t
	bne +
	inc @t + 1
	bne +
	inc @t + 2
+
}
