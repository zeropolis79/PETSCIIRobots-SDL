#ifndef _PLATFORMAMIGA_H
#define _PLATFORMAMIGA_H

#include "Platform.h"

struct BitMap;
struct Screen;
struct Window;
struct Interrupt;
struct IOAudio;
struct MsgPort;
struct BitMap;

class PlatformAmiga : public Platform {
public:
    PlatformAmiga();
    ~PlatformAmiga();

    virtual void setInterrupt(void (*interrupt)(void));
    virtual void show();
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t getin();
    virtual void clearKeyBuffer();
    virtual void load(const char* filename, uint8_t* destination, uint32_t size);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, bool transparent);
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
    virtual void stopShakeScreen();
	virtual void writeToScreenMemory(uint16_t address, uint8_t value);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void renderFrame();

private:
    __saveds void runVerticalBlankInterrupt();
    __asm static void verticalBlankInterruptServer();
    void (*interrupt)(void);
    int framesPerSecond_;
    BitMap* screenBitmap;
    Screen* screen;
    Window* window;
    Interrupt* verticalBlankInterrupt;
    IOAudio* ioAudio;
    MsgPort* messagePort;
    uint32_t clock;
    uint8_t* screenPlanes;
    uint8_t* tilesMask;
    BitMap* tilesBitMap;
    uint16_t bplcon1DefaultValue;
    uint16_t shakeStep;
    static uint16_t addressMap[];
    static uint8_t tileMaskMap[];
};

#endif
