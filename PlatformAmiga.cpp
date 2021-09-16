#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <intuition/intuition.h>
#include <hardware/intbits.h>
#include <devices/audio.h>
#include "PlatformAmiga.h"
#include <stdio.h>

static const char version[] = "$VER:Attack of the PETSCII robots (2021-09-09) (C)2021 David Murray, Vesa Halttunen";

__far extern uint8_t petFont[];
__chip int8_t sample[2] = { 127, -128 };

uint16_t PlatformAmiga::addressMap[40 * 25];

PlatformAmiga::PlatformAmiga() :
    interrupt(0),
    framesPerSecond_(50),
    screen(0),
    window(0),
    verticalBlankInterrupt(new Interrupt),
    ioAudio(new IOAudio),
    messagePort(0),
    clock(3546895),
    screenPlanes(0),
    tilesPlanes(0),
    tilesMask(0),
    tilesBitMap(new BitMap)
{
    for (int y = 0, i = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++, i++) {
            addressMap[i] = y * 40 * 8 + x;
        }
    }

    screenPlanes = (uint8_t*)AllocMem(320 / 8 * 200, MEMF_CHIP | MEMF_CLEAR);
    if (!screenPlanes) {
        return;
    }

    tilesPlanes = (uint8_t*)AllocMem(32 / 8 * 24 * 256, MEMF_CHIP | MEMF_CLEAR);
    if (!tilesPlanes) {
        return;
    }

    tilesMask = (uint8_t*)AllocMem(32 / 8 * 24 * 256, MEMF_CHIP | MEMF_CLEAR);
    if (!tilesMask) {
        return;
    }

    BitMap screenBitmap;
    InitBitMap(&screenBitmap, 1, 320, 200);
    screenBitmap.Planes[0] = screenPlanes;

    InitBitMap(tilesBitMap, 1, 32, 24 * 256);
    tilesBitMap->Planes[0] = tilesPlanes;

    ExtNewScreen newScreen = {0};
    newScreen.Width = 320;
    newScreen.Height = 200;
    newScreen.Depth = 1;
    newScreen.Type = CUSTOMBITMAP | CUSTOMSCREEN | SCREENBEHIND;
    newScreen.DefaultTitle = (UBYTE*)"Attack of the PETSCII robots";
    newScreen.CustomBitMap = &screenBitmap;
    screen = OpenScreen((NewScreen*)&newScreen);
    if (!screen) {
        return;
    }
    SetAPen(&screen->RastPort, 0);

    uint16_t colors[] = { 0x000, 0x0c0 };
    LoadRGB4(&screen->ViewPort, colors, 2);

    ExtNewWindow newWindow = {0};
    newWindow.Width = 320;
    newWindow.Height = 200;
    newWindow.Flags = WFLG_SIMPLE_REFRESH | WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP;
    newWindow.IDCMPFlags = IDCMP_RAWKEY;
    newWindow.Screen = screen;
    newWindow.Type = CUSTOMSCREEN;
    window = OpenWindow((NewWindow*)&newWindow);
    if (!window) {
        return;
    }

    ScreenToFront(screen);

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
        FreeMem(tilesMask, 32 / 8 * 24 * 256);
    }

    if (tilesPlanes) {
        FreeMem(tilesPlanes, 32 / 8 * 24 * 256);
    }

    if (screenPlanes) {
        FreeMem(screenPlanes, 320 / 8 * 200);
    }

    delete tilesBitMap;
    delete ioAudio;
    delete verticalBlankInterrupt;
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

    for (int tile = 0; tile < 256; tile++) {
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
    }
}

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
}

void PlatformAmiga::renderTile(uint8_t tile, uint16_t x, uint16_t y, bool transparent)
{
    if (transparent) {
        BltMaskBitMapRastPort(tilesBitMap, 0, tile * 24, &screen->RastPort, x, y, 24, 24, (ABC|ABNC|ANBC), tilesMask);
    } else {
        BltBitMap(tilesBitMap, 0, tile * 24, &screen->BitMap, x, y, 24, 24, 0xc0, 0xff, 0);
    }
}

void PlatformAmiga::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    BltBitMap(&screen->BitMap, sourceX, sourceY, &screen->BitMap, destinationX, destinationY, width, height, 0xc0, 0xff, 0);
}

void PlatformAmiga::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    RectFill(&screen->RastPort, x, y, x + width - 1, y + height - 1);
}

void PlatformAmiga::writeToScreenMemory(uint16_t address, uint8_t value)
{
    uint8_t* source = petFont + value;
    uint8_t* destination = screen->RastPort.BitMap->Planes[0] + addressMap[address];
    for (int y = 0; y < 8; y++, source += 256, destination += 40) {
        *destination = *source;
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
}
