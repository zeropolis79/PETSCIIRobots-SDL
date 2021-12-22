; Attack of the PETSCII Robots WHDLoad slave by Vesa Halttunen
; Based on example slave code by wepl

	INCDIR	"INCLUDE:"
	INCLUDE	"whdload.i"
	INCLUDE	"whdmacros.i"
	INCLUDE	"libraries/dos_lib.i"

	OUTPUT	"PETSCIIRobots.slave"
	BOPT	O+
	BOPT	OG+
	BOPT	ODd-
	BOPT	ODe-
	BOPT	w4-
	BOPT	wo-
	SUPER

BLACKSCREEN
CHIPMEMSIZE	= $80000
FASTMEMSIZE	= $40000
NUMDRIVES	= 1
WPDRIVES	= %0000
HDINIT
IOCACHE		= 5000
SETPATCH
BOOTDOS

slv_Version	= 16
slv_Flags	= WHDLF_NoError|WHDLF_Examine
slv_keyexit	= $59	; F10

	INCLUDE	"kick13.s"

	dc.b	"$VER: PETSCIIRobots.slave 1.0 (21.12.2021)",0

slv_CurrentDir:	dc.b	"Data",0
slv_name:		dc.b	"Attack of the PETSCII Robots",0
slv_copy:		dc.b	"2021 8-Bit Productions, LLC.",0
slv_info:		dc.b	"Adapted by Vesa Halttunen",10
				dc.b	"Version 1.0 (21.12.2021)",0
programName:	dc.b	"AmigaRobots",0
args:			dc.b	10
argsEnd:		dc.b	0
				EVEN

_bootdos:	move.l	_resload,a2

	lea	_dosname(pc),a1
	move.l	4.w,a6
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,a6

	lea	programName(pc),a0
	move.l	a0,d1
	jsr	_LVOLoadSeg(a6)
	move.l	d0,d7
	beq	.quit

	move.l	d7,a1
	add.l	a1,a1
	add.l	a1,a1
	addq.l	#4,a1

	cmp.w	#$4e92,$2a68(a1)	; jsr(a2)
	beq	.patch
	pea	TDREASON_WRONGVER
	jmp	resload_Abort(a2)

.patch:
	movem.l	a1,-(sp)
	lea	patchList(pc),a0	
	jsr	resload_Patch(a2)
	move.l	(sp)+,a1

	lea	args(pc),a0
	move.l	4(sp),d0
	sub.l	#5*4,d0
	movem.l	d0/d7/a2/a6,-(sp)
	moveq	#argsEnd-args,d0
	jsr	(a1)
	movem.l	(sp)+,d1/d7/a2/a6

	pea	TDREASON_OK
	jmp	resload_Abort(a2)

.quit:
	jsr	_LVOIoErr(a6)
	move.l	a3,-(sp)
	move.l	d0,-(sp)
	pea	TDREASON_DOSREAD
	move.l	_resload(pc),-(sp)
	add.l	#resload_Abort,(sp)
	rts

patchList:
	PL_START
	PL_NOPS	$2a68,1
	PL_END

	END
