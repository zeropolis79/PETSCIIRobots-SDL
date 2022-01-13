#ifndef _PLATFORMPSP_H
#define _PLATFORMPSP_H

#define PlatformClass PlatformPSP

#include <kerneltypes.h>
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
    virtual void renderFrame(bool waitForNextFrame);

private:
    static int CallbackThread(SceSize args, void* argp);
    static int ExitCallback(int arg1, int arg2, void* common);
    void drawRectangle(uint32_t* texture, uint32_t color, uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

    void (*interrupt)(void);
    int framesPerSecond_;
    int* displayList;
    uint16_t joystickStateToReturn;
    uint16_t joystickState;
    uint16_t pendingState;
};

#endif
