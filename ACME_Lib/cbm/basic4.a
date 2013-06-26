;ACME 0.94.4

!ifdef lib_cbm_basic4_a !eof
lib_cbm_basic4_a = 1

!source <cbm/basic2.a>	; from 0x80 to $cb

; CAUTION - these tokens are different to the ones in BASIC 3.5, BASIC 7 and BASIC 10!

; token values
token_CONCAT	= $cc
token_DOPEN	= $cd
token_DCLOSE	= $ce
token_RECORD	= $cf
token_HEADER	= $d0
token_COLLECT	= $d1
token_BACKUP	= $d2
token_COPY	= $d3
token_APPEND	= $d4
token_DSAVE	= $d5
token_DLOAD	= $d6
token_CATALOG	= $d7
token_RENAME	= $d8
token_SCRATCH	= $d9
token_DIRECTORY	= $da

; Macros for inserting BASIC commands:
!macro b_CONCAT		{!by token_CONCAT}
!macro b_DOPEN		{!by token_DOPEN}
!macro b_DCLOSE		{!by token_DCLOSE}
!macro b_RECORD		{!by token_RECORD}
!macro b_HEADER		{!by token_HEADER}
!macro b_COLLECT	{!by token_COLLECT}
!macro b_BACKUP		{!by token_BACKUP}
!macro b_COPY		{!by token_COPY}
!macro b_APPEND		{!by token_APPEND}
!macro b_DSAVE		{!by token_DSAVE}
!macro b_DLOAD		{!by token_DLOAD}
!macro b_CATALOG	{!by token_CATALOG}
!macro b_RENAME		{!by token_RENAME}
!macro b_SCRATCH	{!by token_SCRATCH}
!macro b_DIRECTORY	{!by token_DIRECTORY}
