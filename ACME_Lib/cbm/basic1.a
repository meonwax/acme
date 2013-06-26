;ACME 0.94.4

!ifdef lib_cbm_basic1_a !eof
lib_cbm_basic1_a = 1

; token values
token_END	= $80
token_FOR	= $81
token_NEXT	= $82
token_DATA	= $83
token_INPUT_	= $84	; INPUT#
token_INPUT	= $85
token_DIM	= $86
token_READ	= $87
token_LET	= $88
token_GOTO	= $89
token_RUN	= $8a
token_IF	= $8b
token_RESTORE	= $8c
token_GOSUB	= $8d
token_RETURN	= $8e
token_REM	= $8f
token_STOP	= $90
token_ON	= $91
token_WAIT	= $92
token_LOAD	= $93
token_SAVE	= $94
token_VERIFY	= $95
token_DEF	= $96
token_POKE	= $97
token_PRINT_	= $98	; PRINT#
token_PRINT	= $99
token_CONT	= $9a
token_LIST	= $9b
token_CLR	= $9c
token_CMD	= $9d
token_SYS	= $9e
token_OPEN	= $9f
token_CLOSE	= $a0
token_GET	= $a1
token_NEW	= $a2
token_TAB	= $a3	; the token already includes '('
token_TO	= $a4
token_FN	= $a5
token_SPC	= $a6	; the token already includes '('
token_THEN	= $a7
token_NOT	= $a8
token_STEP	= $a9
token_ADD	= $aa	; '+'
token_SUBTRACT	= $ab	; '-'
token_MULTIPLY	= $ac	; '*'
token_DIVIDE	= $ad	; '/'
token_POWEROF	= $ae	; '^'
token_AND	= $af
token_OR	= $b0
token_GREATER	= $b1	; '>'
token_EQUAL	= $b2	; '='
token_LESS	= $b3	; '<'
token_SGN	= $b4
token_INT	= $b5
token_ABS	= $b6
token_USR	= $b7
token_FRE	= $b8
token_POS	= $b9
token_SQR	= $ba
token_RND	= $bb
token_LOG	= $bc
token_EXP	= $bd
token_COS	= $be
token_SIN	= $bf
token_TAN	= $c0
token_ATN	= $c1
token_PEEK	= $c2
token_LEN	= $c3
token_STR_	= $c4	; STR$
token_VAL	= $c5
token_ASC	= $c6
token_CHR_	= $c7	; CHR$
token_LEFT_	= $c8	; LEFT$
token_RIGHT_	= $c9	; RIGHT$
token_MID_	= $ca	; MID$

token_PI	= $ff	; greek letter pi


; Macros for inserting BASIC commands. Note that "#" and "$" characters in
; BASIC keywords have been converted to "_" in the macro names.
; *All* function macros already include the '(' character.

!macro b_END		{!by token_END}
!macro b_FOR		{!by token_FOR}
!macro b_NEXT		{!by token_NEXT}
!macro b_DATA		{!by token_DATA}
!macro b_INPUT_		{!by token_INPUT_}	; INPUT#
!macro b_INPUT		{!by token_INPUT}
!macro b_DIM		{!by token_DIM}
!macro b_READ		{!by token_READ}
!macro b_LET		{!by token_LET}
!macro b_GOTO		{!by token_GOTO}
!macro b_RUN		{!by token_RUN}
!macro b_IF		{!by token_IF}
!macro b_RESTORE	{!by token_RESTORE}
!macro b_GOSUB		{!by token_GOSUB}
!macro b_RETURN		{!by token_RETURN}
!macro b_REM		{!by token_REM}
!macro b_STOP		{!by token_STOP}
!macro b_ON		{!by token_ON}
!macro b_WAIT		{!by token_WAIT}
!macro b_LOAD		{!by token_LOAD}
!macro b_SAVE		{!by token_SAVE}
!macro b_VERIFY		{!by token_VERIFY}
; As "DEF" cannot be used without "FN", here is a macro called "b_DEFFN"
; instead of one called "b_DEF":
!macro b_DEFFN		{!by token_DEF, token_FN}	; DEFFN
!macro b_POKE		{!by token_POKE}
!macro b_PRINT_		{!by token_PRINT_}	; PRINT#
!macro b_PRINT		{!by token_PRINT}
!macro b_CONT		{!by token_CONT}
!macro b_LIST		{!by token_LIST}
!macro b_CLR		{!by token_CLR}
!macro b_CMD		{!by token_CMD}
!macro b_SYS		{!by token_SYS}
!macro b_OPEN		{!by token_OPEN}
!macro b_CLOSE		{!by token_CLOSE}
!macro b_GET		{!by token_GET}
!macro b_NEW		{!by token_NEW}
!macro b_TAB		{!by token_TAB}		; TAB(		the token already includes '('
!macro b_TO		{!by token_TO}
!macro b_FN		{!by token_FN}
!macro b_SPC		{!by token_SPC}		; SPC(		the token already includes '('
!macro b_THEN		{!by token_THEN}
!macro b_NOT		{!by token_NOT}
!macro b_STEP		{!by token_STEP}
!macro b_ADD		{!by token_ADD}		; +
!macro b_SUBTRACT	{!by token_SUBTRACT}	; -
!macro b_MULTIPLY	{!by token_MULTIPLY}	; *
!macro b_DIVIDE		{!by token_DIVIDE}	; /
!macro b_POWEROF	{!by token_POWEROF}	; ^
!macro b_AND		{!by token_AND}
!macro b_OR		{!by token_OR}
!macro b_GREATER	{!by token_GREATER}	; >
!macro b_EQUAL		{!by token_EQUAL}	; =
!macro b_LESS		{!by token_LESS}	; <
!macro b_SGN		{!by token_SGN, $28}	; SGN(
!macro b_INT		{!by token_INT, $28}	; INT(
!macro b_ABS		{!by token_ABS, $28}	; ABS(
!macro b_USR		{!by token_USR, $28}	; USR(
!macro b_FRE		{!by token_FRE, $28}	; FRE(
!macro b_POS		{!by token_POS, $28}	; POS(
!macro b_SQR		{!by token_SQR, $28}	; SQR(
!macro b_RND		{!by token_RND, $28}	; RND(
!macro b_LOG		{!by token_LOG, $28}	; LOG(
!macro b_EXP		{!by token_EXP, $28}	; EXP(
!macro b_COS		{!by token_COS, $28}	; COS(
!macro b_SIN		{!by token_SIN, $28}	; SIN(
!macro b_TAN		{!by token_TAN, $28}	; TAN(
!macro b_ATN		{!by token_ATN, $28}	; ATN(
!macro b_PEEK		{!by token_PEEK, $28}	; PEEK(
!macro b_LEN		{!by token_LEN, $28}	; LEN(
!macro b_STR_		{!by token_STR_, $28}	; STR$(
!macro b_VAL		{!by token_VAL, $28}	; VAL(
!macro b_ASC		{!by token_ASC, $28}	; ASC(
!macro b_CHR_		{!by token_CHR_, $28}	; CHR$(
!macro b_LEFT_		{!by token_LEFT_, $28}	; LEFT$(
!macro b_RIGHT_		{!by token_RIGHT_, $28}	; RIGHT$(
!macro b_MID_		{!by token_MID_, $28}	; MID$(

!macro b_PI		{!by token_PI}	; greek letter pi
