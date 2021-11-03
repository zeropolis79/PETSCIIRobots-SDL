#ifndef _PLATFORMAMIGA_H
#define _PLATFORMAMIGA_H

#define PLATFORM_MODULE_BASED_AUDIO
#define PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
#define PLATFORM_IMAGE_SUPPORT
#define PLATFORM_COLOR_SUPPORT
#define PLATFORM_CURSOR_SUPPORT
#define PlatformClass PlatformAmiga

#include "Platform.h"

struct BitMap;
struct Screen;
struct Window;
struct Interrupt;
struct IOAudio;
struct MsgPort;
struct BitMap;
struct SimpleSprite;
class Palette;

class PlatformAmiga : public Platform {
public:
    PlatformAmiga(bool moduleBasedAudio = true);
    ~PlatformAmiga();

    virtual void setInterrupt(void (*interrupt)(void));
    virtual void show();
    virtual int framesPerSecond();
    virtual uint8_t getin();
    virtual void clearKeyBuffer();
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset);
    virtual void displayImage(Image image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent);
    virtual void renderSprite(uint8_t sprite, uint16_t x, uint16_t y);
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void renderHealth(uint8_t health, uint16_t x, uint16_t y);
    virtual void renderFace(uint8_t face, uint16_t x, uint16_t y);
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
    virtual void stopShakeScreen();
    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity);
    virtual void stopFadeScreen();
    virtual void writeToScreenMemory(uint16_t address, uint8_t value);
    virtual void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void playModule(Module module);
    virtual void setSongPosition(uint8_t songPosition);
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
    virtual void renderFrame();

private:
    __saveds void runVerticalBlankInterrupt();
    __asm static void verticalBlankInterruptServer();
    __asm static int32_t ungzip(register __a0 void* input, register __a1 void* output);
    void (*interrupt)(void);
    void setSampleData(uint8_t* module);
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
    BitMap* facesBitMap;
    BitMap* tilesBitMap;
    BitMap* spritesBitMap;
    BitMap* itemsBitMap;
    BitMap* healthBitMap;
    SimpleSprite* cursorSprite1;
    SimpleSprite* cursorSprite2;
    Palette* palette;
    uint8_t* moduleData;
    uint16_t bplcon1DefaultValue;
    uint16_t shakeStep;
    Module loadedModule;
};

#endif
