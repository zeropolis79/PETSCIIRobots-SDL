#include "PlatformSDL.h"

PlatformSDL::PlatformSDL() :
    interrupt(0),
    audioSpec({0}),
    audioDeviceID(0),
    window(0),
    renderer(0),
    fontSurface(0),
    fontTexture(0),
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
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    fontSurface = IMG_Load("petfont.png");
    fontTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
    SDL_FreeSurface(fontSurface);
    SDL_RenderClear(renderer);

    platform = this;
}

PlatformSDL::~PlatformSDL()
{
    SDL_DestroyTexture(fontTexture);
    SDL_DestroyRenderer(renderer);     
    SDL_DestroyWindow(window);     
    SDL_CloseAudioDevice(audioDeviceID);
    SDL_Quit();
}

void PlatformSDL::audioCallback(void* data, uint8_t* stream, int bytes) {
    PlatformSDL* platform = (PlatformSDL*)data;
    int words = bytes >> 1;
    int16_t* output = (int16_t*)stream;
    for (int i = 0; i < words; i++) {
        output[i] = platform->audioVolume * sin(platform->audioAngle);
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
            // test keycode
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                return 0x9D;
            case SDLK_RIGHT:
                return 0x1D;
            case SDLK_UP: 
                return 0x91;
            case SDLK_DOWN:
                return 0x11;
            case SDLK_RETURN:
                return 13;
            case SDLK_SPACE:
                return 32;
            case SDLK_ESCAPE:
                return 0x03;
            case SDLK_a:
                return 'A';
            case SDLK_b:
                return 'B';
            case SDLK_c:
                return 'C';
            case SDLK_d:
                return 'D';
            case SDLK_e:
                return 'E';
            case SDLK_f:
                return 'F';
            case SDLK_g:
                return 'G';
            case SDLK_h:
                return 'H';
            case SDLK_i:
                return 'I';
            case SDLK_j:
                return 'J';
            case SDLK_k:
                return 'K';
            case SDLK_l:
                return 'L';
            case SDLK_m:
                return 'M';
            case SDLK_n:
                return 'N';
            case SDLK_o:
                return 'O';
            case SDLK_p:
                return 'P';
            case SDLK_q:
                return 'Q';
            case SDLK_r:
                return 'R';
            case SDLK_s:
                return 'S';
            case SDLK_t:
                return 'T';
            case SDLK_u:
                return 'U';
            case SDLK_v:
                return 'V';
            case SDLK_w:
                return 'W';
            case SDLK_x:
                return 'X';
            case SDLK_y:
                return 'Y';
            case SDLK_z:
                return 'Z';
            case SDLK_COMMA:
                return ',';
            case SDLK_PERIOD:
                return '.';
            default:
                break;
            }
            break;
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

void PlatformSDL::load(const char* filename, uint8_t* destination, uint32_t size)
{
    FILE* file = fopen(filename, "r");
    if (file) {
        fseek(file, 2, SEEK_SET);
        fread(destination, 1, size, file);
        fclose(file);
    }
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
    SDL_RenderCopy(renderer, fontTexture, &sourceRect, &destinationRect);
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

void PlatformSDL::renderFrame()
{
    SDL_RenderPresent(renderer);
}
