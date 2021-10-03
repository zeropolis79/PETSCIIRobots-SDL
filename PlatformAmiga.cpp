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
#include <intuition/intuition.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <devices/audio.h>
#include "PlatformAmiga.h"
#include <stdio.h>
#include <stddef.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_WIDTH_IN_BYTES (SCREEN_WIDTH >> 3)
#define SCREEN_SIZE (SCREEN_WIDTH_IN_BYTES * SCREEN_HEIGHT)
#define PLANES 4
#define TILES_WITH_MASK 34

static const char version[] = "$VER:Attack of the PETSCII robots (2021-10-02) (C)2021 David Murray, Vesa Halttunen";

__far extern Custom custom;
__far extern uint8_t petFont[];
__chip extern uint8_t tilesPlanes[];
__chip int8_t sample[2] = { 127, -128 };
__chip int32_t simpleTileMask = 0xffffff00;
uint16_t PlatformAmiga::addressMap[40 * 25];
uint8_t PlatformAmiga::tileMaskMap[256];

PlatformAmiga::PlatformAmiga() :
    interrupt(0),
    framesPerSecond_(50),
    screenBitmap1(new BitMap),
    screenBitmap2(new BitMap),
    screenBitmap(0),
    screen(0),
    window(0),
    verticalBlankInterrupt(new Interrupt),
    ioAudio(new IOAudio),
    messagePort(0),
    clock(3546895),
    screenPlanes1(0),
    screenPlanes2(0),
    screenPlanes(0),
    tilesMask(0),
    tilesBitMap(new BitMap),
    bplcon1DefaultValue(0),
    shakeStep(0)
{
    for (int y = 0, i = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++, i++) {
            addressMap[i] = y * SCREEN_WIDTH_IN_BYTES * PLANES * 8 + x;
        }
    }

    screenPlanes1 = (uint8_t*)AllocMem(SCREEN_SIZE * PLANES, MEMF_CHIP | MEMF_CLEAR);
    if (!screenPlanes1) {
        return;
    }

    screenPlanes2 = (uint8_t*)AllocMem(SCREEN_SIZE * PLANES, MEMF_CHIP | MEMF_CLEAR);
    if (!screenPlanes2) {
        return;
    }

    tilesMask = (uint8_t*)AllocMem(32 / 8 * 24 * PLANES * TILES_WITH_MASK, MEMF_CHIP | MEMF_CLEAR);
    if (!tilesMask) {
        return;
    }

    InitBitMap(screenBitmap1, PLANES, SCREEN_WIDTH, SCREEN_HEIGHT);
    InitBitMap(screenBitmap2, PLANES, SCREEN_WIDTH, SCREEN_HEIGHT);
    InitBitMap(tilesBitMap, PLANES, 32, 24 * 256);
    screenBitmap1->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    screenBitmap2->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    tilesBitMap->Flags = BMF_DISPLAYABLE | BMF_INTERLEAVED;
    screenBitmap1->BytesPerRow = SCREEN_WIDTH_IN_BYTES * PLANES;
    screenBitmap2->BytesPerRow = SCREEN_WIDTH_IN_BYTES * PLANES;
    tilesBitMap->BytesPerRow = 4 * PLANES;
    uint8_t* screenPlane1 = screenPlanes1;
    uint8_t* screenPlane2 = screenPlanes2;
    uint8_t* tilesPlane = tilesPlanes;
    for (int plane = 0; plane < PLANES; plane++, screenPlane1 += SCREEN_WIDTH_IN_BYTES, screenPlane2 += SCREEN_WIDTH_IN_BYTES, tilesPlane += 4) {
        screenBitmap1->Planes[plane] = screenPlane1;
        screenBitmap2->Planes[plane] = screenPlane2;
        tilesBitMap->Planes[plane] = tilesPlane;
    }
    screenBitmap = screenBitmap2;
    screenPlanes = screenPlanes2;

    ExtNewScreen newScreen = {0};
    newScreen.Width = SCREEN_WIDTH;
    newScreen.Height = SCREEN_HEIGHT;
    newScreen.Depth = PLANES;
    newScreen.Type = CUSTOMBITMAP | CUSTOMSCREEN | SCREENBEHIND | SCREENQUIET;
    newScreen.DefaultTitle = (UBYTE*)"Attack of the PETSCII robots";
    newScreen.CustomBitMap = screenBitmap1;
    screen = OpenScreen((NewScreen*)&newScreen);
    if (!screen) {
        return;
    }
    SetAPen(&screen->RastPort, 0);

    LoadRGB4(&screen->ViewPort, (uint16_t*)(tilesPlanes + 32 / 8 * 24 * 256 * PLANES), (1 << PLANES));

    ExtNewWindow newWindow = {0};
    newWindow.Width = SCREEN_WIDTH;
    newWindow.Height = SCREEN_HEIGHT;
    newWindow.Flags = WFLG_SIMPLE_REFRESH | WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP;
    newWindow.IDCMPFlags = IDCMP_RAWKEY;
    newWindow.Screen = screen;
    newWindow.Type = CUSTOMSCREEN;
    window = OpenWindow((NewWindow*)&newWindow);
    if (!window) {
        return;
    }

    verticalBlankInterrupt->is_Node.ln_Type = NT_INTERRUPT;
    verticalBlankInterrupt->is_Node.ln_Pri = 127;
    verticalBlankInterrupt->is_Node.ln_Name = "Attack of the PETSCII robots";
    verticalBlankInterrupt->is_Data = this;
    verticalBlankInterrupt->is_Code = (__stdargs void(*)())&verticalBlankInterruptServer;
    AddIntServer(INTB_VERTB, verticalBlankInterrupt);

    messagePort = CreatePort(NULL, 0);
    if (!messagePort) {
        return;
    }

    // Don't care which channel gets allocated
    uint8_t requestChannels[4] = { 1, 8, 2, 4 };
    ioAudio->ioa_Request.io_Message.mn_ReplyPort = messagePort;
    ioAudio->ioa_Request.io_Message.mn_Node.ln_Pri = -50;
    ioAudio->ioa_Request.io_Command = ADCMD_ALLOCATE;
    ioAudio->ioa_Request.io_Flags = ADIOF_NOWAIT;
    ioAudio->ioa_Data = requestChannels;
    ioAudio->ioa_Length = 4;

    if (OpenDevice((UBYTE*)AUDIONAME, 0, (IORequest*)ioAudio, 0)) {
        return;
    }

    ioAudio->ioa_Request.io_Command = CMD_WRITE;
    ioAudio->ioa_Request.io_Flags = ADIOF_PERVOL | IOF_QUICK;
    ioAudio->ioa_Volume = 64;
    ioAudio->ioa_Cycles = 0;
    ioAudio->ioa_Data = (uint8_t*)sample;
    ioAudio->ioa_Length = 2;
    ioAudio->ioa_Period = (uint16_t)(clock / 440 / 2);
    BeginIO((IORequest*)ioAudio);

    ioAudio->ioa_Request.io_Command = ADCMD_PERVOL;

    platform = this;
}

PlatformAmiga::~PlatformAmiga()
{
    if (ioAudio->ioa_Request.io_Device) {
        AbortIO((IORequest*)ioAudio);
        CloseDevice((IORequest*)ioAudio);
    }

    if (messagePort) {
        DeletePort(messagePort);
    }

    RemIntServer(INTB_VERTB, verticalBlankInterrupt);

    if (window) {
        CloseWindow(window);
    }

    if (screen) {
        CloseScreen(screen);
    }


    if (tilesMask) {
        FreeMem(tilesMask, 32 / 8 * 24 * PLANES * TILES_WITH_MASK);
    }

    if (screenPlanes2) {
        FreeMem(screenPlanes2, SCREEN_SIZE * PLANES);
    }

    if (screenPlanes1) {
        FreeMem(screenPlanes1, SCREEN_SIZE * PLANES);
    }

    delete tilesBitMap;
    delete ioAudio;
    delete verticalBlankInterrupt;
    delete screenBitmap2;
    delete screenBitmap1;
}

void PlatformAmiga::runVerticalBlankInterrupt()
{
    if (interrupt) {
        interrupt();
    }
}

void PlatformAmiga::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

void PlatformAmiga::show()
{
    renderFrame();

    ScreenToFront(screen);

    uint16_t* copperList = GfxBase->ActiView->LOFCprList->start;
    for (int i = 0; i < GfxBase->ActiView->LOFCprList->MaxCount; i++) {
        if (*copperList++ == offsetof(Custom, bplcon1)) {
            bplcon1DefaultValue = *copperList;
            break;
        } else {
            copperList++;
        }
    }
}

int PlatformAmiga::framesPerSecond()
{
    return framesPerSecond_;
}

void PlatformAmiga::chrout(uint8_t character)
{
    uint8_t buffer[1] = { character == 0x0d ? 0x0a : character };

    Write(Output(), buffer, 1);
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
            case 0x45: // Esc
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
            case 0x63: // Control
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

void PlatformAmiga::load(const char* filename, uint8_t* destination, uint32_t size)
{
    BPTR file = Open((char*)filename, MODE_OLDFILE);
    if (file) {
        Seek(file, 2, OFFSET_BEGINNING);
        Read(file, destination, size);
        Close(file);
    }
}

void PlatformAmiga::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
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
    uint8_t* tiles = tilesPlanes;
    uint8_t* mask = tilesMask;

    for (int tile = 0, tileMask = 0; tile < 256; tile++) {
    /*
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        for (int y = 0; y < 3; y++, tiles += 7 * 4 + 1, mask += 7 * 4 + 1) {
            for (int x = 0; x < 3; x++, tiles++, mask++) {
                uint8_t* font = petFont + characters[y][x];
                for (int offset = 0; offset < 8 * 4; offset += 4, font += 256) {
                    tiles[offset] = *font;
                    mask[offset] = ((tileAttributes[tile] & 0x80) == 0 || characters[y][x] != 0x3a) ? 0xff : 0;
                }
            }
        }
    */
        if ((tile >= 96 && tile <= 103) ||
            tile == 111 ||
            tile == 115 ||
            tile == 130 ||
            tile == 134 ||
            (tile >= 140 && tile <= 142) ||
            tile == 160 ||
            (tile >= 164 && tile <= 165) ||
            tile >= 240) {
            if (tiles[4 * 12 * PLANES + 1] == 0 && tiles[4 * 12 * PLANES + 4 + 1] == 0 && tiles[4 * 12 * PLANES + 8 + 1] == 0 && tiles[4 * 12 * PLANES + 12 + 1] == 0) {
                uint8_t characters[3][3] = {
                    { topLeft[tile], topMiddle[tile], topRight[tile] },
                    { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
                    { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
                };

                for (int y = 0; y < 3; y++, tiles += 8 * 4 * PLANES - 3, mask += 8 * 4 * PLANES - 3) {
                    for (int x = 0; x < 3; x++, tiles++, mask++) {
                        uint8_t character = characters[y][x];
                        uint8_t* font = petFont + character;
                        for (int offset = 0; offset < 8 * 4 * PLANES; offset += 4 * PLANES, font += 256) {
                            tiles[offset] = *font;
                            tiles[offset + 4] = 0;
                            tiles[offset + 8] = 0;
                            tiles[offset + 12] = 0;
                            mask[offset] = character != 0x3a ? *font : 0;
                            mask[offset + 4] = mask[offset];
                            mask[offset + 8] = mask[offset];
                            mask[offset + 12] = mask[offset];
                        }
                    }
                }
            } else {
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
            }
            tileMaskMap[tile] = tileMask++;
        } else {
            tiles += 4 * 24 * PLANES;
        }
    }
}

void PlatformAmiga::updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles)
{
    /*
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

        uint8_t* destination = tilesPlanes + tile * 4 * 24;
        for (int y = 0; y < 3; y++, destination += 7 * 4 + 1) {
            for (int x = 0; x < 3; x++, destination++) {
                uint8_t* font = petFont + characters[y][x];
                for (int offset = 0; offset < 8 * 4; offset += 4, font += 256) {
                    destination[offset] = *font;
                }
            }
        }
    }
    */
}

void PlatformAmiga::renderTile(uint8_t tile, uint16_t x, uint16_t y, bool transparent)
{
    if (transparent) {
//        BltMaskBitMapRastPort(tilesBitMap, 0, tile * 24, &screen->RastPort, x, y, 24, 24, (ABC|ABNC|ANBC), tilesMask);

        OwnBlitter();
        WaitBlit();

        bool shifted = x & 8;
        uint32_t thirdOfTileOffset = tile << 7;
        uint32_t thirdOfTileMaskOffset = tileMaskMap[tile] << 7;
        uint32_t screenOffsetXInWords = x >> 4;
        uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
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
//        BltBitMap(tilesBitMap, 0, tile * 24, screen->RastPort.BitMap, x, y, 24, 24, 0xc0, 0xff, 0);

        OwnBlitter();
        WaitBlit();

        bool shifted = x & 8;
        uint32_t thirdOfTileOffset = tile << 7;
        uint32_t screenOffsetXInWords = x >> 4;
        uint32_t screenOffset = y * SCREEN_WIDTH_IN_BYTES * PLANES + screenOffsetXInWords + screenOffsetXInWords;
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

void PlatformAmiga::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    BltBitMap(screenBitmap1, sourceX, sourceY, screenBitmap1, destinationX, destinationY, width, height, 0xc0, 0xff, 0);
    BltBitMap(screenBitmap2, sourceX, sourceY, screenBitmap2, destinationX, destinationY, width, height, 0xc0, 0xff, 0);
}

void PlatformAmiga::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    BltBitMap(screenBitmap1, x, y, screenBitmap1, x, y, width, height, 0, 0xff, 0);
    BltBitMap(screenBitmap2, x, y, screenBitmap2, x, y, width, height, 0, 0xff, 0);
}

void PlatformAmiga::shakeScreen()
{
    shakeStep++;
    if (shakeStep > 2) {
        shakeStep = 1;
    }

    uint16_t bplcon1Value = bplcon1DefaultValue;
    if (shakeStep == 1) {
        if (bplcon1Value < 0xff) {
            bplcon1Value += 0x11;
        }
    } else if (shakeStep == 2) {
        if (bplcon1Value > 0x00) {
            bplcon1Value -= 0x11;
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

void PlatformAmiga::writeToScreenMemory(uint16_t address, uint8_t value)
{
    uint8_t* source = petFont + value;
    uint8_t* destination1 = screenPlanes1 + addressMap[address];
    uint8_t* destination2 = screenPlanes2 + addressMap[address];
    for (int y = 0; y < 8; y++, source += 256, destination1 += PLANES * SCREEN_WIDTH_IN_BYTES, destination2 += PLANES * SCREEN_WIDTH_IN_BYTES) {
        *destination1 = *source;
        destination1[1 * SCREEN_WIDTH_IN_BYTES] = 0;
        destination1[2 * SCREEN_WIDTH_IN_BYTES] = 0;
        destination1[3 * SCREEN_WIDTH_IN_BYTES] = 0;
        *destination2 = *source;
        destination2[1 * SCREEN_WIDTH_IN_BYTES] = 0;
        destination2[2 * SCREEN_WIDTH_IN_BYTES] = 0;
        destination2[3 * SCREEN_WIDTH_IN_BYTES] = 0;
    }
}

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
    ioAudio->ioa_Volume = noteToFrequency[note] ? 64 : 0;
    if (noteToFrequency[note]) {
        ioAudio->ioa_Period = (uint16_t)(clock / noteToFrequency[note] / 2);
    }
    BeginIO((IORequest*)ioAudio);
}

void PlatformAmiga::stopNote()
{
    ioAudio->ioa_Volume = 0;
    BeginIO((IORequest*)ioAudio);
}

void PlatformAmiga::renderFrame()
{
    MakeScreen(screen);
    RethinkDisplay();

    screenBitmap = screenBitmap == screenBitmap1 ? screenBitmap2 : screenBitmap1;
    screenPlanes = screenPlanes == screenPlanes1 ? screenPlanes2 : screenPlanes1;
    screen->RastPort.BitMap = screenBitmap;
    screen->ViewPort.RasInfo->BitMap = screenBitmap;
}
