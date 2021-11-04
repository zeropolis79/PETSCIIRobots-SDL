#ifndef _PLATFORMSDL_H
#define _PLATFORMSDL_H

#define PlatformClass PlatformSDL

#include <SDL.h>
#include <SDL_image.h>
#include "Platform.h"

class PlatformSDL : public Platform {
public:
    PlatformSDL();
    ~PlatformSDL();

    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t getin();
    virtual void clearKeyBuffer();
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
#ifndef PLATFORM_IMAGE_BASED_TILES
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
#endif
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent);
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
    virtual void writeToScreenMemory(uint16_t address, uint8_t value);
    virtual void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void renderFrame();

private:
    static void audioCallback(void* data, uint8_t* stream, int bytes);
    void (*interrupt)(void);
    SDL_AudioSpec audioSpec;
    SDL_AudioDeviceID audioDeviceID;
    SDL_Window* window;
    SDL_Surface* windowSurface;
    SDL_Surface* fontSurface;
#ifdef PLATFORM_IMAGE_BASED_TILES
    SDL_Surface* tileSurface;
#else
    SDL_Surface* tileSurfaces[256];
#endif
    int framesPerSecond_;
    float audioAngle;
    float audioFrequency;
    int16_t audioVolume;
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
};

#endif
