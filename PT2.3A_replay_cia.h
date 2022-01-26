#ifndef _PT23A_REPLAY_CIA_H
#define _PT23A_REPLAY_CIA_H

#include "Platform.h"

struct AudioChannel {
public:
    AudioChannel(uint8_t id);
    void process(int16_t* buffer, uint32_t samples, uint32_t sampleRate, bool add);
    void start();
    void stop();

    uint8_t id;
    int8_t* data;
    uint16_t length;
    uint16_t period;
    uint16_t volume;
    int8_t* dmaStart;
    float dmaCurrent;
    float dmaEnd;
    bool dmacon;
};

struct SampleData {
    char name[22];
    uint16_t length;
    int8_t finetune;
    uint8_t volume;
    uint16_t repeatPoint;
    uint16_t repeatLength;
};

union Cmd {
    uint16_t word;
    uint8_t byte[2];
};

struct ChanTemp {
    uint16_t n_note;
    Cmd n_cmd;
    int8_t* n_start;
    uint16_t n_length;
    int8_t* n_loopstart;
    uint16_t n_replen;
    int16_t n_period;
    uint8_t n_finetune;
    int8_t n_volume;
    uint16_t n_dmabit;
    uint8_t n_toneportdirec;
    uint8_t n_toneportspeed;
    uint16_t n_wantedperiod;
    uint8_t n_vibratocmd;
    int8_t n_vibratopos;
    uint8_t n_tremolocmd;
    uint8_t n_tremolopos;
    uint8_t n_wavecontrol;
    uint8_t n_glissfunk;
    uint8_t n_sampleoffset;
    uint8_t n_pattpos;
    uint8_t n_loopcount;
    uint8_t n_funkoffset;
    int8_t* n_wavestart;
    uint16_t n_padding;
};

struct ChanInput {
    uint16_t note;
    uint16_t cmd;
};

void putLong(uint8_t* array, uint32_t offset, uint32_t value);
void putWord(uint8_t* array, uint32_t offset, uint32_t value);
uint32_t getLong(uint8_t* array, uint32_t offset);
uint16_t getWord(uint8_t* array, uint32_t offset);

extern AudioChannel channel0;
extern AudioChannel channel1;
extern AudioChannel channel2;
extern AudioChannel channel3;
extern AudioChannel channel4;
extern AudioChannel channel5;
extern AudioChannel channel6;
extern AudioChannel channel7;
extern void processAudio(int16_t* outputBuffer, uint32_t outputLength, uint32_t sampleRate);

extern void mt_init(uint8_t* songData);
extern void mt_music();
extern void mt_end();
extern void mt_start();
extern void mt_music();
extern void mt_NoNewAllChannels();
extern void mt_GetNewNote();
extern void mt_PlayVoice(AudioChannel& channel, ChanTemp& mt_chantemp, uint8_t* patternData, int patternOffset);
extern void mt_NextPosition();
extern void mt_NoNewPosYet();
extern void mt_CheckEfx(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_PerNop(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_Arpeggio(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_FinePortaUp(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_PortaUp(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_FinePortaDown(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_PortaDown(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetTonePorta(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_TonePortamento(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_TonePortNoChange(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_Vibrato(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_Vibrato2(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_TonePlusVolSlide(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_VibratoPlusVolSlide(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_Tremolo(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SampleOffset(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_VolumeSlide(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_VolSlideUp(AudioChannel& channel, ChanTemp& mt_chantemp, int amount);
extern void mt_VolSlideDown(AudioChannel& channel, ChanTemp& mt_chantemp, int amount);
extern void mt_PositionJump(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_VolumeChange(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_PatternBreak(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetSpeed(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_CheckMoreEfx(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_E_Commands(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_FilterOnOff(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetGlissControl(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetVibratoControl(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetFineTune(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_JumpLoop(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_SetTremoloControl(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_RetrigNote(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_DoRetrig(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_VolumeFineUp(AudioChannel& channel, ChanTemp& mt_chantemp); 
extern void mt_VolumeFineDown(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_NoteCut(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_NoteDelay(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_PatternDelay(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_FunkIt(AudioChannel& channel, ChanTemp& mt_chantemp);
extern void mt_UpdateFunk(AudioChannel& channel, ChanTemp& mt_chantemp);

extern uint8_t mt_FunkTable[];
extern uint8_t mt_VibratoTable[];
extern uint16_t mt_PeriodTable[];

extern ChanTemp mt_chan1temp;
extern ChanTemp mt_chan2temp;
extern ChanTemp mt_chan3temp;
extern ChanTemp mt_chan4temp;
extern ChanTemp mt_chan5temp;
extern ChanTemp mt_chan6temp;
extern ChanTemp mt_chan7temp;
extern ChanTemp mt_chan8temp;
extern int8_t* mt_SampleStarts[31];
extern uint8_t* mt_SongDataPtr;
extern uint8_t mt_speed;
extern uint8_t mt_counter;
extern uint8_t mt_SongPos;
extern uint8_t mt_PBreakPos;
extern uint8_t mt_PosJumpFlag;
extern uint8_t mt_PBreakFlag;
extern uint8_t mt_LowMask;
extern uint8_t mt_PattDelTime;
extern uint8_t mt_PattDelTime2;
extern bool mt_Enable;
extern uint16_t mt_PatternPos;
extern ChanInput mt_chaninputs[4];

#endif
