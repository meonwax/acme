;ACME 0.94.4

!ifdef lib_cbm_basic7_a !eof
lib_cbm_basic7_a = 1

; Macros for inserting BASIC commands. Note that "#" and "$" characters in
; BASIC keywords have been converted to "_" in the macro names.
; *All* function macros already include the '(' character.

!source <cbm/basic3.5.a>	; from 0x80 to $fd

; extended (16-bit) tokens, $ce range (replacing RLUM function):

; extended token $ce $00 isn't used
; extended token $ce $01 isn't used
!macro b_POT		{!by $ce, $02:!pet '('}
!macro b_BUMP		{!by $ce, $03:!pet '('}
; if this file gets included via "cbm/basic10.a", do not define PEN
; (because in v10, there is an instruction called PEN):
!ifndef lib_cbm_basic10_a {
	!macro b_PEN		{!by $ce, $04:!pet '('}
}
!macro b_RSPPOS		{!by $ce, $05:!pet '('}
!macro b_RSPRITE	{!by $ce, $06:!pet '('}
!macro b_RSPCOLOR	{!by $ce, $07:!pet '('}
!macro b_XOR		{!by $ce, $08:!pet '('}
!macro b_RWINDOW	{!by $ce, $09:!pet '('}
!macro b_POINTER	{!by $ce, $0a:!pet '('}


; extended (16-bit) tokens, $fe range:

; extended token $fe $00 isn't used
; extended token $fe $01 isn't used
!macro b_BANK		{!by $fe, $02}
!macro b_FILTER		{!by $fe, $03}
!macro b_PLAY		{!by $fe, $04}
!macro b_TEMPO		{!by $fe, $05}
!macro b_MOVSPR		{!by $fe, $06}
!macro b_SPRITE		{!by $fe, $07}
!macro b_SPRCOLOR	{!by $fe, $08}
!macro b_RREG		{!by $fe, $09}
!macro b_ENVELOPE	{!by $fe, $0a}
!macro b_SLEEP		{!by $fe, $0b}
!macro b_CATALOG	{!by $fe, $0c}
!macro b_DOPEN		{!by $fe, $0d}
!macro b_APPEND		{!by $fe, $0e}
!macro b_DCLOSE		{!by $fe, $0f}
!macro b_BSAVE		{!by $fe, $10}
!macro b_BLOAD		{!by $fe, $11}
!macro b_RECORD		{!by $fe, $12}
!macro b_CONCAT		{!by $fe, $13}
!macro b_DVERIFY	{!by $fe, $14}
!macro b_DCLEAR		{!by $fe, $15}
!macro b_SPRSAV		{!by $fe, $16}
!macro b_COLLISION	{!by $fe, $17}
!macro b_BEGIN		{!by $fe, $18}
!macro b_BEND		{!by $fe, $19}
!macro b_WINDOW		{!by $fe, $1a}
!macro b_BOOT		{!by $fe, $1b}
!macro b_WIDTH		{!by $fe, $1c}
!macro b_SPRDEF		{!by $fe, $1d}
!macro b_QUIT		{!by $fe, $1e}	; "unimplemented command"
; if this file gets included via "cbm/basic10.a", do not define STASH/FETCH/SWAP
; (because in v10, they all get decoded to DMA):
!ifndef lib_cbm_basic10_a {
	!macro b_STASH		{!by $fe, $1f}
	; extended token $fe $20 isn't used ($20 is ' ')
	!macro b_FETCH		{!by $fe, $21}
	; extended token $fe $22 isn't used ($22 is '"')
	!macro b_SWAP		{!by $fe, $23}
}
!macro b_OFF		{!by $fe, $24}	; "unimplemented command"
!macro b_FAST		{!by $fe, $25}
!macro b_SLOW		{!by $fe, $26}
