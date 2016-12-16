;ACME 0.94.4

!ifdef lib_cbm_basic10_a !eof
lib_cbm_basic10_a = 1

!source <cbm/basic7.a>	; from 0x80 to 0xff, 0xce 0x02 to 0xce 0x0a, 0xfe 0x02 to 0xfe 0x26

; token values
token_PASTE	= $e3	; was called GSHAPE in basic v7
token_CUT	= $e4	; was called SSHAPE in basic v7
token_LINE	= $e5	; was called DRAW in basic v7
token_DIR	= $ee	; was called DIRECTORY in basic v7

; Macros for inserting BASIC commands
!macro b_PASTE	{!by token_PASTE}	; aka GSHAPE
!macro b_CUT	{!by token_CUT}		; aka SSHAPE
!macro b_LINE	{!by token_LINE}	; aka DRAW
!macro b_DIR	{!by token_DIR}		; aka DIRECTORY

; STASH/FETCH/SWAP are all decoded to DMA:
!macro b_DMA	{!by $fe, $1f}
; extended token $fe $20 isn't used ($20 is ' ')
;!macro b_DMA	{!by $fe, $21}
; extended token $fe $22 isn't used ($20 is '"')
;!macro b_DMA	{!by $fe, $23}

; new instructions:
!macro b_TYPE		{!by $fe, $27}	; display sequential disk file
!macro b_BVERIFY	{!by $fe, $28}
!macro b_ECTORY		{!by $fe, $29}	; no-op in case someone types "DIRECTORY"?
!macro b_ERASE		{!by $fe, $2a}	; delete file
!macro b_FIND		{!by $fe, $2b}	; search in basic program
!macro b_CHANGE		{!by $fe, $2c}	; edit program
!macro b_SET		{!by $fe, $2d}	; set system parameter
!macro b_SCREEN		{!by $fe, $2e}
!macro b_POLYGON	{!by $fe, $2f}
!macro b_ELLIPSE	{!by $fe, $30}
!macro b_VIEWPORT	{!by $fe, $31}	; unimplemented?
!macro b_GCOPY		{!by $fe, $32}	; copy graphics
!macro b_PEN		{!by $fe, $33}	; set pen color
!macro b_PALETTE	{!by $fe, $34}	; set palette color
!macro b_DMODE		{!by $fe, $35}	; set draw mode
!macro b_DPAT		{!by $fe, $36}	; set draw pattern
!macro b_PIC		{!by $fe, $37}
!macro b_GENLOCK	{!by $fe, $38}
!macro b_FOREGROUND	{!by $fe, $39}	; set foreground color
; extended token $fe $3a isn't used ($3a is ':')
!macro b_BACKGROUND	{!by $fe, $3b}	; set background color
!macro b_BORDER		{!by $fe, $3c}	; set border color
!macro b_HIGHLIGHT	{!by $fe, $3d}	; set highlight color
!macro b_MOUSE		{!by $fe, $3e}	; set mouse parameters
!macro b_RMOUSE		{!by $fe, $3f}	; read mouse position
!macro b_DISK		{!by $fe, $40}	; send disc command
