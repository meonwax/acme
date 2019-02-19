;ACME 0.96.4

!ifdef lib_cbm_c64_float_a !eof
lib_cbm_c64_float_a = 2

; Here are some definitions to help you call the floating-point functions of the
; C64's BASIC ROM. They work on "float registers", which are actually just
; structures in zero page:
!address {
	fac1_base	= $61	; base address of floating-point accumulator 1
	fac2_base	= $69	; base address of floating-point accumulator 2
}
; There is really no need to use these addresses directly when calling the ROM
; functions. You'd only need the addresses when using <cbm/flpt.a>.

!source <cbm/flpt.a>	; include macro to store floats in six-byte FLPT format
!source <cbm/mflpt.a>	; include macro to store floats in five-byte MFLPT format

; convenience macros:

; some float functions need a memory address in YYAA
!macro movYYAA @addr {
	lda #<@addr
	ldy #>@addr
}
; ...or in YYXX
!macro movYYXX @addr {
	ldx #<@addr
	ldy #>@addr
}
; other float functions expect or output a value in AAYY
!macro movAAYY @val {
	ldy #<@val
	lda #>@val
}
!macro ldAAYY @addr {
	ldy @addr
	lda @addr + 1
}
!macro stAAYY @addr {
	sty @addr
	sta @addr + 1
}
; ...or in AAXX
!macro ldAAXX @addr {
	ldx @addr
	lda @addr + 1
}

!address {
	; constants in five-byte "mflpt" format
	mflpt_pi		= $aea8	; 3.1415926...
	mflpt_minus32768	= $b1a5	; -32768
	mflpt_1			= $b9bc	; 1
	mflpt_half_sqr2		= $b9d6	; SQR(2) / 2
	mflpt_sqr2		= $b9db ; SQR(2)
	mflpt_minus_point5	= $b9e0	; -.5
	mflpt_log_2		= $b9e5	; LOG(2)
	mflpt_10		= $baf9	; 10
	mflpt_99999999		= $bdb3 ; 99 999 999
	mflpt_999999999		= $bdb8	; 999 999 999
	mflpt_1000000000	= $bdbd	; 1 000 000 000
	mflpt_point5		= $bf11	; .5, also known as 1 / 2
	mflpt_log_2_reciprocal	= $bfbf	; 1 / LOG(2)
	mflpt_half_pi		= $e2e0	; PI / 2
	mflpt_double_pi		= $e2e5	; 2 * PI (also see $e309)
	mflpt_point25		= $e2ea	; .25, also known as 1 / 4
	mflpt_2_pi		= $e309	; 2 * PI (also see $e2e5)

	; functions - a few points to note:
	; fac1/2 might get clobbered even if not mentioned in the function's name,
	;	because stuff like fac1_times_memYYAA will load the value from memory
	;	into fac2 first.
	; for subtraction and division, the left operand is in fac2, the right operand in fac1.
	fac1_print			= $aabc	; print string representation of contents of fac1
	fac1_to_signedAAYY		= $b1aa	; might throw ILLEGAL QUANTITY
	fac1_to_signed16		= $b1bf	; might throw ILLEGAL QUANTITY
	fac1_read_signedAAYY		= $b391	; convert 16 bit signed int to float
	fac1_read_unsignedY		= $b3a2	; convert 8 bit unsigned int to float
	fac1_read_string		= $b7b5	; $22/23 must point to string, A must be string length
	fac1_to_unsignedAAYY		= $b7f7	; might throw ILLEGAL QUANTITY (result is also in $14/15)
	fac1_add_point5			= $b849	; for rounding, call this before fac1_int
	fac1_memYYAA_minus_fac1		= $b850	; subtract fac1 from mflpt value
	fac1_fac2_minus_fac1		= $b853
	fac1_add_memYYAA		= $b867	; add mflpt value
	fac1_add_fac2			= $b86a
	fac1_log			= $b9ea	; LOG()
	fac1_times_memYYAA		= $ba28	; multiply by mflpt value
	fac2_read_memYYAA		= $ba8c	; load mflpt value from memory into fac2
	fac2_read_mem_via0x22ptr	= $ba90	; load mflpt value from memory into fac2
	fac1_times_10			= $bae2
	fac1_divide_by_10		= $bafe	; CAUTION: result is always positive!
	fac1_divide_memYYAA_by_fac1	= $bb0f	; divide mflpt value by fac1 value
	fac1_read_memYYAA		= $bba2	; load mflpt value from memory into fac1
	fac1_read_mem_via0x22ptr	= $bba6	; load mflpt value from memory into fac1
	fac1_to_memYYXX			= $bbd4	; store fac1 to memory as mflpt
	fac1_read_fac2			= $bbfc	; copy fac2 to fac1
	fac2_read_fac1			= $bc0c	; copy fac1 to fac2
	fac1_sign_to_A			= $bc2b	; $ff, $0, $1 for negative, zero, positive
	fac1_sgn			= $bc39 ; SGN()
	fac1_abs			= $bc58 ; ABS()
	fac1_compare_to_memYYAA		= $bc5b	; compare to mflpt value in memory
	fac1_to_signed32		= $bc9b
	fac1_int			= $bccc ; INT()
	fac1_read_string0		= $bcf3	; use b7b5 instead; this only works after calling CHRGET
	fac1_print_unsignedAAXX		= $bdcd
	fac1_to_string			= $bddd	; string is stored at $0100 (address returned in YYAA)
	fac1_sqr			= $bf71	; SQR()
	fac1_fac2_to_the_power_of_memYYAA	= $bf78
	fac1_negate			= $bfb4
	fac1_exp			= $bfed	; EXP()
	; end of basic rom jumps to start of kernal rom!
	fac1_rnd			= $e097	; RND()
	fac1_cos			= $e264 ; COS()
	fac1_sin			= $e26b	; SIN()
	fac1_tan			= $e2b4	; TAN()
	fac1_atn			= $e30e	; ATN()
}

; wrappers for names from older version of this file:
!macro movAY @addr {	+movYYAA @addr	}
!macro movXY @addr {	+movYYXX @addr	}
!macro movYA @val {	+movAAYY @val	}
!macro ldYA @addr {	+ldAAYY @addr	}
!macro stYA @addr {	+stAAYY @addr	}
!macro ldXA @addr {	+ldAAXX @addr	}
	fac1_to_signedYA		= fac1_to_signedAAYY
	fac1_read_signedYA		= fac1_read_signedAAYY
	fac1_to_unsignedYA		= fac1_to_unsignedAAYY
	fac1_memAY_minus_fac1		= fac1_memYYAA_minus_fac1
	fac1_add_memAY			= fac1_add_memYYAA
	fac1_times_memAY		= fac1_times_memYYAA
	fac2_read_memAY			= fac2_read_memYYAA
	fac1_divide_memAY_by_fac1	= fac1_divide_memYYAA_by_fac1
	fac1_read_memAY			= fac1_read_memYYAA
	fac1_to_memXY			= fac1_to_memYYXX
	fac1_compare_to_memAY		= fac1_compare_to_memYYAA
	fac1_print_unsignedXA		= fac1_print_unsignedAAXX
	fac1_fac2_to_the_power_of_memAY	= fac1_fac2_to_the_power_of_memYYAA
