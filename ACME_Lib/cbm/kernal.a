;ACME 0.95

!ifdef lib_cbm_kernal_a !eof
lib_cbm_kernal_a = 1

; Taken from the web.
; Sorry, I can't give credit because I don't have the URL anymore.
; There are alternative names for some calls.

!address {
	; for additional c128 calls, see <cbm/c128/kernal.a>
	k_cint		= $ff81
	k_ioinit	= $ff84
	; cbm-ii rom starts here:
	k_ramtas	= $ff87
	k_restor	= $ff8a
	k_vector	= $ff8d
	k_setmsg	= $ff90
	k_secnd		= $ff93
	k_tksa		= $ff96
	k_memtop	= $ff99
	k_membot	= $ff9c
	k_key		= $ff9f
	k_settmo	= $ffa2
	k_iecin		= $ffa5:k_acptr		= $ffa5
	k_iecout	= $ffa8:k_ciout		= $ffa8
	k_untalk	= $ffab:k_untlk		= $ffab
	k_unlisten	= $ffae:k_unlsn		= $ffae
	k_listen	= $ffb1:k_listn		= $ffb1
	k_talk		= $ffb4
	k_readss	= $ffb7
	k_setlfs	= $ffba	; set file parameters (A = logical file number, X = device, Y = secondary address)
	k_setnam	= $ffbd	; set file name (A = length, YYXX = pointer)
	; pet rom starts here:
	; i/o calls: these may set C on error. in that case, A holds error code, see <cbm/ioerror.a> for the actual values.
	k_open		= $ffc0	; open channel/file (call setlfs/setnam before!)
	k_close		= $ffc3:k_close_A	= $ffc3	; close channel/file (A = logical file number)
	k_chkin		= $ffc6:k_chkin_X	= $ffc6	; set input channel (X = logical file number)
	k_chkout	= $ffc9:k_chkout_X	= $ffc9:k_ckout		= $ffc9	; set output channel (X = logical file number)
	k_clrchn	= $ffcc:k_clrch		= $ffcc	; restore default input/output channels
	k_chrin		= $ffcf:k_basin		= $ffcf	; read byte from current input channel (not the same as $ffe4, see note* below)
		; A is result byte
		; X is preserved
		; Y gets clobbered by tape access (preserved by disk access)
	k_chrout	= $ffd2:k_basout	= $ffd2:k_bsout		= $ffd2	; send byte to current output channel
		; A/X/Y are preserved
	k_load		= $ffd5:k_load_AXY	= $ffd5	; load file to memory, or verify (call setlfs/setnam before!)
		; A: zero means LOAD, nonzero means VERIFY
		; YYXX is desired load address (only used if secondary address == 0), returns end address plus 1
	k_save		= $ffd8:k_save_AXY	= $ffd8	; save memory to file (call setlfs/setnam before!)
		; A is zp address of start ptr(!)
		; YYXX is end address plus 1
	k_settim	= $ffdb	; set time
	k_rdtim		= $ffde	; read time
	k_stop		= $ffe1 ; check STOP key
	k_getin		= $ffe4:k_get		= $ffe4	; get input byte (not the same as $ffcf, see note* below)
		; A is result byte
		; X is preserved
		; Y gets clobbered by tape access (preserved by disk access)
	k_clall		= $ffe7
	k_udtim		= $ffea
	; pet rom stops here!?
	k_scrorg	= $ffed	; returns screen size (X = number of columns, Y = number of lines)
		; CAUTION: the c128 uses a new format:
		; c128: X/Y now return maximum values in current window (so 0..39/0..24 instead of 40/25).
		; c128: A returns max column on current screen (39 or 79)
	k_plot		= $fff0:k_plot_CXY	= $fff0	; get/set cursor (X is line, Y is column)
		; C = 0: set cursor position.
		; C = 1: read cursor position.
	k_iobase	= $fff3	; returns first i/o address (i.e. memory limit) in YYXX
		; cbm-ii:	$dc00
		; vic20:	$9110
		; c64:		$d000
		; 264:		$fd00
		; c128:		$d000
}

;note*
; the difference between CHRIN and GETIN depends on the current input device:
; input device 0 (keyboard): CHRIN reads from input buffer, GETIN reads from keyboard buffer
;	(the same difference as between INPUT and GET in basic)
; input device 2 (rs232): CHRIN does some error handling, GETIN may just return zero on error.
; roughly speaking, CHRIN returns a "processed" byte while GETIN returns a "raw" byte.
; for devices on the IEC bus there should be no difference between the two calls.
; when reading from the console (keyboard/screen), a zero byte means "no data".
; do not expect a valid Z flag in this case! some devices may clobber the Z flag.
