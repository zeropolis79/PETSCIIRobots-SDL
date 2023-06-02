#include <cmath>
#include <algorithm>
#include "PT2.3A_replay_cia.h"

/**************************************************
 *    ----- Protracker V2.3A Playroutine -----    *
 **************************************************/

/*
 * CIA Version 1:
 * Call mt_init to initialize the song. To end the song and turn
 * off all voices, call mt_end.
 *
 * This playroutine is not very fast, optimized or well commented,
 * but all the new commands in PT2.3 should work.
 * If it's not good enough, you'll have to change it yourself.
 * We'll try to write a faster routine soon...
 *
 * Changes from V1.0C playroutine:
 * - Vibrato depth changed to be compatible with Noisetracker 2.0.
 *   You'll have to double all vib. depths on old PT modules.
 * - Funk Repeat changed to Invert Loop.
 * - Period set back earlier when stopping an effect.
 *
 * Converted from ProTracker_v2.3a_CIA.s by Vesuri/dA JoRMaS.
 */

void putLong(uint8_t* array, uint32_t offset, uint32_t value)
{
    array[offset] = (value & 0xff000000) >> 24;
    array[offset + 1] = (value & 0xff0000) >> 16;
    array[offset + 2] = (value & 0xff00) >> 8;
    array[offset + 3] = value & 0xff;
}

void putWord(uint8_t* array, uint32_t offset, uint32_t value)
{
    array[offset] = (value & 0xff00) >> 8;
    array[offset + 1] = value & 0xff;
}

uint32_t getLong(uint8_t* array, uint32_t offset)
{
    return (array[offset] << 24) | (array[offset + 1] << 16) | (array[offset + 2] << 8) | array[offset + 3];
}

uint16_t getWord(uint8_t* array, uint32_t offset) {
    return (array[offset] << 8) | array[offset + 1];
}

AudioChannel::AudioChannel(uint8_t id) :
    id(id),
    data(0),
    length(0),
    period(420),
    volume(0),
    dmaStart(0),
    dmaCurrent(0),
    dmaEnd(0),
    dmacon(false)
{
}

void AudioChannel::process(int16_t* buffer, uint32_t samples, uint32_t sampleRate, bool add) {
    if (!data || !dmacon) {
        if (!add) {
            for (uint32_t i = 0; i < samples; i++) {
                *buffer++ = 0;
            }
        }
        return;
    }

    float dmaPerSample = (float)7093789.2 / period / sampleRate / 2;

    if (add) {
        for (uint32_t i = 0; i < samples; i++) {
            *buffer++ += 32 * dmaStart[(int)dmaCurrent] * volume / 64;
            dmaCurrent += dmaPerSample;
            if (dmaCurrent >= dmaEnd) {
                dmaStart = data;
                dmaCurrent -= dmaEnd;
                dmaEnd = float(length * 2);
            }
        }
    } else {
        for (uint32_t i = 0; i < samples; i++) {
            *buffer++ = 32 * dmaStart[(int)dmaCurrent] * volume / 64;
            dmaCurrent += dmaPerSample;
            if (dmaCurrent >= dmaEnd) {
                dmaStart = data;
                dmaCurrent -= dmaEnd;
                dmaEnd = (float)(length * 2);
            }
        }
    }
}

void AudioChannel::start() {
    dmaStart = data;
    dmaCurrent = 0;
    dmaEnd = (float)(length * 2);
    dmacon = true;
}

void AudioChannel::stop() {
    dmacon = false;
}

// ---- CIA Interrupt ----

bool ciaapra = false;
float ciatar = 0;
float ciataw = 14187;
AudioChannel channel0(0);
AudioChannel channel1(1);
AudioChannel channel2(2);
AudioChannel channel3(3);
AudioChannel channel4(4);
AudioChannel channel5(5);
AudioChannel channel6(6);
AudioChannel channel7(7);
void processAudio(int16_t* outputBuffer, uint32_t outputLength, uint32_t sampleRate)
{
    float timerAdvancePerSample = (float)709378.92 / (float)sampleRate;

    int16_t *bufferPosition = outputBuffer;
    for (uint32_t samplesLeft = outputLength; samplesLeft > 0;) {
        // Number of samples to process before interrupt
        uint32_t samplesToProcess = MIN((uint32_t)(ciatar / timerAdvancePerSample) + 1, samplesLeft);

        // Process each audio channel
        channel0.process(bufferPosition, samplesToProcess, sampleRate, false);
        channel1.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel2.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel3.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel4.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel5.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel6.process(bufferPosition, samplesToProcess, sampleRate, true);
        channel7.process(bufferPosition, samplesToProcess, sampleRate, true);
        bufferPosition += samplesToProcess;

        // Run the vertical blank interrupt if required
        ciatar -= samplesToProcess * timerAdvancePerSample;
        if (ciatar < 0) {
            ciatar += ciataw;
            mt_music();
        }

        // Samples left
        samplesLeft -= samplesToProcess;
    }
};

// ---- Tempo ----

uint16_t RealTempo = 125;
uint32_t TimerValue = 1773447;

// ---- Playroutine ----

void mt_init(uint8_t* songData)
{
    mt_end();

    mt_SongDataPtr = songData;

    uint8_t lastPattern = 0;
    for (int position = 952; position < 952 + 128; position++) {
        uint8_t pattern = mt_SongDataPtr[position];
        if (pattern > lastPattern) {
            lastPattern = pattern;
        }
    }

    for (int sample = 0, sampleDataOffset = 20, sampleStart = ((lastPattern + 1) << 10) + 1084; sample < 15; sample++, sampleDataOffset += 30) {
        if (getWord(mt_SongDataPtr, sampleDataOffset + 28) == 0) {
            putWord(mt_SongDataPtr, sampleDataOffset + 28, 1);
        }
        if (getWord(mt_SongDataPtr, sampleDataOffset + 28) == 1) {
            putWord(mt_SongDataPtr, sampleStart, 0);
        }
        mt_SampleStarts[sample] = (int8_t*)(mt_SongDataPtr + sampleStart);
        sampleStart += getWord(mt_SongDataPtr, sampleDataOffset + 22) * 2;
    }

    ciaapra = true;
    RealTempo = 125;
    mt_speed = 6;
    mt_counter = 0;
    mt_SongPos = 0;
    mt_PatternPos = 0;

    ciataw = (float)std::floor(TimerValue / RealTempo);
    ciatar = ciataw;
}

void mt_end()
{
    mt_Enable = false;
    channel0.volume = 0;
    channel1.volume = 0;
    channel2.volume = 0;
    channel3.volume = 0;
    channel4.volume = 0;
    channel5.volume = 0;
    channel6.volume = 0;
    channel7.volume = 0;
    channel0.dmacon = false;
    channel1.dmacon = false;
    channel2.dmacon = false;
    channel3.dmacon = false;
    channel4.dmacon = false;
    channel5.dmacon = false;
    channel6.dmacon = false;
    channel7.dmacon = false;
}

void mt_start()
{
    mt_Enable = true;
}

void mt_music()
{
    if (!mt_Enable) {
        return;
    }
    mt_counter++;
    if (mt_counter < mt_speed) {
        mt_NoNewAllChannels();
        mt_NoNewPosYet();
    } else {
        mt_counter = 0;
        if (mt_PattDelTime2 == 0) {
            mt_GetNewNote();
        } else {
            mt_NoNewAllChannels();
        }

        mt_PatternPos += 16;
        if (mt_PattDelTime != 0) {
            mt_PattDelTime2 = mt_PattDelTime;
            mt_PattDelTime = 0;
        }
        if (mt_PattDelTime2 != 0) {
            mt_PattDelTime2 -= 1;
            if (mt_PattDelTime2 != 0) {
                mt_PatternPos -= 16;
            }
        }
        if (mt_PBreakFlag != 0) {
            mt_PBreakFlag = 0;
            mt_PatternPos = mt_PBreakPos << 4;
            mt_PBreakPos = 0;
        }
        if (mt_PatternPos < 1024) {
            mt_NoNewPosYet();
        } else {
            mt_NextPosition();
        }
    }
}

void mt_NoNewAllChannels()
{
    mt_CheckEfx(channel0, mt_chan1temp);
    mt_CheckEfx(channel1, mt_chan2temp);
    mt_CheckEfx(channel2, mt_chan3temp);
    mt_CheckEfx(channel3, mt_chan4temp);
    mt_CheckEfx(channel4, mt_chan5temp);
    mt_CheckEfx(channel5, mt_chan6temp);
    mt_CheckEfx(channel6, mt_chan7temp);
    mt_CheckEfx(channel7, mt_chan8temp);
}

void mt_GetNewNote()
{
    int pattpo = 952;
    int patternOffset = (mt_SongDataPtr[pattpo + mt_SongPos] << 10) + mt_PatternPos;
    mt_PlayVoice(channel0, mt_chan1temp, mt_SongDataPtr + 1084, patternOffset);
    mt_PlayVoice(channel1, mt_chan2temp, mt_SongDataPtr + 1084, patternOffset + 4);
    mt_PlayVoice(channel2, mt_chan3temp, mt_SongDataPtr + 1084, patternOffset + 8);
    mt_PlayVoice(channel3, mt_chan4temp, mt_SongDataPtr + 1084, patternOffset + 12);
    if (mt_chaninputs[0].note != 0) {
        mt_PlayVoice(channel4, mt_chan5temp, (uint8_t*)&mt_chaninputs[0], 0);
        mt_chaninputs[0].note = 0;
    }
    if (mt_chaninputs[1].note != 0) {
        mt_PlayVoice(channel5, mt_chan6temp, (uint8_t*)&mt_chaninputs[1], 0);
        mt_chaninputs[1].note = 0;
    }
    if (mt_chaninputs[2].note != 0) {
        mt_PlayVoice(channel6, mt_chan7temp, (uint8_t*)&mt_chaninputs[2], 0);
        mt_chaninputs[2].note = 0;
    }
    if (mt_chaninputs[3].note != 0) {
        mt_PlayVoice(channel7, mt_chan8temp, (uint8_t*)&mt_chaninputs[3], 0);
        mt_chaninputs[3].note = 0;
    }
    channel7.data = mt_chan8temp.n_loopstart;
    channel7.length = mt_chan8temp.n_replen;
    channel6.data = mt_chan7temp.n_loopstart;
    channel6.length = mt_chan7temp.n_replen;
    channel5.data = mt_chan6temp.n_loopstart;
    channel5.length = mt_chan6temp.n_replen;
    channel4.data = mt_chan5temp.n_loopstart;
    channel4.length = mt_chan5temp.n_replen;
    channel3.data = mt_chan4temp.n_loopstart;
    channel3.length = mt_chan4temp.n_replen;
    channel2.data = mt_chan3temp.n_loopstart;
    channel2.length = mt_chan3temp.n_replen;
    channel1.data = mt_chan2temp.n_loopstart;
    channel1.length = mt_chan2temp.n_replen;
    channel0.data = mt_chan1temp.n_loopstart;
    channel0.length = mt_chan1temp.n_replen;
}

void mt_PlayVoice(AudioChannel& channel, ChanTemp& mt_chantemp, uint8_t* patternData, int patternOffset)
{
    int sampledata = 12;
    if (mt_chantemp.n_note == 0 && mt_chantemp.n_cmd.word == 0) {
        mt_PerNop(channel, mt_chantemp);
    }
    mt_chantemp.n_note = getWord(patternData, patternOffset);
    mt_chantemp.n_cmd.word = getWord(patternData, patternOffset + 2);
    int instrument = ((mt_chantemp.n_cmd.word & 0xf000) >> 12) | ((mt_chantemp.n_note & 0xf000) >> 8);
    if (instrument != 0) {
        int instrumentOffset = sampledata + 30 * instrument;
        mt_chantemp.n_start = mt_SampleStarts[instrument - 1];
        mt_chantemp.n_length = getWord(mt_SongDataPtr, instrumentOffset);
        mt_chantemp.n_finetune = mt_SongDataPtr[instrumentOffset + 2];
        mt_chantemp.n_volume = mt_SongDataPtr[instrumentOffset + 3];

        // Get repeat
        int repeat = getWord(mt_SongDataPtr, instrumentOffset + 4);

        // Get start
        // Add repeat
        mt_chantemp.n_loopstart = mt_chantemp.n_start + 2 * repeat;
        mt_chantemp.n_wavestart = mt_chantemp.n_start + 2 * repeat;

        // Save replen
        mt_chantemp.n_replen = getWord(mt_SongDataPtr, instrumentOffset + 6);

        if (repeat != 0) {
            // Get repeat
            // Add replen
            mt_chantemp.n_length = repeat + mt_chantemp.n_replen;
        }

        // Set volume
        channel.volume = mt_chantemp.n_volume;
    }

    // If no note
    uint16_t setPeriod = (mt_chantemp.n_note & 0xfff) != 0;
    if (setPeriod) {
        if ((mt_chantemp.n_cmd.word & 0xff0) == 0xe50) {
            mt_SetFineTune(channel, mt_chantemp);
        } else if ((mt_chantemp.n_cmd.word & 0xf00) == 0x300 || (mt_chantemp.n_cmd.word & 0xf00) == 0x500) {
            // TonePortamento
            mt_SetTonePorta(channel, mt_chantemp);
            setPeriod = false;
        } else if ((mt_chantemp.n_cmd.word & 0xf00) == 0x900) {
            // Sample Offset
            mt_CheckMoreEfx(channel, mt_chantemp);
        }
    }

    if (setPeriod) {
        uint16_t note = mt_chantemp.n_note & 0xfff;

        int periodIndex = 0;
        for (; note < mt_PeriodTable[periodIndex] && periodIndex < 36; periodIndex++);
        mt_chantemp.n_period = mt_PeriodTable[mt_chantemp.n_finetune * 36 + periodIndex];

        // Notedelay
        if ((mt_chantemp.n_cmd.word & 0xff0) != 0x0ed0) {
            channel.stop();
            if ((mt_chantemp.n_wavecontrol & (1 << 2)) == 0) {
                mt_chantemp.n_vibratopos = 0;
            }
            if ((mt_chantemp.n_wavecontrol & (1 << 6)) == 0) {
                mt_chantemp.n_tremolopos = 0;
            }

            // Set start
            channel.data = mt_chantemp.n_start;

            // Set length
            channel.length = mt_chantemp.n_length;

            // Set period
            channel.period = mt_chantemp.n_period;
            channel.start();
        }
    }
    mt_CheckMoreEfx(channel, mt_chantemp);
}

void mt_NextPosition()
{
    mt_PatternPos = mt_PBreakPos << 4;
    mt_PBreakPos = 0;
    mt_PosJumpFlag = 0;
    mt_SongPos = (mt_SongPos + 1) & 0x7f;
    if (mt_SongPos >= mt_SongDataPtr[950]) {
        mt_SongPos = 0;
    }
}

void mt_NoNewPosYet()
{
    if (mt_PosJumpFlag != 0) {
        mt_NextPosition();
    }
}

void mt_CheckEfx(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_UpdateFunk(channel, mt_chantemp);
    if ((mt_chantemp.n_cmd.word & 0xfff) == 0) {
        mt_PerNop(channel, mt_chantemp);
        return;
    }
    switch (mt_chantemp.n_cmd.word & 0xf00) {
    case 0x000:
        return mt_Arpeggio(channel, mt_chantemp);
    case 0x100:
        return mt_PortaUp(channel, mt_chantemp);
    case 0x200:
        return mt_PortaDown(channel, mt_chantemp);
    case 0x300:
        return mt_TonePortamento(channel, mt_chantemp);
    case 0x400:
        return mt_Vibrato(channel, mt_chantemp);
    case 0x500:
        return mt_TonePlusVolSlide(channel, mt_chantemp);
    case 0x600:
        return mt_VibratoPlusVolSlide(channel, mt_chantemp);
    case 0xe00:
        return mt_E_Commands(channel, mt_chantemp);
    default:
        mt_PerNop(channel, mt_chantemp);
        if ((mt_chantemp.n_cmd.word & 0xf00) == 0x700) {
            mt_Tremolo(channel, mt_chantemp);
        } else if ((mt_chantemp.n_cmd.word & 0xf00) == 0xa00) {
            mt_VolumeSlide(channel, mt_chantemp);
        }
        break;
    }
}

void mt_PerNop(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    channel.period = mt_chantemp.n_period;
}

void mt_Arpeggio(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    int amount = 0;

    switch (mt_counter % 3) {
    case 0:
        channel.period = mt_chantemp.n_period;
        return;
    case 1:
        amount = mt_chantemp.n_cmd.byte[0] >> 4;
        break;
    default:
        amount = mt_chantemp.n_cmd.byte[0] & 0x0f;
        break;
    }

    int periodOffset = mt_chantemp.n_finetune * 36;
    int periodIndex = 0;
    for (; mt_chantemp.n_period < mt_PeriodTable[periodOffset + periodIndex] && periodIndex < 36; periodIndex++);
    if (periodIndex < 36) {
        channel.period = mt_PeriodTable[periodOffset + periodIndex + amount];
    }
}

void mt_FinePortaUp(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    mt_LowMask = 0x0f;
    mt_PortaUp(channel, mt_chantemp);
}

void mt_PortaUp(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    int amount = mt_chantemp.n_cmd.byte[0] & mt_LowMask;
    mt_LowMask = 0xff;
    mt_chantemp.n_period -= amount;
    if ((mt_chantemp.n_period & 0x0fff) < 113) {
        mt_chantemp.n_period &= 0xf000;
        mt_chantemp.n_period |= 113;
    }
    channel.period = mt_chantemp.n_period & 0x0fff;
}

void mt_FinePortaDown(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    mt_LowMask = 0x0f;
    mt_PortaDown(channel, mt_chantemp);
}

void mt_PortaDown(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    uint8_t amount = mt_chantemp.n_cmd.byte[0] & mt_LowMask;
    mt_LowMask = 0xff;
    mt_chantemp.n_period += amount;
    if ((mt_chantemp.n_period & 0x0fff) > 856) {
        mt_chantemp.n_period &= 0xf000;
        mt_chantemp.n_period |= 856;
    }
    channel.period = mt_chantemp.n_period & 0x0fff;
}

void mt_SetTonePorta(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    uint16_t note = mt_chantemp.n_note & 0xfff;

    // 37?
    int periodOffset = mt_chantemp.n_finetune * 36;
    int periodIndex = 0;
    for (; note < mt_PeriodTable[periodOffset + periodIndex] && periodIndex < 36; periodIndex++);
    periodIndex = std::min(periodIndex, 35);
    uint8_t finetune = mt_chantemp.n_finetune & 8;
    if (finetune != 0 && periodIndex != 0) {
        periodIndex--;
    }
    mt_chantemp.n_wantedperiod = mt_PeriodTable[periodOffset + periodIndex];
    mt_chantemp.n_toneportdirec = 0;
    if (mt_chantemp.n_wantedperiod == mt_chantemp.n_period) {
        mt_chantemp.n_wantedperiod = 0;
    } else if (mt_chantemp.n_wantedperiod < mt_chantemp.n_period) {
        mt_chantemp.n_toneportdirec = 1;
    }
}

void mt_TonePortamento(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_cmd.byte[0] != 0) {
        mt_chantemp.n_toneportspeed = mt_chantemp.n_cmd.byte[0];
        mt_chantemp.n_cmd.byte[0] = 0;
    }
    mt_TonePortNoChange(channel, mt_chantemp);
}

void mt_TonePortNoChange(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_wantedperiod == 0) {
        return;
    }
    if (mt_chantemp.n_toneportdirec == 0) {
        mt_chantemp.n_period += mt_chantemp.n_toneportspeed;
        if (mt_chantemp.n_wantedperiod <= mt_chantemp.n_period) {
            mt_chantemp.n_period = mt_chantemp.n_wantedperiod;
            mt_chantemp.n_wantedperiod = 0;
        }
    } else {
        mt_chantemp.n_period -= mt_chantemp.n_toneportspeed;
        if (mt_chantemp.n_wantedperiod >= mt_chantemp.n_period) {
            mt_chantemp.n_period = mt_chantemp.n_wantedperiod;
            mt_chantemp.n_wantedperiod = 0;
        }
    }
    int16_t period = mt_chantemp.n_period;
    if ((mt_chantemp.n_glissfunk & 0x0f) != 0) {
        int periodOffset = mt_chantemp.n_finetune * 36;
        int periodIndex = 0;
        for (; period < mt_PeriodTable[periodOffset + periodIndex] && periodIndex < 36; periodIndex++);
        periodIndex = std::min(periodIndex, 35);
        period = mt_PeriodTable[periodOffset + periodIndex];
    }

    // Set period
    channel.period = period;
}

void mt_Vibrato(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_cmd.byte[0] != 0) {
        uint8_t depth = mt_chantemp.n_cmd.byte[0] & 0x0f;
        if (depth != 0) {
            mt_chantemp.n_vibratocmd &= 0xf0;
            mt_chantemp.n_vibratocmd |= depth;
        }
        uint8_t speed = mt_chantemp.n_cmd.byte[0] & 0xf0;
        if (speed != 0) {
            mt_chantemp.n_vibratocmd &= 0x0f;
            mt_chantemp.n_vibratocmd |= speed;
        }
    }
    mt_Vibrato2(channel, mt_chantemp);
}

void mt_Vibrato2(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    uint8_t pos = (mt_chantemp.n_vibratopos >> 2) & 0x1f;
    int value = 255;
    switch (mt_chantemp.n_wavecontrol & 0x03) {
    case 0:
        value = mt_VibratoTable[pos];
        break;
    case 1:
        pos <<= 3;
        value = mt_chantemp.n_vibratopos >= 0 ? pos : 255 - pos;
        break;
    default:
        break;
    }
    uint8_t depth = mt_chantemp.n_vibratocmd & 0x0f;
    value = (value * depth) / 128;
    channel.period =
        mt_chantemp.n_period + (mt_chantemp.n_vibratopos >= 0 ? value : -value);
    mt_chantemp.n_vibratopos =
        (mt_chantemp.n_vibratopos + ((mt_chantemp.n_vibratocmd >> 2) & 0x003c)) &
        0xff;
}

void mt_TonePlusVolSlide(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_TonePortNoChange(channel, mt_chantemp);
    mt_VolumeSlide(channel, mt_chantemp);
}

void mt_VibratoPlusVolSlide(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_Vibrato2(channel, mt_chantemp);
    mt_VolumeSlide(channel, mt_chantemp);
}

void mt_Tremolo(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_cmd.byte[0] != 0) {
        uint8_t depth = mt_chantemp.n_cmd.byte[0] & 0x0f;
        if (depth != 0) {
            mt_chantemp.n_tremolocmd &= 0xf0;
            mt_chantemp.n_tremolocmd |= depth;
        }
        uint8_t speed = mt_chantemp.n_cmd.byte[0] & 0xf0;
        if (speed != 0) {
            mt_chantemp.n_tremolocmd &= 0x0f;
            mt_chantemp.n_tremolocmd |= speed;
        }
    }
    uint8_t pos = (mt_chantemp.n_tremolopos >> 2) & 0x1f;
    int value = 255;
    switch ((mt_chantemp.n_wavecontrol >> 4) & 0x03) {
    case 0:
        value = mt_VibratoTable[pos];
        break;
    case 1:
        pos <<= 3;
        value = mt_chantemp.n_vibratopos >= 0 ? pos : 255 - pos;
        break;
    default:
        break;
    }
    uint8_t depth = mt_chantemp.n_tremolocmd & 0x0f;
    value = (value * depth) / 64;
    channel.volume = std::min(
        0x40,
        std::max(
            0,
            mt_chantemp.n_volume + (mt_chantemp.n_tremolopos < 128 ? value : -value)
            )
        );
    mt_chantemp.n_tremolopos =
    (mt_chantemp.n_tremolopos + ((mt_chantemp.n_tremolocmd >> 2) & 0x003c)) &
    0xff;
}

void mt_SampleOffset(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_cmd.byte[0] != 0) {
        mt_chantemp.n_sampleoffset = mt_chantemp.n_cmd.byte[0];
    }
    int offset = mt_chantemp.n_sampleoffset << 7;
    if (offset >= mt_chantemp.n_length) {
        mt_chantemp.n_length = 0x0001;
    } else {
        mt_chantemp.n_length -= offset;
        mt_chantemp.n_start += 2 * offset;
    }
}

void mt_VolumeSlide(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    int amount = mt_chantemp.n_cmd.byte[0] >> 4;
    if (amount == 0) {
        mt_VolSlideDown(channel, mt_chantemp, mt_chantemp.n_cmd.byte[0] & 0x0f);
    } else {
        mt_VolSlideUp(channel, mt_chantemp, amount);
    }
}

void mt_VolSlideUp(AudioChannel& channel, ChanTemp& mt_chantemp, int amount)
{
    mt_chantemp.n_volume += amount;
    if (mt_chantemp.n_volume > 64) {
        mt_chantemp.n_volume = 0x40;
    }
    channel.volume = mt_chantemp.n_volume;
}

void mt_VolSlideDown(AudioChannel& channel, ChanTemp& mt_chantemp, int amount)
{
    mt_chantemp.n_volume -= amount;
    if (mt_chantemp.n_volume < 0) {
        mt_chantemp.n_volume = 0;
    }
    channel.volume = mt_chantemp.n_volume;
}

void mt_PositionJump(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_SongPos = mt_chantemp.n_cmd.byte[0] - 1;
    mt_PBreakPos = 0;
    mt_PosJumpFlag = 0xff;
}

void mt_VolumeChange(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_chantemp.n_volume = std::min(mt_chantemp.n_cmd.byte[0], (uint8_t)0x40);
    channel.volume = mt_chantemp.n_volume;
}

void mt_PatternBreak(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    int position = (mt_chantemp.n_cmd.byte[0] >> 4) * 10 + (mt_chantemp.n_cmd.byte[0] & 0x0f);
    mt_PBreakPos = position <= 63 ? position : 0;
    mt_PosJumpFlag = 0xff;
}

void mt_SetSpeed(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_chantemp.n_cmd.byte[0] == 0) {
        mt_end();
    } else if (mt_chantemp.n_cmd.byte[0] >= 32) {
        RealTempo = mt_chantemp.n_cmd.byte[0];
        ciataw = (float)std::floor(TimerValue / RealTempo);
    } else {
        mt_counter = 0;
        mt_speed = mt_chantemp.n_cmd.byte[0];
    }
}

void mt_CheckMoreEfx(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_UpdateFunk(channel, mt_chantemp);
    switch (mt_chantemp.n_cmd.word & 0xf00) {
    case 0x900:
        return mt_SampleOffset(channel, mt_chantemp);
    case 0xb00:
        return mt_PositionJump(channel, mt_chantemp);
    case 0xd00:
        return mt_PatternBreak(channel, mt_chantemp);
    case 0xe00:
        return mt_E_Commands(channel, mt_chantemp);
    case 0xf00:
        return mt_SetSpeed(channel, mt_chantemp);
    case 0xc00:
        return mt_VolumeChange(channel, mt_chantemp);
    default:
        return mt_PerNop(channel, mt_chantemp);
    }
}

void mt_E_Commands(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    switch (mt_chantemp.n_cmd.byte[0] & 0xf0) {
    case 0x00:
        return mt_FilterOnOff(channel, mt_chantemp);
    case 0x10:
        return mt_FinePortaUp(channel, mt_chantemp);
    case 0x20:
        return mt_FinePortaDown(channel, mt_chantemp);
    case 0x30:
        return mt_SetGlissControl(channel, mt_chantemp);
    case 0x40:
        return mt_SetVibratoControl(channel, mt_chantemp);
    case 0x50:
        return mt_SetFineTune(channel, mt_chantemp);
    case 0x60:
        return mt_JumpLoop(channel, mt_chantemp);
    case 0x70:
        return mt_SetTremoloControl(channel, mt_chantemp);
    case 0x90:
        return mt_RetrigNote(channel, mt_chantemp);
    case 0xa0:
        return mt_VolumeFineUp(channel, mt_chantemp);
    case 0xb0:
        return mt_VolumeFineDown(channel, mt_chantemp);
    case 0xc0:
        return mt_NoteCut(channel, mt_chantemp);
    case 0xd0:
        return mt_NoteDelay(channel, mt_chantemp);
    case 0xe0:
        return mt_PatternDelay(channel, mt_chantemp);
    case 0xf0:
        return mt_FunkIt(channel, mt_chantemp);
    default:
        break;
    }
}

void mt_FilterOnOff(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    ciaapra = !!mt_chantemp.n_cmd.byte[0];
}

void mt_SetGlissControl(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_chantemp.n_glissfunk &= 0xf0;
    mt_chantemp.n_glissfunk |= mt_chantemp.n_cmd.byte[0] & 0x0f;
}

void mt_SetVibratoControl(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_chantemp.n_wavecontrol &= 0xf0;
    mt_chantemp.n_wavecontrol |= mt_chantemp.n_cmd.byte[0] & 0x0f;
}

void mt_SetFineTune(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_chantemp.n_finetune = mt_chantemp.n_cmd.byte[0] & 0x0f;
}

void mt_JumpLoop(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    uint8_t count = mt_chantemp.n_cmd.byte[0] & 0x0f;
    if (count != 0) {
        if (mt_chantemp.n_loopcount > 0) {
            mt_chantemp.n_loopcount--;
            if (mt_chantemp.n_loopcount == 0) {
                return;
            }
        } else {
            mt_chantemp.n_loopcount = count;
        }
        mt_PBreakPos = mt_chantemp.n_pattpos;
        mt_PBreakFlag = 0xff;
    } else {
        mt_chantemp.n_pattpos = mt_PatternPos >> 4;
    }
}

void mt_SetTremoloControl(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    mt_chantemp.n_wavecontrol &= 0x0f;
    mt_chantemp.n_wavecontrol |= (mt_chantemp.n_cmd.byte[0] & 0x0f) << 4;
}

void mt_RetrigNote(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    uint8_t tick = mt_chantemp.n_cmd.byte[0] & 0x0f;
    if (tick == 0) {
        return;
    }
    if (mt_counter == 0) {
        if ((mt_chantemp.n_note & 0x0fff) != 0) {
            return;
        }
    }
    if (mt_counter % tick != 0) {
        return;
    }
    mt_DoRetrig(channel, mt_chantemp);
}

void mt_DoRetrig(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    // Channel DMA off
    channel.stop();

    // Set sampledata pointer
    channel.data = mt_chantemp.n_start;

    // Set length
    channel.length = mt_chantemp.n_length;

    channel.start();

    channel.data = mt_chantemp.n_loopstart;
    channel.length = mt_chantemp.n_replen;
    channel.period = mt_chantemp.n_period;
}

void mt_VolumeFineUp(AudioChannel& channel, ChanTemp& mt_chantemp) 
{
    if (mt_counter != 0) {
        return;
    }
    mt_VolSlideUp(channel, mt_chantemp, mt_chantemp.n_cmd.byte[0] & 0x0f);
}

void mt_VolumeFineDown(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    mt_VolSlideDown(channel, mt_chantemp, mt_chantemp.n_cmd.byte[0] & 0x0f);
}

void mt_NoteCut(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != (mt_chantemp.n_cmd.byte[0] & 0x0f)) {
        return;
    }
    mt_chantemp.n_volume = 0;
    channel.volume = 0;
}

void mt_NoteDelay(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != (mt_chantemp.n_cmd.byte[0] & 0x0f)) {
        return;
    }
    if (mt_chantemp.n_note == 0) {
        return;
    }
    mt_DoRetrig(channel, mt_chantemp);
}

void mt_PatternDelay(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    if (mt_PattDelTime2 == 0) {
        mt_PattDelTime = (mt_chantemp.n_cmd.byte[0] & 0x0f) + 1;
    }
}

void mt_FunkIt(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    if (mt_counter != 0) {
        return;
    }
    uint8_t amount = (mt_chantemp.n_cmd.byte[0] & 0x0f) << 4;
    mt_chantemp.n_glissfunk &= 0x0f;
    mt_chantemp.n_glissfunk |= amount;
    if (amount != 0) {
        mt_UpdateFunk(channel, mt_chantemp);
    }
}

void mt_UpdateFunk(AudioChannel& channel, ChanTemp& mt_chantemp)
{
    int speed = mt_chantemp.n_glissfunk >> 4;
    if (speed == 0) {
        return;
    }
    uint8_t funk = mt_FunkTable[speed];
    mt_chantemp.n_funkoffset += funk;
    if ((mt_chantemp.n_funkoffset & (1 << 7)) != 0) {
        return;
    }
    mt_chantemp.n_funkoffset = 0;
    int8_t* offset = mt_chantemp.n_loopstart + mt_chantemp.n_replen * 2;
    mt_chantemp.n_wavestart++;
    if (mt_chantemp.n_wavestart >= offset) {
        mt_chantemp.n_wavestart = mt_chantemp.n_loopstart;
    }
    *mt_chantemp.n_wavestart = -1 - *mt_chantemp.n_wavestart;
}

uint8_t mt_FunkTable[] = {
    0,
    5,
    6,
    7,
    8,
    10,
    11,
    13,
    16,
    19,
    22,
    26,
    32,
    43,
    64,
    128
};

uint8_t mt_VibratoTable[] = {
    0,
    24,
    49,
    74,
    97,
    120,
    141,
    161,
    180,
    197,
    212,
    224,
    235,
    244,
    250,
    253,
    255,
    253,
    250,
    244,
    235,
    224,
    212,
    197,
    180,
    161,
    141,
    120,
    97,
    74,
    49,
    24
};

uint16_t mt_PeriodTable[] = {
    // Tuning 0, Normal
    856,
    808,
    762,
    720,
    678,
    640,
    604,
    570,
    538,
    508,
    480,
    453,
    428,
    404,
    381,
    360,
    339,
    320,
    302,
    285,
    269,
    254,
    240,
    226,
    214,
    202,
    190,
    180,
    170,
    160,
    151,
    143,
    135,
    127,
    120,
    113,
    // Tuning 1
    850,
    802,
    757,
    715,
    674,
    637,
    601,
    567,
    535,
    505,
    477,
    450,
    425,
    401,
    379,
    357,
    337,
    318,
    300,
    284,
    268,
    253,
    239,
    225,
    213,
    201,
    189,
    179,
    169,
    159,
    150,
    142,
    134,
    126,
    119,
    113,
    // Tuning 2
    844,
    796,
    752,
    709,
    670,
    632,
    597,
    563,
    532,
    502,
    474,
    447,
    422,
    398,
    376,
    355,
    335,
    316,
    298,
    282,
    266,
    251,
    237,
    224,
    211,
    199,
    188,
    177,
    167,
    158,
    149,
    141,
    133,
    125,
    118,
    112,
    // Tuning 3
    838,
    791,
    746,
    704,
    665,
    628,
    592,
    559,
    528,
    498,
    470,
    444,
    419,
    395,
    373,
    352,
    332,
    314,
    296,
    280,
    264,
    249,
    235,
    222,
    209,
    198,
    187,
    176,
    166,
    157,
    148,
    140,
    132,
    125,
    118,
    111,
    // Tuning 4
    832,
    785,
    741,
    699,
    660,
    623,
    588,
    555,
    524,
    495,
    467,
    441,
    416,
    392,
    370,
    350,
    330,
    312,
    294,
    278,
    262,
    247,
    233,
    220,
    208,
    196,
    185,
    175,
    165,
    156,
    147,
    139,
    131,
    124,
    117,
    110,
    // Tuning 5
    826,
    779,
    736,
    694,
    655,
    619,
    584,
    551,
    520,
    491,
    463,
    437,
    413,
    390,
    368,
    347,
    328,
    309,
    292,
    276,
    260,
    245,
    232,
    219,
    206,
    195,
    184,
    174,
    164,
    155,
    146,
    138,
    130,
    123,
    116,
    109,
    // Tuning 6
    820,
    774,
    730,
    689,
    651,
    614,
    580,
    547,
    516,
    487,
    460,
    434,
    410,
    387,
    365,
    345,
    325,
    307,
    290,
    274,
    258,
    244,
    230,
    217,
    205,
    193,
    183,
    172,
    163,
    154,
    145,
    137,
    129,
    122,
    115,
    109,
    // Tuning 7
    814,
    768,
    725,
    684,
    646,
    610,
    575,
    543,
    513,
    484,
    457,
    431,
    407,
    384,
    363,
    342,
    323,
    305,
    288,
    272,
    256,
    242,
    228,
    216,
    204,
    192,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    114,
    108,
    // Tuning -8
    907,
    856,
    808,
    762,
    720,
    678,
    640,
    604,
    570,
    538,
    508,
    480,
    453,
    428,
    404,
    381,
    360,
    339,
    320,
    302,
    285,
    269,
    254,
    240,
    226,
    214,
    202,
    190,
    180,
    170,
    160,
    151,
    143,
    135,
    127,
    120,
    // Tuning -7
    900,
    850,
    802,
    757,
    715,
    675,
    636,
    601,
    567,
    535,
    505,
    477,
    450,
    425,
    401,
    379,
    357,
    337,
    318,
    300,
    284,
    268,
    253,
    238,
    225,
    212,
    200,
    189,
    179,
    169,
    159,
    150,
    142,
    134,
    126,
    119,
    // Tuning -6
    894,
    844,
    796,
    752,
    709,
    670,
    632,
    597,
    563,
    532,
    502,
    474,
    447,
    422,
    398,
    376,
    355,
    335,
    316,
    298,
    282,
    266,
    251,
    237,
    223,
    211,
    199,
    188,
    177,
    167,
    158,
    149,
    141,
    133,
    125,
    118,
    // Tuning -5
    887,
    838,
    791,
    746,
    704,
    665,
    628,
    592,
    559,
    528,
    498,
    470,
    444,
    419,
    395,
    373,
    352,
    332,
    314,
    296,
    280,
    264,
    249,
    235,
    222,
    209,
    198,
    187,
    176,
    166,
    157,
    148,
    140,
    132,
    125,
    118,
    // Tuning -4
    881,
    832,
    785,
    741,
    699,
    660,
    623,
    588,
    555,
    524,
    494,
    467,
    441,
    416,
    392,
    370,
    350,
    330,
    312,
    294,
    278,
    262,
    247,
    233,
    220,
    208,
    196,
    185,
    175,
    165,
    156,
    147,
    139,
    131,
    123,
    117,
    // Tuning -3
    875,
    826,
    779,
    736,
    694,
    655,
    619,
    584,
    551,
    520,
    491,
    463,
    437,
    413,
    390,
    368,
    347,
    328,
    309,
    292,
    276,
    260,
    245,
    232,
    219,
    206,
    195,
    184,
    174,
    164,
    155,
    146,
    138,
    130,
    123,
    116,
    // Tuning -2
    868,
    820,
    774,
    730,
    689,
    651,
    614,
    580,
    547,
    516,
    487,
    460,
    434,
    410,
    387,
    365,
    345,
    325,
    307,
    290,
    274,
    258,
    244,
    230,
    217,
    205,
    193,
    183,
    172,
    163,
    154,
    145,
    137,
    129,
    122,
    115,
    // Tuning -1
    862,
    814,
    768,
    725,
    684,
    646,
    610,
    575,
    543,
    513,
    484,
    457,
    431,
    407,
    384,
    363,
    342,
    323,
    305,
    288,
    272,
    256,
    242,
    228,
    216,
    203,
    192,
    181,
    171,
    161,
    152,
    144,
    136,
    128,
    121,
    114
};

ChanTemp mt_chan1temp;
ChanTemp mt_chan2temp;
ChanTemp mt_chan3temp;
ChanTemp mt_chan4temp;
ChanTemp mt_chan5temp;
ChanTemp mt_chan6temp;
ChanTemp mt_chan7temp;
ChanTemp mt_chan8temp;

int8_t* mt_SampleStarts[31] = {0};

uint8_t* mt_SongDataPtr = 0;
uint8_t mt_speed = 6;
uint8_t mt_counter = 0;
uint8_t mt_SongPos = 0;
uint8_t mt_PBreakPos = 0;
uint8_t mt_PosJumpFlag = 0;
uint8_t mt_PBreakFlag = 0;
uint8_t mt_LowMask = 0;
uint8_t mt_PattDelTime = 0;
uint8_t mt_PattDelTime2 = 0;
bool mt_Enable = false;
uint16_t mt_PatternPos = 0;
ChanInput mt_chaninputs[4];
/* End of File */
