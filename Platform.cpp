#include "Platform.h"

Platform::Platform() :
    quit(false)
{
}

#ifndef _AMIGA
Platform::~Platform()
{
}
#endif

void Platform::show()
{
}

void Platform::chrout(uint8_t)
{
}

void Platform::keyRepeat()
{
}

bool Platform::isKeyOrJoystickPressed(bool)
{
    return false;
}

uint16_t Platform::readJoystick(bool)
{
    return 0;
}

void Platform::displayImage(Image)
{
}

void Platform::updateTiles(uint8_t*, uint8_t*, uint8_t)
{
}

void Platform::renderTiles(uint8_t, uint8_t, uint16_t, uint16_t, uint8_t, uint8_t)
{
}

void Platform::renderItem(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderKey(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderHealth(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderFace(uint8_t, uint16_t, uint16_t)
{
}

void Platform::renderLiveMap(uint8_t*)
{
}

void Platform::renderLiveMapTile(uint8_t*, uint8_t, uint8_t)
{
}

void Platform::renderLiveMapUnits(uint8_t*, uint8_t*, uint8_t*, uint8_t*, uint8_t, bool)
{
}

void Platform::showCursor(uint16_t, uint16_t)
{
}

void Platform::hideCursor()
{
}

#ifdef PLATFORM_CURSOR_SHAPE_SUPPORT
void Platform::setCursorShape(CursorShape)
{
}
#endif

void Platform::fillRect(uint16_t, uint16_t, uint16_t, uint16_t, uint8_t)
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

void Platform::pauseModule()
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

void Platform::renderFrame(bool)
{
}

void Platform::waitForScreenMemoryAccess()
{
}

Platform* platform = 0;
