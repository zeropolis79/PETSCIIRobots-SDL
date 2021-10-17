#ifndef _PLATFORMAMIGA_H
#define _PLATFORMAMIGA_H

#define PLATFORM_MODULE_BASED_AUDIO
#define PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
#define PLATFORM_IMAGE_SUPPORT
#define PLATFORM_COLOR_SUPPORT
#define PlatformClass PlatformAmiga

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
    PlatformAmiga(bool moduleBasedAudio = true);
    ~PlatformAmiga();

    virtual void setInterrupt(void (*interrupt)(void));
    virtual void show();
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t getin();
    virtual void clearKeyBuffer();
    virtual void load(const char* filename, uint8_t* destination, uint32_t size);
    virtual void displayImage(uint8_t image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, bool transparent);
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
    virtual void stopShakeScreen();
    virtual void writeToScreenMemory(uint16_t address, uint8_t value);
    virtual void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void playModule(uint8_t module);
    virtual void setSongPosition(uint8_t songPosition);
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
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
    BitMap* itemsBitMap;
    uint16_t bplcon1DefaultValue;
    uint16_t shakeStep;
    static uint16_t addressMap[];
    static uint8_t tileMaskMap[];
};

#endif
