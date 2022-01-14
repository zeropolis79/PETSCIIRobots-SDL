#include <iostream>
#include <exception>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include <errno.h>
#include <fatms.h>
#include <libgu.h>
#include <displaysvc.h>
#include <ctrlsvc.h>
#include <libfpu.h>
#include <impose.h>
#include <utility/utility_common.h>
#include <malloc.h>
#include "PT2.3A_replay_cia.h"
#include "PlatformPSP.h"

SCE_MODULE_INFO(petrobots, 0, 1, 0 );
int sce_newlib_heap_kb_size = 18430;
unsigned int sce_user_main_thread_stack_kb_size = 16;
unsigned int sce_user_main_thread_attribute = SCE_KERNEL_TH_USE_VFPU;

#define DISPLAYLIST_SIZE (1179648/sizeof(int))
#define CACHE_SIZE 131072

static char cache[CACHE_SIZE];
static int cacheSize = 0;

#ifdef PLATFORM_MODULE_BASED_AUDIO
#define LARGEST_MODULE_SIZE 105654
#define TOTAL_SAMPLE_SIZE 75755
#endif

extern uint32_t font[];
extern uint32_t tiles[];
extern uint32_t introScreen[];
extern uint32_t gameScreen[];
extern uint32_t gameOver[];
extern uint8_t levelA[];

uint32_t* images[] = { introScreen, gameScreen, gameOver };

#ifdef PLATFORM_SPRITE_SUPPORT
static int8_t tileSpriteMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1, 49, 50, 57, 58, 59, 60, -1, -1, -1, -1, -1, -1, -1, 48,
    -1, -1, -1, 73, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     1,  0,  3, -1, 53, 54, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 76, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#endif
static int8_t animTileMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 16,
    -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,  8, 10, -1, -1, 12, 14, -1, -1, 20, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#ifdef PLATFORM_IMAGE_SUPPORT
static const char* imageFilenames[] = {
    "introscreen.png",
    "gamescreen.png",
    "gameover.png"
};
#endif
#ifdef PLATFORM_MODULE_BASED_AUDIO
static const char* moduleFilenames[] = {
    "mod.soundfx",
    "mod.metal heads",
    "mod.win",
    "mod.lose",
    "mod.metallic bop amiga",
    "mod.get psyched",
    "mod.robot attack",
    "mod.rushin in"
};
static const char* sampleFilenames[] = {
    "sounds_dsbarexp.raw",
    "SOUND_MEDKIT.raw",
    "SOUND_EMP.raw",
    "SOUND_MAGNET2.raw",
    "SOUND_SHOCK.raw",
    "SOUND_MOVE.raw",
    "SOUND_PLASMA_FASTER.raw",
    "sounds_dspistol.raw",
    "SOUND_FOUND_ITEM.raw",
    "SOUND_ERROR.raw",
    "SOUND_CYCLE_WEAPON.raw",
    "SOUND_CYCLE_ITEM.raw",
    "SOUND_DOOR_FASTER.raw",
    "SOUND_BEEP2.raw",
    "SOUND_BEEP.raw",
    "SquareWave.raw"
};
#endif

static uint8_t standardControls[] = {
    0, // MOVE UP orig: 56 (8)
    0, // MOVE DOWN orig: 50 (2)
    0, // MOVE LEFT orig: 52 (4)
    0, // MOVE RIGHT orig: 54 (6)
    0, // FIRE UP
    0, // FIRE DOWN
    0, // FIRE LEFT
    0, // FIRE RIGHT
    0, // CYCLE WEAPONS
    0, // CYCLE ITEMS
    0, // USE ITEM
    0, // SEARCH OBEJCT
    0, // MOVE OBJECT
    0, // LIVE MAP
    0, // LIVE MAP ROBOTS
    0, // PAUSE
    0, // MUSIC
    0, // CHEAT
    0, // CURSOR UP
    0, // CURSOR DOWN
    0, // CURSOR LEFT
    0, // CURSOR RIGHT
    0, // SPACE
    0, // RETURN
    0, // YES
    0 // NO
};

void debug(const char* message, ...)
{
    FILE* f = fopen(SCE_FATMS_ALIAS_NAME "/log.txt", "a");
    va_list argList;
    va_start(argList, message);
    vfprintf(f, message, argList);
    fflush(f);
    fclose(f);
    va_end(argList);
}

PlatformPSP::PlatformPSP() :
    interrupt(0),
    framesPerSecond_(60),
    joystickStateToReturn(0),
    joystickState(0),
    pendingState(0)
{
    // Increase thread priority
    sceKernelChangeThreadPriority(SCE_KERNEL_TH_SELF, 40);

    // Disable all exceptions
    unsigned int FCR31 = sceFpuGetFCR31();
    FCR31 &= ~(SCE_ALLEGREX_FCR31_ENABLES_V |
                       SCE_ALLEGREX_FCR31_ENABLES_Z |
                       SCE_ALLEGREX_FCR31_ENABLES_O |
                       SCE_ALLEGREX_FCR31_ENABLES_U |
                       SCE_ALLEGREX_FCR31_ENABLES_I |
                       SCE_ALLEGREX_FCR31_CAUSE_E |
                       SCE_ALLEGREX_FCR31_CAUSE_V |
                       SCE_ALLEGREX_FCR31_CAUSE_Z |
                       SCE_ALLEGREX_FCR31_CAUSE_O |
                       SCE_ALLEGREX_FCR31_CAUSE_U |
                       SCE_ALLEGREX_FCR31_CAUSE_I |
                       SCE_ALLEGREX_FCR31_FLAGS_V |
                       SCE_ALLEGREX_FCR31_FLAGS_Z |
                       SCE_ALLEGREX_FCR31_FLAGS_O |
                       SCE_ALLEGREX_FCR31_FLAGS_U |
                       SCE_ALLEGREX_FCR31_FLAGS_I);
    sceFpuSetFCR31(FCR31);

    // Set the impose language
    sceImposeSetLanguageMode(SCE_UTILITY_LANG_ENGLISH, SCE_UTILITY_CTRL_ASSIGN_CROSS_IS_ENTER);

    sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALONLY);

    // Initialize graphics
    displayList = new int[DISPLAYLIST_SIZE];

    sceGuInit();

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    sceGuDrawBuffer(SCEGU_PF8888, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
    sceGuDispBuffer(SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT, SCEGU_VRAM_BP32_0, SCEGU_VRAM_WIDTH);
    sceGuDepthBuffer(SCEGU_VRAM_BP32_2, SCEGU_VRAM_WIDTH);

    sceGuOffset(SCEGU_SCR_OFFSETX, SCEGU_SCR_OFFSETY);
    sceGuViewport(2048, 2048, SCEGU_SCR_WIDTH, SCEGU_SCR_HEIGHT);
    sceGuDepthRange(100, 65535);

    sceGuScissor(0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32);
    sceGuDisable(SCEGU_SCISSOR_TEST);
    sceGuDisable(SCEGU_CLIP_PLANE);

    sceGuDepthFunc(SCEGU_LEQUAL);
    sceGuEnable(SCEGU_TEXTURE);

    sceGuColor(0xffffffff);
    sceGuClearColor(0xff000000);
    sceGuClearDepth(0);
    sceGuClearStencil(0);
    sceGuClear(SCEGU_CLEAR_ALL);

    sceGuTexEnvColor(0xffffff);
    sceGuTexScale(1.0f, 1.0f);
    sceGuTexOffset(0.0f, 0.0f);
    sceGuTexWrap(SCEGU_CLAMP, SCEGU_CLAMP);
    sceGuTexMapMode(SCEGU_UV_MAP, 0, 1);
    sceGuTexFilter(SCEGU_NEAREST, SCEGU_NEAREST);
    sceGuModelColor(0x00000000, 0xffffffff, 0xffffffff, 0xffffffff);

    sceGuFrontFace(SCEGU_CW);
    sceGuShadeModel(SCEGU_SMOOTH);
    sceGuDisable(SCEGU_DEPTH_TEST);
    sceGuEnable(SCEGU_BLEND);
    sceGuDisable(SCEGU_FOG);
    sceGuDisable(SCEGU_LIGHTING);
    sceGuBlendFunc(SCEGU_ADD, SCEGU_SRC_ALPHA, SCEGU_ONE_MINUS_SRC_ALPHA, 0, 0);
    sceGuTexFunc(SCEGU_TEX_MODULATE, SCEGU_RGBA);

    sceGuDisplay(SCEGU_DISPLAY_ON);

    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));

    platform = this;

    PlatformPSP* p = this;
    sceKernelStartThread(sceKernelCreateThread("update_thread", CallbackThread, 0x11, 2048, 0, NULL), sizeof(PlatformPSP*), &p);
}

PlatformPSP::~PlatformPSP()
{
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);
    sceGuTerm();

    delete[] displayList;

    sceKernelExitGame();
}

int PlatformPSP::CallbackThread(SceSize args, void* argp)
{
    PlatformPSP* platform = *((PlatformPSP**)argp);
    sceKernelRegisterExitCallback(sceKernelCreateCallback("Exit Callback", ExitCallback, platform));

    // Display callback notifications
    while (!platform->quit) {
        sceDisplayWaitVblankStartCB();
    }

    return 0;
}

int PlatformPSP::ExitCallback(int arg1, int arg2, void* common)
{
    PlatformPSP* platform = (PlatformPSP*)common;
    platform->quit = true;

    return 0;
}

void PlatformPSP::drawRectangle(uint32_t* texture, uint32_t color, uint16_t tx, uint16_t ty, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    sceGuEnable(SCEGU_TEXTURE);
    sceGuTexImage(0, texture[2], texture[3], texture[2], texture + 5);
    sceGuTexMode(SCEGU_PF8888, 0, 0, SCEGU_TEXBUF_NORMAL);
    sceGuColor(color);

    int oldCacheSize = cacheSize;
    uint16_t* data = (uint16_t*)(cache + cacheSize);
    data[0 * 5 + 0] = tx;
    data[0 * 5 + 1] = ty;
    data[0 * 5 + 2] = x;
    data[0 * 5 + 3] = y;
    data[0 * 5 + 4] = 0;
    data[1 * 5 + 0] = tx + width;
    data[1 * 5 + 1] = ty + height;
    data[1 * 5 + 2] = x + width;
    data[1 * 5 + 3] = y + height;
    data[1 * 5 + 4] = 0;
    cacheSize += 2 * 5 * sizeof(uint16_t);

    sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
    sceGuDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_TEXTURE_USHORT | SCEGU_VERTEX_SHORT | SCEGU_THROUGH, 2, 0, data);
}
uint8_t* PlatformPSP::standardControls() const
{
    return ::standardControls;
}

void PlatformPSP::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

int PlatformPSP::framesPerSecond()
{
    return framesPerSecond_;
}

uint8_t PlatformPSP::readKeyboard()
{
    return 0xff;
}

void PlatformPSP::keyRepeat()
{
    joystickStateToReturn = joystickState;
}

void PlatformPSP::clearKeyBuffer()
{
    joystickStateToReturn = 0;
}

bool PlatformPSP::isKeyOrJoystickPressed(bool gamepad)
{
    return joystickState != 0 && joystickState != JoystickPlay;
}

uint16_t PlatformPSP::readJoystick(bool gamepad)
{
    uint16_t state = 0;

    // Read new input
    SceCtrlData ct;
    if (sceCtrlReadBufferPositive(&ct, 1) >= 0) {
        if (ct.Buttons & SCE_CTRL_LEFT) {
            state |= JoystickLeft;
        }
        if (ct.Buttons & SCE_CTRL_RIGHT) {
            state |= JoystickRight;
        }
        if (ct.Buttons & SCE_CTRL_UP) {
            state |= JoystickUp;
        }
        if (ct.Buttons & SCE_CTRL_DOWN) {
            state |= JoystickDown;
        }
        if (ct.Buttons & SCE_CTRL_SQUARE) {
            state |= JoystickGreen;
        }
        if (ct.Buttons & SCE_CTRL_CIRCLE) {
            state |= JoystickBlue;
        }
        if (ct.Buttons & SCE_CTRL_TRIANGLE) {
            state |= JoystickYellow;
        }
        if (ct.Buttons & SCE_CTRL_CROSS) {
            state |= JoystickRed;
        }
        if (ct.Buttons & SCE_CTRL_L) {
            state |= JoystickReverse;
        }
        if (ct.Buttons & SCE_CTRL_R) {
            state |= JoystickForward;
        }
        if (ct.Buttons & SCE_CTRL_START) {
            state |= JoystickPlay;
        }
    }

    if (joystickState != state) {
        if (joystickState == 0) {
            pendingState = state == JoystickPlay ? JoystickPlay : 0;
        } else if (state != 0) {
            pendingState &= state == JoystickPlay ? JoystickPlay : 0;
        }

        // Return Play button press only when released
        if (state != 0) {
            joystickStateToReturn = state != (gamepad ? JoystickPlay : JoystickBlue) ? state : 0;
        } else {
            joystickStateToReturn = pendingState ? pendingState : state;
        }
        joystickState = state;
    }

    uint16_t result = joystickStateToReturn;
    joystickStateToReturn = 0;
    return result;
}

uint32_t PlatformPSP::load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset)
{
    uint8_t* source = 0;
    if (strcmp(filename, "level-A") == 0) {
        source = levelA;
    }

    if (source) {
        memcpy(destination, source + offset, size);
        return size;
    } else {
        return 0;
    }
}

uint8_t* PlatformPSP::loadTileset(const char* filename)
{
    return 0;
}

void PlatformPSP::displayImage(Image image)
{
    this->clearRect(0, 0, PLATFORM_SCREEN_WIDTH - 1, PLATFORM_SCREEN_HEIGHT - 1);

    if (image == ImageGame) {
        drawRectangle(images[image], 0xffffffff, 320 - 56, 0, PLATFORM_SCREEN_WIDTH - 56, 0, 56, 128);

        for (int y = 128; y < (PLATFORM_SCREEN_HEIGHT - 32); y += 40) {
            drawRectangle(images[image], 0xffffffff, 320 - 56, 128, PLATFORM_SCREEN_WIDTH - 56, y, 56, MIN(40, PLATFORM_SCREEN_HEIGHT - 32 - y));
        }

        drawRectangle(images[image], 0xffffffff, 320 - 56, 168, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32, 56, 32);

        drawRectangle(images[image], 0xffffffff, 0, 168, 0, PLATFORM_SCREEN_HEIGHT - 32, 104, 8);

        for (int x = 104; x < (PLATFORM_SCREEN_WIDTH - 56); x += 160) {
            drawRectangle(images[image], 0xffffffff, 104, 168, x, PLATFORM_SCREEN_HEIGHT - 32, MIN(160, PLATFORM_SCREEN_WIDTH - 56 - x), 8);
        }
    } else {
        drawRectangle(images[image], 0xffffffff, 0, 0, 0, 0, images[image][0], images[image][1]);

        if (image == ImageIntro) {
            this->clearRect(32, 36, 80, 8);
        }
    }
}

void PlatformPSP::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
}

void PlatformPSP::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    sceGuEnable(SCEGU_SCISSOR_TEST);

    /*
    if (transparent) {
        if (tileSpriteMap[tile] >= 0) {
            renderSprite(tileSpriteMap[tile] + variant, x, y);
            return;
        }
    } else {
        if (animTileMap[tile] >= 0) {
            renderAnimTile(animTileMap[tile] + variant, x, y);
            return;
        }
    }
    */

    drawRectangle(tiles, 0xffffffff, (tile & 15) * 24, (tile >> 4) * 24, x, y, 24, 24);

    sceGuDisable(SCEGU_SCISSOR_TEST);
}

void PlatformPSP::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
}

void PlatformPSP::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    sceGuDisable(SCEGU_TEXTURE);
    sceGuColor(0xff000000);

    int oldCacheSize = cacheSize;
    uint16_t* data = (uint16_t*)(cache + cacheSize);
    data[0 * 3 + 0] = x;
    data[0 * 3 + 1] = y;
    data[0 * 3 + 2] = 0;
    data[1 * 3 + 0] = data[0] + width;
    data[1 * 3 + 1] = data[1] + height;
    data[1 * 3 + 2] = 0;
    cacheSize += 2 * 3 * sizeof(uint16_t);

    sceKernelDcacheWritebackRange(data, cacheSize - oldCacheSize);
    sceGuDrawArray(SCEGU_PRIM_RECTANGLES, SCEGU_VERTEX_SHORT | SCEGU_THROUGH, 2, 0, data);
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value)
{
    drawRectangle(font, 0xff55bb77, (value >> 2) & 0x0010, (value << 3) & 0xfff8, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, (address / SCREEN_WIDTH_IN_CHARACTERS) << 3, 8, 8);
}

void PlatformPSP::writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    drawRectangle(font, color == 14 ? 0xffffffff : 0xff55bb77, (value >> 2) & 0x0010, (value << 3) & 0xfff8, (address % SCREEN_WIDTH_IN_CHARACTERS) << 3, ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset, 8, 8);
}

void PlatformPSP::renderFrame(bool waitForNextFrame)
{
    sceGuFinish();
    sceGuSync(SCEGU_SYNC_FINISH, SCEGU_SYNC_WAIT);

    if (waitForNextFrame) {
        sceDisplayWaitVblankStart();
    }

    cacheSize = 0;

    sceGuStart(SCEGU_IMMEDIATE, displayList, DISPLAYLIST_SIZE * sizeof(int));
}
