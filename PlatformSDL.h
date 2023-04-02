#ifndef _PLATFORMSDL_H
#define _PLATFORMSDL_H

#define PlatformClass PlatformSDL

#include <SDL.h>
#include <SDL_image.h>
#include "Platform.h"

class PlatformSDL : public Platform {
public:
    PlatformSDL();
    virtual ~PlatformSDL();

    virtual uint8_t* standardControls() const;
    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual void chrout(uint8_t);
    virtual uint8_t readKeyboard();
    virtual void keyRepeat();
    virtual void clearKeyBuffer();
    virtual bool isKeyOrJoystickPressed(bool gamepad);
    virtual uint16_t readJoystick(bool gamepad);
    virtual void loadMap(Map map, uint8_t* destination);
    virtual uint8_t* loadTileset();
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
#ifdef PLATFORM_LIVE_MAP_SUPPORT
    virtual void renderLiveMap(uint8_t* map);
    virtual void renderLiveMapTile(uint8_t* map, uint8_t x, uint8_t y);
    virtual void renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots);
#endif
#ifdef PLATFORM_CURSOR_SUPPORT
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    virtual void setCursorShape(CursorShape shape);
#endif
#endif
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void shakeScreen();
#ifdef PLATFORM_FADE_SUPPORT
    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity, bool immediate);
    virtual void stopFadeScreen();
#endif
    virtual void writeToScreenMemory(address_t address, uint8_t value);
    virtual void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset);
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

private:
    uint32_t load(const char* filename, uint8_t* destination, uint32_t size);
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
    static void audioCallback(void* data, uint8_t* stream, int bytes);
    void (*interrupt)(void);
    SDL_AudioSpec audioSpec;
    SDL_AudioDeviceID audioDeviceID;
    SDL_Joystick* joystick;
    SDL_Window* window;
    SDL_Surface* windowSurface;
    SDL_Surface* bufferSurface;
    SDL_Surface* fadeSurface;
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
    SDL_Palette* palette;
#ifdef PLATFORM_SPRITE_SUPPORT
    SDL_Surface* spritesSurface;
#endif
#endif
#ifdef PLATFORM_CURSOR_SUPPORT
    SDL_Surface* cursorSurface;
    SDL_Rect cursorRect;
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    CursorShape cursorShape;
#endif
#endif
    int framesPerSecond_;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    uint8_t* moduleData;
    Module loadedModule;
    int8_t* sampleData;
    int8_t* soundExplosion;
    int8_t* soundMedkit;
    int8_t* soundEMP;
    int8_t* soundMagnet;
    int8_t* soundShock;
    int8_t* soundMove;
    int8_t* soundPlasma;
    int8_t* soundPistol;
    int8_t* soundItemFound;
    int8_t* soundError;
    int8_t* soundCycleWeapon;
    int8_t* soundCycleItem;
    int8_t* soundDoor;
    int8_t* soundMenuBeep;
    int8_t* soundShortBeep;
    int8_t* squareWave;
    uint8_t effectChannel;
#else
    float audioAngle;
    float audioFrequency;
    int16_t audioVolume;
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    Image loadedImage;
#endif
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
    uint32_t fadeBaseColor;
    uint16_t fadeIntensity;
    uint16_t joystickStateToReturn;
    uint16_t joystickState;
    uint16_t pendingState;
    uint8_t keyToReturn;
    uint8_t downKey;
    uint8_t shift;
};

#endif
