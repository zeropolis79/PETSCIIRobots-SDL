	xdef	_renderLiveMapTiles__13PlatformAmigaFPUc
	xdef	_readCD32Pad__13PlatformAmigaFv
	xdef	_enableLowpassFilter__13PlatformAmigaFv
	xdef	_disableLowpassFilter__13PlatformAmigaFv
	xdef	_verticalBlankInterruptServer__13PlatformAmigaFv
	xref	_tileLiveMap
	xref	_liveMapToPlane1
	xref	_liveMapToPlane3
	xref	@runVerticalBlankInterrupt__13PlatformAmigaFv
	xref	_addressMap

SCREEN_WIDTH_IN_BYTES	equ	40
PLANES	equ	4

	section	code

_renderLiveMapTiles__13PlatformAmigaFPUc:
	movem.l	d2-d7/a2-a3/a5-a6,-(sp)

	move.l	$26(a0),a2		; screenPlanes
	lea	PLANES*20*SCREEN_WIDTH_IN_BYTES(a2),a2
	lea	_tileLiveMap,a3
	lea	_liveMapToPlane1,a5
	lea	_liveMapToPlane3,a6
	moveq   #0,d5
	moveq   #64-1,d7
.yLoop:
	moveq   #8-1,d6
.xLoop:
	moveq   #0,d0
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	move.b  (a5,d0.w),d1
	move.b  (a6,d0.w),d3
	add.w	#256,d0
	move.b  (a5,d0.w),d2
	move.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.w   #4,d1
	lsl.w   #4,d2
	lsl.w   #4,d3
	lsl.w   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.w   #4,d1
	lsl.w   #4,d2
	lsl.w   #4,d3
	lsl.w   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.w   #4,d1
	lsl.w   #4,d2
	lsl.w   #4,d3
	lsl.w   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.l   #4,d1
	lsl.l   #4,d2
	lsl.l   #4,d3
	lsl.l   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.l   #4,d1
	lsl.l   #4,d2
	lsl.l   #4,d3
	lsl.l   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.l   #4,d1
	lsl.l   #4,d2
	lsl.l   #4,d3
	lsl.l   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	moveq   #0,d0
	lsl.l   #4,d1
	lsl.l   #4,d2
	lsl.l   #4,d3
	lsl.l   #4,d4
	move.b  (a1)+,d5
	move.b  (a3,d5.w),d0
	lsl.w   #4,d0
	move.b  (a1)+,d5
	or.b    (a3,d5.w),d0
	or.b  (a5,d0.w),d1
	or.b  (a6,d0.w),d3
	add.w	#256,d0
	or.b  (a5,d0.w),d2
	or.b  (a6,d0.w),d4

	move.l d1,(a2)+
	move.l d2,(1*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d3,(2*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d4,(3*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d1,(4*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d2,(5*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d3,(6*SCREEN_WIDTH_IN_BYTES-4)(a2)
	move.l d4,(7*SCREEN_WIDTH_IN_BYTES-4)(a2)
	dbra    d6,.xLoop
	lea (2*PLANES*SCREEN_WIDTH_IN_BYTES-32)(a2),a2
	dbra    d7,.yLoop

	movem.l	(sp)+,d2-d7/a2-a3/a5-a6
	rts

_readCD32Pad__13PlatformAmigaFv:
	movem.l	d2-d5,-(sp)

	lea	$bfe001,a0
	lea $dff000,a1
	move.w	$16(a1),d2	; potgor
	moveq	#7,d3	; CIAB_GAMEPORT1 (red)
	moveq	#14,d4	; 2nd button (blue)
	bset	d3,$200(a0)	; set direction of the fire button pin
	bclr	d3,(a0)		; clear current value of the fire button pin
	move.w	d2,d1
	and.w	#$cfff,d1	; mask bits 12 and 13 out
	or.w	#$2000,d1	; output enable for port 2 fire button pin
	move.w	d1,$34(a1)	; potgo

	moveq	#0,d0
	moveq	#10-1,d5
	bra.s	.timingSkip
.buttonLoop:
	tst.b	(a0)
	tst.b	(a0)
.timingSkip:
	tst.b	(a0)
	tst.b	(a0)
	tst.b	(a0)
	tst.b	(a0)
	tst.b	(a0)
	tst.b	(a0)
	move.w	$16(a1),d1	; potgor
	bset	d3,(a0)	; set and clear current value of the fire button pin
	bclr	d3,(a0)
	btst	d4,d1
	bne.s	.noButton
	bset	d5,d0
.noButton:
	dbra	d5,.buttonLoop

	bclr	d3,$200(a0)	; reset fire button pin direction
	move.w	d2,$34(a1)	; potgo

	movem.l	(sp)+,d2-d5
	rts

_enableLowpassFilter__13PlatformAmigaFv:
	bclr	#1,$bfe001
	rts

_disableLowpassFilter__13PlatformAmigaFv:
	bset	#1,$bfe001
	rts

_verticalBlankInterruptServer__13PlatformAmigaFv:
	movem.l d2-d7/a2-a4,-(sp)

	move.l	a1,a0
	jsr	@runVerticalBlankInterrupt__13PlatformAmigaFv

	movem.l	(sp)+,d2-d7/a2-a4
	lea	$dff000,a0
	moveq	#0,d0
	rts

	end
