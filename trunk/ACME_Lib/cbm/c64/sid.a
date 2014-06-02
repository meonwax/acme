;ACME 0.95

!ifdef lib_cbm_c64_sid_a !eof
lib_cbm_c64_sid_a = 1

!address {
	; write-only registers:
	; voice 1:
	sid_v1_freq_lo		= $d400
	sid_v1_freq_hi		= $d401
	sid_v1_width_lo		= $d402
	sid_v1_width_hi		= $d403
	sid_v1_control		= $d404	; see below for bits
	sid_v1_attack_decay	= $d405	; hi-nibble: attack length, low-nibble: decay length
	sid_v1_sustain_release	= $d406	; hi-nibble: sustain volumne, low-nibble: release length
	; voice 2:
	sid_v2_freq_lo		= $d407
	sid_v2_freq_hi		= $d408
	sid_v2_width_lo		= $d409
	sid_v2_width_hi		= $d40a
	sid_v2_control		= $d40b	; see below for bits
	sid_v2_attack_decay	= $d40c	; hi-nibble: attack length, low-nibble: decay length
	sid_v2_sustain_release	= $d40d	; hi-nibble: sustain volumne, low-nibble: release length
	; voice 3:
	sid_v3_freq_lo		= $d40e
	sid_v3_freq_hi		= $d40f
	sid_v3_width_lo		= $d410
	sid_v3_width_hi		= $d411
	sid_v3_control		= $d412	; see below for bits
	sid_v3_attack_decay	= $d413	; hi-nibble: attack length, low-nibble: decay length
	sid_v3_sustain_release	= $d414	; hi-nibble: sustain volumne, low-nibble: release length
}
; voice control bits:
sid_VOICECONTROL_NOISE		= %#.......
sid_VOICECONTROL_RECTANGLE	= %.#......
sid_VOICECONTROL_SAWTOOTH	= %..#.....
sid_VOICECONTROL_TRIANGLE	= %...#....
sid_VOICECONTROL_DISABLE_RESET	= %....#...	; 1 = disable voice, reset noise generator
sid_VOICECONTROL_RINGMODULATION	= %.....#..
sid_VOICECONTROL_SYNC		= %......#.
sid_VOICECONTROL_ON		= %.......#	; 0 = release, 1 = attack/sustain/decay
!address {
	; registers shared by all voices:
	sid_filter_cutoff_lo	= $d415	; only bits 0/1/2!
	sid_filter_cutoff_hi	= $d416
	sid_filter_control	= $d417 ; hi-nibble: resonance, lo-nibble: filter ext/v3/v2/v1
	sid_filter_volume	= $d418	; hi-nibble: filter mode (disable v3, high, band, low), lo-nibble: volume
	; read-only registers:
	sid_potx		= $d419
	sid_poty		= $d41a
	sid_v3_waveform_output	= $d41b
	sid_v3_adsr_output	= $d41c
}
; Do not use the shadow copies of these registers at $d5xx, $d6xx or $d7xx:
; on a C128 they do not exist!
