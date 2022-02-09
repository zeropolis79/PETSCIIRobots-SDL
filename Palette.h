#ifndef _PALETTE_H
#define _PALETTE_H

#include "Platform.h"

class Palette {
public:
    static void initialize();

    Palette(const uint16_t* palette = 0, uint16_t colorCount = 0, uint16_t fade = 15, uint16_t fadeBaseColor = 0x000);
    ~Palette();

    void setPalette(const uint16_t* palette, uint16_t colorCount);
    void setFade(uint16_t fade);
    __inline uint16_t fade() const;
    void setFadeBaseColor(uint16_t fadeBaseColor);
    __inline uint16_t* palette() const;

private:
    void update();

    uint16_t* sourcePalette;
    uint16_t* currentPalette;
    uint16_t colorCount_;
    uint16_t fade_;
    uint16_t fadeBaseColor;
};

#endif
