;ACME 0.95

!ifdef lib_cbm_c128_mmu_a !eof
lib_cbm_c128_mmu_a = 2

; Memory Management Unit (MMU) 8722


; registers in i/o area (i/o needs to be enabled to access these):

!address {
	; configuration register
	mmu_cr_d500	= $d500	; same as "mmu_cr" at $ff00. Use "mmu_cr" instead, as that is *always* available.
	; preconfiguration registers (internal format just like mmu_cr)
	mmu_pcr_a	= $d501	; c128 kernal default is $3f (BANK 0)
	mmu_pcr_b	= $d502	; c128 kernal default is $7f (BANK 1)
	mmu_pcr_c	= $d503	; c128 kernal default is $01 (BANK 14)
	mmu_pcr_d	= $d504	; c128 kernal default is $41 (all system roms, with ram 1)
}
; contents of cr and all four pcr:
mmu_CR_RAMBANK_MASK	= %##......	; this controls which RAM bank is used in areas where RAM is enabled
;mmu_CR_RAMBANK_0	= %........
mmu_CR_RAMBANK_1	= %.#......
mmu_CR_RAMBANK_2	= %#.......	; on an unmodified c128, there is no ram bank 2 (0 will be used instead)
mmu_CR_RAMBANK_3	= %##......	; on an unmodified c128, there is no ram bank 3 (1 will be used instead)

mmu_CR_HIGH_MASK	= %..##....	; this controls the "high area" (c000..ffff), but i/o (d000..dfff) is separate
;mmu_CR_HIGH_SYSROM	= %........	; editor, charset (or i/o, see below), kernal
mmu_CR_HIGH_INTFUNCROM	= %...#....
mmu_CR_HIGH_EXTFUNCROM	= %..#.....
mmu_CR_HIGH_RAM		= %..##....

mmu_CR_MID_MASK		= %....##..	; this controls the "middle area" (8000..bfff)
;mmu_CR_MID_SYSROM	= %........	; this is the upper half of basic
mmu_CR_MID_INTFUNCROM	= %.....#..
mmu_CR_MID_EXTFUNCROM	= %....#...
mmu_CR_MID_RAM		= %....##..

mmu_CR_LOW_MASK		= %......#.	; this controls the "low area" (4000..7fff)
;mmu_CR_LOW_SYSROM	= %........	; this is the lower half of basic
mmu_CR_LOW_RAM		= %......#.

mmu_CR_IO_MASK		= %.......#	; this controls i/o space (d000..dfff)
;mmu_CR_IO_ON		= %........
mmu_CR_IO_OFF		= %.......#	; if i/o is off, contents depend on "high area"

; configuration register values used by C128 firmware (lookup table at $f7f0, see end of file):
mmu_CR_BANK0	= $3f	; full 64 KiB ram bank 0
mmu_CR_BANK1	= $7f	; full 64 KiB ram bank 1
mmu_CR_BANK2	= $bf	; full 64 KiB ram bank 2
mmu_CR_BANK3	= $ff	; full 64 KiB ram bank 3
mmu_CR_BANK4	= $16	; 32 KiB bank 0; 32 KiB IFROM with i/o overlay
mmu_CR_BANK5	= $56	; 32 KiB bank 1; 32 KiB IFROM with i/o overlay
mmu_CR_BANK6	= $96	; 32 KiB bank 2; 32 KiB IFROM with i/o overlay
mmu_CR_BANK7	= $d6	; 32 KiB bank 3; 32 KiB IFROM with i/o overlay
mmu_CR_BANK8	= $2a	; 32 KiB bank 0; 32 KiB EFROM with i/o overlay
mmu_CR_BANK9	= $6a	; 32 KiB bank 1; 32 KiB EFROM with i/o overlay
mmu_CR_BANK10	= $aa	; 32 KiB bank 2; 32 KiB EFROM with i/o overlay
mmu_CR_BANK11	= $ea	; 32 KiB bank 3; 32 KiB EFROM with i/o overlay
mmu_CR_BANK12	= $06	; 32 KiB bank 0; 16 KiB IFROM; 16 KiB kernal with i/o overlay
mmu_CR_BANK13	= $0a	; 32 KiB bank 0; 16 KiB EFROM; 16 KiB kernal with i/o overlay
mmu_CR_BANK14	= $01	; 16 KiB bank 0; 32 KiB basic; 16 KiB kernal with font overlay
mmu_CR_BANK15	= $00	; 16 KiB bank 0; 32 KiB basic; 16 KiB kernal with i/o overlay
; An unmodified C128 does not have a "ram bank 2" or "ram bank 3".
; Whenever one of these is activated, ram banks 0 and 1 will be used instead.
; IFROM means internal function ROM (socket U36)
; EFROM means external function ROM (socket in a REU, for example)

!address {
	; mode configuration register
	mmu_mcr		= $d505
}
; contents:
mmu_MCR_40COLUMNS	= %#.......	; (pin 48) 40/80 key: 0 means pressed, 1 means released (writable! if cleared, will always read as 0!)
mmu_MCR_C64MODE		= %.#......	; (pin 47) setting this bit makes the MMU disappear from the memory map :)
mmu_MCR_EXROM		= %..#.....	; (pin 46) if zero on boot, system will enter c64 mode (writable!)
mmu_MCR_GAME		= %...#....	; (pin 45) if zero on boot, system will enter c64 mode (writable!)
mmu_MCR_FSDIR_OUTPUT	= %....#...	; (pin 44) direction of fast serial bus
mmu_MCR_UNUSED		= %.....##.	; always set
mmu_MCR_8502MODE	= %.......#	; (pin 43 inverted) setting this to zero switches to Z80 cpu

!address {
	; ram configuration register
	mmu_rcr		= $d506
}
; contents:
mmu_RCR_VICBANK_MASK	= %##......	; this controls which RAM bank is "seen" by VIC
;mmu_RCR_VICBANK_0	= %........
mmu_RCR_VICBANK_1	= %.#......
mmu_RCR_VICBANK_2	= %#.......	; on an unmodified c128, there is no ram bank 2 (0 will be used instead)
mmu_RCR_VICBANK_3	= %##......	; on an unmodified c128, there is no ram bank 3 (1 will be used instead)

mmu_RCR_RAMBLOCK_MASK	= %..##....	; on an unmodified c128, these bits are irrelevant (they select 256 KiB of 1 MiB of memory)
;mmu_RCR_RAMBLOCK_0	= %........
mmu_RCR_RAMBLOCK_1	= %...#....	; on an unmodified c128, there is only ram block 0
mmu_RCR_RAMBLOCK_2	= %..#.....	; on an unmodified c128, there is only ram block 0
mmu_RCR_RAMBLOCK_3	= %..##....	; on an unmodified c128, there is only ram block 0

mmu_RCR_SHARE_MASK	= %....##..
;mmu_RCR_SHARE_NONE	= %........
mmu_RCR_SHARE_BOTTOM	= %.....#..	; system default
mmu_RCR_SHARE_TOP	= %....#...

mmu_RCR_SHARESIZE_MASK	= %......##
;mmu_RCR_SHARESIZE_1K	= %........	; system default
mmu_RCR_SHARESIZE_4K	= %.......#
mmu_RCR_SHARESIZE_8K	= %......#.
mmu_RCR_SHARESIZE_16K	= %......##

!address {
	; page pointers for zero page and stack:
	; write to "bank" register will be latched (reading gives old value)
	; until "page" register is written to as well.
	mmu_zp_page	= $d507	; address bits a8..a15, default $00
	mmu_zp_bank	= $d508	; address bits a16..a19, default $0 (on an unmodified c128, only bit0 is meaningful)
	mmu_stack_page	= $d509	; address bits a8..a15, default $01
	mmu_stack_bank	= $d50a	; address bits a16..a19, default $0 (on an unmodified c128, only bit0 is meaningful)
}
mmu_PxH_UNUSED		= %####....	; always set

mmu_PxH_RAMBLOCK_MASK	= %....##..
;mmu_PxH_RAMBLOCK_0	= %........
mmu_PxH_RAMBLOCK_1	= %.....#..	; on an unmodified c128, there is only ram block 0
mmu_PxH_RAMBLOCK_2	= %....#...	; on an unmodified c128, there is only ram block 0
mmu_PxH_RAMBLOCK_3	= %....##..	; on an unmodified c128, there is only ram block 0

mmu_PxH_RAMBANK_MASK	= %......##
;mmu_PxH_RAMBANK_0	= %........
mmu_PxH_RAMBANK_1	= %.......#
mmu_PxH_RAMBANK_2	= %......#.	; on an unmodified c128, there is no ram bank 2 (0 will be used instead)
mmu_PxH_RAMBANK_3	= %......##	; on an unmodified c128, there is no ram bank 3 (1 will be used instead)

!address {
	; version register
	mmu_vr		= $d50b	; read-only, value is $20
}
mmu_VR_BANKS_MASK	= %####....	; 2 ram banks
mmu_VR_VERSION_MASK	= %....####	; mmu version 0

; reading addresses up until $d5ff returns $ff


; these registers are always available (in *every* memory configuration) unless C64 mode is entered:

!address {
	; configuration register
	mmu_cr		= $ff00	; always use this instead of $d500
	; load configuration registers:
	; a read access will return the value of the corresponding preconfiguration register
	; any write access will copy the value of the corresponding preconfiguration register to mmu_cr
	mmu_lcr_a	= $ff01	; c128 kernal default is $3f (BANK 0)
	mmu_lcr_b	= $ff02	; c128 kernal default is $7f (BANK 1)
	mmu_lcr_c	= $ff03	; c128 kernal default is $01 (BANK 14)
	mmu_lcr_d	= $ff04	; c128 kernal default is $41 (all system roms, with ram 1)

	; the c128 ROMs contain a look-up table to convert bank numbers to their
	; corresponding configuration register values:
	; romc_* needs "high rom area" enabled ($c000..$ffff)
	romc_bank_to_cr_table	= $f7f0	; 3f 7f bf ff 16 56 96 d6 2a 6a aa ea 06 0a 01 00
}
