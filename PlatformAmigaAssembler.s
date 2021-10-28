	xdef	_verticalBlankInterruptServer__13PlatformAmigaFv
	xdef	@writeToScreenMemory__13PlatformAmigaFUsUc
	xref	@runVerticalBlankInterrupt__13PlatformAmigaFv
	xref	_addressMap
	xref	_c64Font

	section	code

_verticalBlankInterruptServer__13PlatformAmigaFv:
	movem.l d2-d7/a2-a4,-(sp)

	move.l	a1,a0
	jsr	@runVerticalBlankInterrupt__13PlatformAmigaFv

	movem.l	(sp)+,d2-d7/a2-a4
	lea	$dff000,a0
	moveq	#0,d0
	rts

@writeToScreenMemory__13PlatformAmigaFUsUc:
	lea	_addressMap,a1
	add.w	d0,d0
	move.w	(a1,d0.w),d0
	move.l	$2a(a0),a1
	add.w	d0,a1
	lea	_c64Font,a0
	add.w	d1,d1
	add.w	d1,d1
	add.w	d1,a0
	add.w	d1,a0
	moveq	#8-1,d0
wTSMLoop:
	move.b	(a0)+,(a1)
	clr.b	40(a1)
	clr.b	80(a1)
	clr.b	120(a1)
	lea	160(a1),a1
	dbra	d0,wTSMLoop
	rts

	end
