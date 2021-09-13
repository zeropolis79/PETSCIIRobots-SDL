#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <exec/libraries.h>
#include <exec/interrupts.h>
#include <exec/io.h>
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
    clock(3546895)
{
    for (int y = 0, i = 0; y < 25; y++) {
        for (int x = 0; x < 40; x++, i++) {
            addressMap[i] = y * 40 * 8 + x;
        }
    }

    ExtNewScreen newScreen = {0};
    newScreen.Width = 320;
    newScreen.Height = 200;
    newScreen.Depth = 1;
    newScreen.Type = CUSTOMSCREEN | SCREENBEHIND;
    newScreen.DefaultTitle = (UBYTE*)"Attack of the PETSCII robots";
    screen = OpenScreen((NewScreen*)&newScreen);
    if (!screen) {
        return;
    }

    uint16_t colors[] = { 0x000, 0x0c0 };
    LoadRGB4(&screen->ViewPort, colors, 2);

    ExtNewWindow newWindow = {0};
    newWindow.Width = 320;
    newWindow.Height = 200;
    newWindow.Flags = WFLG_SIMPLE_REFRESH | WFLG_BACKDROP | WFLG_BORDERLESS | WFLG_ACTIVATE | WFLG_RMBTRAP;
    newWindow.IDCMPFlags = IDCMP_RAWKEY| IDCMP_VANILLAKEY;
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

uint8_t PlatformAmiga::getin()
{
    IntuiMessage* message;
    while ((message = (IntuiMessage*)GetMsg(window->UserPort))) {
        uint32_t messageClass = message->Class;
        uint16_t messageCode = message->Code;

        ReplyMsg((Message*)message);

        switch (messageClass) {
        case IDCMP_RAWKEY:
            switch (messageCode) {
            case 0x4f:
                return 0x9D;
            case 0x4e:
                return 0x1D;
            case 0x4c: 
                return 0x91;
            case 0x4d:
                return 0x11;
            default:
                break;
            }
            break;
        case IDCMP_VANILLAKEY:
            switch (messageCode) {
            case 0x1b:
                quit = true;
                break;
            default:
                return (uint8_t)(messageCode >= 'a' && messageCode <= 'z' ? (messageCode - 32) : (messageCode >= 'A' && messageCode <= 'Z' ? (messageCode + 128) : messageCode));
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

void PlatformAmiga::writeToScreenMemory(uint16_t address, uint8_t value)
{
    uint8_t* source = petFont + value;
    uint8_t* destination = screen->RastPort.BitMap->Planes[0] + addressMap[address];
    for (int Y = 0; Y != 8; Y++, source += 256, destination += 40) {
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
