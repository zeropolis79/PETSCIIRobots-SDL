#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <intuition/intuition.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <devices/audio.h>
#include "PT2.3F_replay_cia.h"
#include "Palette.h"
#include "PlatformAmiga.h"
#include <stdio.h>
#include <stddef.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH_IN_BYTES (SCREEN_WIDTH >> 3)
#define SCREEN_SIZE (SCREEN_WIDTH_IN_BYTES * SCREEN_HEIGHT)
#define PLANES 4
#ifdef PLATFORM_IMAGE_BASED_TILES
#define TILES_WITH_MASK 12
#else
#define TILES_WITH_MASK 30
#endif
#define LARGEST_MODULE_SIZE 103754

static const char version[] = "$VER:Attack of the PETSCII robots (2021-11-07) (C)2021 David Murray, Vesa Halttunen";

struct SpriteData {
    uint16_t posctl[2];
    uint16_t data[28][2];
    uint16_t reserved[2];
};

struct SampleData {
    char name[22];
    uint16_t length;
    int8_t finetune;
    uint8_t volume;
    uint16_t repeatPoint;
    uint16_t repeatLength;
};

__far extern Custom custom;
__far extern uint8_t c64Font[];
__chip extern uint8_t facesPlanes[];
__chip extern uint8_t tilesPlanes[];
__chip extern uint8_t spritesPlanes[];
__chip extern uint8_t spritesMask[];
__chip extern uint8_t itemsPlanes[];
__chip extern uint8_t healthPlanes[];
#ifdef PLATFORM_MODULE_BASED_AUDIO
__chip extern uint8_t soundFXModule[];
__chip extern int8_t soundExplosion[];
__chip extern int8_t soundMedkit[];
__chip extern int8_t soundPlasma[];
__chip extern int8_t soundPistol[];
__chip extern int8_t soundError[];
__chip extern int8_t soundCycleWeapon[];
__chip extern int8_t soundCycleItem[];
__chip extern int8_t soundDoor[];
__chip extern int8_t soundMenuBeep[];
#endif
__chip extern int8_t squareWave[];
__chip static int32_t simpleTileMask = 0xffffff00;
__chip static SpriteData cursorData1 = {
    { 0, 0 },
    {
        { 0xffff, 0 },
        { 0xffff, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xc000, 0 },
        { 0xffff, 0 },
        { 0xffff, 0 }
    },
    { 0, 0 }
};
__chip static SpriteData cursorData2 = {
    { 0, 0 },
    {
        { 0xfff0, 0 },
        { 0xfff0, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0x0030, 0 },
        { 0xfff0, 0 },
        { 0xfff0, 0 }
    },
    { 0, 0 }
};
__chip static uint16_t pointer[4];
uint16_t addressMap[40 * 25];
static uint8_t tileMaskMap[256];
#ifdef PLATFORM_SPRITE_SUPPORT
static int8_t tileSpriteMap[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0, 1, 49, 50, 59, 60, 61, 62, -1, -1, -1, -1, -1, -1, -1, 48,
    -1, -1, -1, 53, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    1, 0, 3, -1, 54, 55, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
static uint16_t blackPalette[16] = { 0 };
#else
static uint16_t blackPalette[16] = { 0x000, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0, 0x0f0 };
#endif
static const char* imageFilenames[] = {
    "IntroScreen.raw.gz",
    "GameScreen.raw.gz",
    "GameOver.raw.gz"
};
static const char* moduleFilenames[] = {
    "mod.metal heads.gz",
    "mod.win.gz",
    "mod.lose.gz",
    "mod.metallic bop amiga.gz",
    "mod.get psyched.gz",
    "mod.robot attack.gz",
    "mod.rushin in.gz"
};
static char* unableToAllocateMemoryError = "Unable to allocate memory\n";
static char* unableToInitializeDisplayError = "Unable to initialize display\n";
static char* unableToInitializeAudioError = "Unable to initialize audio\n";
static char* unableToLoadDataError = "Unable to load data\n";

PlatformAmiga::PlatformAmiga() :
    interrupt(0),
    framesPerSecond_(50),
    screenBitmap(new BitMap),
    screen(0),
    window(0),
    verticalBlankInterrupt(new Interrupt),
#ifdef PLATFORM_MODULE_BASED_AUDIO
    moduleData(0),
    loadedModule(ModuleSoundFX),
#else
    ioAudio(0),
    messagePort(0),
#endif
    clock(3546895),
    screenPlanes(0),
    tilesMask(0),
    combinedTilePlanes(0),
    facesBitMap(new BitMap),
    tilesBitMap(new BitMap),
    spritesBitMap(new BitMap),
    itemsBitMap(new BitMap),
    healthBitMap(new BitMap),
    cursorSprite1(new SimpleSprite),
    cursorSprite2(new SimpleSprite),
    palette(new Palette(blackPalette, (1 << PLANES))),
    bplcon1DefaultValue(0),
    shakeStep(0)
{
    Palette::initialize();

    for (int y = 0, i = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++, i++) {
            addressMap[i] = y * SCREEN_WIDTH_IN_BYTES * PLANES * 8 + x;
        }
    }

    screenPlanes = (uint8_t*)AllocMem(SCREEN_SIZE * PLANES + 16 * 2, MEMF_CHIP | MEMF_CLEAR);
    if (!screenPlanes) {
        Write(Output(), unableToAllocateMemoryError, 26);
        return;
    }

    tilesMask = (uint8_t*)AllocMem(32 / 8 * 24 * PLANES * (TILES_WITH_MASK + 1), MEMF_CHIP | MEMF_CLEAR);
    if (!tilesMask) {
        Write(Output(), unableToAllocateMemoryError, 26);
        return;
    }
    combinedTilePlanes = tilesMask + 32 / 8 * 24 * PLANES * TILES_WITH_MASK;

#ifdef PLATFORM_MODULE_BASED_AUDIO
    moduleData = (uint8_t*)AllocMem(LARGEST_MODULE_SIZE, MEMF_CHIP | MEMF_CLEAR);
    if (!moduleData) {
        Write(Output(), unableToAllocateMemoryError, 26);
        return;
    }
#endif

    InitBitMap(screenBitmap, PLANES, SCREEN_WIDTH, SCREEN_HEIGHT);
    InitBitMap(facesBitMap, PLANES, 16, 24 * 3);
    InitBitMap(tilesBitMap, PLANES, 32, 24 * 256);
    InitBitMap(spritesBitMap, PLANES, 32, 24 * 73);
    InitBitMap(itemsBitMap, PLANES, 48, 32 * 6);
    InitBitMap(healthBitMap, PLANES, 48, 56 * 6);
    screenBitmap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    facesBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    tilesBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    spritesBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    itemsBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    healthBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    screenBitmap->BytesPerRow = SCREEN_WIDTH_IN_BYTES * PLANES;
    facesBitMap->BytesPerRow = 2 * PLANES;
    tilesBitMap->BytesPerRow = 4 * PLANES;
    spritesBitMap->BytesPerRow = 4 * PLANES;
    itemsBitMap->BytesPerRow = 6 * PLANES;
    healthBitMap->BytesPerRow = 6 * PLANES;
    uint8_t* screenPlane = screenPlanes;
    uint8_t* facesPlane = facesPlanes;
    uint8_t* tilesPlane = tilesPlanes;
    uint8_t* spritesPlane = spritesPlanes;
    uint8_t* itemsPlane = itemsPlanes;
    uint8_t* healthPlane = healthPlanes;
    for (int plane = 0; plane < PLANES; plane++, screenPlane += SCREEN_WIDTH_IN_BYTES, facesPlane += 2, tilesPlane += 4, spritesPlane += 4, itemsPlane += 6, healthPlane += 6) {
        screenBitmap->Planes[plane] = screenPlane;
        facesBitMap->Planes[plane] = facesPlane;
        tilesBitMap->Planes[plane] = tilesPlane;
        spritesBitMap->Planes[plane] = spritesPlane;
        itemsBitMap->Planes[plane] = itemsPlane;
        healthBitMap->Planes[plane] = healthPlane;
    }

    ExtNewScreen newScreen = {0};
    newScreen.Width = SCREEN_WIDTH;
    newScreen.Height = SCREEN_HEIGHT;
    newScreen.Depth = PLANES;
    newScreen.ViewModes = SPRITES;
    newScreen.Type = CUSTOMBITMAP | CUSTOMSCREEN | SCREENBEHIND | SCREENQUIET;
    newScreen.DefaultTitle = (UBYTE*)"Attack of the PETSCII robots";
    newScreen.CustomBitMap = screenBitmap;
    screen = OpenScreen((NewScreen*)&newScreen);
    if (!screen) {
        Write(Output(), unableToInitializeDisplayError, 29);
        return;
    }
    LoadRGB4(&screen->ViewPort, blackPalette, (1 << PLANES));
    SetAPen(&screen->RastPort, 0);

    ExtNewWindow newWindow = {0};
    newWindow.Width = SCREEN_WIDTH;
    newWindow.Height = SCREEN_HEIGHT;
    newWindow.Flags = WFLG_SIMPLE_REFRESH | WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_RMBTRAP;
    newWindow.IDCMPFlags = IDCMP_RAWKEY;
    newWindow.Screen = screen;
    newWindow.Type = CUSTOMSCREEN;
    window = OpenWindow((NewWindow*)&newWindow);
    if (!window) {
        Write(Output(), unableToInitializeDisplayError, 29);
        return;
    }

    SetPointer(window, pointer, 0, 0, 0, 0);
    GetSprite(cursorSprite1, 2);
    GetSprite(cursorSprite2, 3);

    verticalBlankInterrupt->is_Node.ln_Type = NT_INTERRUPT;
    verticalBlankInterrupt->is_Node.ln_Pri = 127;
    verticalBlankInterrupt->is_Node.ln_Name = "Attack of the PETSCII robots";
    verticalBlankInterrupt->is_Data = this;
    verticalBlankInterrupt->is_Code = (__stdargs void(*)())&verticalBlankInterruptServer;
    AddIntServer(INTB_VERTB, verticalBlankInterrupt);

#ifdef PLATFORM_MODULE_BASED_AUDIO
    // Clear the first two bytes of effect samples to enable the 2-byte no-loop loop
    *((uint16_t*)soundExplosion) = 0;
    *((uint16_t*)soundMedkit) = 0;
    *((uint16_t*)soundPlasma) = 0;
    *((uint16_t*)soundPistol) = 0;
    *((uint16_t*)soundError) = 0;
    *((uint16_t*)soundCycleWeapon) = 0;
    *((uint16_t*)soundCycleItem) = 0;
    *((uint16_t*)soundDoor) = 0;
    *((uint16_t*)soundMenuBeep) = 0;

    setSampleData(soundFXModule);

    SetCIAInt();
#else
    messagePort = CreatePort(NULL, 0);
    if (!messagePort) {
        Write(Output(), unableToInitializeAudioError, 27);
        return;
    }

    // Don't care which channel gets allocated
    uint8_t requestChannels[4] = { 1, 8, 2, 4 };
    ioAudio = new IOAudio;
    ioAudio->ioa_Request.io_Message.mn_ReplyPort = messagePort;
    ioAudio->ioa_Request.io_Message.mn_Node.ln_Pri = -50;
    ioAudio->ioa_Request.io_Command = ADCMD_ALLOCATE;
    ioAudio->ioa_Request.io_Flags = ADIOF_NOWAIT;
    ioAudio->ioa_Data = requestChannels;
    ioAudio->ioa_Length = 4;

    if (OpenDevice((UBYTE*)AUDIONAME, 0, (IORequest*)ioAudio, 0)) {
        Write(Output(), unableToInitializeAudioError, 27);
        return;
    }

    ioAudio->ioa_Request.io_Command = CMD_WRITE;
    ioAudio->ioa_Request.io_Flags = ADIOF_PERVOL | IOF_QUICK;
    ioAudio->ioa_Volume = 64;
    ioAudio->ioa_Cycles = 0;
    ioAudio->ioa_Data = (uint8_t*)squareWave;
    ioAudio->ioa_Length = 2;
    ioAudio->ioa_Period = (uint16_t)(clock / 440 / 2);
    BeginIO((IORequest*)ioAudio);

    ioAudio->ioa_Request.io_Command = ADCMD_PERVOL;
#endif

    platform = this;
}

PlatformAmiga::~PlatformAmiga()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    stopModule();

    ResetCIAInt();
#else
    if (ioAudio && ioAudio->ioa_Request.io_Device) {
        AbortIO((IORequest*)ioAudio);
        CloseDevice((IORequest*)ioAudio);
    }

    if (messagePort) {
        DeletePort(messagePort);
    }
#endif

    if (verticalBlankInterrupt->is_Data == this) {
        RemIntServer(INTB_VERTB, verticalBlankInterrupt);
    }

    if (cursorSprite2->num != -1) {
        FreeSprite(cursorSprite2->num);
    }

    if (cursorSprite1->num != -1) {
        FreeSprite(cursorSprite1->num);
    }

    if (window) {
        CloseWindow(window);
    }

    if (screen) {
        CloseScreen(screen);
    }

#ifdef PLATFORM_MODULE_BASED_AUDIO
    if (moduleData) {
        FreeMem(moduleData, LARGEST_MODULE_SIZE);
    }
#endif

    if (tilesMask) {
        FreeMem(tilesMask, 32 / 8 * 24 * PLANES * (TILES_WITH_MASK + 1));
    }

    if (screenPlanes) {
        FreeMem(screenPlanes, SCREEN_SIZE * PLANES + 16 * 2);
    }

    delete palette;
    delete cursorSprite2;
    delete cursorSprite1;
    delete healthBitMap;
    delete itemsBitMap;
    delete spritesBitMap;
    delete tilesBitMap;
    delete facesBitMap;
#ifndef PLATFORM_MODULE_BASED_AUDIO
    delete ioAudio;
#endif
    delete verticalBlankInterrupt;
    delete screenBitmap;
}

void PlatformAmiga::runVerticalBlankInterrupt()
{
    if (interrupt) {
        interrupt();
    }
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformAmiga::undeltaSamples(uint8_t* module, uint32_t moduleSize)
{
    uint8_t numPatterns = 0;
    for (int i = 0; i < module[950]; i++) {
        numPatterns = MAX(numPatterns, module[952 + i]);
    }
    numPatterns++;

    int8_t* samplesStart = (int8_t*)(module + 1084 + (numPatterns << 10));
    int8_t* samplesEnd = (int8_t*)(module + moduleSize);

    int8_t sample = 0;
    for (int8_t* sampleData = samplesStart; sampleData < samplesEnd; sampleData++) {
        int8_t delta = *sampleData;
        sample += delta;
        *sampleData = sample;
    }
}

void PlatformAmiga::setSampleData(uint8_t* module)
{
    SampleData* sampleData = (SampleData*)(module + 20);
    sampleData[15 + 0].length = (uint16_t)(soundMedkit - soundExplosion) >> 1;
    sampleData[15 + 1].length = 0; // TODO
    sampleData[15 + 2].length = (uint16_t)(soundPlasma - soundMedkit) >> 1;
    sampleData[15 + 3].length = 0; // TODO
    sampleData[15 + 4].length = 0; // TODO
    sampleData[15 + 5].length = 0; // TODO
    sampleData[15 + 6].length = 0; // TODO
    sampleData[15 + 7].length = 0; // TODO
    sampleData[15 + 8].length = (uint16_t)(soundPistol - soundPlasma) >> 1;
    sampleData[15 + 9].length = (uint16_t)(soundError - soundPistol) >> 1;
    sampleData[15 + 10].length = (uint16_t)(soundCycleWeapon - soundError) >> 1;
    sampleData[15 + 11].length = 0; // TODO
    sampleData[15 + 12].length = (uint16_t)(soundCycleItem - soundCycleWeapon) >> 1;
    sampleData[15 + 13].length = (uint16_t)(soundDoor - soundCycleItem) >> 1;
    sampleData[15 + 14].length = (uint16_t)(soundMenuBeep - soundDoor) >> 1;
    sampleData[15 + 15].length = (uint16_t)(squareWave - soundMenuBeep) >> 1;
    for (int i = 0; i < 16; i++) {
        sampleData[15 + i].volume = 64;
    }
}
#endif

void PlatformAmiga::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

void PlatformAmiga::show()
{
    ScreenToFront(screen);
    ActivateWindow(window);

    uint16_t* copperList = GfxBase->ActiView->LOFCprList->start;
    for (int i = 0; i < GfxBase->ActiView->LOFCprList->MaxCount; i++) {
        if (*copperList++ == offsetof(Custom, bplcon1)) {
            bplcon1DefaultValue = *copperList;
            break;
        } else {
            copperList++;
        }
    }

    if (cursorSprite1->num != -1 && cursorSprite2->num != -1) {
        cursorSprite1->height = 0;
        cursorSprite2->height = 0;
        SetRGB4(&screen->ViewPort, 21, 15, 15, 15);
        ChangeSprite(&screen->ViewPort, cursorSprite1, (uint8_t*)&cursorData1);
        ChangeSprite(&screen->ViewPort, cursorSprite2, (uint8_t*)&cursorData2);
        MoveSprite(&screen->ViewPort, cursorSprite1, 0, 0);
        MoveSprite(&screen->ViewPort, cursorSprite2, 16, 0);
    }
}

int PlatformAmiga::framesPerSecond()
{
    return framesPerSecond_;
}

const uint8_t rawKeyMap[] = {
    '~', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '+', '|',   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',   0,   0,   0,   0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '*',   0,   0,   0,   0,   0,
      0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,   0,   0,   0,   0,
    ' '
};

uint8_t PlatformAmiga::getin()
{
    IntuiMessage* message;
    while ((message = (IntuiMessage*)GetMsg(window->UserPort))) {
        uint32_t messageClass = message->Class;
        uint16_t messageCode = message->Code;
        uint16_t messageQualifier = message->Qualifier;

        ReplyMsg((Message*)message);

        switch (messageClass) {
        case IDCMP_RAWKEY:
            switch (messageCode) {
            case 0x59: // F10
                quit = true;
                break;
            case 0x4f: // Cursor left
                return 0x9D;
            case 0x4e: // Cursor right
                return 0x1D;
            case 0x4c: // Cursor down
                return 0x91;
            case 0x4d: // Cursor up
                return 0x11;
            case 0x45: // Esc
                return 0x03;
            case 0x44: // Return
                return 0x0d;
            default:
                if (messageCode <= 0x40) {
                    return rawKeyMap[messageCode] + ((messageQualifier & IEQUALIFIER_LSHIFT) ? 128 : 0);
                }
                break;
            }
            break;
        default:
            break;
        }
    }

    return 0;
}

void PlatformAmiga::clearKeyBuffer()
{
    IntuiMessage* message;
    while ((message = (IntuiMessage*)GetMsg(window->UserPort))) {
        ReplyMsg((Message*)message);
    }
}

uint32_t PlatformAmiga::load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset)
{
    uint32_t bytesRead = 0;
    const char* nameEnd = filename;
    while (*++nameEnd);
    if ((nameEnd - filename) >= 3 && *--nameEnd == 'z' && *--nameEnd == 'g' && *--nameEnd == '.') {
        BPTR lock = Lock((char*)filename, ACCESS_READ);
        if (lock) {
            FileInfoBlock fib;
            if (Examine(lock, &fib)) {
                UnLock(lock);

                BPTR file = Open((char*)filename, MODE_OLDFILE);
                if (file) {
                    uint8_t* data = new uint8_t[fib.fib_Size];
                    if (data) {
                        Read(file, data, fib.fib_Size);
                        ungzip(data, destination);

                        bytesRead = (data[fib.fib_Size - 1] << 24) | (data[fib.fib_Size - 2] << 16) | (data[fib.fib_Size - 3] << 8) | data[fib.fib_Size - 4];

                        delete[] data;
                    }
                    Close(file);
                }
            }
        }
    } else {
        BPTR file = Open((char*)filename, MODE_OLDFILE);
        if (file) {
            if (offset > 0) {
                Seek(file, offset, OFFSET_BEGINNING);
            }
            bytesRead = Read(file, destination, size);
            Close(file);
        }
    }
    if (bytesRead == 0) {
        Write(Output(), unableToLoadDataError, 20);
    }
    return bytesRead;
}

void PlatformAmiga::displayImage(Image image)
{
    load(imageFilenames[image], screenPlanes, SCREEN_SIZE * PLANES + (2 << PLANES), 0);

    palette->setPalette((uint16_t*)(screenPlanes + SCREEN_SIZE * PLANES), (1 << PLANES));
}

void PlatformAmiga::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
    uint8_t* tiles = tilesPlanes;
    uint8_t* mask = tilesMask;

#ifdef PLATFORM_IMAGE_BASED_TILES
    for (int tile = 0, tileMask = 0; tile < 256; tile++) {
        if (tile == 130 ||                  // BOMB
            tile == 134 ||                  // MAGNET
            (tile >= 240 && tile <= 241) || // VERT/HORIZ PLASMA
            (tile >= 244 && tile <= 245) || // VERT/HORIZ PISTOL
            tile == 246 ||                  // BIG EXPLOSION
            (tile >= 248 && tile <= 252)) { // SMALL EXPLOSION
            uint32_t* tilesLong = (uint32_t*)tiles;
            uint32_t* maskLong = (uint32_t*)mask;
            for (int y = 0; y < 24; y++) {
                uint32_t allPlanes = *tilesLong++ | *tilesLong++ | *tilesLong++ | *tilesLong++;
                *maskLong++ = allPlanes;
                *maskLong++ = allPlanes;
                *maskLong++ = allPlanes;
                *maskLong++ = allPlanes;
            }
            tiles += 4 * 24 * PLANES;
            mask += 4 * 24 * PLANES;
            tileMaskMap[tile] = tileMask++;
        } else {
            tiles += 4 * 24 * PLANES;
        }
    }
#else
    uint8_t* topLeft = tileData;
    uint8_t* topMiddle = topLeft + 256;
    uint8_t* topRight = topMiddle + 256;
    uint8_t* middleLeft = topRight + 256;
    uint8_t* middleMiddle = middleLeft + 256;
    uint8_t* middleRight = middleMiddle + 256;
    uint8_t* bottomLeft = middleRight + 256;
    uint8_t* bottomMiddle = bottomLeft + 256;
    uint8_t* bottomRight = bottomMiddle + 256;

    for (int tile = 0, tileMask = 0; tile < 256; tile++) {
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        if ((tile >= 96 && tile <= 103) ||  // PLAYER, HOVERBOT, EVILBOT
            tile == 111 ||                  // DEAD PLAYER
            tile == 115 ||                  // DEAD ROBOT
            tile == 130 ||                  // BOMB
            tile == 134 ||                  // MAGNET
            (tile >= 140 && tile <= 142) || // WATER DROID
            (tile == 160 && tile <= 162) || // DEMATERIALIZE
            (tile >= 164 && tile <= 165) || // ROLLERBOT
            (tile >= 240 && tile <= 241) || // VERT/HORIZ PLASMA
            (tile >= 244 && tile <= 246) || // VERT/HORIZ PISTOL, BIG EXPLOSION
            (tile >= 248 && tile <= 252)) { // SMALL EXPLOSION
            for (int y = 0; y < 3; y++, tiles += 8 * 4 * PLANES - 3, mask += 8 * 4 * PLANES - 3) {
                for (int x = 0; x < 3; x++, tiles++, mask++) {
                    uint8_t character = characters[y][x];
                    bool reverse = character > 127;
                    uint8_t* font = c64Font + ((character & 127) << 3);
                    for (int offset = 0; offset < 8 * 4 * PLANES; offset += 4 * PLANES, font++) {
                        uint8_t byte = reverse ? ~*font : *font;
                        tiles[offset] = 0;
                        tiles[offset + 4] = byte;
                        tiles[offset + 8] = 0;
                        tiles[offset + 12] = byte;
                        mask[offset] = character != 0x3a ? 0xff : 0;
                        mask[offset + 4] = mask[offset];
                        mask[offset + 8] = mask[offset];
                        mask[offset + 12] = mask[offset];
                    }
                }
            }
            tileMaskMap[tile] = tileMask++;
        } else {
            for (int y = 0; y < 3; y++, tiles += 8 * 4 * PLANES - 3) {
                for (int x = 0; x < 3; x++, tiles++) {
                    uint8_t character = characters[y][x];
                    bool reverse = character > 127;
                    uint8_t* font = c64Font + ((character & 127) << 3);
                    for (int offset = 0; offset < 8 * 4 * PLANES; offset += 4 * PLANES, font++) {
                        uint8_t byte = reverse ? ~*font : *font;
                        tiles[offset] = 0;
                        tiles[offset + 4] = byte;
                        tiles[offset + 8] = 0;
                        tiles[offset + 12] = byte;
                    }
                }
            }
        }
    }
#endif
}

#ifndef PLATFORM_IMAGE_BASED_TILES
void PlatformAmiga::updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles)
{
    uint8_t* topLeft = tileData;
    uint8_t* topMiddle = topLeft + 256;
    uint8_t* topRight = topMiddle + 256;
    uint8_t* middleLeft = topRight + 256;
    uint8_t* middleMiddle = middleLeft + 256;
    uint8_t* middleRight = middleMiddle + 256;
    uint8_t* bottomLeft = middleRight + 256;
    uint8_t* bottomMiddle = bottomLeft + 256;
    uint8_t* bottomRight = bottomMiddle + 256;

    for (int i = 0; i < numTiles; i++) {
        uint8_t tile = tiles[i];
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        uint32_t thirdOfTileOffset = tile << 7;
        uint8_t* destination = tilesPlanes + thirdOfTileOffset + thirdOfTileOffset + thirdOfTileOffset;
        for (int y = 0; y < 3; y++, destination += 8 * 4 * PLANES - 3) {
            for (int x = 0; x < 3; x++, destination++) {
                uint8_t character = characters[y][x];
                bool reverse = character > 127;
                uint8_t* font = c64Font + ((character & 127) << 3);
                for (int offset = 0; offset < 8 * 4 * PLANES; offset += 4 * PLANES, font++) {
                    uint8_t byte = reverse ? ~*font : *font;
                    destination[offset] = 0;
                    destination[offset + 4] = byte;
                    destination[offset + 8] = 0;
                    destination[offset + 12] = byte;
                }
            }
        }
    }
}
#endif

void PlatformAmiga::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    if (transparent) {
#ifdef PLATFORM_SPRITE_SUPPORT
        if (tileSpriteMap[tile] >= 0) {
            renderSprite(tileSpriteMap[tile] + variant, x, y);
            return;
        }
#endif

        bool shifted = x & 8;
        uint32_t thirdOfTileOffset = tile << 7;
        uint32_t thirdOfTileMaskOffset = tileMaskMap[tile] << 7;
        uint32_t screenOffsetXInWords = x >> 4;
        uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
        OwnBlitter();
        WaitBlit();
        custom.bltafwm = 0xffff;
        custom.bltalwm = 0xff00;
        custom.bltcon1 = (uint16_t)(shifted ? (8 << 12) : 0);
        custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC | (shifted ? (8 << 12) : 0));
        custom.bltamod = 0;
        custom.bltbmod = 0;
        custom.bltcmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltdmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltapt = tilesPlanes + thirdOfTileOffset + thirdOfTileOffset + thirdOfTileOffset;
        custom.bltbpt = tilesMask + thirdOfTileMaskOffset + thirdOfTileMaskOffset + thirdOfTileMaskOffset;
        custom.bltcpt = screenPlanes + screenOffset;
        custom.bltdpt = screenPlanes + screenOffset;
        custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));

        DisownBlitter();
    } else {
        bool shifted = x & 8;
        uint32_t thirdOfTileOffset = tile << 7;
        uint32_t screenOffsetXInWords = x >> 4;
        uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
        OwnBlitter();
        WaitBlit();
        custom.bltafwm = 0xffff;
        custom.bltalwm = 0xff00;
        custom.bltcon1 = (uint16_t)(shifted ? (8 << 12) : 0);
        custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC | (shifted ? (8 << 12) : 0));
        custom.bltamod = 0;
        custom.bltbmod = -4;
        custom.bltcmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltdmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltapt = tilesPlanes + thirdOfTileOffset + thirdOfTileOffset + thirdOfTileOffset;
        custom.bltbpt = &simpleTileMask;
        custom.bltcpt = screenPlanes + screenOffset;
        custom.bltdpt = screenPlanes + screenOffset;
        custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));

        DisownBlitter();
    }
}

void PlatformAmiga::renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t foregroundVariant)
{
#ifdef PLATFORM_SPRITE_SUPPORT
        bool shifted = x & 8;
        uint32_t thirdOfBackgroundTileOffset = backgroundTile << 7;
        uint32_t screenOffsetXInWords = x >> 4;
        uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
        OwnBlitter();
        if (tileSpriteMap[foregroundTile] >= 0) {
            uint8_t sprite = tileSpriteMap[foregroundTile] + foregroundVariant;
            uint32_t thirdOfSpriteOffset = sprite << 7;
            WaitBlit();
            custom.bltafwm = 0xffff;
            custom.bltalwm = 0xff00;
            custom.bltcon1 = 0;
            custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC);
            custom.bltamod = 0;
            custom.bltbmod = 0;
            custom.bltcmod = 0;
            custom.bltdmod = 0;
            custom.bltapt = spritesPlanes + thirdOfSpriteOffset + thirdOfSpriteOffset + thirdOfSpriteOffset;
            custom.bltbpt = spritesMask + thirdOfSpriteOffset + thirdOfSpriteOffset + thirdOfSpriteOffset;
            custom.bltcpt = tilesPlanes + thirdOfBackgroundTileOffset + thirdOfBackgroundTileOffset + thirdOfBackgroundTileOffset;
            custom.bltdpt = combinedTilePlanes;
            custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));
        } else {
            uint32_t thirdOfForegroundTileOffset = foregroundTile << 7;
            uint32_t thirdOfForegroundTileMaskOffset = tileMaskMap[foregroundTile] << 7;
            WaitBlit();
            custom.bltafwm = 0xffff;
            custom.bltalwm = 0xff00;
            custom.bltcon1 = 0;
            custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC);
            custom.bltamod = 0;
            custom.bltbmod = 0;
            custom.bltcmod = 0;
            custom.bltdmod = 0;
            custom.bltapt = tilesPlanes + thirdOfForegroundTileOffset + thirdOfForegroundTileOffset + thirdOfForegroundTileOffset;
            custom.bltbpt = tilesMask + thirdOfForegroundTileMaskOffset + thirdOfForegroundTileMaskOffset + thirdOfForegroundTileMaskOffset;
            custom.bltcpt = tilesPlanes + thirdOfBackgroundTileOffset + thirdOfBackgroundTileOffset + thirdOfBackgroundTileOffset;
            custom.bltdpt = combinedTilePlanes;
            custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));
        }

        WaitBlit();
        custom.bltafwm = 0xffff;
        custom.bltalwm = 0xff00;
        custom.bltcon1 = (uint16_t)(shifted ? (8 << 12) : 0);
        custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC | (shifted ? (8 << 12) : 0));
        custom.bltamod = 0;
        custom.bltbmod = -4;
        custom.bltcmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltdmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
        custom.bltapt = combinedTilePlanes;
        custom.bltbpt = &simpleTileMask;
        custom.bltcpt = screenPlanes + screenOffset;
        custom.bltdpt = screenPlanes + screenOffset;
        custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));

        DisownBlitter();
#endif
}

void PlatformAmiga::renderSprite(uint8_t sprite, uint16_t x, uint16_t y)
{
    bool shifted = x & 8;
    uint32_t thirdOfSpriteOffset = sprite << 7;
    uint32_t screenOffsetXInWords = x >> 4;
    uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
    OwnBlitter();
    WaitBlit();
    custom.bltafwm = 0xffff;
    custom.bltalwm = 0xff00;
    custom.bltcon1 = (uint16_t)(shifted ? (8 << 12) : 0);
    custom.bltcon0 = (uint16_t)(BC0F_SRCA | BC0F_SRCB | BC0F_SRCC | BC0F_DEST | ABC | ABNC | NANBC | ANBC | (shifted ? (8 << 12) : 0));
    custom.bltamod = 0;
    custom.bltbmod = 0;
    custom.bltcmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
    custom.bltdmod = SCREEN_WIDTH_IN_BYTES - (32 >> 3);
    custom.bltapt = spritesPlanes + thirdOfSpriteOffset + thirdOfSpriteOffset + thirdOfSpriteOffset;
    custom.bltbpt = spritesMask + thirdOfSpriteOffset + thirdOfSpriteOffset + thirdOfSpriteOffset;
    custom.bltcpt = screenPlanes + screenOffset;
    custom.bltdpt = screenPlanes + screenOffset;
    custom.bltsize = (uint16_t)(((24 * PLANES) << 6) | (32 >> 4));

    DisownBlitter();
}

void PlatformAmiga::renderItem(uint8_t item, uint16_t x, uint16_t y)
{
    BltBitMap(itemsBitMap, 0, item * 32, screen->RastPort.BitMap, x, y, 48, 32, 0xc0, 0xff, 0);
}

void PlatformAmiga::renderHealth(uint8_t health, uint16_t x, uint16_t y)
{
    BltBitMap(healthBitMap, 0, health * 56, screen->RastPort.BitMap, x, y, 48, 56, 0xc0, 0xff, 0);
}

void PlatformAmiga::renderFace(uint8_t face, uint16_t x, uint16_t y)
{
    BltBitMap(facesBitMap, 0, face * 24, screen->RastPort.BitMap, x, y, 16, 24, 0xc0, 0xff, 0);
}

void PlatformAmiga::showCursor(uint16_t x, uint16_t y)
{
    x <<= 3;
    y <<= 3;
    x += x + x - 3;
    y += y + y - 2;
    if (cursorSprite1->num != -1 && cursorSprite2->num != -1) {
        cursorSprite1->height = 28;
        cursorSprite2->height = 28;
        ChangeSprite(&screen->ViewPort, cursorSprite1, (uint8_t*)&cursorData1);
        ChangeSprite(&screen->ViewPort, cursorSprite2, (uint8_t*)&cursorData2);
        MoveSprite(&screen->ViewPort, cursorSprite1, x, y);
        MoveSprite(&screen->ViewPort, cursorSprite2, x + 16, y);
    }
}

void PlatformAmiga::hideCursor()
{
    if (cursorSprite1->num != -1 && cursorSprite2->num != -1) {
        cursorSprite1->height = 0;
        cursorSprite2->height = 0;
        ChangeSprite(&screen->ViewPort, cursorSprite1, (uint8_t*)&cursorData1);
        ChangeSprite(&screen->ViewPort, cursorSprite2, (uint8_t*)&cursorData2);
    }
}

void PlatformAmiga::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    BltBitMap(screenBitmap, sourceX, sourceY, screenBitmap, destinationX, destinationY, width, height, 0xc0, 0xff, 0);
}

void PlatformAmiga::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    BltBitMap(screenBitmap, x, y, screenBitmap, x, y, width, height, 0, 0xff, 0);
}

#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
void PlatformAmiga::shakeScreen()
{
    shakeStep++;
    if (shakeStep > 4) {
        shakeStep = 1;
    }

    uint16_t bplcon1Value = bplcon1DefaultValue;
    if (shakeStep < 3) {
        if (bplcon1Value <= 0xcc) {
            bplcon1Value += 0x33;
        }
    } else if (shakeStep == 2) {
        if (bplcon1Value >= 0x33) {
            bplcon1Value -= 0x33;
        }
    }
    uint16_t* copperList = GfxBase->ActiView->LOFCprList->start;
    for (int i = 0; i < GfxBase->ActiView->LOFCprList->MaxCount; i++) {
        if (*copperList++ == offsetof(Custom, bplcon1)) {
            *copperList++ = bplcon1Value;
            break;
        } else {
            copperList++;
        }
    }
}

void PlatformAmiga::stopShakeScreen()
{
    shakeStep = 0;

    uint16_t* copperList = GfxBase->ActiView->LOFCprList->start;
    for (int i = 0; i < GfxBase->ActiView->LOFCprList->MaxCount; i++) {
        if (*copperList++ == offsetof(Custom, bplcon1)) {
            *copperList++ = bplcon1DefaultValue;
            break;
        } else {
            copperList++;
        }
    }
}
#endif

void PlatformAmiga::startFadeScreen(uint16_t color, uint16_t intensity)
{
    palette->setFadeBaseColor(color);
    palette->setFade(intensity);
    LoadRGB4(&screen->ViewPort, palette->palette(), (1 << PLANES));
}

void PlatformAmiga::fadeScreen(uint16_t intensity)
{
    palette->setFade(intensity);
    LoadRGB4(&screen->ViewPort, palette->palette(), (1 << PLANES));
}

void PlatformAmiga::stopFadeScreen()
{
    palette->setFade(15);
    LoadRGB4(&screen->ViewPort, palette->palette(), (1 << PLANES));
}

void PlatformAmiga::writeToScreenMemory(uint16_t address, uint8_t value)
{
    bool reverse = value > 127;
    uint8_t* source = c64Font + ((value & 127) << 3);
    uint8_t* destination = screenPlanes + addressMap[address];
    if (reverse) {
        for (int y = 0; y < 8; y++, destination += PLANES * SCREEN_WIDTH_IN_BYTES) {
            uint8_t font = ~*source++;
            *destination = font;
            destination[1 * SCREEN_WIDTH_IN_BYTES] = font;
            destination[2 * SCREEN_WIDTH_IN_BYTES] = font;
            destination[3 * SCREEN_WIDTH_IN_BYTES] = font;
        }
    } else {
        for (int y = 0; y < 8; y++, destination += PLANES * SCREEN_WIDTH_IN_BYTES) {
            uint8_t font = *source++;
            *destination = font;
            destination[1 * SCREEN_WIDTH_IN_BYTES] = font;
            destination[2 * SCREEN_WIDTH_IN_BYTES] = font;
            destination[3 * SCREEN_WIDTH_IN_BYTES] = font;
        }
    }
}

void PlatformAmiga::writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    bool reverse = value > 127;
    bool writePlane1 = color & 1;
    bool writePlane2 = color & 2;
    bool writePlane3 = color & 4;
    bool writePlane4 = color & 8;
    uint8_t* source = c64Font + ((value & 127) << 3);
    uint8_t* destination = screenPlanes + addressMap[address];
    if (yOffset > 0) {
        destination += yOffset * PLANES * SCREEN_WIDTH_IN_BYTES;
    }
    if (reverse) {
        for (int y = 0; y < 8; y++, destination += PLANES * SCREEN_WIDTH_IN_BYTES) {
            uint8_t font = ~*source++;
            *destination = writePlane1 ? font : 0;
            destination[1 * SCREEN_WIDTH_IN_BYTES] = writePlane2 ? font : 0;
            destination[2 * SCREEN_WIDTH_IN_BYTES] = writePlane3 ? font : 0;
            destination[3 * SCREEN_WIDTH_IN_BYTES] = writePlane4 ? font : 0;
        }
    } else {
        for (int y = 0; y < 8; y++, destination += PLANES * SCREEN_WIDTH_IN_BYTES) {
            uint8_t font = *source++;
            *destination = writePlane1 ? font : 0;
            destination[1 * SCREEN_WIDTH_IN_BYTES] = writePlane2 ? font : 0;
            destination[2 * SCREEN_WIDTH_IN_BYTES] = writePlane3 ? font : 0;
            destination[3 * SCREEN_WIDTH_IN_BYTES] = writePlane4 ? font : 0;
        }
    }
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformAmiga::loadModule(Module module)
{
    if (loadedModule != module) {
        uint32_t moduleSize = load(moduleFilenames[module - 1], moduleData, LARGEST_MODULE_SIZE, 0);
        undeltaSamples(moduleData, moduleSize);
        setSampleData(moduleData);
        loadedModule = module;
    }
}

void PlatformAmiga::playModule(Module module)
{
    stopModule();
    stopSample();

    if (module == ModuleSoundFX) {
        mt_init(soundFXModule);
    } else {
        loadModule(module);
        mt_init(moduleData);
    }
    mt_SampleStarts[15 + 0] = soundExplosion;
    mt_SampleStarts[15 + 1] = 0; // TODO
    mt_SampleStarts[15 + 2] = soundMedkit;
    mt_SampleStarts[15 + 3] = 0; // TODO
    mt_SampleStarts[15 + 4] = 0; // TODO
    mt_SampleStarts[15 + 5] = 0; // TODO
    mt_SampleStarts[15 + 6] = 0; // TODO
    mt_SampleStarts[15 + 7] = 0; // TODO
    mt_SampleStarts[15 + 8] = soundPlasma;
    mt_SampleStarts[15 + 9] = soundPistol;
    mt_SampleStarts[15 + 10] = soundError;
    mt_SampleStarts[15 + 11] = 0; // TODO
    mt_SampleStarts[15 + 12] = soundCycleWeapon;
    mt_SampleStarts[15 + 13] = soundCycleItem;
    mt_SampleStarts[15 + 14] = soundDoor;
    mt_SampleStarts[15 + 15] = soundMenuBeep;
    mt_Enable = true;
}

void PlatformAmiga::setSongPosition(uint8_t songPosition)
{
    mt_SongPos = songPosition;
    mt_PatternPos = 0;
}

void PlatformAmiga::stopModule()
{
    if (mt_Enable) {
        mt_end();
        mt_Enable = false;
    }
}

void PlatformAmiga::playSample(uint8_t sample)
{
    mt_chan4data[0] = 0x1000 + 320;
    mt_chan4data[1] = (sample < 16 ? sample : 15) << 12;
}

void PlatformAmiga::stopSample()
{
    mt_chan4data[0] = 0;
    mt_chan4data[1] = 0;
}
#else
static const uint16_t noteToFrequency[] = {
    0,
    247,
    262,
    277,
    294,
    311,
    330,
    349,
    370,
    392,
    415,
    440,
    466,
    494,
    523,
    554,
    587,
    622,
    659,
    698,
    740,
    784,
    831,
    880,
    932,
    988,
    1047,
    1109,
    1175,
    1245,
    1319,
    1397,
    1480,
    1568,
    1661,
    1760,
    1865,
    1976,
    0
};

void PlatformAmiga::playNote(uint8_t note)
{
    if (ioAudio) {
        ioAudio->ioa_Volume = noteToFrequency[note] ? 64 : 0;
        if (noteToFrequency[note]) {
            ioAudio->ioa_Period = (uint16_t)(clock / noteToFrequency[note] / 2);
        }
        BeginIO((IORequest*)ioAudio);
    }
}

void PlatformAmiga::stopNote()
{
    if (ioAudio) {
        ioAudio->ioa_Volume = 0;
        BeginIO((IORequest*)ioAudio);
    }
}
#endif
