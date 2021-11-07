#include "Platform.h"

Platform::Platform() :
    quit(false)
{
}

void Platform::show()
{
}

void Platform::chrout(uint8_t)
{
}

void Platform::displayImage(Image)
{
}

void Platform::updateTiles(uint8_t*, uint8_t*, uint8_t)
{
}

void Platform::renderTiles(uint8_t, uint8_t, uint16_t, uint16_t, uint8_t)
{
}

void Platform::renderItem(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderHealth(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderFace(uint8_t, uint16_t, uint16_t)
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

void Platform::shakeScreen()
{
}

void Platform::stopShakeScreen()
{
}

void Platform::startFadeScreen(uint16_t, uint16_t)
{
}

void Platform::fadeScreen(uint16_t, bool)
{
}

void Platform::stopFadeScreen()
{
}

void Platform::playNote(uint8_t)
{
}

void Platform::stopNote()
{
}

void Platform::loadModule(Module)
{
}

void Platform::playModule(Module)
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
