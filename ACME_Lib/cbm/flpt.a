;ACME 0.96.4

!ifdef lib_cbm_flpt_a !eof
lib_cbm_flpt_a = 1

; CAUTION! The Commodore BASIC interpreter uses two different formats for
; handling floating-point values, so do not confuse them:
; The "float registers" fac1 and fac2 (actually structures in zero page) use a
; six-byte format commonly known as "flpt" (floating point).
; When storing values in variables (or reading values from ROM), a compressed
; five-byte format is used, commonly known as "mflpt" (memory floating point).

; This file contains a macro for writing floating point numbers in the six-byte
; "flpt" format, where the sign bit occupies the sixth byte.
; There are no interpreter functions to use this format, so you will have to
; write your own functions for "copy-mem-to-fac1", "copy-fac2-to-mem" etc.

; Use the macro like this:
;		+flpt 3.1415926	; each use will take up six bytes of memory


; now for the technical stuff (stop reading right now if you value your sanity)

; six-byte layout in memory:
; eeeeeeee 1mmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm sxxxxxxx	; eight bits exponent, 32 bits mantissa with leading '1', sign byte

; exponent byte:
;	exponent has a bias of 128 (128 means the decimal point is right before the mantissa's leading digit)
;	if exponent is zero, number value is considered to be zero, regardless of mantissa
;	exponents 1..128 are for values < 1
;	exponents 129..255 are for values >= 1

; mantissa:
;	mantissa is stored big-endian(!)
;	the mantissa's leading digit is always '1' (unless the whole value represents zero)

; sign byte:
;	most significant bit is sign: 0 means positive number, 1 means negative number
;	the seven lower bits are unused

; so logically, this is equivalent to:
; + .1mmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm * 2^(eeeeeeee - 128) if sign bit is 0
; - .1mmmmmmm mmmmmmmm mmmmmmmm mmmmmmmm * 2^(eeeeeeee - 128) if sign bit is 1



; this is ugly, but it gets the job done
; (if it's stupid, but it works, then it's not stupid)
!macro flpt @value {
	!set @float = float(@value)	; make sure to do passes until value is defined
	!ifndef @float {
		!by $ff, $ff, $ff, $ff, $ff, $ff	; six place holder bytes
	} else {
		; value is defined, so split up into sign and non-negative value
		!if @float < 0 {
			!set @sign = $80
			!set @float = -@float
		} else {
			!set @sign = $00
		}
		!if @float = 0 {
			!by 0, 0, 0, 0, 0, 0	; six zeroes (zero is represented by all bits zero)
		} else {
			; split up into exponent and mantissa
			!set @exponent = 128 + 32	; 128 is cbm's bias, 32 is this algo's bias
			; if mantissa is too large, shift right and adjust exponent
			!do while @float >= (2.0 ^ 32.0) {
				!set @float = @float >> 1
				!set @exponent = @exponent + 1
			}
			; if mantissa is too small, shift left and adjust exponent
			!do while @float < (2.0 ^ 31.0) {
				!set @float = @float << 1
				!set @exponent = @exponent - 1
			}
			!if @exponent < 1 {
				!warn "FLPT underflow, using zero instead"
				!set @float = 0
				!set @exponent = 0
				!set @sign = 0
			}
			!if @exponent > 255 {
				!error "FLPT overflow"
			}
			!by @exponent
			!by 255 & int(@float >> 24)
			!by 255 & int(@float >> 16)
			!by 255 & int(@float >> 8)
			!by 255 & int(@float)
			!by @sign
		}
	}
}
