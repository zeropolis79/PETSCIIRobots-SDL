#include "PlatformSDL.h"

static uint8_t standardControls[] = {
    SDL_SCANCODE_I, // MOVE UP orig: 56 (8)
    SDL_SCANCODE_K, // MOVE DOWN orig: 50 (2)
    SDL_SCANCODE_J, // MOVE LEFT orig: 52 (4)
    SDL_SCANCODE_L, // MOVE RIGHT orig: 54 (6)
    SDL_SCANCODE_W, // FIRE UP
    SDL_SCANCODE_S, // FIRE DOWN
    SDL_SCANCODE_A, // FIRE LEFT
    SDL_SCANCODE_F, // FIRE RIGHT
    SDL_SCANCODE_COMMA, // CYCLE WEAPONS
    SDL_SCANCODE_PERIOD, // CYCLE ITEMS
    SDL_SCANCODE_SPACE, // USE ITEM
    SDL_SCANCODE_Z, // SEARCH OBEJCT
    SDL_SCANCODE_M, // MOVE OBJECT
    SDL_SCANCODE_TAB, // LIVE MAP
    SDL_SCANCODE_ESCAPE, // PAUSE
    SDL_SCANCODE_F1, // MUSIC
    SDL_SCANCODE_F12, // CHEAT
    SDL_SCANCODE_UP, // CURSOR UP
    SDL_SCANCODE_DOWN, // CURSOR DOWN
    SDL_SCANCODE_LEFT, // CURSOR LEFT
    SDL_SCANCODE_RIGHT, // CURSOR RIGHT
    SDL_SCANCODE_SPACE, // SPACE
    SDL_SCANCODE_RETURN, // RETURN
    SDL_SCANCODE_Y, // YES
    SDL_SCANCODE_N // NO
};

PlatformSDL::PlatformSDL() :
    interrupt(0),
    audioSpec({0}),
    audioDeviceID(0),
    window(0),
    windowSurface(0),
    fontSurface(0),
    framesPerSecond_(50),
    audioAngle(0),
    audioFrequency(440),
    audioVolume(INT16_MAX >> 4),
    interruptIntervalInSamples(0),
    samplesSinceInterrupt(0)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        printf("Error initializing SDL_image: %s\n", IMG_GetError());
    }

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
        printf("Failed to open audio device: %s\n", SDL_GetError());
    }

    interruptIntervalInSamples = audioSpec.freq / framesPerSecond_;
    samplesSinceInterrupt = interruptIntervalInSamples;
    SDL_PauseAudioDevice(audioDeviceID, 0);

    window = SDL_CreateWindow("Attack of the PETSCII robots", 0, 0, 320, 200, 0);
    windowSurface = SDL_GetWindowSurface(window);
    fontSurface = IMG_Load("petfont.png");
#ifdef PLATFORM_IMAGE_BASED_TILES
    tileSurface = IMG_Load("tiles.png");
#else
    for (int i = 0; i < 256; i++) {
        tileSurfaces[i] = SDL_CreateRGBSurface(0, 24, 24, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    }
#endif
    SDL_SetSurfaceBlendMode(fontSurface, SDL_BLENDMODE_NONE);

    platform = this;
}

PlatformSDL::~PlatformSDL()
{
#ifdef PLATFORM_IMAGE_BASED_TILES
    SDL_FreeSurface(tileSurface);
#else
    for (int i = 0; i < 256; i++) {
        SDL_FreeSurface(tileSurfaces[i]);
    }
#endif
    SDL_FreeSurface(fontSurface);
    SDL_DestroyWindow(window);     
    SDL_CloseAudioDevice(audioDeviceID);
    SDL_Quit();
}

void PlatformSDL::audioCallback(void* data, uint8_t* stream, int bytes) {
    PlatformSDL* platform = (PlatformSDL*)data;
    int words = bytes >> 1;
    int16_t* output = (int16_t*)stream;
    for (int i = 0; i < words; i++) {
        output[i] = platform->audioVolume * (sin(platform->audioAngle) >= 0 ? 1 : -1);
        platform->audioAngle += 2 * M_PI * platform->audioFrequency / platform->audioSpec.freq;
    }
    platform->samplesSinceInterrupt += words;
    while (platform->samplesSinceInterrupt >= platform->interruptIntervalInSamples) {
        if (platform->interrupt) {
            (*platform->interrupt)();
        }
        platform->samplesSinceInterrupt -= platform->interruptIntervalInSamples;
    }
}

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

uint8_t PlatformSDL::getin()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYDOWN:
            return event.key.keysym.scancode;
        default:
            break;
        }
    }

    return 0;
}

void PlatformSDL::clearKeyBuffer()
{
    SDL_Event event;
    while (SDL_PollEvent(&event));
}

uint32_t PlatformSDL::load(const char* filename, uint8_t* destination, uint32_t size, uint32_t offset)
{
    uint32_t bytesRead = 0;

    FILE* file = fopen(filename, "r");
    if (file) {
        if (offset > 0) {
            fseek(file, offset, SEEK_SET);
        }

        bytesRead = fread(destination, 1, size, file);

        fclose(file);
    }

    return bytesRead;
}

void PlatformSDL::generateTiles(uint8_t* tileData, uint8_t* tileAttributes)
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
    for (int tile = 0; tile < 256; tile++) {
#ifdef PLATFORM_IMAGE_BASED_TILES
        if ((tile >= 96 && tile <= 103) ||
            tile == 111 ||
            tile == 115 ||
            tile == 130 ||
            tile == 134 ||
            (tile >= 140 && tile <= 142) ||
            tile == 160 ||
            (tile >= 164 && tile <= 165)) {
            uint8_t characters[3][3] = {
                { topLeft[tile], topMiddle[tile], topRight[tile] },
                { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
                { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
            };

            for (int y = 0; y < 3; y++) {
                for (int x = 0; x < 3; x++) {
                    sourceRect.x = characters[y][x] << 3;
                    sourceRect.y = 0;
                    destinationRect.x = x << 3;
                    destinationRect.y = tile * 24 + (y << 3);
                    if (characters[y][x] != 0x3A) {
                        SDL_BlitSurface(fontSurface, &sourceRect, tileSurface, &destinationRect);
                    }
                }
            }
        }
#else
        uint8_t characters[3][3] = {
            { topLeft[tile], topMiddle[tile], topRight[tile] },
            { middleLeft[tile], middleMiddle[tile], middleRight[tile] },
            { bottomLeft[tile], bottomMiddle[tile], bottomRight[tile] }
        };

        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 3; x++) {
                sourceRect.x = characters[y][x] << 3;
                sourceRect.y = 0;
                destinationRect.x = x << 3;
                destinationRect.y = y << 3;
                SDL_SetSurfaceAlphaMod(fontSurface, ((tileAttributes[tile] & 0x80) == 0 || characters[y][x] != 0x3A) ? 255 : 0);
                SDL_BlitSurface(fontSurface, &sourceRect, tileSurfaces[tile], &destinationRect);
            }
        }
#endif
    }
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
                sourceRect.x = characters[y][x] << 3;
                sourceRect.y = 0;
                destinationRect.x = x << 3;
                destinationRect.y = y << 3;
                SDL_BlitSurface(fontSurface, &sourceRect, tileSurfaces[tile], &destinationRect);
            }
        }
    }
}
#endif

void PlatformSDL::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
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
    SDL_BlitSurface(tileSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_BlitSurface(tileSurfaces[tile], &sourceRect, windowSurface, &destinationRect);
#endif
}

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
    SDL_BlitSurface(windowSurface, &sourceRect, windowSurface, &destinationRect);
}

void PlatformSDL::clearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;
    SDL_FillRect(windowSurface, &rect, 0xff000000);
}

void PlatformSDL::shakeScreen()
{
    copyRect(8, 0, 0, 0, 256, 168);
}

void PlatformSDL::writeToScreenMemory(uint16_t address, uint8_t value)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = value << 3;
    sourceRect.y = 0;
    sourceRect.w = 8;
    sourceRect.h = 8;
    destinationRect.x = (address % 40) << 3;
    destinationRect.y = (address / 40) << 3;
    destinationRect.w = 8;
    destinationRect.h = 8;
    SDL_BlitSurface(fontSurface, &sourceRect, windowSurface, &destinationRect); // blit it to the screen
}

void PlatformSDL::writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    writeToScreenMemory(address, value);
}

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

void PlatformSDL::renderFrame(bool)
{
    SDL_UpdateWindowSurface(window);
}
