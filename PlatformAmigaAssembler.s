	xdef	_verticalBlankInterruptServer__13PlatformAmigaFv
	xref	@runVerticalBlankInterrupt__13PlatformAmigaFv

	section	code

_verticalBlankInterruptServer__13PlatformAmigaFv:
	movem.l d2-d7/a2-a4,-(sp)

	move.l	a1,a0
	jsr	@runVerticalBlankInterrupt__13PlatformAmigaFv

	movem.l	(sp)+,d2-d7/a2-a4
	lea	$dff000,a0
	moveq	#0,d0
	rts

	end
