#ifndef _PLATFORMAMIGA_H
#define _PLATFORMAMIGA_H

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
    PlatformAmiga();
    ~PlatformAmiga();

    virtual uint8_t* standardControls() const;
    virtual void setInterrupt(void (*interrupt)(void));
    virtual void show();
    virtual int framesPerSecond();
    virtual uint8_t readKeyboard();
    virtual void keyRepeat();
    virtual void clearKeyBuffer();
    virtual bool isKeyOrJoystickPressed();
    virtual uint16_t readJoystick(bool gamepad);
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset);
    virtual uint8_t* loadTileset(const char* filename);
    virtual void displayImage(Image image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
#ifndef PLATFORM_IMAGE_BASED_TILES
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
#endif
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent);
    virtual void renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant);
#ifdef PLATFORM_IMAGE_SUPPORT
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void renderKey(uint8_t key, uint16_t x, uint16_t y);
    virtual void renderHealth(uint8_t health, uint16_t x, uint16_t y);
    virtual void renderFace(uint8_t face, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    virtual void renderLiveMap(uint8_t* map);
    virtual void renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots);
#endif
#ifdef PLATFORM_SPRITE_SUPPORT
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
#endif
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    virtual void shakeScreen();
    virtual void stopShakeScreen();
#endif
#ifdef PLATFORM_FADE_SUPPORT
    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity, bool immediate);
    virtual void stopFadeScreen();
#endif
    virtual void writeToScreenMemory(uint16_t address, uint8_t value);
    virtual void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset);
#ifdef PLATFORM_MODULE_BASED_AUDIO
    virtual void loadModule(Module module);
    virtual void playModule(Module module);
    virtual void pauseModule();
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
#else
    virtual void playNote(uint8_t note);
    virtual void stopNote();
#endif
    virtual void renderFrame(bool waitForNextFrame);
    virtual void waitForScreenMemoryAccess();

private:
    __saveds void runVerticalBlankInterrupt();
    __asm static void verticalBlankInterruptServer();
    __asm static int32_t ungzip(register __a0 void* input, register __a1 void* output);
    void (*interrupt)(void);
#ifdef PLATFORM_MODULE_BASED_AUDIO
    void undeltaSamples(uint8_t* module, uint32_t moduleSize);
    void setSampleData(uint8_t* module);
#endif
#ifdef PLATFORM_SPRITE_SUPPORT
    void renderSprite(uint8_t sprite, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_IMAGE_BASED_TILES
    void renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    __asm void renderLiveMapTiles(register __a1 uint8_t* map);
//    void renderLiveMapTiles(uint8_t* map);
#endif
    __asm static uint16_t readCD32Pad();
    __asm static void enableLowpassFilter();
    __asm static void disableLowpassFilter();
    int framesPerSecond_;
    uint32_t clock;
    BitMap* screenBitmap;
    Screen* screen;
    Window* window;
    Interrupt* verticalBlankInterrupt;
    uint8_t* chipMemory;
    uint8_t* screenPlanes;
#ifndef PLATFORM_IMAGE_BASED_TILES
    uint8_t* tilesPlanes;
#endif
    uint8_t* tilesMask;
    uint8_t* combinedTilePlanes;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    uint8_t* moduleData;
    Module loadedModule;
#endif
    IOAudio* ioAudio;
    MsgPort* messagePort;
    BitMap* tilesBitMap;
#ifdef PLATFORM_IMAGE_SUPPORT
    BitMap* facesBitMap;
    BitMap* itemsBitMap;
    BitMap* healthBitMap;
#endif
#ifdef PLATFORM_SPRITE_SUPPORT
    SimpleSprite* cursorSprite1;
    SimpleSprite* cursorSprite2;
#endif
    Palette* palette;
    uint16_t bplcon1DefaultValue;
    uint8_t shakeStep;
    uint8_t keyToReturn;
    uint8_t downKey;
    uint8_t shift;
    uint16_t joystickStateToReturn;
    uint16_t joystickState;
    bool filterState;
};

#endif
