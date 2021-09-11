#ifndef _PLATFORMAMIGA_H
#define _PLATFORMAMIGA_H

#include "Platform.h"

struct Screen;
struct Window;
struct Interrupt;
struct IOAudio;
struct MsgPort;

class PlatformAmiga : public Platform {
public:
    PlatformAmiga();
    ~PlatformAmiga();

    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t getin();
    virtual void clearKeyBuffer();
    virtual void load(const char* filename, uint8_t* destination, uint32_t size);
	virtual void writeToScreenMemory(uint16_t address, uint8_t value);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void renderFrame();

private:
    __saveds void runVerticalBlankInterrupt();
    __asm static void verticalBlankInterruptServer();
    void (*interrupt)(void);
    int framesPerSecond_;
    Screen* screen;
    Window* window;
    Interrupt* verticalBlankInterrupt;
    IOAudio* ioAudio;
    MsgPort* messagePort;
    uint32_t clock;
    static uint16_t addressMap[40 * 25];
};

#endif
