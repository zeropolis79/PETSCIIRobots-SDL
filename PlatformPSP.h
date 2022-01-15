#ifndef _PLATFORMPSP_H
#define _PLATFORMPSP_H

#define PlatformClass PlatformPSP

#include <kerneltypes.h>
#include <psptypes.h>
#include "Platform.h"

extern void debug(const char *message, ...);

class PlatformPSP : public Platform {
public:
    PlatformPSP();
    virtual ~PlatformPSP();

    virtual uint8_t* standardControls() const;
    virtual void setInterrupt(void (*interrupt)(void));
    virtual int framesPerSecond();
    virtual uint8_t readKeyboard();
    virtual void keyRepeat();
    virtual void clearKeyBuffer();
    virtual bool isKeyOrJoystickPressed(bool gamepad);
    virtual uint16_t readJoystick(bool gamepad);
    virtual uint32_t load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset = 0);
    virtual uint8_t* loadTileset(const char* filename);
    virtual void displayImage(Image image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant = 0, bool transparent = false);
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height);
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    virtual void writeToScreenMemory(address_t address, uint8_t value);
    virtual void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset);
    virtual void loadModule(Module module);
    virtual void playModule(Module module);
    virtual void pauseModule();
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
    virtual void renderFrame(bool waitForNextFrame);

private:
    static int callbackThread(SceSize args, void* argp);
    static int exitCallback(int arg1, int arg2, void* common);
    static SceInt32 audioThread(SceSize args, SceVoid *argb);
    void drawRectangle(uint32_t* texture, uint32_t color, uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void undeltaSamples(uint8_t* module, uint32_t moduleSize);
    void setSampleData(uint8_t* module);

    void (*interrupt)(void);
    int framesPerSecond_;
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
    int16_t *audioBuffer;
    SceShort16 *audioOutputBuffer;
    uint32_t audioOutputBufferOffset;
    SceUID audioThreadId;
    uint16_t interruptIntervalInSamples;
    uint16_t samplesSinceInterrupt;
    int* displayList;
    uint16_t joystickStateToReturn;
    uint16_t joystickState;
    uint16_t pendingState;
};

#endif
