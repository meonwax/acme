;ACME 0.94.4

!ifdef lib_cbm_basic3_5_a !eof
lib_cbm_basic3_5_a = 1

!source <cbm/basic2.a>	; from 0x80 to $cb

; token values
token_RGR	= $cc
token_RCLR	= $cd
; if this file gets included via "cbm/basic7.a" or "cbm/basic10.a", do not define RLUM
; (because v7 and higher use $ce as prefix byte for additional functions):
!ifndef lib_cbm_basic7_a {
	!ifndef lib_cbm_basic10_a {
		token_RLUM	= $ce
	}
}
token_JOY	= $cf
token_RDOT	= $d0
token_DEC	= $d1
token_HEX_	= $d2	; HEX$
token_ERR_	= $d3	; ERR$
token_INSTR	= $d4
token_ELSE	= $d5
token_RESUME	= $d6
token_TRAP	= $d7
token_TRON	= $d8
token_TROFF	= $d9
token_SOUND	= $da
token_VOL	= $db
token_AUTO	= $dc
token_PUDEF	= $dd
token_GRAPHIC	= $de
token_PAINT	= $df
token_CHAR	= $e0
token_BOX	= $e1
token_CIRCLE	= $e2
; if this file gets included via "cbm/basic10.a", do not define GSHAPE/SSHAPE/DRAW
; (because in v10, they are called PASTE/CUT/LINE):
!ifndef lib_cbm_basic10_a {
	token_GSHAPE	= $e3
	token_SSHAPE	= $e4
	token_DRAW	= $e5
}
token_LOCATE	= $e6
token_COLOR	= $e7
token_SCNCLR	= $e8
token_SCALE	= $e9
token_HELP	= $ea
token_DO	= $eb
token_LOOP	= $ec
token_EXIT	= $ed
; if this file gets included via "cbm/basic10.a", do not define DIRECTORY
; (because in v10, it is called DIR):
!ifndef lib_cbm_basic10_a {
	token_DIRECTORY	= $ee
}
token_DSAVE	= $ef
token_DLOAD	= $f0
token_HEADER	= $f1
token_SCRATCH	= $f2
token_COLLECT	= $f3
token_COPY	= $f4
token_RENAME	= $f5
token_BACKUP	= $f6
token_DELETE	= $f7
token_RENUMBER	= $f8
token_KEY	= $f9
token_MONITOR	= $fa
token_USING	= $fb
token_UNTIL	= $fc
token_WHILE	= $fd

; Macros for inserting BASIC commands. Note that "#" and "$" characters in
; BASIC keywords have been converted to "_" in the macro names.
; *All* function macros already include the '(' character.
!macro b_RGR		{!by token_RGR, $28}	; RGR(
!macro b_RCLR		{!by token_RCLR, $28}	; RCLR(
!macro b_RLUM		{!by token_RLUM, $28}	; RLUM(
!macro b_JOY		{!by token_JOY, $28}	; JOY(
!macro b_RDOT		{!by token_RDOT, $28}	; RDOT(
!macro b_DEC		{!by token_DEC, $28}	; DEC(
!macro b_HEX_		{!by token_HEX_, $28}	; HEX$(
!macro b_ERR_		{!by token_ERR_, $28}	; ERR$(
!macro b_INSTR		{!by token_INSTR, $28}	; INSTR(
!macro b_ELSE		{!by token_ELSE}
!macro b_RESUME		{!by token_RESUME}
!macro b_TRAP		{!by token_TRAP}
!macro b_TRON		{!by token_TRON}
!macro b_TROFF		{!by token_TROFF}
!macro b_SOUND		{!by token_SOUND}
!macro b_VOL		{!by token_VOL}
!macro b_AUTO		{!by token_AUTO}
!macro b_PUDEF		{!by token_PUDEF}
!macro b_GRAPHIC	{!by token_GRAPHIC}
!macro b_PAINT		{!by token_PAINT}
!macro b_CHAR		{!by token_CHAR}
!macro b_BOX		{!by token_BOX}
!macro b_CIRCLE		{!by token_CIRCLE}
!macro b_GSHAPE		{!by token_GSHAPE}
!macro b_SSHAPE		{!by token_SSHAPE}
!macro b_DRAW		{!by token_DRAW}
!macro b_LOCATE		{!by token_LOCATE}
!macro b_COLOR		{!by token_COLOR}
!macro b_SCNCLR		{!by token_SCNCLR}
!macro b_SCALE		{!by token_SCALE}
!macro b_HELP		{!by token_HELP}
!macro b_DO		{!by token_DO}
!macro b_LOOP		{!by token_LOOP}
!macro b_EXIT		{!by token_EXIT}
!macro b_DIRECTORY	{!by token_DIRECTORY}
!macro b_DSAVE		{!by token_DSAVE}
!macro b_DLOAD		{!by token_DLOAD}
!macro b_HEADER		{!by token_HEADER}
!macro b_SCRATCH	{!by token_SCRATCH}
!macro b_COLLECT	{!by token_COLLECT}
!macro b_COPY		{!by token_COPY}
!macro b_RENAME		{!by token_RENAME}
!macro b_BACKUP		{!by token_BACKUP}
!macro b_DELETE		{!by token_DELETE}
!macro b_RENUMBER	{!by token_RENUMBER}
!macro b_KEY		{!by token_KEY}
!macro b_MONITOR	{!by token_MONITOR}
!macro b_USING		{!by token_USING}
!macro b_UNTIL		{!by token_UNTIL}
!macro b_WHILE		{!by token_WHILE}
