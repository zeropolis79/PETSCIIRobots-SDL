#include "Palette.h"

static uint8_t fadeTable[16][16];

void Palette::initialize()
{
    for (int fade = 1; fade < 16; fade++) {
        for (int value = 0; value < 16; value++) {
            fadeTable[fade][value] = (uint8_t)((fade * value) / 15);
        }
    }
}

Palette::Palette(const uint16_t* palette, uint16_t colorCount, uint16_t fade, uint16_t fadeBaseColor) :
    sourcePalette(0),
    currentPalette(0),
    fade(fade),
    fadeBaseColor(fadeBaseColor)
{
    setPalette(palette, colorCount);

    if (fade != 15) {
        update();
    }
}

Palette::~Palette()
{
    delete[] sourcePalette;
    delete[] currentPalette;
}

void Palette::setPalette(const uint16_t* palette, uint16_t colorCount)
{
    delete[] sourcePalette;
    delete[] currentPalette;

    sourcePalette = new uint16_t[colorCount];
    currentPalette = new uint16_t[colorCount];
    colorCount_ = colorCount;

    for (int i = 0; i < colorCount; i++) {
        sourcePalette[i] = palette[i];
        currentPalette[i] = palette[i];
    }
}

uint16_t Palette::colorCount() const
{
    return colorCount_;
}

void Palette::setFade(uint16_t fade)
{
    this->fade = fade;
    update();
}

void Palette::setFadeBaseColor(uint16_t fadeBaseColor)
{
    this->fadeBaseColor = fadeBaseColor;
}

uint16_t* Palette::palette() const
{
    return currentPalette;
}

void Palette::update()
{
    uint16_t baseR = fadeBaseColor >> 8;
    uint16_t baseG = (fadeBaseColor & 0x0f0) >> 4;
    uint16_t baseB = fadeBaseColor & 0x00f;
    for (uint16_t i = 0; i < colorCount_; i++) {
        uint16_t color = sourcePalette[i];
        uint16_t r = color >> 8;
        uint16_t g = (color & 0x0f0) >> 4;
        uint16_t b = color & 0x00f;
        int16_t rDelta = r - baseR;
        int16_t gDelta = g - baseG;
        int16_t bDelta = b - baseB;
        uint16_t fadedR = baseR + (rDelta >= 0 ? fadeTable[fade][rDelta] : -fadeTable[fade][-rDelta]);
        uint16_t fadedG = baseG + (gDelta >= 0 ? fadeTable[fade][gDelta] : -fadeTable[fade][-gDelta]);
        uint16_t fadedB = baseB + (bDelta >= 0 ? fadeTable[fade][bDelta] : -fadeTable[fade][-bDelta]);
        currentPalette[i] = (uint16_t)((fadedR << 8) | (fadedG << 4) | fadedB);
    }
}
