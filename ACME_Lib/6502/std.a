;ACME 0.96.4

!ifdef lib_6502_std_a !eof
lib_6502_std_a = 1

; labels and macros for plain 6502 processor

!address {
	cpu_nmi		= $fffa
	cpu_reset	= $fffc
	cpu_irq		= $fffe
}

; skip byte
!macro bit8 {
		!byte $24	; opcode of BIT $.. command
}

; skip word
!macro bit16 {
		!byte $2c	; opcode of BIT $.... command
}

; increase 16-bit counter
!macro inc16 @t {
		inc @t
		bne +
		inc @t + 1
+
}

; far branches
!macro bcc @t {
		bcs +
		jmp @t
+
}
!macro bcs @t {
		bcc +
		jmp @t
+
}

!macro beq @t {
		bne +
		jmp @t
+
}

!macro bne @t {
		beq +
		jmp @t
+
}

!macro bmi @t {
		bpl +
		jmp @t
+
}

!macro bpl @t {
		bmi +
		jmp @t
+
}

!macro bvc @t {
		bvs +
		jmp @t
+
}

!macro bvs @t {
		bvc +
		jmp @t
+
}
