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

    virtual uint8_t* standardControls() const;
    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t readKeyboard();
    virtual void clearKeyBuffer();
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset);
    virtual uint8_t* loadTileset(const char* filename);
#ifdef PLATFORM_IMAGE_SUPPORT
    virtual void displayImage(Image image);
#endif
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
#ifdef PLATFORM_CURSOR_SUPPORT
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
#endif
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
    virtual void writeToScreenMemory(address_t address, uint8_t value);
    virtual void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void renderFrame(bool waitForNextFrame);

private:
#ifdef PLATFORM_SPRITE_SUPPORT
    void renderSprite(uint8_t sprite, uint16_t x, uint16_t y);
#endif
#ifdef PLATFORM_IMAGE_BASED_TILES
    void renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y);
#endif
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
#ifdef PLATFORM_IMAGE_SUPPORT
    SDL_Surface* imageSurfaces[3];
    SDL_Surface* itemsSurface;
    SDL_Surface* keysSurface;
    SDL_Surface* healthSurface;
    SDL_Surface* facesSurface;
    SDL_Surface* animTilesSurface;
#ifdef PLATFORM_SPRITE_SUPPORT
    SDL_Surface* spritesSurface;
#endif
#endif
#ifdef PLATFORM_CURSOR_SUPPORT
    SDL_Surface* cursorSurface;
    SDL_Rect cursorRect;
#endif
    int framesPerSecond_;
    float audioAngle;
    float audioFrequency;
    int16_t audioVolume;
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
};

#endif
