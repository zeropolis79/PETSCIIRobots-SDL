#include "Platform.h"

Platform::Platform() :
    quit(false)
{
}

void Platform::show()
{
}

void Platform::displayImage(uint8_t)
{
}

void Platform::updateTiles(uint8_t*, uint8_t*, uint8_t)
{
}

void Platform::renderSprite(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderItem(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderHealth(uint8_t, uint16_t, uint16_t)
{
}

void Platform::showCursor(uint16_t, uint16_t)
{
}

void Platform::hideCursor()
{
}

void Platform::startShakeScreen()
{
}

void Platform::stopShakeScreen()
{
}

void Platform::startFlashScreen()
{
}

void Platform::flashScreen()
{
}

void Platform::stopFlashScreen()
{
}

void Platform::playModule(uint8_t)
{
}

void Platform::setSongPosition(uint8_t)
{
}

void Platform::stopModule()
{
}

void Platform::playSample(uint8_t)
{
}

void Platform::stopSample()
{
}

void Platform::renderFrame()
{
}

Platform* platform = 0;
