#ifndef _PLATFORMSDL_H
#define _PLATFORMSDL_H

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
    virtual void load(const char* filename, uint8_t* destination, uint32_t size);
	virtual void writeToScreenMemory(uint16_t address, uint8_t value);
	virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void renderFrame();

private:
    static void audioCallback(void* data, uint8_t* stream, int bytes);
    void (*interrupt)(void);
    SDL_AudioSpec audioSpec;
    SDL_AudioDeviceID audioDeviceID;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Surface* fontSurface;
    SDL_Texture* fontTexture;
    int framesPerSecond_;
    float audioAngle;
    float audioFrequency;
    int16_t audioVolume;
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
};

#endif
