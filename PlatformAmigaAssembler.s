	xdef	_renderLiveMapTiles__13PlatformAmigaFPUc
	xdef	_readCD32Pad__13PlatformAmigaFv
	xdef	_enableLowpassFilter__13PlatformAmigaFv
	xdef	_disableLowpassFilter__13PlatformAmigaFv
	xdef	_verticalBlankInterruptServer__13PlatformAmigaFv
	xdef	_ungzip__13PlatformAmigaFPvPv
	xref	_tileLiveMap
	xref	_liveMapToPlane1
	xref	_liveMapToPlane3
	xref	@runVerticalBlankInterrupt__13PlatformAmigaFv
	xref	_addressMap
	xref	_c64Font

SCREEN_WIDTH_IN_BYTES	equ	40
PLANES	equ	4

GZIP_MAGIC	equ	$1f8b
DEFLATED	equ	8
CONTINUATION_BIT	equ	1
EXTRA_FIELD_BIT	equ	2
ORIG_NAME_BIT	equ	3
COMMENT_BIT	equ	4
ENCRYPTED_BIT	equ	5
RESERVED1_BIT	equ	6
RESERVED2_BIT	equ	7

MAXBITS		equ	15	; maximum bits in a code
MAXLCODES	equ	286	; maximum number of literal/length codes
MAXDCODES	equ	30	; maximum number of distance codes
MAXCODES	equ	(MAXLCODES+MAXDCODES)	; maximum codes lengths to read
FIXLCODES	equ	288	; number of fixed literal/length codes

fixed_lencnt	equ	0
fixed_lensym	equ	fixed_lencnt+(MAXBITS+1)*2
fixed_distcnt	equ	fixed_lensym+FIXLCODES*2
fixed_distsym	equ	fixed_distcnt+(MAXBITS+1)*2
fixed_lengths	equ	fixed_distsym+MAXDCODES*2

dynamic_lengths	equ	0
dynamic_lencnt	equ	dynamic_lengths+MAXCODES*2
dynamic_lensym	equ	dynamic_lencnt+(MAXBITS+1)*2
dynamic_distcnt	equ	dynamic_lensym+MAXLCODES*2
dynamic_distsym	equ	dynamic_distcnt+(MAXBITS+1)*2

huffman_count	equ	0
huffman_symbol	equ	4

codes_lens	equ	0
codes_lext	equ	29*2
codes_dists	equ	codes_lext+29*2
codes_dext	equ	codes_dists+30*2

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

_ungzip__13PlatformAmigaFPvPv:
	move.l	a1,output
	bsr	get_method
	tst.w	d0
	bpl	.unzip
	rts

.unzip:	bsr	inflate
	rts

	;  In: a0 = input buffer
	; Out: d0 = method or error code
get_method:
	move.l	d1,-(sp)
	move.w	(a0)+,d0	; magic
	cmp.w	#GZIP_MAGIC,d0
	beq.s	.gzip
	moveq	#-16,d0
	bra.s	.get_method_done
.gzip:	move.b	(a0)+,d0	; method
	cmp.b	#DEFLATED,d0
	beq.s	.deflated
	moveq	#-17,d0
	bra.s	.get_method_done
.deflated:
	move.b	(a0)+,d0	; flags
	btst	#ENCRYPTED_BIT,d0
	beq.s	.not_encrypted
	moveq	#-18,d0
	bra.s	.get_method_done
.not_encrypted:
	btst	#CONTINUATION_BIT,d0
	beq.s	.not_continuation
	moveq	#-19,d0
	bra.s	.get_method_done
.not_continuation:
	btst	#RESERVED1_BIT,d0
	bne.s	.reserved
	btst	#RESERVED2_BIT,d0
	beq.s	.not_reserved
.reserved:
	moveq	#-20,d0
	bra.s	.get_method_done
.not_reserved:
	addq	#6,a0		; stamp, extra flags, os type
	btst	#EXTRA_FIELD_BIT,d0
	beq.s	.no_extra_field
	move.w	(a0)+,d1
	add.w	d1,a0
.no_extra_field:
	btst	#ORIG_NAME_BIT,d0
	beq.s	.no_orig_name
.name:	tst.b	(a0)+
	bne.s	.name
.no_orig_name:
	btst	#COMMENT_BIT,d0
	beq.s	.no_comment
.comment:
	tst.b	(a0)+
	bne.s	.comment
.no_comment:
	moveq	#DEFLATED,d0
.get_method_done:
	move.l	(sp)+,d1
	rts

	;  In: a0 = input buffer
	;  In: a1 = output buffer
	; Out: d0 = error code
inflate:
	move.l	d1,-(sp)
	clr.l	bitbuf
	clr.l	bitcnt

.inflate_loop:
	moveq	#1,d0
	bsr	bits
	move.l	d0,d1		; one if last block

	moveq	#2,d0
	bsr	bits		; block type 0..3

	tst.w	d0
	bne.s	.not_stored
	bsr	stored
	bra.s	.next

.not_stored:
	cmp.w	#1,d0
	bne.s	.not_fixed
	bsr	fixed
	bra.s	.next

.not_fixed:
	cmp.w	#2,d0
	bne.s	.not_dynamic
	bsr	dynamic
	bra.s	.next

.not_dynamic:
	moveq	#-1,d0
	bra.s	.inflate_done

.next:	tst.w	d1
	beq.s	.inflate_loop
	moveq	#0,d0
.inflate_done:
	move.l	(sp)+,d1
	rts

	;  In: a0 = buffer
	;  In: d0 = need
	; Out: d0 = need bits from buffer
bits:	movem.l	d1-d4,-(sp)

	move.l	bitbuf,d1	; load at least need bits into val
	move.l	bitcnt,d3
.bits_loop:
	cmp.l	d0,d3
	bge.s	.bits_done
	moveq	#0,d2
	move.b	(a0)+,d2	; load eight bits
	move.l	d3,d4
	lsl.l	d4,d2
	or.l	d2,d1
	addq	#8,d3
	bra.s	.bits_loop

.bits_done:
	move.l	d1,d2		; drop need bits and update buffer, always zero to seven bits left
	lsr.l	d0,d2
	move.l	d2,bitbuf
	sub.l	d0,d3
	move.l	d3,bitcnt

	moveq	#1,d2		; return need bits, zeroing the bits above that
	lsl.l	d0,d2
	subq	#1,d2
	and.l	d2,d1
	move.l	d1,d0

	movem.l	(sp)+,d1-d4
	rts

	;  In: a0 = input buffer
	;  In: a1 = output buffer
	; Out: d0 = error code
stored:	move.l	d1,-(sp)
	clr.l	bitbuf		; discard leftover bits from current byte (assumes s->bitcnt < 8)
	clr.l	bitcnt

	move.w	(a0)+,d0
	move.w	(a0)+,d1
	not.w	d1
	cmp.w	d0,d1
	beq.s	.stored_ok
	moveq	#-2,d0
	bra.s	.stored_done

.stored_ok:
	subq	#1,d0
	bmi.s	.stored_none_left
.stored_loop:
	move.b	(a0)+,(a1)+
	dbra	d0,.stored_loop

.stored_none_left:
	moveq	#0,d0
.stored_done:
	move.l	(sp)+,d1
	rts

	;  In: a0 = input buffer
	;  In: a1 = output buffer
	; Out: d0 = error code
fixed:	tst.b	virgin
	beq.w	.not_virgin

	; init lencode and distcode structure pointers (must be done before calling construct)
	lea	codes_data,a2
	move.l	a2,lencode+huffman_count
	move.l	a2,lencode+huffman_symbol
	move.l	a2,distcode+huffman_count
	move.l	a2,distcode+huffman_symbol
	add.l	#fixed_lencnt,lencode+huffman_count
	add.l	#fixed_lensym,lencode+huffman_symbol
	add.l	#fixed_distcnt,distcode+huffman_count
	add.l	#fixed_distsym,distcode+huffman_symbol

	; literal/length table
	add.w	#fixed_lengths,a2
	move.w	#144-1,d7
.fixed_loop1:
	move.w	#8,(a2)+
	dbra	d7,.fixed_loop1
	moveq	#256-144-1,d7
.fixed_loop2:
	move.w	#9,(a2)+
	dbra	d7,.fixed_loop2
	moveq	#280-256-1,d7
.fixed_loop3:
	move.w	#7,(a2)+
	dbra	d7,.fixed_loop3
	moveq	#FIXLCODES-280-1,d7
.fixed_loop4:
	move.w	#8,(a2)+
	dbra	d7,.fixed_loop4

	movem.l	a0-a1,-(sp)
	lea	lencode,a0
	lea	codes_data,a1
	add.w	#fixed_lengths,a1
	move.l	#FIXLCODES,d0
	bsr	construct
	movem.l	(sp)+,a0-a1

	; distance table
	lea	codes_data,a2
	add.w	#fixed_lengths,a2
	moveq	#MAXDCODES-1,d7
.fixed_loop5:
	move.w	#5,(a2)+
	dbra	d7,.fixed_loop5

	movem.l	a0-a1,-(sp)
	lea	distcode,a0
	lea	codes_data,a1
	add.w	#fixed_lengths,a1
	moveq	#MAXDCODES,d0
	bsr	construct
	movem.l	(sp)+,a0-a1

	clr.b	virgin

.not_virgin:
	; decode data until end-of-block code
	lea	lencode,a2
	lea	distcode,a3
	bsr	codes
	rts

	;  In: a0 = input buffer
	;  In: a1 = output buffer
	; Out: d0 = error code
dynamic:
	movem.l	d1-d7/a2-a4,-(sp)

	; init structure pointers
	lea	lencode,a2
	lea	codes_data,a3
	move.l	a3,lencode+huffman_count
	move.l	a3,lencode+huffman_symbol
	move.l	a3,distcode+huffman_count
	move.l	a3,distcode+huffman_symbol
	add.l	#dynamic_lencnt,lencode+huffman_count
	add.l	#dynamic_lensym,lencode+huffman_symbol
	add.l	#dynamic_distcnt,distcode+huffman_count
	add.l	#dynamic_distsym,distcode+huffman_symbol
	add.w	#dynamic_lengths,a3

	; get number of lengths in each table, check lengths
	moveq	#5,d0
	bsr	bits
	add.w	#257,d0
	cmp.w	#MAXLCODES,d0
	ble.s	.nlen_ok
	moveq	#-3,d0		; bad count
	bra.w	.dynamic_done
.nlen_ok:
	move.w	d0,d1		; nlen

	moveq	#5,d0
	bsr	bits
	addq	#1,d0
	cmp.w	#MAXDCODES,d0
	ble.s	.ndist_ok
	moveq	#-3,d0		; bad count
	bra.w	.dynamic_done
.ndist_ok:
	move.w	d0,d2		; ndist

	moveq	#4,d0
	bsr	bits
	addq	#4,d0
	move.w	d0,d3		; ncode

	; read code length code lengths (really), missing lengths are zero
	lea	order,a4
	moveq	#0,d4
	moveq	#0,d7
.dynamic_loop1:
	moveq	#3,d0
	bsr	bits
	move.b	(a4,d7.w),d4
	move.w	d0,(a3,d4.w)
	addq	#1,d7
	cmp.w	d3,d7
	bcs.s	.dynamic_loop1

.dynamic_loop2:
	move.b	(a4,d7.w),d4
	clr.w	(a3,d4.w)
	addq	#1,d7
	cmp.w	#19,d7
	bcs.s	.dynamic_loop2

	; build huffman table for code lengths (use lencode temporarily)
	movem.l	a0-a1,-(sp)
	lea	lencode,a0
	move.l	a3,a1
	moveq	#19,d0
	bsr	construct
	movem.l	(sp)+,a0-a1
	tst.w	d0
	beq.s	.lengths_ok
	moveq	#-4,d0			; require complete code set here
	bra.w	.dynamic_done

.lengths_ok:
	; read length/literal and distance code length tables
	moveq	#0,d7			; index
.dynamic_loop3:
	move.w	d1,d4
	add.w	d2,d4
	cmp.w	d4,d7
	bge.w	.dynamic_next

	bsr	decode
	cmp.w	#16,d0
	bcs.s	.length_15

	moveq	#0,d4			; len
	cmp.w	#16,d0
	bne.s	.length_not_16
	tst.w	d7
	bne.s	.index_ok
	moveq	#-5,d0			; no last length!
	bra.w	.dynamic_done
.index_ok:
	move.w	d7,d5
	subq	#1,d5
	add.w	d5,d5
	move.w	(a3,d5.w),d4		; last length

	moveq	#2,d0
	bsr	bits
	addq	#3,d0
	bra.s	.symbol_ok

.length_not_16:
	cmp.w	#17,d0			; repeat zero 3..10 times
	bne.s	.length_not_17
	moveq	#3,d0
	bsr	bits
	addq	#3,d0
	bra.s	.symbol_ok

.length_not_17:
	moveq	#7,d0			; == 18, repeat zero 11..138 times
	bsr	bits
	add.w	#11,d0

.symbol_ok:
	move.w	d1,d5
	add.w	d2,d5
	move.w	d7,d6
	add.w	d0,d6
	cmp.w	d5,d6
	bls.s	.length_ok
	moveq	#-6,d0			; too many lengths!
	bra.w	.dynamic_done

.length_ok:
	subq	#1,d0
	bmi.s	.dynamic_loop3
.repeat:
	move.w	d7,d5			; repeat last or zero symbol times
	add.w	d5,d5
	move.w	d4,(a3,d5.w)
	addq	#1,d7
	dbra	d0,.repeat
	bra.s	.dynamic_loop3

.length_15:
	move.w	d7,d4			; length in 0..15
	add.w	d4,d4
	move.w	d0,(a3,d4.w)
	addq	#1,d7
	bra.w	.dynamic_loop3

	; check for end-of-block code -- there better be one!
.dynamic_next:
	tst.w	256*2(a3)
	bne.s	.end_of_block
	moveq	#-9,d0
	bra.w	.dynamic_done

.end_of_block:
	; build huffman table for literal/length codes
	movem.l	a0-a1,-(sp)
	lea	lencode,a0
	move.l	a3,a1
	move.w	d1,d0
	bsr	construct
	movem.l	(sp)+,a0-a1

	tst.w	d0
	bmi.s	.bad_length_codes
	beq.s	.length_done
	move.l	lencode+huffman_count,a4
	move.l	d1,d5
	sub.l	(a4),d5
	cmp.w	#1,d5
	beq.s	.length_done
.bad_length_codes:
	moveq	#-7,d0
	bra.s	.dynamic_done

.length_done:
	; build huffman table for distance codes
	movem.l	a0-a1,-(sp)
	lea	distcode,a0
	move.l	a3,a1
	add.w	d1,a1
	add.w	d1,a1
	move.w	d2,d0
	bsr	construct
	movem.l	(sp)+,a0-a1

	tst.w	d0
	bmi.s	.bad_distance_codes
	beq.s	.distance_done
	move.l	distcode+huffman_count,a4
	move.l	d2,d5
	sub.l	(a4),d5
	cmp.w	#1,d5
	beq.s	.distance_done
.bad_distance_codes:
	moveq	#-8,d0
	bra.s	.dynamic_done

.distance_done:
	lea	lencode,a2
	lea	distcode,a3
	bsr	codes

.dynamic_done:
	movem.l	(sp)+,d1-d7/a2-a4
	rts

	;  In: a0 = input buffer
	;  In: a1 = output buffer
	;  In: a2 = lencode
	;  In: a3 = distcode
	; Out: d0 = error code
codes:	movem.l	d1-d2/a4-a5,-(sp)
	lea	codes_tables,a4

	; decode literals and length/distance pairs
.codes_loop:
	bsr	decode
	tst.w	d0
	bmi	.codes_done		; invalid symbol
	cmp.w	#256,d0
	beq.s	.codes_done
	bhi.s	.huff
	move.b	d0,(a1)+	; literal: symbol is the byte
	bra.s	.codes_loop
.huff:	sub.w	#257,d0		; get and compute length
	cmp.w	#29,d0
	bcs.s	.codes_ok
	moveq	#-10,d0		; invalid fixed code
	bra.s	.codes_done

.codes_ok:
	add.w	d0,d0
	move.w	codes_lens(a4,d0.w),d1
	move.w	codes_lext(a4,d0.w),d0
	bsr	bits
	add.w	d0,d1

	; get and check distance
	move.l	a2,a5
	move.l	a3,a2
	bsr	decode
	move.l	a5,a2
	tst.w	d0
	bmi	.codes_done		; invalid symbol

	add.w	d0,d0
	move.w	codes_dists(a4,d0.w),d2
	add.w	#codes_dext,d0
	move.w	(a4,d0.w),d0
	bsr	bits
	add.w	d0,d2

	; copy length bytes from distance bytes back
	move.l	a1,a5
	sub.l	output,a5
	neg.w	d2
	move.w	d1,d7
	subq	#1,d7
	bmi.s	.codes_loop
.copy:	move.b	(a1,d2.w),(a1)+
	dbra	d7,.copy
	bra	.codes_loop

.codes_done:
	movem.l	(sp)+,d1-d2/a4-a5
	rts

	;  In: a0 = huffman
	;  In: a1 = length
	;  In: d0 = n
	; Out: d0 = left
construct:
	movem.l	d1-d3/d7/a2-a4,-(sp)
	moveq	#0,d2

	; count number of codes of each length
	move.l	huffman_count(a0),a2
	moveq	#MAXBITS,d7
.construct_loop1:
	clr.w	(a2)+
	dbra	d7,.construct_loop1

	; assumes lengths are within bounds
	move.l	huffman_count(a0),a2
	moveq	#0,d7
.construct_loop2:
	move.w	d7,d1
	add.w	d1,d1
	move.w	(a1,d1.w),d1
	add.w	d1,d1
	add.w	#1,(a2,d1.w)
	addq	#1,d7
	cmp.w	d0,d7
	bcs.s	.construct_loop2

	cmp.w	(a2),d0	; no codes!
	beq.s	.construct_done	; complete, but decode will fail

	; check for an over-subscribed or incomplete set of lengths
	moveq	#1,d2		; one possible code of zero length
	moveq	#2,d7
.construct_loop3:
	add.w	d2,d2		; one mor bit, double codes left
	sub.w	(a2,d7.w),d2	; deduct count from possible codes
	bmi	.construct_done		; over-subscribed--return negative
	addq	#2,d7		; left > 0 means incomplete
	cmp.w	#MAXBITS*2,d7
	ble	.construct_loop3

	; generate offsets into symbol table for each length for sorting
	lea	offs,a3
	clr.w	2(a3)
	moveq	#2,d7
.construct_loop4:
	move.w	(a3,d7.w),d3
	add.w	(a2,d7.w),d3
	move.w	d3,2(a3,d7.w)
	addq	#2,d7
	cmp.w	#MAXBITS*2,d7
	bcs.s	.construct_loop4

	; put symbols in table sorted by length, by symbol order within each length
	move.l	huffman_symbol(a0),a2
	moveq	#0,d7
.construct_loop5:
	move.w	d7,d3
	add.w	d3,d3
	move.w	(a1,d3.w),d3
	beq.s	.zero
	add.w	d3,d3
	move.w	(a3,d3.w),d1
	add.w	d1,d1
	move.w	d7,(a2,d1.w)
	add.w	#1,(a3,d3.w)
.zero:	addq	#1,d7
	cmp.w	d0,d7
	bcs.s	.construct_loop5

.construct_done:	; return zero for complete set, positive for incomplete set
	move.w	d2,d0
	movem.l	(sp)+,d1-d3/d7/a2-a4
	rts

	;  In: a0 = input buffer
	;  In: a2 = huffman
	; Out: d0 = symbol
decode:	movem.l	d1-d7/a3,-(sp)
	move.l	bitbuf,d0	; bitbuf
	move.l	bitcnt,d7	; left
	moveq	#0,d1		; code
	moveq	#0,d2		; first
	moveq	#0,d3		; count
	moveq	#0,d4		; index
	moveq	#1,d5		; len
	move.l	huffman_count(a2),a3	; next
	addq	#2,a3
	subq	#1,d7
	bmi.s	.decode_none_left
.decode_loop:
	move.l	d0,d6
	and.l	#1,d6
	or.l	d6,d1
	lsr.l	#1,d0
	move.w	(a3)+,d3
	move.w	d1,d6
	sub.w	d3,d6
	cmp.w	d2,d6
	bge.s	.update
	move.l	d0,bitbuf
	sub.l	d5,bitcnt
	and.l	#7,bitcnt

	sub.w	d2,d1
	add.w	d4,d1
	add.w	d1,d1
	move.l	huffman_symbol(a2),a3
	move.w	(a3,d1.w),d0
	bra.s	.decode_done

.update:
	add.w	d3,d4
	add.w	d3,d2
	add.w	d2,d2
	add.w	d1,d1
	addq	#1,d5
	dbra	d7,.decode_loop

.decode_none_left:
	move.w	#MAXBITS+1,d7
	sub.w	d5,d7
	bne.s	.ok1
	moveq	#-10,d0
	bra.s	.decode_done
.ok1:	move.b	(a0)+,d0
	cmp.w	#8,d7
	bls.s	.ok2
	moveq	#8,d7
.ok2:	subq	#1,d7
	bra.s	.decode_loop

.decode_done:
	movem.l	(sp)+,d1-d7/a3
	rts

		section	__MERGED,data

codes_tables:	; Size base for length codes 257..285
		dc.w	3
		dc.w	4
		dc.w	5
		dc.w	6
		dc.w	7
		dc.w	8
		dc.w	9
		dc.w	10
		dc.w	11
		dc.w	13
		dc.w	15
		dc.w	17
		dc.w	19
		dc.w	23
		dc.w	27
		dc.w	31
		dc.w	35
		dc.w	43
		dc.w	51
		dc.w	59
		dc.w	67
		dc.w	83
		dc.w	99
		dc.w	115
		dc.w	131
		dc.w	163
		dc.w	195
		dc.w	227
		dc.w	258
		; Extra bits for length codes 257..285
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	1
		dc.w	1
		dc.w	1
		dc.w	1
		dc.w	2
		dc.w	2
		dc.w	2
		dc.w	2
		dc.w	3
		dc.w	3
		dc.w	3
		dc.w	3
		dc.w	4
		dc.w	4
		dc.w	4
		dc.w	4
		dc.w	5
		dc.w	5
		dc.w	5
		dc.w	5
		dc.w	0
		; Offset base for distance codes 0..29
		dc.w	1
		dc.w	2
		dc.w	3
		dc.w	4
		dc.w	5
		dc.w	7
		dc.w	9
		dc.w	13
		dc.w	17
		dc.w	25
		dc.w	33
		dc.w	49
		dc.w	65
		dc.w	97
		dc.w	129
		dc.w	193
		dc.w	257
		dc.w	385
		dc.w	513
		dc.w	769
		dc.w	1025
		dc.w	1537
		dc.w	2049
		dc.w	3073
		dc.w	4097
		dc.w	6145
		dc.w	8193
		dc.w	12289
		dc.w	16385
		dc.w	24577
		; Extra bits for distance codes 0..29
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	1
		dc.w	1
		dc.w	2
		dc.w	2
		dc.w	3
		dc.w	3
		dc.w	4
		dc.w	4
		dc.w	5
		dc.w	5
		dc.w	6
		dc.w	6
		dc.w	7
		dc.w	7
		dc.w	8
		dc.w	8
		dc.w	9
		dc.w	9
		dc.w	10
		dc.w	10
		dc.w	11
		dc.w	11
		dc.w	12
		dc.w	12
		dc.w	13
		dc.w	13
		; permutation of code length codes
order: 		dc.b	16*2
		dc.b	17*2
		dc.b	18*2
		dc.b	0*2
		dc.b	8*2
		dc.b	7*2
		dc.b	9*2
		dc.b	6*2
		dc.b	10*2
		dc.b	5*2
		dc.b	11*2
		dc.b	4*2
		dc.b	12*2
		dc.b	3*2
		dc.b	13*2
		dc.b	2*2
		dc.b	14*2
		dc.b	1*2
		dc.b	15*2
virgin:		dc.b	1

		section	__MERGED,bss

output:		ds.l	1
bitbuf:		ds.l	1
bitcnt:		ds.l	1
lencode:	ds.l	2
distcode:	ds.l	2
codes_data:	ds.b	1328
offs:		ds.w	MAXBITS+1

	end
