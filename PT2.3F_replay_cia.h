#ifndef _PT23F_REPLAY_CIA_H
#define _PT23F_REPLAY_CIA_H

#include "Platform.h"

__asm extern void SetCIAInt(void);
__asm extern void ResetCIAInt(void);
__asm extern void mt_init(register __a0 uint8_t* songData);
__asm extern void mt_music(void);
__asm extern void mt_end(void);
__far extern bool mt_Enable;
__far extern uint8_t mt_SongPos;
__far extern uint16_t mt_PatternPos;
__far extern uint16_t mt_chan4data[2];

#endif
