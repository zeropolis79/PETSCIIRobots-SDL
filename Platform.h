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

class Platform {
public:
    Platform();

    virtual void setInterrupt(void (*interrupt)(void)) = 0;
    virtual int framesPerSecond() = 0;
    virtual void chrout(uint8_t) = 0;
    virtual uint8_t getin() = 0;
    virtual void load(const char* filename, uint8_t* destination, uint32_t size) = 0;
	virtual void writeToScreenMemory(uint16_t address, uint8_t value) = 0;
	virtual void playNote(uint8_t note) = 0;
    virtual void stopNote() = 0;
    virtual void renderFrame() = 0;
    bool quit;
};

extern Platform* platform;

#endif
