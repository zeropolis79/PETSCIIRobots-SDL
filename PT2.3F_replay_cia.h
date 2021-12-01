#ifndef _PT23F_REPLAY_CIA_H
#define _PT23F_REPLAY_CIA_H

#include "Platform.h"

struct SampleData {
    char name[22];
    uint16_t length;
    int8_t finetune;
    uint8_t volume;
    uint16_t repeatPoint;
    uint16_t repeatLength;
};

struct ChanTemp {
    uint16_t note;
    uint16_t cmd;
    uint32_t start;
    uint16_t length;
    uint32_t loopstart;
    uint16_t replen;
    uint16_t period;
    uint8_t finetune;
    uint8_t volume;
    uint16_t dmabit;
    uint8_t toneportdirec;
    uint8_t toneportspeed;
    uint16_t wantedperiod;
    uint8_t vibratocmd;
    uint8_t vibratopos;
    uint8_t tremolocmd;
    uint8_t tremolopos;
    uint8_t wavecontrol;
    uint8_t glissfunk;
    uint8_t sampleoffset;
    uint8_t pattpos;
    uint8_t loopcount;
    uint8_t funkoffset;
    uint32_t wavestart;
    uint16_t padding;
};

struct ChanInput {
    uint16_t note;
    uint16_t cmd;
};

__asm extern void SetCIAInt(void);
__asm extern void ResetCIAInt(void);
__asm extern void mt_init(register __a0 uint8_t* songData);
__asm extern void mt_music(void);
__asm extern void mt_end(void);
__far extern ChanTemp mt_chan1temp;
__far extern ChanTemp mt_chan2temp;
__far extern ChanTemp mt_chan3temp;
__far extern ChanTemp mt_chan4temp;
__far extern int8_t* mt_SampleStarts[31];
__far extern uint8_t* mt_data;
__far extern uint8_t mt_speed;
__far extern uint8_t mt_SongPos;
__far extern bool mt_Enable;
__far extern uint16_t mt_PatternPos;
__far extern ChanInput mt_chan1input;
__far extern ChanInput mt_chan2input;
__far extern ChanInput mt_chan3input;
__far extern ChanInput mt_chan4input;

#endif
