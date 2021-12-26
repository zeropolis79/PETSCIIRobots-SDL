CXXFLAGS :=-g -std=c++11 -DPLATFORM_SCREEN_WIDTH=320 -DPLATFORM_SCREEN_HEIGHT=200 -DPLATFORM_MAP_WINDOW_TILES_WIDTH=11 -DPLATFORM_MAP_WINDOW_TILES_HEIGHT=7 -DPLATFORM_TILE_BASED_RENDERING -DPLATFORM_IMAGE_BASED_TILES
LDFLAGS :=

CXXFLAGS += `pkg-config --cflags sdl2`
CXXFLAGS += `pkg-config --cflags SDL2_image`
LDFLAGS += `pkg-config --libs sdl2`
LDFLAGS += `pkg-config --libs SDL2_image`

SOURCES := petrobots.cpp Platform.cpp PlatformSDL.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := petrobots

all: $(TARGET)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)
