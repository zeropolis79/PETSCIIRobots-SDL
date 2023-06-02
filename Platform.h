#ifndef _PLATFORM_H
#define _PLATFORM_H

#define SCREEN_WIDTH_IN_CHARACTERS (PLATFORM_SCREEN_WIDTH / 8)
#define SCREEN_HEIGHT_IN_CHARACTERS (PLATFORM_SCREEN_HEIGHT / 8)

#ifndef PLATFORM_MAP_COUNT
#define PLATFORM_MAP_COUNT 14
#endif

#ifndef PLATFORM_INTRO_OPTIONS
#define PLATFORM_INTRO_OPTIONS 4
#endif

#ifndef PLATFORM_DEFAULT_CONTROL
#define PLATFORM_DEFAULT_CONTROL 0
#endif

#if __cplusplus < 201103L && !defined(_WIN32)
typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
#ifdef _AMIGA
typedef unsigned char bool;
#endif
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

typedef uint32_t address_t;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ABS(a) ((a) >= 0 ? (a) : -(a))

class Platform {
public:
    Platform();
    virtual ~Platform();

    enum Map {
        Map01,
        Map02,
        Map03,
        Map04,
        Map05,
        Map06,
        Map07,
        Map08,
        Map09,
        Map10,
        Map11,
        Map12,
        Map13,
        Map14
    };

    enum Image {
        ImageIntro,
        ImageGame,
        ImageGameOver
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

    enum JoystickBits {
        JoystickBitRight,
        JoystickBitLeft,
        JoystickBitDown,
        JoystickBitUp,
        JoystickBitRed,
        JoystickBitBlue,
        JoystickBitGreen,
        JoystickBitYellow,
        JoystickBitPlay,
        JoystickBitReverse,
        JoystickBitForward,
        JoystickBitExtra
    };

    enum JoystickMask {
        JoystickRight = (1 << JoystickBitRight),
        JoystickLeft = (1 << JoystickBitLeft),
        JoystickDown = (1 << JoystickBitDown),
        JoystickUp = (1 << JoystickBitUp),
        JoystickRed = (1 << JoystickBitRed),
        JoystickBlue = (1 << JoystickBitBlue),
        JoystickGreen = (1 << JoystickBitGreen),
        JoystickYellow = (1 << JoystickBitYellow),
        JoystickPlay = (1 << JoystickBitPlay),
        JoystickReverse = (1 << JoystickBitReverse),
        JoystickForward = (1 << JoystickBitForward),
        JoystickExtra = (1 << JoystickBitExtra)
    };

    enum CursorShape {
        ShapeUse,
        ShapeSearch,
        ShapeMove
    };

    virtual uint8_t* standardControls() const = 0;
    virtual void setInterrupt(void (*interrupt)(void)) = 0;
    virtual void show();
    virtual int framesPerSecond() = 0;
    virtual void chrout(uint8_t);
    virtual uint8_t readKeyboard() = 0;
    virtual void keyRepeat();
    virtual void clearKeyBuffer() = 0;
    virtual bool isKeyOrJoystickPressed(bool gamepad);
    virtual uint16_t readJoystick(bool gamepad);
    virtual void loadMap(Map map, uint8_t* destination) = 0;
    virtual uint8_t* loadTileset() = 0;
    virtual void displayImage(Image image);
    virtual void generateTiles(uint8_t* tileData, uint8_t* tileAttributes) = 0;
    virtual void updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles);
    virtual void renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant = 0, bool transparent = false) = 0;
    virtual void renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant = 0, uint8_t foregroundVariant = 0);
    virtual void renderItem(uint8_t item, uint16_t x, uint16_t y);
    virtual void renderKey(uint8_t key, uint16_t x, uint16_t y);
    virtual void renderHealth(uint8_t health, uint16_t x, uint16_t y);
    virtual void renderFace(uint8_t face, uint16_t x, uint16_t y);
    virtual void renderLiveMap(uint8_t* map);
    virtual void renderLiveMapTile(uint8_t* map, uint8_t x, uint8_t y);
    virtual void renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots);
    virtual void showCursor(uint16_t x, uint16_t y);
    virtual void hideCursor();
    virtual void setCursorShape(CursorShape shape);
    virtual void copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height) = 0;
    virtual void clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) = 0;
    virtual void fillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
    virtual void startShakeScreen();
    virtual void shakeScreen();
    virtual void stopShakeScreen();
    virtual void startFadeScreen(uint16_t color, uint16_t intensity);
    virtual void fadeScreen(uint16_t intensity, bool immediate = true);
    virtual void stopFadeScreen();
    virtual void writeToScreenMemory(address_t address, uint8_t value) = 0;
    virtual void writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset) = 0;
    virtual void playNote(uint8_t note);
    virtual void stopNote();
    virtual void loadModule(Module module);
    virtual void playModule(Module module);
    virtual void pauseModule();
    virtual void stopModule();
    virtual void playSample(uint8_t sample);
    virtual void stopSample();
    virtual void renderFrame(bool waitForNextFrame = false);
    virtual void waitForScreenMemoryAccess();
    virtual void setHighlightedMenuRow(uint16_t row);
    bool quit;
};

extern Platform* platform;

#endif
