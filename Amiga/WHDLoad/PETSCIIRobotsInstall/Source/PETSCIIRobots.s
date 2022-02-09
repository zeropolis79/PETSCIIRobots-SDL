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

	dc.b	"$VER: PETSCIIRobots.slave 1.1 (09.02.2022)",0

slv_CurrentDir:	dc.b	"Data",0
slv_name:		dc.b	"Attack of the PETSCII Robots",0
slv_copy:		dc.b	"2021 8-Bit Productions, LLC.",0
slv_info:		dc.b	"Adapted by Vesa Halttunen",10
				dc.b	"Version 1.1 (09.02.2022)",0
programName:	dc.b	"AmigaRobots",0
args:			dc.b	10
argsEnd:		dc.b	0
				EVEN

_bootdos:
	move.l	_resload,a2
	lea	programName(pc),a3

	lea	_dosname(pc),a1
	move.l	4.w,a6
	jsr	_LVOOldOpenLibrary(a6)
	move.l	d0,a6

	move.l	a3,d1
	jsr	_LVOLoadSeg(a6)
	tst.l	d0
	bne.s	.checkVersion

.readFailure:
	jsr	_LVOIoErr(a6)
	move.l	a3,-(sp)
	move.l	d0,-(sp)
	pea	TDREASON_DOSREAD
	move.l	_resload(pc),-(sp)
	add.l	#resload_Abort,(sp)
	rts

.checkVersion:
	add.l	d0,d0
	add.l	d0,d0
	addq	#4,d0
	move.l	d0,a4

	move.l	a3,d1
	move.l	#MODE_OLDFILE,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d1
	beq.s	.readFailure

	move.l	#300,d3
	sub.l	d3,sp
	move.l	sp,d2
	jsr	_LVORead(a6)

	move.l	d3,d0
	move.l	sp,a0
	jsr	resload_CRC16(a2)

	add.l	d3,sp

	cmp.w	#$20af,d0			; version 1.0
	beq	.checkJSR
	cmp.w	#$ce1b,d0			; version 1.1
	beq	.start

.wrongVersion:
	pea	TDREASON_WRONGVER
	jmp	resload_Abort(a2)

.checkJSR:
	cmp.w	#$4e92,$2a68(a4)	; jsr(a2)
	bne.s	.wrongVersion

	lea	patchList(pc),a0	
	move.l	a4,a1
	jsr	resload_Patch(a2)

.start:
	lea	args(pc),a0
	move.l	4(sp),d0
	sub.l	#5*4,d0
	moveq	#argsEnd-args,d0
	jsr	(a4)

	move.l	_resload,a2
	pea	TDREASON_OK
	jmp	resload_Abort(a2)

patchList:
	PL_START
	PL_NOPS	$2a68,1
	PL_END

	END
