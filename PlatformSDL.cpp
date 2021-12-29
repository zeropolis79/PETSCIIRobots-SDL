#include "PlatformSDL.h"

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
static uint32_t palette[16] = {
    0x000000,
    0xffffff,
    0x445577,
    0x778899,
    0xaabbcc,
    0x003399,
    0x3366bb,
    0x00aaff,
    0x556600,
    0x779900,
    0xaadd00,
    0x774400,
    0xbb7700,
    0xffcc00,
    0xeeaa99,
    0xee0000
};
static const char* imageFilenames[] = {
    "introscreen.png",
    "gamescreen.png",
    "gameover.png"
};

static uint8_t standardControls[] = {
    SDL_SCANCODE_I, // MOVE UP orig: 56 (8)
    SDL_SCANCODE_K, // MOVE DOWN orig: 50 (2)
    SDL_SCANCODE_J, // MOVE LEFT orig: 52 (4)
    SDL_SCANCODE_L, // MOVE RIGHT orig: 54 (6)
    SDL_SCANCODE_W, // FIRE UP
    SDL_SCANCODE_S, // FIRE DOWN
    SDL_SCANCODE_A, // FIRE LEFT
    SDL_SCANCODE_D, // FIRE RIGHT
    SDL_SCANCODE_COMMA, // CYCLE WEAPONS
    SDL_SCANCODE_PERIOD, // CYCLE ITEMS
    SDL_SCANCODE_SPACE, // USE ITEM
    SDL_SCANCODE_Z, // SEARCH OBEJCT
    SDL_SCANCODE_M, // MOVE OBJECT
    SDL_SCANCODE_TAB, // LIVE MAP
    SDL_SCANCODE_GRAVE, // LIVE MAP ROBOTS
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

    window = SDL_CreateWindow("Attack of the PETSCII Robots", 0, 0, PLATFORM_SCREEN_WIDTH, PLATFORM_SCREEN_HEIGHT, 0);
    windowSurface = SDL_GetWindowSurface(window);
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
    SDL_SetSurfaceBlendMode(fontSurface, SDL_BLENDMODE_NONE);

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

uint8_t PlatformSDL::readKeyboard()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYDOWN:
            return event.key.keysym.scancode > 0 ? event.key.keysym.scancode : 0xff;
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

uint8_t* PlatformSDL::loadTileset(const char* filename)
{
    uint8_t* tileset = new uint8_t[2818];
    load("tileset.amiga", tileset, 2818, 0);
    return tileset;
}

#ifdef PLATFORM_IMAGE_SUPPORT
void PlatformSDL::displayImage(Image image)
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = PLATFORM_SCREEN_WIDTH;
    rect.h = PLATFORM_SCREEN_HEIGHT;
    SDL_BlitSurface(imageSurfaces[image], &rect, windowSurface, &rect);
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

void PlatformSDL::renderTile(uint8_t tile, uint16_t x, uint16_t y, uint8_t variant, bool transparent)
{
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

void PlatformSDL::renderTiles(uint8_t backgroundTile, uint8_t foregroundTile, uint16_t x, uint16_t y, uint8_t backgroundVariant, uint8_t foregroundVariant)
{
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
        SDL_BlitSurface(backgroundSurface, &sourceRect, windowSurface, &destinationRect);

        sourceRect.y = sprite * 24;
        SDL_BlitSurface(spritesSurface, &sourceRect, windowSurface, &destinationRect);
    } else {
#endif
        sourceRect.y = backgroundTile * 24;
        destinationRect.x = x;
        destinationRect.y = y;
        SDL_BlitSurface(backgroundSurface, &sourceRect, windowSurface, &destinationRect);

        sourceRect.y = foregroundTile * 24;
        SDL_BlitSurface(tileSurface, &sourceRect, windowSurface, &destinationRect);
#ifdef PLATFORM_IMAGE_BASED_TILES
    }
#endif
}

#ifdef PLATFORM_SPRITE_SUPPORT
void PlatformSDL::renderSprite(uint8_t sprite, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = sprite * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_BlitSurface(spritesSurface, &sourceRect, windowSurface, &destinationRect);
}
#endif

#ifdef PLATFORM_IMAGE_BASED_TILES
void PlatformSDL::renderAnimTile(uint8_t animTile, uint16_t x, uint16_t y)
{
    SDL_Rect sourceRect, destinationRect;
    sourceRect.x = 0;
    sourceRect.y = animTile * 24;
    sourceRect.w = 24;
    sourceRect.h = 24;
    destinationRect.x = x;
    destinationRect.y = y;
    destinationRect.w = 24;
    destinationRect.h = 24;
    SDL_BlitSurface(animTilesSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_BlitSurface(itemsSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_BlitSurface(keysSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_BlitSurface(healthSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_BlitSurface(facesSurface, &sourceRect, windowSurface, &destinationRect);
}
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
    SDL_BlitSurface(fontSurface, &sourceRect, windowSurface, &destinationRect);
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
    SDL_SetSurfaceColorMod(fontSurface, palette[color] >> 16, (palette[color] >> 8) & 0xff, palette[color] & 0xff);
    SDL_BlitSurface(fontSurface, &sourceRect, windowSurface, &destinationRect);
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
