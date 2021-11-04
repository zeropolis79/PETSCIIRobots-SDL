#ifndef _PLATFORM_H
#define _PLATFORM_H

#if __cplusplus < 201103L
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned char bool;
#define true 1
#define false 0
#define INT32_MAX 0x7fffffff
#define INT32_MIN 0x80000000
#define INT16_MAX 0x7fff
#define INT16_MIN 0x8000
#define INT8_MAX 0x7f
#define INT8_MIN 0x80
#else
#include <cstdint>
#endif

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))

class Platform {
public:
    Platform();

    enum Image {
        ImageIntro,
        ImageGame
    };

    enum Module {
        ModuleSoundFX,
        ModuleIntro,
        ModuleWin,
        ModuleLose,
        ModuleInGame1,
        ModuleInGame2,
        ModuleInGame3,
        ModuleInGame4
    };

    virtual void setInterrupt(void (*interrupt)(void)) = 0;
    virtual void show();
    virtual int framesPerSecond() = 0;
    virtual void chrout(uint8_t);
    virtual uint8_t getin() = 0;
    virtual void clearKeyBuffer() = 0;
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset = 0) = 0;
    virtual void displayImage(Image image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes) = 0;
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant = 0, bool transparent = false) = 0;
    virtual void renderSprite(uint8_t sprite, uint16_t x, uint16_t y);
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void renderHealth(uint8_t health, uint16_t x, uint16_t y);
    virtual void renderFace(uint8_t face, uint16_t x, uint16_t y);
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height) = 0;
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
    virtual void startShakeScreen();
    virtual void shakeScreen() = 0;
    virtual void stopShakeScreen();
    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity);
    virtual void stopFadeScreen();
    virtual void writeToScreenMemory(uint16_t address, uint8_t value) = 0;
    virtual void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset) = 0;
    virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void loadModule(Module module);
    virtual void playModule(Module module);
    virtual void setSongPosition(uint8_t songPosition);
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
    virtual void renderFrame();
    bool quit;
};

extern Platform* platform;

#endif
