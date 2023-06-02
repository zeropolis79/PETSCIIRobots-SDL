#include "PT2.3A_replay_cia.h"
#include "PlatformSDL.h"
#include <stdio.h>

#define JOYSTICK_AXIS_THRESHOLD 25000

#ifdef PLATFORM_MODULE_BASED_AUDIO
#define LARGEST_MODULE_SIZE 105654
#define TOTAL_SAMPLE_SIZE 75755
#endif

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
static char MAPNAME[] = "level-a";
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
    SDL_SCANCODE_I, // MOVE UP orig: 56 (8)
    SDL_SCANCODE_K, // MOVE DOWN orig: 50 (2)
    SDL_SCANCODE_J, // MOVE LEFT orig: 52 (4)
    SDL_SCANCODE_L, // MOVE RIGHT orig: 54 (6)
    SDL_SCANCODE_W, // FIRE UP
    SDL_SCANCODE_S, // FIRE DOWN
    SDL_SCANCODE_A, // FIRE LEFT
    SDL_SCANCODE_D, // FIRE RIGHT
    SDL_SCANCODE_F1, // CYCLE WEAPONS
    SDL_SCANCODE_F2, // CYCLE ITEMS
    SDL_SCANCODE_SPACE, // USE ITEM
    SDL_SCANCODE_Z, // SEARCH OBJECT
    SDL_SCANCODE_M, // MOVE OBJECT
    SDL_SCANCODE_TAB, // LIVE MAP
    SDL_SCANCODE_TAB + 0x80, // LIVE MAP ROBOTS
    SDL_SCANCODE_ESCAPE, // PAUSE
    SDL_SCANCODE_F6, // MUSIC
    SDL_SCANCODE_C + 0x80, // CHEAT
    SDL_SCANCODE_UP, // CURSOR UP
    SDL_SCANCODE_DOWN, // CURSOR DOWN
    SDL_SCANCODE_LEFT, // CURSOR LEFT
    SDL_SCANCODE_RIGHT, // CURSOR RIGHT
    SDL_SCANCODE_SPACE, // SPACE
    SDL_SCANCODE_RETURN, // RETURN
    SDL_SCANCODE_Y, // YES
    SDL_SCANCODE_N // NO
};

#define LIVE_MAP_ORIGIN_X ((PLATFORM_SCREEN_WIDTH - 56 - 128 * 3) / 2)
#define LIVE_MAP_ORIGIN_Y ((PLATFORM_SCREEN_HEIGHT - 32 - 64 * 3) / 2)

uint8_t unitTypes[48];
uint8_t unitX[48];
uint8_t unitY[48];

static uint16_t joystickButtons[] = {
    Platform::JoystickRed, // 0
    Platform::JoystickBlue, // 1
    Platform::JoystickGreen, // 2
    Platform::JoystickYellow, // 3
    Platform::JoystickPlay, // 4
    0, // 5
    Platform::JoystickExtra, // 6
    0, // 7
    0, // 8
    Platform::JoystickReverse, // 9
    Platform::JoystickForward, // 10
    Platform::JoystickUp, // 11
    Platform::JoystickDown, // 12
    Platform::JoystickLeft, // 13
    Platform::JoystickRight, // 14
    0, // 15
    0 // 16
};

PlatformSDL::PlatformSDL() :
    interrupt(0),
    audioSpec({0}),
    audioDeviceID(0),
    joystick(0),
    window(0),
    windowSurface(0),
    bufferSurface(0),
    fadeSurface(0),
    fontSurface(0),
#ifdef PLATFORM_IMAGE_BASED_TILES
    tileSurface(0),
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    imageSurfaces(),
    itemsSurface(0),
    keysSurface(0),
    healthSurface(0),
    facesSurface(0),
    animTilesSurface(0),
    palette(0),
#ifdef PLATFORM_SPRITE_SUPPORT
    spritesSurface(0),
#endif
#endif
#ifdef PLATFORM_CURSOR_SUPPORT
    cursorSurface(0),
    cursorRect({0}),
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
    cursorShape(ShapeUse),
#endif
#endif
    framesPerSecond_(60),
#ifdef PLATFORM_MODULE_BASED_AUDIO
    moduleData(new uint8_t[LARGEST_MODULE_SIZE]),
    loadedModule(ModuleSoundFX),
    sampleData(new int8_t[TOTAL_SAMPLE_SIZE]),
    effectChannel(0),
#else
    audioAngle(0),
    audioFrequency(440),
    audioVolume(INT16_MAX >> 4),
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    loadedImage(ImageIntro),
#endif
    interruptIntervalInSamples(0),
    samplesSinceInterrupt(0),
    fadeBaseColor(0),
    fadeIntensity(0),
    joystickStateToReturn(0),
    joystickState(0),
    pendingState(0),
    keyToReturn(0xff),
    downKey(0xff),
    shift(0)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Error initializing SDL_image: %s\n", IMG_GetError());
    }

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    SDL_AudioSpec requestedAudioSpec;
    SDL_zero(requestedAudioSpec);
    requestedAudioSpec.freq = 44100;
    requestedAudioSpec.format = AUDIO_S16LSB;
    requestedAudioSpec.channels = 1;
    requestedAudioSpec.samples = 512;
    requestedAudioSpec.callback = audioCallback;
    requestedAudioSpec.userdata = this;
    audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &requestedAudioSpec, &audioSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (!audioDeviceID) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
    }

    interruptIntervalInSamples = audioSpec.freq / framesPerSecond_;
    samplesSinceInterrupt = interruptIntervalInSamples;
    SDL_PauseAudioDevice(audioDeviceID, 0);

    joystick = SDL_JoystickOpen(0);

    window = SDL_CreateWindow("Attack of the PETSCII Robots", 0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, 0);
    windowSurface = SDL_GetWindowSurface(window);
    bufferSurface = SDL_CreateRGBSurface(0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    fadeSurface = SDL_CreateRGBSurface(0, 1, 1, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#ifdef PLATFORM_COLOR_SUPPORT
    fontSurface = IMG_Load("c64font.png");
#else
    fontSurface = IMG_Load("petfont.png");
#endif
#ifdef PLATFORM_IMAGE_BASED_TILES
    tileSurface = IMG_Load("tilesalpha.png");
#else
    for (int i = 0; i < 256; i++) {
        tileSurfaces[i] = SDL_CreateRGBSurface(0, 24, 24, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    }
#endif
#ifdef PLATFORM_IMAGE_SUPPORT
    for (int i = 0; i < 3; i++) {
        imageSurfaces[i] = IMG_Load(imageFilenames[i]);
    }
    itemsSurface = IMG_Load("items.png");
    keysSurface = IMG_Load("keys.png");
    healthSurface = IMG_Load("health.png");
    facesSurface = IMG_Load("faces.png");
    animTilesSurface = IMG_Load("animtiles.png");
#ifdef PLATFORM_SPRITE_SUPPORT
    spritesSurface = IMG_Load("spritesalpha.png");
    SDL_SetColorKey(spritesSurface, SDL_TRUE, 16);
#endif
#endif
#ifdef PLATFORM_CURSOR_SUPPORT
    cursorSurface = SDL_CreateRGBSurface(0, 28, 28, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
#endif
    SDL_SetSurfaceBlendMode(fontSurface, SDL_BLENDMODE_NONE);
#ifdef PLATFORM_MODULE_BASED_AUDIO
    int sample = 0;
    int8_t* destination = sampleData;
    soundExplosion = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMedkit = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundEMP = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMagnet = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundShock = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMove = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundPlasma = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundPistol = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundItemFound = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundError = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundCycleWeapon = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundCycleItem = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundDoor = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundMenuBeep = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    soundShortBeep = destination;
    destination += load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);
    squareWave = destination;
    load(sampleFilenames[sample++], (uint8_t*)destination, TOTAL_SAMPLE_SIZE);

    // Clear the first two bytes of effect samples to enable the 2-byte no-loop loop
    *((uint16_t*)soundExplosion) = 0;
    *((uint16_t*)soundMedkit) = 0;
    *((uint16_t*)soundEMP) = 0;
    *((uint16_t*)soundMagnet) = 0;
    *((uint16_t*)soundShock) = 0;
    *((uint16_t*)soundMove) = 0;
    *((uint16_t*)soundPlasma) = 0;
    *((uint16_t*)soundPistol) = 0;
    *((uint16_t*)soundItemFound) = 0;
    *((uint16_t*)soundError) = 0;
    *((uint16_t*)soundCycleWeapon) = 0;
    *((uint16_t*)soundCycleItem) = 0;
    *((uint16_t*)soundDoor) = 0;
    *((uint16_t*)soundMenuBeep) = 0;
    *((uint16_t*)soundShortBeep) = 0;
#endif

    platform = this;
}

PlatformSDL::~PlatformSDL()
{
#ifdef PLATFORM_IMAGE_SUPPORT
#ifdef PLATFORM_SPRITE_SUPPORT
    SDL_FreeSurface(spritesSurface);
#endif
    SDL_FreeSurface(animTilesSurface);
    SDL_FreeSurface(facesSurface);
    SDL_FreeSurface(healthSurface);
    SDL_FreeSurface(keysSurface);
    SDL_FreeSurface(itemsSurface);
    for (int i = 0; i < 3; i++) {
        SDL_FreeSurface(imageSurfaces[i]);
    }
#endif
#ifdef PLATFORM_IMAGE_BASED_TILES
    SDL_FreeSurface(tileSurface);
#else
    for (int i = 0; i < 256; i++) {
        SDL_FreeSurface(tileSurfaces[i]);
    }
#endif
    SDL_FreeSurface(fadeSurface);
    SDL_FreeSurface(bufferSurface);
    SDL_FreeSurface(fontSurface);
    SDL_DestroyWindow(window);
    SDL_JoystickClose(joystick);
    SDL_CloseAudioDevice(audioDeviceID);
    SDL_Quit();
#ifdef PLATFORM_MODULE_BASED_AUDIO
    delete[] sampleData;
    delete[] moduleData;
#endif
}

void PlatformSDL::audioCallback(void* data, uint8_t* stream, int bytes) {
    PlatformSDL* platform = (PlatformSDL*)data;
    int words = bytes >> 1;
    int16_t* output = (int16_t*)stream;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    processAudio(output, words, platform->audioSpec.freq);
#else
    for (int i = 0; i < words; i++) {
        output[i] = platform->audioVolume * (sin(platform->audioAngle) >= 0 ? 1 : -1);
        platform->audioAngle += 2 * M_PI * platform->audioFrequency / platform->audioSpec.freq;
    }
#endif
    platform->samplesSinceInterrupt += words;
    while (platform->samplesSinceInterrupt >= platform->interruptIntervalInSamples) {
        if (platform->interrupt) {
            (*platform->interrupt)();
        }
        platform->samplesSinceInterrupt -= platform->interruptIntervalInSamples;
    }
}


#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformSDL::undeltaSamples(uint8_t* module, uint32_t moduleSize)
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

void PlatformSDL::setSampleData(uint8_t* module)
{
    mt_SampleStarts[15 + 0] = soundExplosion;
    mt_SampleStarts[15 + 1] = soundShortBeep;
    mt_SampleStarts[15 + 2] = soundMedkit;
    mt_SampleStarts[15 + 3] = soundEMP;
    mt_SampleStarts[15 + 4] = soundMagnet;
    mt_SampleStarts[15 + 5] = soundShock;
    mt_SampleStarts[15 + 6] = soundMove;
    mt_SampleStarts[15 + 7] = soundShock;
    mt_SampleStarts[15 + 8] = soundPlasma;
    mt_SampleStarts[15 + 9] = soundPistol;
    mt_SampleStarts[15 + 10] = soundItemFound;
    mt_SampleStarts[15 + 11] = soundError;
    mt_SampleStarts[15 + 12] = soundCycleWeapon;
    mt_SampleStarts[15 + 13] = soundCycleItem;
    mt_SampleStarts[15 + 14] = soundDoor;
    mt_SampleStarts[15 + 15] = soundMenuBeep;

    SampleData* sampleData = (SampleData*)(module + 20);
    putWord((uint8_t*)&sampleData[15 + 0].length, 0, (uint16_t)(soundMedkit - soundExplosion) >> 1);
    putWord((uint8_t*)&sampleData[15 + 1].length, 0, (uint16_t)(squareWave - soundShortBeep) >> 1);
    putWord((uint8_t*)&sampleData[15 + 2].length, 0, (uint16_t)(soundEMP - soundMedkit) >> 1);
    putWord((uint8_t*)&sampleData[15 + 3].length, 0, (uint16_t)(soundMagnet - soundEMP) >> 1);
    putWord((uint8_t*)&sampleData[15 + 4].length, 0, (uint16_t)(soundShock - soundMagnet) >> 1);
    putWord((uint8_t*)&sampleData[15 + 5].length, 0, (uint16_t)(soundMove - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 6].length, 0, (uint16_t)(soundPlasma - soundMove) >> 1);
    putWord((uint8_t*)&sampleData[15 + 7].length, 0, (uint16_t)(soundMove - soundShock) >> 1);
    putWord((uint8_t*)&sampleData[15 + 8].length, 0, (uint16_t)(soundPistol - soundPlasma) >> 1);
    putWord((uint8_t*)&sampleData[15 + 9].length, 0, (uint16_t)(soundItemFound - soundPistol) >> 1);
    putWord((uint8_t*)&sampleData[15 + 10].length, 0, (uint16_t)(soundError - soundItemFound) >> 1);
    putWord((uint8_t*)&sampleData[15 + 11].length, 0, (uint16_t)(soundCycleWeapon - soundError) >> 1);
    putWord((uint8_t*)&sampleData[15 + 12].length, 0, (uint16_t)(soundCycleItem - soundCycleWeapon) >> 1);
    putWord((uint8_t*)&sampleData[15 + 13].length, 0, (uint16_t)(soundDoor - soundCycleItem) >> 1);
    putWord((uint8_t*)&sampleData[15 + 14].length, 0, (uint16_t)(soundMenuBeep - soundDoor) >> 1);
    putWord((uint8_t*)&sampleData[15 + 15].length, 0, (uint16_t)(soundShortBeep - soundMenuBeep) >> 1);
    for (int i = 0; i < 16; i++) {
        sampleData[15 + i].volume = 64;
    }
}
#endif

uint8_t* PlatformSDL::standardControls() const
{
    return ::standardControls;
}

void PlatformSDL::setInterrupt(void (*interrupt)(void))
{
    this->interrupt = interrupt;
}

int PlatformSDL::framesPerSecond()
{
    return framesPerSecond_;
}

void PlatformSDL::chrout(uint8_t character)
{
    putchar(character == 0x0d ? 0x0a : character);
}

uint8_t PlatformSDL::readKeyboard()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            bool keyDown = event.type == SDL_KEYDOWN;
            uint8_t keyCode = event.key.keysym.scancode & 0x7f;
            uint8_t keyCodeWithShift = keyCode | shift;

            if (event.key.keysym.scancode == SDL_SCANCODE_LSHIFT || event.key.keysym.scancode == SDL_SCANCODE_RSHIFT) {
                if (keyDown) {
                    shift = 0x80;
                    downKey |= 0x80;
                } else {
                    shift = 0x00;
                    if (downKey != 0xff) {
                        downKey &= 0x7f;
                    }
                }
            } else if (keyDown) {
                if (downKey != keyCodeWithShift && !event.key.repeat) {
                    downKey = keyCodeWithShift;
                    keyToReturn = downKey;
                }
            } else if (downKey == keyCodeWithShift) {
                downKey = 0xff;
            }
        }
        break;
        case SDL_JOYBUTTONDOWN:
            if (event.jbutton.button < sizeof(joystickButtons) / sizeof(uint16_t)) {
                pendingState |= joystickButtons[event.jbutton.button];
            }
            break;
        case SDL_JOYBUTTONUP:
            if (event.jbutton.button < sizeof(joystickButtons) / sizeof(uint16_t)) {
                pendingState &= ~joystickButtons[event.jbutton.button];
            }
            break;
        default:
            break;
        }
    }

    uint8_t result = keyToReturn;
    keyToReturn = 0xff;
    return result;
}

void PlatformSDL::keyRepeat()
{
    keyToReturn = downKey;
    joystickStateToReturn = joystickState;
}

void PlatformSDL::clearKeyBuffer()
{
    SDL_Event event;
    while (SDL_PollEvent(&event));
    keyToReturn = 0xff;
    downKey = 0xff;
    pendingState = 0;
    joystickStateToReturn = 0;
}

bool PlatformSDL::isKeyOrJoystickPressed(bool gamepad)
{
    return downKey != 0xff || (joystickState != 0 && joystickState != JoystickPlay);
}

uint16_t PlatformSDL::readJoystick(bool gamepad)
{
    uint16_t state = 0;
    int16_t leftStickX = SDL_JoystickGetAxis(joystick, 0);
    int16_t leftStickY = SDL_JoystickGetAxis(joystick, 1);
    int16_t rightStickX = SDL_JoystickGetAxis(joystick, 2);
    int16_t rightStickY = SDL_JoystickGetAxis(joystick, 3);
    if (leftStickX < -JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickLeft;
    } else if (leftStickX > JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickRight;
    }
    if (leftStickY < -JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickUp;
    } else if (leftStickY > JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickDown;
    }
    if (rightStickX < -JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickLeft | JoystickPlay;
        state &= ~(JoystickRight | JoystickUp | JoystickDown);
        pendingState &= ~(JoystickRight | JoystickUp | JoystickDown);
    } else if (rightStickX > JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickRight | JoystickPlay;
        state &= ~(JoystickLeft | JoystickUp | JoystickDown);
        pendingState &= ~(JoystickLeft | JoystickUp | JoystickDown);
    }
    if (rightStickY < -JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickUp | JoystickPlay;
        state &= ~(JoystickDown | JoystickLeft | JoystickRight);
        pendingState &= ~(JoystickDown | JoystickLeft | JoystickRight);
    } else if (rightStickY > JOYSTICK_AXIS_THRESHOLD) {
        state |= JoystickDown | JoystickPlay;
        state &= ~(JoystickUp | JoystickLeft | JoystickRight);
        pendingState &= ~(JoystickUp | JoystickLeft | JoystickRight);
    }
    state |= pendingState;

    if (joystickState != state) {
        // Don't return Play button press
        joystickStateToReturn = state != JoystickPlay ? state : 0;
        joystickState = state;
    }

    uint16_t result = joystickStateToReturn;
    joystickStateToReturn = 0;
    return result;
}

uint32_t PlatformSDL::load(const char* filename, uint8_t* destination, uint32_t size)
{
    uint32_t bytesRead = 0;

    FILE* file = fopen(filename, "r");
    if (file) {
        bytesRead = (uint32_t)fread(destination, 1, size, file);

        fclose(file);
    }

    return bytesRead;
}

void PlatformSDL::loadMap(Map map, uint8_t* destination)
{
    MAPNAME[6] = 'a' + map;

    load(MAPNAME, destination, 8960);
}

uint8_t* PlatformSDL::loadTileset()
{
    uint8_t* tileset = new uint8_t[2818];
    load("tileset.amiga", tileset, 2818);
    return tileset;
}

#ifdef PLATFORM_IMAGE_SUPPORT
void PlatformSDL::displayImage(Image image)
{
    SDL_Rect clearRect = { 0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT };
    SDL_FillRect(bufferSurface, &clearRect, 0xff000000);

    if (image == ImageGame) {
        SDL_Rect sourceRect = { 320 - 56, 0, 56, 128 };
        SDL_Rect destinationRect = { PLATFORM_SCREEN_WIDTH - 56, 0, 56, 128 };
        SDL_BlitSurface(imageSurfaces[image], &sourceRect, bufferSurface, &destinationRect);

        sourceRect.y = 128;
        for (destinationRect.y = 128; destinationRect.y < (PLATFORM_SCREEN_HEIGHT - 32); destinationRect.y += 40) {
            sourceRect.h = MIN(40, PLATFORM_SCREEN_HEIGHT - 32 - destinationRect.y);
            destinationRect.h = sourceRect.h;
            SDL_BlitSurface(imageSurfaces[image], &sourceRect, bufferSurface, &destinationRect);
        }

        sourceRect.y = 168;
        sourceRect.h = 32;
        destinationRect.y = PLATFORM_SCREEN_HEIGHT - 32;
        destinationRect.h = 32;
        SDL_BlitSurface(imageSurfaces[image], &sourceRect, bufferSurface, &destinationRect);

        sourceRect.x = 0;
        sourceRect.y = 168;
        sourceRect.w = 104;
        sourceRect.h = 8;
        destinationRect.x = 0;
        destinationRect.y = PLATFORM_SCREEN_HEIGHT - 32;
        destinationRect.w = sourceRect.w;
        destinationRect.h = sourceRect.h;
        SDL_BlitSurface(imageSurfaces[image], &sourceRect, bufferSurface, &destinationRect);

        sourceRect.x = 104;
        for (destinationRect.x = 104; destinationRect.x < (PLATFORM_SCREEN_WIDTH - 56); destinationRect.x += 160) {
            sourceRect.w = MIN(160, PLATFORM_SCREEN_WIDTH - 56 - destinationRect.x);
            destinationRect.w = sourceRect.w;
            SDL_BlitSurface(imageSurfaces[image], &sourceRect, bufferSurface, &destinationRect);
        }
    } else {
        SDL_Rect rect = { 0, 0, 320, 200 };
        SDL_BlitSurface(imageSurfaces[image], &rect, bufferSurface, &rect);
    }

    if (imageSurfaces[image]->format->palette == NULL) {
        SDL_Color colors[256];
        for (int i = 0; i < 256; i++)
            colors[i].r = colors[i].g = colors[i].b = (Uint8)i;

        imageSurfaces[image]->format->palette = new SDL_Palette{ 256, colors };
    }

    palette = imageSurfaces[image]->format->palette;
    loadedImage = image;
}
#endif

void PlatformSDL::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
{
#ifndef PLATFORM_IMAGE_BASED_TILES
    uint8_t* topLeft = tileData;
    uint8_t* topMiddle = topLeft + 256;
    uint8_t* topRight = topMiddle + 256;
    uint8_t* middleLeft = topRight + 256;
    uint8_t* middleMiddle = middleLeft + 256;
    uint8_t* middleRight = middleMiddle + 256;
    uint8_t* bottomLeft = middleRight + 256;
    uint8_t* bottomMiddle = bottomLeft + 256;
    uint8_t* bottomRight = bottomMiddle + 256;

    SDL_Rect sourceRect, destinationRect;
    sourceRect.w = 8;
    sourceRect.h = 8;
    destinationRect.w = 8;
    destinationRect.h = 8;

    for (int tile = 0; tile < 256; tile++) {
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                sourceRect.x = 0;
                sourceRect.y = characters[y][x] << 3;
                destinationRect.x = x << 3;
                destinationRect.y = y << 3;
                SDL_SetSurfaceAlphaMod(fontSurface, ((tileAttributes[tile] & 0x80) == 0 || characters[y][x] != 0x3A) ? 255 : 0);
                SDL_BlitSurface(fontSurface, &sourceRect, tileSurfaces[tile], &destinationRect);
            }
        }
    }
#endif
}

#ifndef PLATFORM_IMAGE_BASED_TILES
void PlatformSDL::updateTiles(uint8_t* tileData, uint8_t* tiles, uint8_t numTiles)
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

    SDL_Rect sourceRect, destinationRect;
    sourceRect.w = 8;
    sourceRect.h = 8;
    destinationRect.w = 8;
    destinationRect.h = 8;
    for (int i = 0; i < numTiles; i++) {
        uint8_t tile = tiles[i];
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                sourceRect.x = 0;
                sourceRect.y = characters[y][x] << 3;
                destinationRect.x = x << 3;
                destinationRect.y = y << 3;
                SDL_BlitSurface(fontSurface, &sourceRect, tileSurfaces[tile], &destinationRect);
            }
        }
    }
}
#endif

SDL_Rect clipRect = { 0, 0, PLATFORM_SCREEN_WIDTH - 56, PLATFORM_SCREEN_HEIGHT - 32 };

void PlatformSDL::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
    SDL_SetClipRect(bufferSurface, &clipRect);
    if (transparent) {
#ifdef PLATFORM_SPRITE_SUPPORT
        if (tileSpriteMap[tile] >= 0) {
            renderSprite(tileSpriteMap[tile] + variant, x, y);
            return;
        }
#endif
    } else {
#ifdef PLATFORM_IMAGE_BASED_TILES
        if (animTileMap[tile] >= 0) {
            renderAnimTile(animTileMap[tile] + variant, x, y);
            return;
        }
#endif
    }

#ifdef PLATFORM_IMAGE_BASED_TILES
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = tile * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_BlitSurface(tileSurface, &sourceRect, bufferSurface, &destinationRect);
#else
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = 0;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_SetSurfaceBlendMode(tileSurfaces[tile], transparent ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
    SDL_BlitSurface(tileSurfaces[tile], &sourceRect, bufferSurface, &destinationRect);
#endif
    SDL_SetClipRect(bufferSurface, 0);
}

void PlatformSDL::renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant)
{
    SDL_SetClipRect(bufferSurface, &clipRect);
    SDL_Surface* backgroundSurface = tileSurface;
#ifdef PLATFORM_IMAGE_BASED_TILES
    if (animTileMap[backgroundTile] >= 0) {
        backgroundTile = animTileMap[backgroundTile] + backgroundVariant;
        backgroundSurface = animTilesSurface;
    }
#endif
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.w = 24;
    destinationRect.h = 24;
#ifdef PLATFORM_IMAGE_BASED_TILES
    if (tileSpriteMap[foregroundTile] >= 0) {
        uint8_t sprite = tileSpriteMap[foregroundTile] + foregroundVariant;
        sourceRect.y = backgroundTile * 24;
        destinationRect.x = x;
        destinationRect.y = y;
        SDL_BlitSurface(backgroundSurface, &sourceRect, bufferSurface, &destinationRect);

        sourceRect.y = sprite * 24;
        SDL_BlitSurface(spritesSurface, &sourceRect, bufferSurface, &destinationRect);
    } else {
#endif
        sourceRect.y = backgroundTile * 24;
        destinationRect.x = x;
        destinationRect.y = y;
        SDL_BlitSurface(backgroundSurface, &sourceRect, bufferSurface, &destinationRect);

        sourceRect.y = foregroundTile * 24;
        SDL_BlitSurface(tileSurface, &sourceRect, bufferSurface, &destinationRect);
#ifdef PLATFORM_IMAGE_BASED_TILES
    }
#endif
    SDL_SetClipRect(bufferSurface, 0);
}

#ifdef PLATFORM_SPRITE_SUPPORT
void PlatformSDL::renderSprite(uint8_t sprite, uint16_t x, uint16_t y)
{
    SDL_SetClipRect(bufferSurface, &clipRect);
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = sprite * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_BlitSurface(spritesSurface, &sourceRect, bufferSurface, &destinationRect);
    SDL_SetClipRect(bufferSurface, 0);
}
#endif

#ifdef PLATFORM_IMAGE_BASED_TILES
void PlatformSDL::renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y)
{
    SDL_SetClipRect(bufferSurface, &clipRect);
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = animTile * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_BlitSurface(animTilesSurface, &sourceRect, bufferSurface, &destinationRect);
    SDL_SetClipRect(bufferSurface, 0);
}
#endif

#ifdef PLATFORM_IMAGE_SUPPORT
void PlatformSDL::renderItem(uint8_t item, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = item * 21;
    sourceRect.w = 48;
    sourceRect.h = 21;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 48;
    destinationRect.h = 21;
    SDL_BlitSurface(itemsSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::renderKey(uint8_t key, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = key * 14;
    sourceRect.w = 16;
    sourceRect.h = 14;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 16;
    destinationRect.h = 14;
    SDL_BlitSurface(keysSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::renderHealth(uint8_t health, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = health * 51;
    sourceRect.w = 48;
    sourceRect.h = 51;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 48;
    destinationRect.h = 51;
    SDL_BlitSurface(healthSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::renderFace(uint8_t face, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = face * 24;
    sourceRect.w = 16;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 16;
    destinationRect.h = 24;
    SDL_BlitSurface(facesSurface, &sourceRect, bufferSurface, &destinationRect);
}
#endif

#ifdef PLATFORM_LIVE_MAP_SUPPORT
void PlatformSDL::renderLiveMap(uint8_t* map)
{
    clearRect(0, 0, PLATFORM_SCREEN_WIDTH - 56, LIVE_MAP_ORIGIN_Y);
    clearRect(0, LIVE_MAP_ORIGIN_Y, LIVE_MAP_ORIGIN_X, PLATFORM_SCREEN_HEIGHT - 32 - 2 * LIVE_MAP_ORIGIN_Y);
    clearRect(PLATFORM_SCREEN_WIDTH - 56 - LIVE_MAP_ORIGIN_X, LIVE_MAP_ORIGIN_Y, LIVE_MAP_ORIGIN_X, PLATFORM_SCREEN_HEIGHT - 32 - 2 * LIVE_MAP_ORIGIN_Y);
    clearRect(0, PLATFORM_SCREEN_HEIGHT - 32 - LIVE_MAP_ORIGIN_Y, PLATFORM_SCREEN_WIDTH - 56, LIVE_MAP_ORIGIN_Y);

    SDL_Rect sourceRect, destinationRect;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.w = 3;
    destinationRect.h = 3;

    for (int mapY = 0; mapY < 64; mapY++) {
        for (int mapX = 0; mapX < 128; mapX++) {
            sourceRect.x = 0;
            sourceRect.y = *map++ * 24;
            destinationRect.x = LIVE_MAP_ORIGIN_X + mapX * 3;
            destinationRect.y = LIVE_MAP_ORIGIN_Y + mapY * 3;
            SDL_BlitScaled(tileSurface, &sourceRect, bufferSurface, &destinationRect);
        }
    }

    for (int i = 0; i < 48; i++) {
        unitTypes[i] = 255;
    }
}

void PlatformSDL::renderLiveMapTile(uint8_t* map, uint8_t mapX, uint8_t mapY)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = map[(mapY << 7) + mapX] * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = LIVE_MAP_ORIGIN_X + mapX * 3;
    destinationRect.y = LIVE_MAP_ORIGIN_Y + mapY * 3;
    destinationRect.w = 3;
    destinationRect.h = 3;
    SDL_BlitScaled(tileSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::renderLiveMapUnits(uint8_t* map, uint8_t* unitTypes, uint8_t* unitX, uint8_t* unitY, uint8_t playerColor, bool showRobots)
{
    for (int i = 0; i < 48; i++) {
        if ((i < 28 || unitTypes[i] == 22) && (unitX[i] != ::unitX[i] || unitY[i] != ::unitY[i] || (i > 0 && (!showRobots || unitTypes[i] == 22 || unitTypes[i] != ::unitTypes[i])) || (i == 0 && playerColor != ::unitTypes[i]))) {
            // Remove old dot if any
            if (::unitTypes[i] != 255) {
                renderLiveMapTile(map, ::unitX[i], ::unitY[i]);

                if (i > 0 && !showRobots) {
                    ::unitTypes[i] = 255;
                }
            }

            if (i == 0 ||
                (unitTypes[i] == 22 && (unitX[i] != unitX[0] || unitY[i] != unitY[0])) ||
                (showRobots &&
                 (unitTypes[i] == 1 ||
                 (unitTypes[i] >= 2 && unitTypes[i] <= 5) ||
                 (unitTypes[i] >= 17 && unitTypes[i] <= 18) ||
                 unitTypes[i] == 9))) {
                // Render new dot
                int x = unitX[i];
                int y = unitY[i];
                SDL_Rect clearRect = { LIVE_MAP_ORIGIN_X + x * 3, LIVE_MAP_ORIGIN_Y + y * 3, 3, 3 };
                SDL_Color* color = &palette->colors[(i > 0 || playerColor == 1) ? 1 : 0];
                SDL_FillRect(bufferSurface, &clearRect, SDL_MapRGB(bufferSurface->format, color->r, color->g, color->b));

                ::unitTypes[i] = i == 0 ? playerColor : unitTypes[i];
                ::unitX[i] = unitX[i];
                ::unitY[i] = unitY[i];
            }
        }
    }
}
#endif

#ifdef PLATFORM_CURSOR_SUPPORT
static SDL_Rect cursorSurfaceRect = {
    0, 0, 28, 28
};
void PlatformSDL::showCursor(uint16_t x, uint16_t y)
{
    if (cursorRect.h > 0) {
        SDL_BlitSurface(cursorSurface, &cursorSurfaceRect, bufferSurface, &cursorRect);
    }
    cursorRect.x = x * 24 - 2;
    cursorRect.y = y * 24 - 2;
    cursorRect.w = 28;
    cursorRect.h = 28;
    SDL_BlitSurface(bufferSurface, &cursorRect, cursorSurface, &cursorSurfaceRect);
}

void PlatformSDL::hideCursor()
{
    if (cursorRect.h > 0) {
        SDL_BlitSurface(cursorSurface, &cursorSurfaceRect, bufferSurface, &cursorRect);
        cursorRect.h = 0;
    }
}

#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
void PlatformSDL::setCursorShape(CursorShape shape)
{
    cursorShape = shape;
}
#endif
#endif

void PlatformSDL::copyRect(uint16_t sourceX, uint16_t sourceY, uint16_t destinationX, uint16_t destinationY, uint16_t width, uint16_t height)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = sourceX;
    sourceRect.y = sourceY;
    sourceRect.w = width;
    sourceRect.h = height;
    destinationRect.x = destinationX;
    destinationRect.y = destinationY;
    destinationRect.w = width;
    destinationRect.h = height;
    SDL_BlitSurface(bufferSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    SDL_FillRect(bufferSurface, &rect, 0xff000000);
}

void PlatformSDL::shakeScreen()
{
    copyRect(8, 0, 0, 0, 256, 168);
}

#ifdef PLATFORM_FADE_SUPPORT
void PlatformSDL::startFadeScreen(uint16_t color, uint16_t intensity)
{
    uint32_t r = (color & 0xf00) >> 8;
    uint32_t g = (color & 0x0f0) << 4;
    uint32_t b = (color & 0x00f) << 16;
    uint32_t bgr = r |  g | b;
    fadeBaseColor = bgr | (bgr << 4);
    fadeIntensity = intensity;
}

void PlatformSDL::fadeScreen(uint16_t intensity, bool immediate)
{
    if (fadeIntensity != intensity) {
        if (immediate) {
            fadeIntensity = intensity;
         } else {
            int16_t fadeDelta = intensity > fadeIntensity ? 1 : -1;
            do {
                fadeIntensity += fadeDelta;

                clearKeyBuffer();

                this->renderFrame(true);
            } while (fadeIntensity != intensity);
        }
    }
}

void PlatformSDL::stopFadeScreen()
{
    fadeIntensity = 15;
}
#endif

void PlatformSDL::writeToScreenMemory(address_t address, uint8_t value)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = value << 3;
    sourceRect.w = 8;
    sourceRect.h = 8;
    destinationRect.x = (address % SCREEN_WIDTH_IN_CHARACTERS) << 3;
    destinationRect.y = (address / SCREEN_WIDTH_IN_CHARACTERS) << 3;
    destinationRect.w = 8;
    destinationRect.h = 8;
    SDL_SetSurfaceColorMod(fontSurface, 0x77, 0xbb, 0x55);
    SDL_BlitSurface(fontSurface, &sourceRect, bufferSurface, &destinationRect);
}

void PlatformSDL::writeToScreenMemory(address_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = value << 3;
    sourceRect.w = 8;
    sourceRect.h = 8;
    destinationRect.x = (address % SCREEN_WIDTH_IN_CHARACTERS) << 3;
    destinationRect.y = ((address / SCREEN_WIDTH_IN_CHARACTERS) << 3) + yOffset;
    destinationRect.w = 8;
    destinationRect.h = 8;
    SDL_SetSurfaceColorMod(fontSurface, palette->colors[color].r, palette->colors[color].g, palette->colors[color].b);
    SDL_BlitSurface(fontSurface, &sourceRect, bufferSurface, &destinationRect);
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
void PlatformSDL::loadModule(Module module)
{
    if (loadedModule != module) {
        uint32_t moduleSize = load(moduleFilenames[module], moduleData, LARGEST_MODULE_SIZE);
        undeltaSamples(moduleData, moduleSize);
        setSampleData(moduleData);
        loadedModule = module;
    }
}

void PlatformSDL::playModule(Module module)
{
    stopModule();
    stopSample();

    loadModule(module);
    mt_init(moduleData);

    mt_Enable = true;
}

void PlatformSDL::pauseModule()
{
    mt_speed = 0;
    mt_music();
    mt_Enable = false;
    channel0.volume = 0;
    channel1.volume = 0;
    channel2.volume = 0;
    channel3.volume = 0;
}

void PlatformSDL::stopModule()
{
    mt_end();
}

void PlatformSDL::playSample(uint8_t sample)
{
    ChanInput* input = &mt_chaninputs[effectChannel < 2 ? effectChannel : (5 - effectChannel)];

    effectChannel++;
    effectChannel &= 3;

    putWord((uint8_t*)&input->note, 0, 0x1000 + 320);
    if (sample < 16) {
        putWord((uint8_t*)&input->cmd, 0, sample << 12);
    } else if (sample == 16) {
        putWord((uint8_t*)&input->cmd, 0, 1 << 12);
    } else {
        putWord((uint8_t*)&input->cmd, 0, 15 << 12);
    }
}

void PlatformSDL::stopSample()
{
    for (int i = 0; i < 4; i++) {
        mt_chaninputs[i].note = 0;
        mt_chaninputs[i].cmd = 0;
    }
}
#else
static const float noteToFrequency[] = {
    0,
    246.94,
    261.63,
    277.18,
    293.66,
    311.13,
    329.63,
    349.23,
    369.99,
    392.00,
    415.30,
    440.00,
    466.16,
    493.88,
    523.25,
    554.37,
    587.33,
    622.25,
    659.25,
    698.46,
    739.99,
    783.99,
    830.61,
    880.00,
    932.33,
    987.77,
    1046.50,
    1108.73,
    1174.66,
    1244.51,
    1318.51,
    1396.91,
    1479.98,
    1567.98,
    1661.22,
    1760.00,
    1864.66,
    1975.53,
    0
};

void PlatformSDL::playNote(uint8_t note)
{
    audioFrequency = noteToFrequency[note];
    audioVolume = audioFrequency > 0 ? (INT16_MAX >> 4) : 0;
    audioAngle = 0;
}

void PlatformSDL::stopNote()
{
    audioVolume = 0;
}
#endif

void PlatformSDL::renderFrame(bool)
{
    if (cursorRect.h > 0) {
        SDL_Rect rects[4] = {
            { cursorRect.x, cursorRect.y, cursorRect.w, 2 },
            { cursorRect.x, cursorRect.y + 2, 2, cursorRect.h - 4 },
            { cursorRect.x + cursorRect.w - 2, cursorRect.y + 2, 2, cursorRect.h - 4 },
            { cursorRect.x, cursorRect.y + cursorRect.h - 2, cursorRect.w, 2 }
        };
        SDL_FillRects(bufferSurface, rects, 4, 0xffffffff);
#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
        if (cursorShape != ShapeUse) {
            renderSprite(cursorShape == ShapeSearch ? 83 : 84, cursorRect.x + 2, cursorRect.y + 2);
        }
#endif
    }

    SDL_Rect bufferRect = { 0, 0, loadedImage == ImageGame ? PLATFORM_SCREEN_WIDTH : 320, loadedImage == ImageGame ? PLATFORM_SCREEN_HEIGHT : 200 };
    SDL_Rect windowRect = { 0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT };
    SDL_BlitScaled(bufferSurface, &bufferRect, windowSurface, &windowRect);
    if (fadeIntensity != 15) {
        uint32_t intensity = (15 - fadeIntensity) << 24;
        uint32_t abgr = intensity | (intensity << 4) | fadeBaseColor;
        SDL_Rect fadeRect = { 0, 0, 1, 1 };
        SDL_FillRect(fadeSurface, &fadeRect, abgr);
        SDL_BlitScaled(fadeSurface, &fadeRect, windowSurface, &windowRect);
    }
    SDL_UpdateWindowSurface(window);
}
