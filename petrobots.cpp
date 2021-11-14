/**
 * PETSCII Robots (Portable version)
 * by David Murray 2020
 * dfwgreencars@gmail.com
 * ported by Vesa Halttunen 2021
 * vesuri@jormas.com
 */

#ifdef _AMIGA
#include "PlatformAmiga.h"
#else
#include "PlatformSDL.h"
#endif
#include "petrobots.h"

uint8_t DESTRUCT_PATH[256]; // Destruct path array (256 bytes)
uint8_t TILE_ATTRIB[256];   // Tile attrib array (256 bytes)
#ifndef PLATFORM_SPRITE_SUPPORT
uint8_t TILE_DATA_TL[256];  // Tile character top-left (256 bytes)
uint8_t TILE_DATA_TM[256];  // Tile character top-middle (256 bytes)
uint8_t TILE_DATA_TR[256];  // Tile character top-right (256 bytes)
uint8_t TILE_DATA_ML[256];  // Tile character middle-left (256 bytes)
uint8_t TILE_DATA_MM[256];  // Tile character middle-middle (256 bytes)
uint8_t TILE_DATA_MR[256];  // Tile character middle-right (256 bytes)
uint8_t TILE_DATA_BL[256];  // Tile character bottom-left (256 bytes)
uint8_t TILE_DATA_BM[256];  // Tile character bottom-middle (256 bytes)
uint8_t TILE_DATA_BR[256];  // Tile character bottom-right (256 bytes)
#endif

// These arrays can go anywhere in RAM
uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
uint8_t UNIT_DIRECTION[32]; // Movement direction of unit (32 bytes)
uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
uint8_t MAP_PRECALC[77];    // Stores pre-calculated objects for map window (77 bytes)
uint8_t MAP_PRECALC_DIRECTION[77];    // Stores pre-calculated object directions for map window (77 bytes)
#ifdef OPTIMIZED_MAP_RENDERING
uint8_t PREVIOUS_MAP_BACKGROUND[77];
uint8_t PREVIOUS_MAP_FOREGROUND[77];
uint8_t PREVIOUS_MAP_FOREGROUND_VARIANT[77];
#endif

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
uint8_t KEY_CONFIG[26];
enum KEYS {
    KEY_MOVE_UP,
    KEY_MOVE_DOWN,
    KEY_MOVE_LEFT,
    KEY_MOVE_RIGHT,
    KEY_FIRE_UP,
    KEY_FIRE_DOWN,
    KEY_FIRE_LEFT,
    KEY_FIRE_RIGHT,
    KEY_CYCLE_WEAPONS,
    KEY_CYCLE_ITEMS,
    KEY_USE,
    KEY_SEARCH,
    KEY_MOVE,
    KEY_LIVE_MAP,
    KEY_LIVE_MAP_ROBOTS,
    KEY_PAUSE,
    KEY_MUSIC,
    KEY_CHEAT,
    KEY_CURSOR_UP,
    KEY_CURSOR_DOWN,
    KEY_CURSOR_LEFT,
    KEY_CURSOR_RIGHT,
    KEY_SPACE,
    KEY_RETURN,
    KEY_YES,
    KEY_NO
};

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
uint8_t UNIT_TYPE[64];  // Unit type 0=none (64 bytes)	
uint8_t UNIT_LOC_X[64]; // Unit X location (64 bytes)
uint8_t UNIT_LOC_Y[64]; // Unit X location (64 bytes)
uint8_t UNIT_A[64];
uint8_t UNIT_B[64];
uint8_t UNIT_C[64];
uint8_t UNIT_D[64];
int8_t UNIT_HEALTH[64];    // Unit health (0 to 11) (64 bytes)
uint8_t MAP_UNUSED[256];
uint8_t MAP[8 * 1024];      // Location of MAP (8K)
// END OF MAP FILE

uint8_t TILE;           // The tile number to be plotted
uint8_t DIRECTION;      // The direction of the tile to be plotted
uint8_t WALK_FRAME;     // Player walking animation frame
uint8_t MAP_X;          // Current X location on map
uint8_t MAP_Y;          // Current Y location on map
uint8_t MAP_WINDOW_X;   // Top left location of what is displayed in map window
uint8_t MAP_WINDOW_Y;   // Top left location of what is displayed in map window
uint8_t DECNUM;         // a decimal number to be displayed onscreen as 3 digits.
uint8_t ATTRIB;         // Tile attribute value
uint8_t UNIT;           // Current unit being processed
uint8_t TEMP_A;         // used within some routines
uint8_t TEMP_B;         // used within some routines
uint8_t TEMP_C;         // used within some routines
uint8_t TEMP_D;         // used within some routines
uint8_t CURSOR_X;       // For on-screen cursor
uint8_t CURSOR_Y;       // For on-screen cursor
uint8_t CURSOR_ON;      // Is cursor active or not? 1=yes 0=no
uint8_t REDRAW_WINDOW;  // 1=yes 0=no
uint8_t MOVE_RESULT;    // 1=Move request success, 0=fail.
uint8_t UNIT_FIND;      // 255=no unit present.
uint8_t MOVE_TYPE;      // %00000001=WALK %00000010=HOVER
uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

void (*CINV)(void);     // $90-$91 Vector: Hardware Interrupt
uint8_t LSTX;           // $97 Current Key Pressed: 255 = No Key
uint8_t* MAP_SOURCE;    // $FD
uint8_t SCREEN_MEMORY[40 * 25]; // $8000

int main(int argc, char *argv[])
{
    PlatformClass platformInstance;

    if (!platform) {
        return 1;
    }

    int i;
    for (i = 0; i < sizeof(MAPNAME); i++) {
#ifdef PLATFORM_GZIP_SUPPORT
        MAPNAME[i] = "level-a.gz"[i];
#else
        MAPNAME[i] = "level-a"[i];
#endif
    }
    for (i = 0; i < sizeof(LOAD_MSG2); i++) {
        LOAD_MSG2[i] = "loading map:"[i];
    }

    platform->stopNote(); // RESET SOUND TO ZERO
    DISPLAY_LOAD_MESSAGE1();
    TILE_LOAD_ROUTINE();
    SETUP_INTERRUPT();
    SET_CONTROLS(); // copy initial key controls
    while (!platform->quit) {
        INTRO_SCREEN();
    }
    return 0;
}

void INIT_GAME()
{
    SCREEN_SHAKE = 0;
    LIVE_MAP_ON = 0;
    RESET_KEYS_AMMO();
    platform->fadeScreen(0, false);
    DISPLAY_GAME_SCREEN();
    DISPLAY_LOAD_MESSAGE2();
    platform->fadeScreen(15, false);
    MAP_LOAD_ROUTINE();
#ifdef PLATFORM_MODULE_BASED_AUDIO
    START_IN_GAME_MUSIC();
#endif
    SET_DIFF_LEVEL();
    ANIMATE_PLAYER();
    CACULATE_AND_REDRAW();
#ifdef OPTIMIZED_MAP_RENDERING
    INVALIDATE_PREVIOUS_MAP();
#endif
    DRAW_MAP_WINDOW();
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    UNIT_TYPE[0] = 1;
    SET_INITIAL_TIMERS();
    PRINT_INTRO_MESSAGE();
    KEYTIMER = 30;
    MAIN_GAME_LOOP();
}

const char* TILENAME = "tileset.pet";
#ifdef PLATFORM_GZIP_SUPPORT
char MAPNAME[11];
#else
char MAPNAME[8];
#endif
const char* SNDNAME = "pdrv-pet";
const char* LOADMSG1 = "loading tiles...\x0d";
const char* LOADMSG2 = "\x93loading map...\x0d";
uint8_t KEYS = 0; // bit0=spade bit2=heart bit3=star
uint8_t AMMO_PISTOL = 0; // how much ammo for the pistol
uint8_t AMMO_PLASMA = 0; // how many shots of the plasmagun
uint8_t INV_BOMBS = 0; // How many bombs do we have
uint8_t INV_EMP = 0; // How many EMPs do we have
uint8_t INV_MEDKIT = 0; // How many medkits do we have?
uint8_t INV_MAGNET = 0; // How many magnets do we have?
uint8_t SELECTED_WEAPON = 0; // 0=none 1=pistol 2=plasmagun
uint8_t SELECTED_ITEM = 0; // 0=none 1=bomb 2=emp 3=medkit 4=magnet
uint8_t SELECT_TIMEOUT = 0; // can only change weapons once it hits zero
uint8_t ANIMATE = 1; // 0=DISABLED 1=ENABLED
uint8_t BIG_EXP_ACT = 0; // 0=No explosion active 1=big explosion active
uint8_t MAGNET_ACT = 0; // 0=no magnet active 1=magnet active
uint8_t PLASMA_ACT = 0; // 0=No plasma fire active 1=plasma fire active
uint8_t RANDOM = 0; // used for random number generation
uint8_t BORDER = 0; // Used for border flash timing
uint8_t SCREEN_SHAKE = 0; // 1=shake 0=no shake
uint8_t CONTROL = 0; // 0=keyboard 1=custom keys 2=snes
uint16_t BORDER_COLOR = 0xf00; // Used for border flash coloring
#ifdef _AMIGA
const char* INTRO_MESSAGE   = "welcome to amiga-robots!\xff"
                              "by david murray 2021\xff"
                              "amiga port by vesa halttunen";
#else
const char* INTRO_MESSAGE	= "welcome to sdl-robots!\xff"
                              "by david murray 2021\xff"
                              "sdl port by vesa halttunen";
#endif
const char* MSG_CANTMOVE = "can't move that!";
const char* MSG_BLOCKED = "blocked!";
const char* MSG_SEARCHING = "searching";
const char* MSG_NOTFOUND = "nothing found here.";
const char* MSG_FOUNDKEY = "you found a key card!";
const char* MSG_FOUNDGUN = "you found a pistol!";
const char* MSG_FOUNDEMP = "you found an emp device!";
const char* MSG_FOUNDBOMB = "you found a timebomb!";
const char* MSG_FOUNDPLAS = "you found a plasma gun!";
const char* MSG_FOUNDMED = "you found a medkit!";
const char* MSG_FOUNDMAG = "you found a magnet!";
const char* MSG_MUCHBET = "ahhh, much better!";
const char* MSG_EMPUSED = "emp activated!\xff"
                          "nearby robots are rebooting.";
const char* MSG_TERMINATED = "you're terminated!";
const char* MSG_TRANS1 = "transporter will not activate\xff"
                         "until all robots destroyed.";
const char* MSG_ELEVATOR = "[ elevator panel ]  down\xff"
                           "[  select level  ]  opens";
const char* MSG_LEVELS = "[                ]  door";
const char* MSG_PAUSED = "game paused.\xff"
                         "exit game (y/n)";
const char* MSG_MUSICON = "music on.";
const char* MSG_MUSICOFF = "music off.";
uint8_t SELECTED_MAP = 0;
const char* MAP_NAMES = "01-research lab "
                        "02-headquarters "
                        "03-the village  "
                        "04-the islands  "
                        "05-downtown     "
                        "06-pi university"
                        "07-more islands "
                        "08-robot hotel  "
                        "09-forest moon  "
                        "10-death tower  "
                        "11-river death  "
                        "12-bunker       "
                        "13-castle       ";
// THE FOLLOWING ARE USED BY THE SOUND SYSTEM*
uint8_t TEMPO_TIMER = 0; // used for counting down to the next tick
uint8_t TEMPO = 7; // How many IRQs between ticks
uint8_t DATA_LINE = 0; // used for playback to keep track of which line we are executing.
uint8_t ARP_MODE = 0; // 0=no 1=major 2=minor 3=sus4
uint8_t CHORD_ROOT = 0; // root note of the chord
uint8_t MUSIC_ON = 0; // 0=off 1=on
uint8_t SOUND_EFFECT = 0xff; // FF=OFF or number of effect in progress

void DISPLAY_LOAD_MESSAGE1()
{
    for (int Y = 0; Y != 17; Y++) {
        platform->chrout(LOADMSG1[Y]);
    }
}

// Displays loading message for map.
void DISPLAY_LOAD_MESSAGE2()
{
    int Y;
    for (Y = 0; Y != 12; Y++) {
        writeToScreenMemory(0x190 + Y, convertToPETSCII(LOAD_MSG2[Y]));
    }
    const char* name = CALC_MAP_NAME();
    for (Y = 0; Y != 16; Y++) {
        writeToScreenMemory(0x19c + Y, convertToPETSCII(name[Y]));
    }
}

char LOAD_MSG2[13];

void SETUP_INTERRUPT()
{
    platform->setInterrupt(&RUNIRQ);
}

// This is the routine that runs every 60 seconds from the IRQ.
// BGTIMER1 is always set to 1 every cycle, after which the main
// program will reset it to 0 when it is done with it's work for
// that cycle.  BGTIMER2 is a count-down to zero and then stays
// there.
void RUNIRQ()
{
#ifndef PLATFORM_MODULE_BASED_AUDIO
    MUSIC_ROUTINE();
#endif
    UPDATE_GAME_CLOCK();
    ANIMATE_WATER();
    BGTIMER1 = 1;
    if (BGTIMER2 != 0) {
        BGTIMER2--;
    }
    if (KEYTIMER != 0) {
        KEYTIMER--;
    }
    if (BORDER != 0) {
        BORDER--;
        platform->fadeScreen(15 - BORDER);
    }
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    if (CLOCK_ACTIVE != 0 && SCREEN_SHAKE != 0) {
        platform->shakeScreen();
    }
#endif
    // Back to usual IRQ routine
}
uint8_t BGTIMER1 = 0;
uint8_t BGTIMER2 = 0;
uint8_t KEYTIMER = 0; // Used for repeat of movement

// Since the PET has no real-time clock, and the Jiffy clock
// is a pain to read from assembly language, I have created my own.
void UPDATE_GAME_CLOCK()
{
    if (CLOCK_ACTIVE != 1) {
        return;
    }
    CYCLES++;
    if (CYCLES != platform->framesPerSecond()) { // 60 for ntsc or 50 for pal
        return;
    }
    CYCLES = 0;
    SECONDS++;
    if (SECONDS != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES++;
    if (MINUTES != 60) {
        return;
    }
    SECONDS = 0;
    MINUTES = 0;
    HOURS++;
}

uint8_t HOURS = 0;
uint8_t MINUTES = 0;
uint8_t SECONDS = 0;
uint8_t CYCLES = 0;
uint8_t CLOCK_ACTIVE = 0;

// This routine spaces out the timers so that not everything
// is running out once. It also starts the game_clock.
void SET_INITIAL_TIMERS()
{
    CLOCK_ACTIVE = 1;
    for (int X = 1; X != 48; X++) {
        UNIT_TIMER_A[X] = X;
        UNIT_TIMER_B[X] = 0;
    }
}

void MAIN_GAME_LOOP()
{
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] != 1) { // Is player unit alive
            GAME_OVER();
            return;
        }
        if (CONTROL != 2) {
            // Keyboard controls here.
            KEY_REPEAT();
            uint8_t A = platform->getin();
            if (A != 0) {
                KEYTIMER = 5;
                if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                    UNIT = 0;
                    MOVE_TYPE = 1; // %00000001
                    REQUEST_WALK_RIGHT();
                    AFTER_MOVE();
                } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_LEFT();
                    AFTER_MOVE();
                } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_DOWN();
                    AFTER_MOVE();
                } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_UP();
                    AFTER_MOVE();
                } else if (A == KEY_CONFIG[KEY_CYCLE_WEAPONS]) {
                    CYCLE_WEAPON();
                    CLEAR_KEY_BUFFER();
                } else if (A == KEY_CONFIG[KEY_CYCLE_ITEMS]) {
                    CYCLE_ITEM();
                    CLEAR_KEY_BUFFER();
                } else if (A == KEY_CONFIG[KEY_MOVE]) {
                    MOVE_OBJECT();
                    CLEAR_KEY_BUFFER();
                } else if (A == KEY_CONFIG[KEY_SEARCH]) {
                    SEARCH_OBJECT();
                    CLEAR_KEY_BUFFER();
                } else if (A == KEY_CONFIG[KEY_USE]) {
                    USE_ITEM();
                    CLEAR_KEY_BUFFER();
                } else if (A == KEY_CONFIG[KEY_FIRE_UP]) {
                    FIRE_UP();
                    KEYTIMER = 20;
                } else if (A == KEY_CONFIG[KEY_FIRE_LEFT]) {
                    FIRE_LEFT();
                    KEYTIMER = 20;
                } else if (A == KEY_CONFIG[KEY_FIRE_DOWN]) {
                    FIRE_DOWN();
                    KEYTIMER = 20;
                } else if (A == KEY_CONFIG[KEY_FIRE_RIGHT]) {
                    FIRE_RIGHT();
                    KEYTIMER = 20;
                } else if (A == KEY_CONFIG[KEY_PAUSE]) { // RUN/STOP
                    done = PAUSE_GAME();
                } else if (A == KEY_CONFIG[KEY_CHEAT]) { // SHIFT-C
                    CHEATER();
                } else if (A == KEY_CONFIG[KEY_MUSIC]) { // SHIFT-M
                    TOGGLE_MUSIC();
                    CLEAR_KEY_BUFFER();
                }
#ifdef PLATFORM_LIVE_MAP_SUPPORT
                else if (A == KEY_CONFIG[KEY_LIVE_MAP]) {
                    TOGGLE_LIVE_MAP();
                } else if (A == KEY_CONFIG[KEY_LIVE_MAP_ROBOTS]) {
                    TOGGLE_LIVE_MAP_ROBOTS();
                }
#endif
            }
        } else {
            // SNES CONTROLLER starts here
            // TODO implement SC01
        }
    }
}

void TOGGLE_MUSIC()
{
    if (MUSIC_ON == 1) {
        PRINT_INFO(MSG_MUSICOFF);
        MUSIC_ON = 0;
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->playModule(Platform::ModuleSoundFX);
#else
        platform->stopNote(); // turn off sound
#endif
    } else {
        PRINT_INFO(MSG_MUSICON);
        MUSIC_ON = 1;
        START_IN_GAME_MUSIC();
    }
}

void START_IN_GAME_MUSIC()
{
    MUSIC_ON = 1;
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playModule(LEVEL_MUSIC[SELECTED_MAP]);
#else
    if (SOUND_EFFECT == 0xFF) { // FF=NO sound effect in progress
        DATA_LINE = 0;
        CUR_PATTERN = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    } else {
        // apparently a sound-effect is active, so we do things differently.
        DATA_LINE_TEMP = 0;
        PATTERN_TEMP = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    }
#endif
}

#ifdef PLATFORM_MODULE_BASED_AUDIO
Platform::Module LEVEL_MUSIC[] = {
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1,
    Platform::ModuleInGame2,
    Platform::ModuleInGame3,
    Platform::ModuleInGame4,
    Platform::ModuleInGame1
};
#else
uint8_t LEVEL_MUSIC[] = { 0,1,2,0,1,2,0,1,2,0,1,2,0 };
#endif

// TEMP ROUTINE TO GIVE ME ALL ITEMS AND WEAPONS
void CHEATER()
{
    KEYS = 7;
    AMMO_PISTOL = 100;
    AMMO_PLASMA = 100;
    INV_BOMBS = 100;
    INV_EMP = 100;
    INV_MEDKIT = 100;
    INV_MAGNET = 100;
    SELECTED_WEAPON = 1;
    SELECTED_ITEM = 1;
#ifdef PLATFORM_IMAGE_SUPPORT
    REDRAW_WINDOW = 1;
#endif
    DISPLAY_KEYS();
    DISPLAY_WEAPON();
    DISPLAY_ITEM();
}

bool PAUSE_GAME()
{
    PLAY_SOUND(15);
    // pause clock
    CLOCK_ACTIVE = 0;
    // display message to user
    SCROLL_INFO();
    PRINT_INFO(MSG_PAUSED);
//    for (BGTIMER1 = 0; BGTIMER1 != 1;); // to prevent double-tap of run/stop
    CLEAR_KEY_BUFFER();
    platform->renderFrame();
    while (!platform->quit) {
        uint8_t A = platform->getin();
        if (A == KEY_CONFIG[KEY_PAUSE] || // RUN/STOP
            A == KEY_CONFIG[KEY_NO]) { // N-KEY
            SCROLL_INFO();
            SCROLL_INFO();
            SCROLL_INFO();
            CLEAR_KEY_BUFFER();
            CLOCK_ACTIVE = 1;
            PLAY_SOUND(15);
            return false;
        } else if (A == KEY_CONFIG[KEY_YES]) { // Y-KEY
            UNIT_TYPE[0] = 0; // make player dead
            PLAY_SOUND(15);
            GOM4();
            return true;
        }
    }
    return false;
}

void CLEAR_KEY_BUFFER()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    KEYTIMER = 20;
}

void USE_ITEM()
{
    // check select timeout to prevent accidental double-tap
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    // First figure out which item to use.
    switch (SELECTED_ITEM) {
    case 1: // BOMB
        USE_BOMB();
        break;
    case 2: // EMP
        USE_EMP();
        break;
    case 3: // MEDKIT
        USE_MEDKIT();
        break;
    case 4: // MAGNET
        USE_MAGNET();
        break;
    default:
        break;
    }
}

void USE_BOMB()
{
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        // Now scan for any units at that location:
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) { // 255 means no unit found.
            for (int X = 28; X != 32; X++) { // Start of weapons units
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 6; // bomb AI
                    UNIT_TILE[X] = 130; // bomb tile
                    UNIT_LOC_X[X] = MAP_X;
                    UNIT_LOC_Y[X] = MAP_Y;
                    UNIT_TIMER_A[X] = 100; // How long until explosion?
                    UNIT_A[X] = 0;
                    INV_BOMBS--;
                    DISPLAY_ITEM();
                    REDRAW_WINDOW = 1;
                    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used, pet version only
                    PLAY_SOUND(6); // SOUND PLAY
                    return;
                }
            }
            return; // no slots available right now, abort.
        }
    }
    BOMB_MAGNET_COMMON2();
}

void USE_MAGNET()
{
    if (MAGNET_ACT != 0) { // only one magnet active at a time.
        return;
    }
    USER_SELECT_OBJECT();
    // NOW TEST TO SEE IF THAT SPOT IS OPEN
    if (BOMB_MAGNET_COMMON1()) {
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 20; // MAGNET AI
                UNIT_TILE[X] = 134; // MAGNET tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 1; // How long until ACTIVATION
                UNIT_TIMER_B[X] = 255; // how long does it live -A
                UNIT_A[X] = 3; // how long does it live -B
                MAGNET_ACT = 1; // only one magnet allowed at a time.
                INV_MAGNET--;
                DISPLAY_ITEM();
                REDRAW_WINDOW = 1;
                PLAY_SOUND(6); // move sound, SOUND PLAY
                return;
            }
        }
        return; // no slots available right now, abort.
    }
    BOMB_MAGNET_COMMON2();
}

bool BOMB_MAGNET_COMMON1()
{
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->hideCursor();
#else
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x01) == 0x01; // %00000001 is that spot available for something to move onto it?
}

void BOMB_MAGNET_COMMON2()
{
    PRINT_INFO(MSG_BLOCKED);
    PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
}

void USE_EMP()
{
    EMP_FLASH();
    REDRAW_WINDOW = 0;  // attempt to delay window redrawing (pet only)
    PLAY_SOUND(3);  // EMP sound, SOUND PLAY
    INV_EMP--;
    DISPLAY_ITEM();
    for (int X = 1; X != 28; X++) { // start with unit#1 (skip player)
        if (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
            UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
            UNIT_LOC_X[X] <= (MAP_WINDOW_X + 10) &&  // NOW CHECK VERTICAL
            UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
            UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + 6)) {
            UNIT_TIMER_A[X] = 255;
            // test to see if unit is above water
            MAP_X = UNIT_LOC_X[X];
            MAP_Y = UNIT_LOC_Y[X];
            GET_TILE_FROM_MAP();
            if (TILE == 204) {  // WATER
                UNIT_TYPE[X] = 5;
                UNIT_TIMER_A[X] = 5;
                UNIT_A[X] = 60; // how long to show sparks.
                UNIT_TILE[X] = 140; // Electrocuting tile
            }
        }
    }
    PRINT_INFO(MSG_EMPUSED);
    SELECT_TIMEOUT = 3; // 3 cycles before next item can be used
}

void USE_MEDKIT()
{
    if (UNIT_HEALTH[0] == 12) {    // Do we even need the medkit?
        return;
    }
    // Now figure out how many HP we need to be healthy.
    TEMP_A = 12 - UNIT_HEALTH[0];  // how many we need.
    if (INV_MEDKIT >= TEMP_A) {
        // we had more than we need, so go to full health.
        UNIT_HEALTH[0] = 12;
        INV_MEDKIT -= TEMP_A;
    } else {
        // we had less than we need, so we'll use what is available.
        UNIT_HEALTH[0] += INV_MEDKIT;
        INV_MEDKIT = 0;
    }
    DISPLAY_PLAYER_HEALTH();
    DISPLAY_ITEM();
    PLAY_SOUND(2); // MEDKIT SOUND, SOUND PLAY
    PRINT_INFO(MSG_MUCHBET);
}

void FIRE_UP()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_UP_PISTOL();
    } else {
        FIRE_UP_PLASMA();
    }
    UNIT_DIRECTION[0] = 0;
}

void FIRE_UP_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_UP_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 12; // Fire pistol up AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_DOWN_PISTOL();
    } else {
        FIRE_DOWN_PLASMA();
    }
    UNIT_DIRECTION[0] = 1;
}

void FIRE_DOWN_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 244; // tile for vertical weapons fire
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_DOWN_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 13; // Fire pistol DOWN AI routine
            UNIT_TILE[X] = 240; // tile for vertical plasma bolt
            UNIT_A[X] = 3; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_LEFT_PISTOL();
    } else {
        FIRE_LEFT_PLASMA();
    }
    UNIT_DIRECTION[0] = 2;
}

void FIRE_LEFT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_LEFT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 14; // Fire pistol LEFT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT()
{
    if (SELECTED_WEAPON == 0) {
        return;
    } else if (SELECTED_WEAPON == 1) {
        FIRE_RIGHT_PISTOL();
    } else {
        FIRE_RIGHT_PLASMA();
    }
    UNIT_DIRECTION[0] = 3;
}

void FIRE_RIGHT_PISTOL()
{
    if (AMMO_PISTOL == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 245; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 0; // weapon-type = pistol
            AFTER_FIRE(X);
            return;
        }
    }
}

void FIRE_RIGHT_PLASMA()
{
    if (BIG_EXP_ACT == 1) {
        return;
    }
    if (PLASMA_ACT == 1) {
        return;
    }
    if (AMMO_PLASMA == 0) {
        return;
    }
    for (int X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            UNIT_TYPE[X] = 15; // Fire pistol RIGHT AI routine
            UNIT_TILE[X] = 241; // tile for horizontal weapons fire
            UNIT_A[X] = 5; // travel distance.
            UNIT_B[X] = 1; // weapon-type = plasma
            PLASMA_ACT = 1;
            AFTER_FIRE(X);
            return;
        }
    }
}

void AFTER_FIRE(int X)
{
    UNIT_TIMER_A[X] = 0;
    UNIT_LOC_X[X] = UNIT_LOC_X[0];
    UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
    UNIT = X;
    if (SELECTED_WEAPON != 2) {
        PLAY_SOUND(9); // PISTOL-SOUND, SOUND PLAY
        AMMO_PISTOL--;
        DISPLAY_WEAPON();
    } else {
        PLAY_SOUND(8); // PLASMA-GUN-SOUND, SOUND PLAY
        AMMO_PLASMA--;
        DISPLAY_WEAPON();
    }
}

// This routine checks KEYTIMER to see if it has
// reached zero yet.  If so, it clears the LSTX
// variable used by the kernal, so that it will
// register a new keypress.
void KEY_REPEAT()
{
    if (KEYTIMER != 0) {
        return;
    }
    if (LSTX != 255) { // no key pressed
        LSTX = 255; // clear LSTX register
        KEYTIMER = 6;
    } else {
        // No key pressed, reset all to defaults
        KEY_FAST = 0;
        KEYTIMER = 6;
    }
}

// This routine handles things that are in common to
// all 4 directions of movement.
void AFTER_MOVE()
{
    if (MOVE_RESULT == 1) {
        ANIMATE_PLAYER();
    }
    CACULATE_AND_REDRAW();
    // now reset key-repeat rate
    if (KEY_FAST == 0) {
        // FIRST REPEAT
        KEYTIMER = 13;
        KEY_FAST++;
    } else {
        // SUBSEQUENT REPEATS
        KEYTIMER = 6;
    }
}

uint8_t KEY_FAST = 0; // 0=DEFAULT STATE

// This routine is invoked when the user presses S to search
// an object such as a crate, chair, or plant.
void SEARCH_OBJECT()
{
    USER_SELECT_OBJECT();
    REDRAW_WINDOW = 1;
    // first check of object is searchable
    CALC_COORDINATES();
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x40) == 0) { // %01000000 can search attribute
#ifdef PLATFORM_CURSOR_SUPPORT
        platform->hideCursor();
#else
        CURSOR_ON = 0;
#endif
    } else {
        // is the tile a crate?
        if (TILE == 41 || TILE == 45 || TILE == 199) { // BIG CRATE / small CRATE / "Pi" CRATE
            TILE = DESTRUCT_PATH[TILE];
            PLOT_TILE_TO_MAP();
        }
        // Now check if there is an object there.
        PRINT_INFO(MSG_SEARCHING);
        for (SEARCHBAR = 0; SEARCHBAR != 8; SEARCHBAR++) {
            for (BGTIMER2 = 18; BGTIMER2 != 0;) { // delay time between search periods
                PET_SCREEN_SHAKE();
                BACKGROUND_TASKS();
            }
            writeToScreenMemory(0x3C9 + SEARCHBAR, 46); // PERIOD
        }
#ifdef PLATFORM_CURSOR_SUPPORT
        platform->hideCursor();
#else
        CURSOR_ON = 0;
        DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
        CALC_COORDINATES();
        CHECK_FOR_HIDDEN_UNIT();
        if (UNIT_FIND == 255) {
            PRINT_INFO(MSG_NOTFOUND);
            return;
        }
        TEMP_A = UNIT_TYPE[UNIT_FIND];  // store object type
        TEMP_B = UNIT_A[UNIT_FIND]; // store secondary info
        UNIT_TYPE[UNIT_FIND] = 0; // DELETE ITEM ONCE FOUND
        // ***NOW PROCESS THE ITEM FOUND***
        PLAY_SOUND(10); // ITEM-FOUND-SOUND, SOUND PLAY
        if (TEMP_A == 128) {    // key
            if (TEMP_B == 0) {  // WHICH SORT OF KEY?
                KEYS |= 0x01; // %00000001 Add spade key
            } else if (TEMP_B == 1) {
                KEYS |= 0x02; // %00000010 Add heart key
            } else {
                KEYS |= 0x04; // %00000100 Add star key
            }
            PRINT_INFO(MSG_FOUNDKEY);
            DISPLAY_KEYS();
        } else if (TEMP_A == 129) { // TIME BOMB
            INV_BOMBS += TEMP_B;
            PRINT_INFO(MSG_FOUNDBOMB);
            DISPLAY_ITEM();
        } else if (TEMP_A == 130) { // EMP
            INV_EMP += TEMP_B;
            PRINT_INFO(MSG_FOUNDEMP);
            DISPLAY_ITEM();
        } else if (TEMP_A == 131) { // PISTOL
            if (AMMO_PISTOL + TEMP_B > 255) { // If we rolled over past 255
                AMMO_PISTOL = 255; // set it to 255.
            } else {
                AMMO_PISTOL += TEMP_B;
            }
            PRINT_INFO(MSG_FOUNDGUN);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 132) { // PLASMA GUN
            AMMO_PLASMA += TEMP_B;
            PRINT_INFO(MSG_FOUNDPLAS);
            DISPLAY_WEAPON();
        } else if (TEMP_A == 133) { // MEDKIT
            INV_MEDKIT += TEMP_B;
            PRINT_INFO(MSG_FOUNDMED);
            DISPLAY_ITEM();
        } else if (TEMP_A == 134) { // MAGNET
            INV_MAGNET += TEMP_B;
            PRINT_INFO(MSG_FOUNDMAG);
            DISPLAY_ITEM();
        }
        // ADD CODE HERE FOR OTHER OBJECT TYPES
    }
}

uint8_t SEARCHBAR = 0; // to count how many periods to display.

// combines cursor location with window location
// to determine coordinates for MAP_X and MAP_Y
void CALC_COORDINATES()
{
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
}

// This routine is called by routines such as the move, search,
// or use commands.  It displays a cursor and allows the user
// to pick a direction of an object.
void USER_SELECT_OBJECT()
{
    PLAY_SOUND(16); // beep sound, SOUND PLAY
    CURSOR_X = 5;
    CURSOR_Y = 3;
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->showCursor(CURSOR_X, CURSOR_Y);
#else
    CURSOR_ON = 1;
    REVERSE_TILE();
#endif
    // First ask user which object to move
    while (!platform->quit) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
#ifdef PLATFORM_CURSOR_SUPPORT
            platform->hideCursor();
#else
            CURSOR_ON = 0;
#endif
            return;
        }
        if (CONTROL != 2) {
            uint8_t A = platform->getin();
            if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                UNIT_DIRECTION[0] = 3;
                CURSOR_X++;
#ifdef PLATFORM_CURSOR_SUPPORT
                platform->showCursor(CURSOR_X, CURSOR_Y);
#endif
                return;
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                UNIT_DIRECTION[0] = 2;
                CURSOR_X--;
#ifdef PLATFORM_CURSOR_SUPPORT
                platform->showCursor(CURSOR_X, CURSOR_Y);
#endif
                return;
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                UNIT_DIRECTION[0] = 1;
                CURSOR_Y++;
#ifdef PLATFORM_CURSOR_SUPPORT
                platform->showCursor(CURSOR_X, CURSOR_Y);
#endif
                return;
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                UNIT_DIRECTION[0] = 0;
                CURSOR_Y--;
#ifdef PLATFORM_CURSOR_SUPPORT
                platform->showCursor(CURSOR_X, CURSOR_Y);
#endif
                return;
            }
        } else {
            // SNES controls for this routine
            // TODO implement MVSNES
        }
    }
}

void MOVE_OBJECT()
{
    USER_SELECT_OBJECT();
    // now test that object to see if it
    // is allowed to be moved.
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->hideCursor();
#else
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
    CALC_COORDINATES();
    CHECK_FOR_HIDDEN_UNIT();
    MOVTEMP_U = UNIT_FIND;
    GET_TILE_FROM_MAP();
    if ((TILE_ATTRIB[TILE] & 0x04) == 0) { // can it be moved?
        PRINT_INFO(MSG_CANTMOVE);
        PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
        return;
    }
    MOVTEMP_O = TILE; // Store which tile it is we are moving
    MOVTEMP_X = MAP_X; // Store original location of object
    MOVTEMP_Y = MAP_Y;
#ifdef PLATFORM_CURSOR_SUPPORT
    platform->showCursor(CURSOR_X, CURSOR_Y);
#else
    CURSOR_ON = 1;
    REVERSE_TILE();
#endif
    // NOW ASK THE USER WHICH DIRECTION TO MOVE IT TO
    while (!platform->quit) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
#ifdef PLATFORM_CURSOR_SUPPORT
            platform->hideCursor();
#else
            CURSOR_ON = 0;
#endif
            return;
        }
        if (CONTROL != 2) { // which controller are we using?
            // keyboard control
            uint8_t A = platform->getin();
            if (A == 0) {
                continue;
            } else if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                CURSOR_X++;
            } else if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                CURSOR_X--;
            } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                CURSOR_Y++;
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                CURSOR_Y--;
            }
        } else {
            // SNES controls
            // TODO
        }
        // NOW TEST TO SEE IF THAT SPOT IS OPEN
#ifdef PLATFORM_CURSOR_SUPPORT
        platform->hideCursor();
#else
        CURSOR_ON = 0;
        DRAW_MAP_WINDOW(); // ERASE THE CURSOR
#endif
        MAP_X = CURSOR_X + MAP_WINDOW_X;
        MOVTEMP_UX = MAP_X;
        MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
        MOVTEMP_UY = MAP_Y;
        GET_TILE_FROM_MAP();
        if (TILE_ATTRIB[TILE] & 0x20) { // %00100000 is that spot available for something to move onto it?
            // Now scan for any units at that location:
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) { // 255 means no unit found.
                PLAY_SOUND(6); // move sound, SOUND PLAY
                MOVTEMP_D = MAP_SOURCE[0]; // Grab current object
                MAP_SOURCE[0] = MOVTEMP_O; // replace with obect we are moving
                MAP_X = MOVTEMP_X; // RETRIEVE original location of object
                MAP_Y = MOVTEMP_Y;
                GET_TILE_FROM_MAP();
                int A = MOVTEMP_D;
                if (A == 148) { // trash compactor tile
                    A = 9; // Floor tile
                }
                MAP_SOURCE[0] = A; // Replace former location
                REDRAW_WINDOW = 1; // See the result
                if (MOVTEMP_U == 255) {
                    return;
                }
                UNIT_LOC_X[MOVTEMP_U] = MOVTEMP_UX;
                UNIT_LOC_Y[MOVTEMP_U] = MOVTEMP_UY;
                return;
            }
        }
        PRINT_INFO(MSG_BLOCKED);
        PLAY_SOUND(11); // ERROR SOUND, SOUND PLAY
        return;
    }
}

uint8_t MOVTEMP_O = 0; // origin tile
uint8_t MOVTEMP_D = 0; // destination tile
uint8_t MOVTEMP_X = 0; // x-coordinate
uint8_t MOVTEMP_Y = 0; // y-coordinate
uint8_t MOVTEMP_U = 0; // unit number (255=none)
uint8_t MOVTEMP_UX = 0;
uint8_t MOVTEMP_UY = 0;

void CACULATE_AND_REDRAW()
{
    MAP_WINDOW_X = UNIT_LOC_X[0] - 5; // no index needed since it's player unit
    MAP_WINDOW_Y = UNIT_LOC_Y[0] - 3; // no index needed since it's player unit
    REDRAW_WINDOW = 1;
}

// This routine checks all units from 0 to 31 and figures out if it should be dislpayed
// on screen, and then grabs that unit's tile and stores it in the MAP_PRECALC array
// so that when the window is drawn, it does not have to search for units during the
// draw, speeding up the display routine.
void MAP_PRE_CALCULATE()
{
    // CLEAR OLD BUFFER
    for (int Y = 0; Y != 77; Y++) {
        MAP_PRECALC[Y] = 0;
    }
    for (int X = 0; X != 32; X++) {
        if (X == 0 || // skip the check for unit zero, always draw it.
            (UNIT_TYPE[X] != 0 &&                    // CHECK THAT UNIT EXISTS
             UNIT_LOC_X[X] >= MAP_WINDOW_X &&        // CHECK HORIZONTAL POSITION
             UNIT_LOC_X[X] <= (MAP_WINDOW_X + 10) && // NOW CHECK VERTICAL
             UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
             UNIT_LOC_Y[X] <= (MAP_WINDOW_Y + 6))) {
            // Unit found in map window, now add that unit's
            // tile to the precalc map.
            int Y = UNIT_LOC_X[X] - MAP_WINDOW_X + PRECALC_ROWS[UNIT_LOC_Y[X] - MAP_WINDOW_Y];
            if (UNIT_TILE[X] == 130 || UNIT_TILE[X] == 134) { // is it a bomb, is it a magnet?
                // What to do in case of bomb or magnet that should
                // go underneath the unit or robot.
                if (MAP_PRECALC[Y] != 0) {
                    continue;
                }
            }
            MAP_PRECALC[Y] = UNIT_TILE[X];
            MAP_PRECALC_DIRECTION[Y] = UNIT_DIRECTION[X];
        }
        // continue search
    }
}

uint8_t PRECALC_ROWS[] = { 0,11,22,33,44,55,66 };

// This routine is where the MAP is displayed on the screen
#ifdef OPTIMIZED_MAP_RENDERING
void INVALIDATE_PREVIOUS_MAP()
{
    for (int i = 0; i < 77; i++) {
        PREVIOUS_MAP_BACKGROUND[i] = 255;
    }
}

void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    MAP_SOURCE = MAP + ((MAP_WINDOW_Y << 7) + MAP_WINDOW_X);
    for (uint8_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != 7; TEMP_Y++, MAP_SOURCE += 117) {
        for (uint8_t TEMP_X = 0; TEMP_X != 11; TEMP_X++, MAP_SOURCE++, PRECALC_COUNT++) {
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
            TILE = MAP_SOURCE[0];
            uint8_t FG_TILE = MAP_PRECALC[PRECALC_COUNT];
            uint8_t FG_VARIANT = 0;
            if (FG_TILE != 0) {
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
                if (FG_TILE == 96 || (FG_TILE >= 100 && FG_TILE <= 103)) {
                    if (DIRECTION == 0) {
                        FG_VARIANT = 8;
                    } else if (DIRECTION == 2) {
                        FG_VARIANT = 12;
                    } else if (DIRECTION == 3) {
                        FG_VARIANT = 4;
                    }
                    if (FG_TILE == 96) {
                        FG_VARIANT += WALK_FRAME + (SELECTED_WEAPON << 4);
                    }
                }
                if (TILE != PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] ||
                    FG_TILE != PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] ||
                    FG_VARIANT != PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT]) {
                    platform->renderTiles(*MAP_SOURCE, FG_TILE, TEMP_X * 24, TEMP_Y * 24, FG_VARIANT);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;
                }
            } else {
                if (TILE != PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] ||
                    FG_TILE != PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] ||
                    FG_VARIANT != PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT]) {
                    platform->renderTile(*MAP_SOURCE, TEMP_X * 24, TEMP_Y * 24);
                    PREVIOUS_MAP_BACKGROUND[PRECALC_COUNT] = TILE;
                    PREVIOUS_MAP_FOREGROUND[PRECALC_COUNT] = FG_TILE;
                    PREVIOUS_MAP_FOREGROUND_VARIANT[PRECALC_COUNT] = FG_VARIANT;
                }
            }
        }
    }
}
#else
// This is a temporary routine, taken from the map editor.
void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    for (uint8_t TEMP_Y = 0, PRECALC_COUNT = 0; TEMP_Y != 7; TEMP_Y++) {
        for (uint8_t TEMP_X = 0; TEMP_X != 11; TEMP_X++) {
            // FIRST CALCULATE WHERE THE BYTE IS STORED IN THE MAP
            MAP_SOURCE = MAP + (((MAP_WINDOW_Y + TEMP_Y) << 7) + TEMP_X + MAP_WINDOW_X);
            TILE = MAP_SOURCE[0];
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
#ifdef PLATFORM_SPRITE_SUPPORT
            if (MAP_PRECALC[PRECALC_COUNT] != 0) {
                uint8_t FG_TILE = MAP_PRECALC[PRECALC_COUNT];
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
                uint8_t variant = 0;
                if (FG_TILE == 96 || (FG_TILE >= 100 && FG_TILE <= 103)) {
                    if (DIRECTION == 0) {
                        variant = 8;
                    } else if (DIRECTION == 2) {
                        variant = 12;
                    } else if (DIRECTION == 3) {
                        variant = 4;
                    }
                    if (FG_TILE == 96) {
                        variant += WALK_FRAME + (SELECTED_WEAPON << 4);
                    }
                }
                platform->renderTiles(TILE, FG_TILE, TEMP_X * 24, TEMP_Y * 24, variant);
            } else {
                platform->renderTile(TILE, TEMP_X * 24, TEMP_Y * 24);
            }
#else
#ifdef PLATFORM_TILE_BASED_RENDERING
            PLOT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X, TEMP_X, TEMP_Y);
#else
            PLOT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
#endif
            // now check for sprites in this location
            if (MAP_PRECALC[PRECALC_COUNT] != 0) {
                TILE = MAP_PRECALC[PRECALC_COUNT];
                DIRECTION = MAP_PRECALC_DIRECTION[PRECALC_COUNT];
#ifdef PLATFORM_TILE_BASED_RENDERING
                PLOT_TRANSPARENT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X, TEMP_X, TEMP_Y);
#else
                PLOT_TRANSPARENT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
#endif
            }
#endif
            PRECALC_COUNT++;
        }
#ifndef PLATFORM_CURSOR_SUPPORT
        // CHECK FOR CURSOR
        if (CURSOR_ON == 1) { // Is cursor even on?
            if (TEMP_Y == CURSOR_Y) { // is cursor on the same row that were drawing?
                REVERSE_TILE();
            }
        }
#endif
    }
}
#endif

#ifdef PLATFORM_LIVE_MAP_SUPPORT
void TOGGLE_LIVE_MAP()
{
    if (LIVE_MAP_ON != 1) {
        LIVE_MAP_ON = 1;

        platform->clearRect(0, 0, 264, 168);
        platform->renderLiveMap(MAP);
    } else {
        LIVE_MAP_ON = 0;

#ifdef OPTIMIZED_MAP_RENDERING
        INVALIDATE_PREVIOUS_MAP();
#endif
    }
    REDRAW_WINDOW = 1;
}

void TOGGLE_LIVE_MAP_ROBOTS()
{
    LIVE_MAP_ROBOTS_ON = LIVE_MAP_ROBOTS_ON == 1 ? 0 : 1;
    LIVE_MAP_ROBOTS_BLINK = 0;
}

void DRAW_LIVE_MAP()
{
    if (LIVE_MAP_ROBOTS_ON == 1) {
        LIVE_MAP_ROBOTS_BLINK++;
        LIVE_MAP_ROBOTS_BLINK &= 31;
    }
    platform->renderLiveMapUnits(MAP, UNIT_TYPE, UNIT_LOC_X, UNIT_LOC_Y, (LIVE_MAP_ROBOTS_ON == 1 && LIVE_MAP_ROBOTS_BLINK < 16) ? true : false);
}

uint8_t LIVE_MAP_ON = 0;
uint8_t LIVE_MAP_ROBOTS_ON = 0;
uint8_t LIVE_MAP_ROBOTS_BLINK = 0;
#endif

#ifdef PLATFORM_TILE_BASED_RENDERING
// This routine plots a 3x3 tile from the tile database anywhere
// on screen.  But first you must define the tile number in the
// TILE variable, as well as the starting screen address must
// be defined in $FB.
void PLOT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
#ifndef PLATFORM_SPRITE_SUPPORT
    // DRAW THE TOP 3 CHARACTERS
    SCREEN_MEMORY[destination + 0] = TILE_DATA_TL[TILE];
    SCREEN_MEMORY[destination + 1] = TILE_DATA_TM[TILE];
    SCREEN_MEMORY[destination + 2] = TILE_DATA_TR[TILE];
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    SCREEN_MEMORY[destination + 40] = TILE_DATA_ML[TILE];
    SCREEN_MEMORY[destination + 41] = TILE_DATA_MM[TILE];
    SCREEN_MEMORY[destination + 42] = TILE_DATA_MR[TILE];
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    SCREEN_MEMORY[destination + 80] = TILE_DATA_BL[TILE];
    SCREEN_MEMORY[destination + 81] = TILE_DATA_BM[TILE];
    SCREEN_MEMORY[destination + 82] = TILE_DATA_BR[TILE];
#endif
    platform->renderTile(TILE, x * 24, y * 24);
}

// This routine plots a transparent tile from the tile database
// anywhere on screen.  But first you must define the tile number
// in the TILE variable, as well as the starting screen address must
// be defined in $FB.  Also, this routine is slower than the usual
// tile routine, so is only used for sprites.  The ":" character ($3A)
// is not drawn.
void PLOT_TRANSPARENT_TILE(uint16_t destination, uint16_t x, uint16_t y)
{
#ifndef PLATFORM_SPRITE_SUPPORT
    // DRAW THE TOP 3 CHARACTERS
    if (TILE_DATA_TL[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 0] = TILE_DATA_TL[TILE];
    }
    if (TILE_DATA_TM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 1] = TILE_DATA_TM[TILE];
    }
    if (TILE_DATA_TR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 2] = TILE_DATA_TR[TILE];
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    if (TILE_DATA_ML[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 40] = TILE_DATA_ML[TILE];
    }
    if (TILE_DATA_MM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 41] = TILE_DATA_MM[TILE];
    }
    if (TILE_DATA_MR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 42] = TILE_DATA_MR[TILE];
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    if (TILE_DATA_BL[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 80] = TILE_DATA_BL[TILE];
    }
    if (TILE_DATA_BM[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 81] = TILE_DATA_BM[TILE];
    }
    if (TILE_DATA_BR[TILE] != 0x3A) {
        SCREEN_MEMORY[destination + 82] = TILE_DATA_BR[TILE];
    }
#endif
    uint8_t variant = 0;
    if (TILE == 96 || (TILE >= 100 && TILE <= 103)) {
        if (DIRECTION == 0) {
            variant = 8;
        } else if (DIRECTION == 2) {
            variant = 12;
        } else if (DIRECTION == 3) {
            variant = 4;
        }
        if (TILE == 96) {
            variant += WALK_FRAME + (SELECTED_WEAPON << 4);
        }
    }
    platform->renderTile(TILE, x * 24, y * 24, variant, true);
}
#else
void PLOT_TILE(uint16_t destination)
{
    // DRAW THE TOP 3 CHARACTERS
    writeToScreenMemory(destination + 0, TILE_DATA_TL[TILE]);
    writeToScreenMemory(destination + 1, TILE_DATA_TM[TILE]);
    writeToScreenMemory(destination + 2, TILE_DATA_TR[TILE]);
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    writeToScreenMemory(destination + 40, TILE_DATA_ML[TILE]);
    writeToScreenMemory(destination + 41, TILE_DATA_MM[TILE]);
    writeToScreenMemory(destination + 42, TILE_DATA_MR[TILE]);
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    writeToScreenMemory(destination + 80, TILE_DATA_BL[TILE]);
    writeToScreenMemory(destination + 81, TILE_DATA_BM[TILE]);
    writeToScreenMemory(destination + 82, TILE_DATA_BR[TILE]);
}

void PLOT_TRANSPARENT_TILE(uint16_t destination)
{
    // DRAW THE TOP 3 CHARACTERS
    if (TILE_DATA_TL[TILE] != 0x3A) {
        writeToScreenMemory(destination + 0, TILE_DATA_TL[TILE]);
    }
    if (TILE_DATA_TM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 1, TILE_DATA_TM[TILE]);
    }
    if (TILE_DATA_TR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 2, TILE_DATA_TR[TILE]);
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE MIDDLE 3 CHARACTERS
    if (TILE_DATA_ML[TILE] != 0x3A) {
        writeToScreenMemory(destination + 40, TILE_DATA_ML[TILE]);
    }
    if (TILE_DATA_MM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 41, TILE_DATA_MM[TILE]);
    }
    if (TILE_DATA_MR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 42, TILE_DATA_MR[TILE]);
    }
    // MOVE DOWN TO NEXT LINE
    // DRAW THE BOTTOM 3 CHARACTERS
    if (TILE_DATA_BL[TILE] != 0x3A) {
        writeToScreenMemory(destination + 80, TILE_DATA_BL[TILE]);
    }
    if (TILE_DATA_BM[TILE] != 0x3A) {
        writeToScreenMemory(destination + 81, TILE_DATA_BM[TILE]);
    }
    if (TILE_DATA_BR[TILE] != 0x3A) {
        writeToScreenMemory(destination + 82, TILE_DATA_BR[TILE]);
    }
}
#endif

void REVERSE_TILE()
{
    uint16_t destination = MAP_CHART[CURSOR_Y] + CURSOR_X + CURSOR_X + CURSOR_X;
    writeToScreenMemory(destination + 0, SCREEN_MEMORY[destination + 0] ^= 0x80);
    writeToScreenMemory(destination + 1, SCREEN_MEMORY[destination + 1] ^= 0x80);
    writeToScreenMemory(destination + 2, SCREEN_MEMORY[destination + 2] ^= 0x80);
    writeToScreenMemory(destination + 40, SCREEN_MEMORY[destination + 40] ^= 0x80);
    writeToScreenMemory(destination + 41, SCREEN_MEMORY[destination + 41] ^= 0x80);
    writeToScreenMemory(destination + 42, SCREEN_MEMORY[destination + 42] ^= 0x80);
    writeToScreenMemory(destination + 80, SCREEN_MEMORY[destination + 80] ^= 0x80);
    writeToScreenMemory(destination + 81, SCREEN_MEMORY[destination + 81] ^= 0x80);
    writeToScreenMemory(destination + 82, SCREEN_MEMORY[destination + 82] ^= 0x80);
}

// This routine checks to see if UNIT is occupying any space
// that is currently visible in the window.  If so, the
// flag for redrawing the window will be set.
void CHECK_FOR_WINDOW_REDRAW()
{
    if (UNIT_LOC_X[UNIT] >= MAP_WINDOW_X && // FIRST CHECK HORIZONTAL
        UNIT_LOC_X[UNIT] <= (MAP_WINDOW_X + 10) &&
        UNIT_LOC_Y[UNIT] >= MAP_WINDOW_Y && // NOW CHECK VERTICAL
        UNIT_LOC_Y[UNIT] <= (MAP_WINDOW_Y + 6)) {
        REDRAW_WINDOW = 1;
    }
}

void DECWRITE(uint16_t destination, uint8_t color)
{
    for (int X = 2; X >= 0; X--) {
        writeToScreenMemory(destination + X, 0x30 + (DECNUM % 10), color);
        DECNUM /= 10;
    }
}

uint8_t DECA[] = { 128, 160, 200 };
uint8_t DECB = 1;
uint8_t SCREENPOS = 0;
uint8_t DECTEMP = 0;

// The following routine loads the tileset from disk
void TILE_LOAD_ROUTINE()
{
#ifdef PLATFORM_SPRITE_SUPPORT
    platform->load(TILENAME, DESTRUCT_PATH, 512, 2);
    platform->generateTiles(0, TILE_ATTRIB);
#else
    platform->load(TILENAME, DESTRUCT_PATH, 2816, 2);
    platform->generateTiles(TILE_DATA_TL, TILE_ATTRIB);
#endif
}

// The following routine loads the map from disk
void MAP_LOAD_ROUTINE()
{
    platform->load(MAPNAME, UNIT_TYPE, 8960);
}

void DISPLAY_GAME_SCREEN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->displayImage(Platform::ImageGame);

    writeToScreenMemory(0x3E2, 0x71, 15);
    writeToScreenMemory(0x3E3, 0x71, 15);
    writeToScreenMemory(0x3E4, 0x71, 12);
    writeToScreenMemory(0x3E5, 0x71, 12);
    writeToScreenMemory(0x3E6, 0x71, 9);
    writeToScreenMemory(0x3E7, 0x71, 9);
#else
    DECOMPRESS_SCREEN(SCR_TEXT);
#endif
}

const char* introText[] = {
    "start game",
    "select map",
    "difficulty",
    "controls  "
};

void DISPLAY_INTRO_SCREEN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    uint8_t* row = SCREEN_MEMORY + MENU_CHART_L[0];
    for (int Y = 0; Y < 4; Y++, row += 40) {
        for (int X = 0; X < 10; X++) {
            row[X] = convertToPETSCII(introText[Y][X]);
        }
    }
    platform->displayImage(Platform::ImageIntro);
#else
    DECOMPRESS_SCREEN(INTRO_TEXT);
#endif
}

void DISPLAY_ENDGAME_SCREEN()
{
    int X;
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->displayImage(Platform::ImageGameOver);
#else
    DECOMPRESS_SCREEN(SCR_ENDGAME);
#endif
    // display map name
    const char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
        writeToScreenMemory(0x12E + Y, convertToPETSCII(name[Y]));
    }
    // display elapsed time
    DECNUM = HOURS;
    DECWRITE(0x17D);
    DECNUM = MINUTES;
    DECWRITE(0x180);
    DECNUM = SECONDS;
    DECWRITE(0x183);
    writeToScreenMemory(0x17D, 32); // SPACE
    writeToScreenMemory(0x180, 58); // COLON
    writeToScreenMemory(0x183, 58);
    // count robots remaining
    DECNUM = 0;
    for (X = 1; X != 28; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(0x1CE);
    // Count secrets remaining
    DECNUM = 0;
    for (X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0) {
            DECNUM++;
        }
    }
    DECWRITE(0x21E);
    // display difficulty level
    const char* WORD = DIFF_LEVEL_WORDS[DIFF_LEVEL];
    for (X = 0; WORD[X] != 0; X++) {
        writeToScreenMemory(0x26E + X, convertToPETSCII(WORD[X]));
    }
}

const char* DIFF_LEVEL_WORDS[] = {
    "easy",
    "normal",
    "hard"
};

void DECOMPRESS_SCREEN(uint8_t* source, uint8_t color)
{
    uint16_t destination = 0;

    while (!platform->quit) {
        if (*source != 96) { // REPEAT FLAG
            writeToScreenMemory(destination, *source, color);
        } else {
            // REPEAT CODE
            source++;
            RPT = *source;
            source++;
            for (int X = *source; X >= 0; X--) {
                writeToScreenMemory(destination++, RPT, color);
            }
            destination--;
        }
        // CHECK TO SEE IF WE REACHED $83E7 YET.
        if (destination == 0x3E7) {
            return;
        }
        source++;
        destination++;
    }
}

uint8_t RPT = 0; // repeat value

void DISPLAY_PLAYER_HEALTH()
{
    TEMP_A = UNIT_HEALTH[0] >> 1; // No index needed because it is the player, divide by two
    int Y = 0;
    while (Y != TEMP_A) {
        writeToScreenMemory(0x3BA + Y++, 0x66); // GRAY BLOCK
    }
    if (UNIT_HEALTH[0] & 0x01) {
        writeToScreenMemory(0x3BA + Y++, 0x5C); // HALF GRAY BLOCK
    }
    while (Y != 6) {
        writeToScreenMemory(0x3BA + Y++, 0x20); // SPACE
    }

#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderHealth(5 - MIN(TEMP_A, 5), 272, 128);
#endif
}

void CYCLE_ITEM()
{
    PLAY_SOUND(13); // CHANGE-ITEM-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_ITEM++;
    if (SELECTED_ITEM == 5) {
        SELECTED_ITEM = 0;
    }
    DISPLAY_ITEM();
}

void DISPLAY_ITEM()
{
    PRESELECT_ITEM();
    while (SELECTED_ITEM != 0) {
        if (SELECTED_ITEM == 5) { // number too high!
            SELECTED_ITEM = 0;
            return;
        } else if (SELECTED_ITEM == 1) { // bomb
            if (INV_BOMBS != 0) { // did we run out?
                DISPLAY_TIMEBOMB();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 2) { // emp
            if (INV_EMP != 0) { // did we run out?
                DISPLAY_EMP();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 3) { // medkit
            if (INV_MEDKIT != 0) { // did we run out?
                DISPLAY_MEDKIT();
                return;
            }
            SELECTED_ITEM++;
        } else if (SELECTED_ITEM == 4) { // magnet
            if (INV_MAGNET != 0) { // did we run out?
                DISPLAY_MAGNET();
                return;
            }
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        } else {
            SELECTED_ITEM = 0;
            PRESELECT_ITEM();
        }
    }
    // no items to show
    // add routine to draw blank space
}

// This routine checks to see if currently selected
// item is zero.  And if it is, then it checks inventories
// of other items to decide which item to automatically
// select for the user.
void PRESELECT_ITEM()
{
    if (SELECTED_ITEM != 0) { // If item already selected, return
        return;
    }
    if (INV_BOMBS != 0) {
        SELECTED_ITEM = 1; // BOMB
        return;
    }
    if (INV_EMP != 0) {
        SELECTED_ITEM = 2; // EMP
        return;
    }
    if (INV_MEDKIT != 0) {
        SELECTED_ITEM = 3; // MEDKIT
        return;
    }
    if (INV_MAGNET != 0) {
        SELECTED_ITEM = 4; // MAGNET
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_ITEM = 0; // nothing in inventory
    DISPLAY_BLANK_ITEM();
}

void DISPLAY_TIMEBOMB()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(5, 272, 48);
    DECNUM = INV_BOMBS;
    DECWRITE(0x1B5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, TBOMB1A[Y]);
        writeToScreenMemory(0x18A + Y, TBOMB1B[Y]);
        writeToScreenMemory(0x1B2 + Y, TBOMB1C[Y]);
        writeToScreenMemory(0x1DA + Y, TBOMB1D[Y]);
    }
    DECNUM = INV_BOMBS;
    DECWRITE(0x205);
#endif
}

void DISPLAY_EMP()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(3, 272, 48);
    DECNUM = INV_EMP;
    DECWRITE(0x1B5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, EMP1A[Y]);
        writeToScreenMemory(0x18A + Y, EMP1B[Y]);
        writeToScreenMemory(0x1B2 + Y, EMP1C[Y]);
        writeToScreenMemory(0x1DA + Y, EMP1D[Y]);
    }
    DECNUM = INV_EMP;
    DECWRITE(0x205);
#endif
}

void DISPLAY_MEDKIT()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(2, 272, 48);
    DECNUM = INV_MEDKIT;
    DECWRITE(0x1B5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, MED1A[Y]);
        writeToScreenMemory(0x18A + Y, MED1B[Y]);
        writeToScreenMemory(0x1B2 + Y, MED1C[Y]);
        writeToScreenMemory(0x1DA + Y, MED1D[Y]);
    }
    DECNUM = INV_MEDKIT;
    DECWRITE(0x205);
#endif
}

void DISPLAY_MAGNET()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(4, 272, 48);
    DECNUM = INV_MAGNET;
    DECWRITE(0x1B5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, MAG1A[Y]);
        writeToScreenMemory(0x18A + Y, MAG1B[Y]);
        writeToScreenMemory(0x1B2 + Y, MAG1C[Y]);
        writeToScreenMemory(0x1DA + Y, MAG1D[Y]);
    }
    DECNUM = INV_MAGNET;
    DECWRITE(0x205);
#endif
}

void DISPLAY_BLANK_ITEM()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->clearRect(272, 48, 48, 32);
#else
    platform->clearRect(272, 56, 48, 32);
#endif
/*
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, 32);
        writeToScreenMemory(0x18A + Y, 32);
        writeToScreenMemory(0x1B2 + Y, 32);
        writeToScreenMemory(0x1DA + Y, 32);
        writeToScreenMemory(0x202 + Y, 32);
    }
*/
}

void CYCLE_WEAPON()
{
    PLAY_SOUND(12); // CHANGE WEAPON-SOUND, SOUND PLAY
    if (SELECT_TIMEOUT != 0) {
        return;
    }
    SELECT_TIMEOUT = 3; // RESET THE TIMEOUT
    KEYTIMER = 20;
    SELECTED_WEAPON++;
    if (SELECTED_WEAPON != 2) {
        SELECTED_WEAPON = 0;
    }
#ifdef PLATFORM_IMAGE_SUPPORT
    REDRAW_WINDOW = 1;
#endif
    DISPLAY_WEAPON();
}

void DISPLAY_WEAPON()
{
    while (!platform->quit) {
        PRESELECT_WEAPON();
        if (SELECTED_WEAPON == 0) { // no weapon to show
            // add routine to draw blank space
            return;
        } else if (SELECTED_WEAPON == 1) { // PISTOL
            if (AMMO_PISTOL != 0) { // did we run out?
                DISPLAY_PISTOL();
                return;
            }
            SELECTED_WEAPON = 0;
        } else if (SELECTED_WEAPON == 2) { // PLASMA GUN
            if (AMMO_PLASMA != 0) { // did we run out?
                DISPLAY_PLASMA_GUN();
                return;
            }
            SELECTED_WEAPON = 0;
        } else {
            SELECTED_WEAPON = 0; // should never happen
        }
    }
}

// This routine checks to see if currently selected
// weapon is zero.  And if it is, then it checks inventories
// of other weapons to decide which item to automatically
// select for the user.
void PRESELECT_WEAPON()
{
    if (SELECTED_WEAPON != 0) { // If item already selected, return
        return;
    }
    if (AMMO_PISTOL != 0) {
        SELECTED_WEAPON = 1; // PISTOL
        return;
    }
    if (AMMO_PLASMA != 0) {
        SELECTED_WEAPON = 2; // PLASMAGUN
        return;
    }
    // Nothing found in inventory at this point, so set
    // selected-item to zero.
    SELECTED_WEAPON = 0; // nothing in inventory
    DISPLAY_BLANK_WEAPON();
}

void DISPLAY_PLASMA_GUN()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(1, 272, 8);
    DECNUM = AMMO_PLASMA;
    DECWRITE(0x0C5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y, WEAPON1A[Y]);
        writeToScreenMemory(0x072 + Y, WEAPON1B[Y]);
        writeToScreenMemory(0x09A + Y, WEAPON1C[Y]);
        writeToScreenMemory(0x0C2 + Y, WEAPON1D[Y]);
    }
    DECNUM = AMMO_PLASMA;
    DECWRITE(0x0ED);
#endif
}

void DISPLAY_PISTOL()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderItem(0, 272, 8);
    DECNUM = AMMO_PISTOL;
    DECWRITE(0x0C5, 1);
#else
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y, PISTOL1A[Y]);
        writeToScreenMemory(0x072 + Y, PISTOL1B[Y]);
        writeToScreenMemory(0x09A + Y, PISTOL1C[Y]);
        writeToScreenMemory(0x0C2 + Y, PISTOL1D[Y]);
    }
    DECNUM = AMMO_PISTOL;
    DECWRITE(0x0ED);
#endif
}

void DISPLAY_BLANK_WEAPON()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->clearRect(272, 8, 48, 32);
#else
    platform->clearRect(272, 16, 48, 32);
#endif
    /*
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y, 32);
        writeToScreenMemory(0x072 + Y, 32);
        writeToScreenMemory(0x09A + Y, 32);
        writeToScreenMemory(0x0C2 + Y, 32);
        writeToScreenMemory(0x0EA + Y, 32);
    }
    */
}

void DISPLAY_KEYS()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->clearRect(272, 96, 48, 16); // ERASE ALL 3 SPOTS
    /*
    writeToScreenMemory(0x27A, 32); // ERASE ALL 3 SPOTS
    writeToScreenMemory(0x27B, 32);
    writeToScreenMemory(0x27C, 32);
    writeToScreenMemory(0x27D, 32);
    writeToScreenMemory(0x27E, 32);
    writeToScreenMemory(0x27F, 32);
    writeToScreenMemory(0x2A2, 32);
    writeToScreenMemory(0x2A3, 32);
    writeToScreenMemory(0x2A4, 32);
    writeToScreenMemory(0x2A5, 32);
    writeToScreenMemory(0x2A6, 32);
    writeToScreenMemory(0x2A7, 32);
    */
    if (KEYS & 0x01) { // %00000001 Spade key
        writeToScreenMemory(0x22A, 0x63, 14);
        writeToScreenMemory(0x22B, 0x4D, 14);
        writeToScreenMemory(0x252, 0x41, 14);
        writeToScreenMemory(0x253, 0x67, 14);
    }
    if (KEYS & 0x02) { // %00000010 heart key
        writeToScreenMemory(0x22C, 0x63, 10);
        writeToScreenMemory(0x22D, 0x4D, 10);
        writeToScreenMemory(0x254, 0x53, 10);
        writeToScreenMemory(0x255, 0x67, 10);
    }
    if (KEYS & 0x04) { // %00000100 star key
        writeToScreenMemory(0x22E, 0x63, 7);
        writeToScreenMemory(0x22F, 0x4D, 7);
        writeToScreenMemory(0x256, 0x2A, 7);
        writeToScreenMemory(0x257, 0x67, 7);
    }
#else
    platform->clearRect(272, 120, 48, 16); // ERASE ALL 3 SPOTS
    /*
    writeToScreenMemory(0x27A, 32); // ERASE ALL 3 SPOTS
    writeToScreenMemory(0x27B, 32);
    writeToScreenMemory(0x27C, 32);
    writeToScreenMemory(0x27D, 32);
    writeToScreenMemory(0x27E, 32);
    writeToScreenMemory(0x27F, 32);
    writeToScreenMemory(0x2A2, 32);
    writeToScreenMemory(0x2A3, 32);
    writeToScreenMemory(0x2A4, 32);
    writeToScreenMemory(0x2A5, 32);
    writeToScreenMemory(0x2A6, 32);
    writeToScreenMemory(0x2A7, 32);
    */
    if (KEYS & 0x01) { // %00000001 Spade key
        writeToScreenMemory(0x27A, 0x63);
        writeToScreenMemory(0x27B, 0x4D);
        writeToScreenMemory(0x2A2, 0x41);
        writeToScreenMemory(0x2A3, 0x67);
    }
    if (KEYS & 0x02) { // %00000010 heart key
        writeToScreenMemory(0x27C, 0x63);
        writeToScreenMemory(0x27D, 0x4D);
        writeToScreenMemory(0x2A4, 0x53);
        writeToScreenMemory(0x2A5, 0x67);
    }
    if (KEYS & 0x04) { // %00000100 star key
        writeToScreenMemory(0x27E, 0x63);
        writeToScreenMemory(0x27F, 0x4D);
        writeToScreenMemory(0x2A6, 0x2A);
        writeToScreenMemory(0x2A7, 0x67);
    }
#endif
}

void GAME_OVER()
{
    // stop game clock
    CLOCK_ACTIVE = 0;
    // disable music
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playModule(Platform::ModuleSoundFX);
#else
    MUSIC_ON = 0;
    platform->stopNote(); // turn off sound
#endif
    // Did player die or win?
    if (UNIT_TYPE[0] == 0) {
        UNIT_TILE[0] = 111; // // dead player tile
        KEYTIMER = 100;
    }
    while (KEYTIMER != 0) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
    }
    for (int X = 0; X != 11; X++) {
        writeToScreenMemory(0x173 + X, GAMEOVER1[X]);
        writeToScreenMemory(0x19B + X, GAMEOVER2[X]);
        writeToScreenMemory(0x1C3 + X, GAMEOVER3[X]);
    }
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    platform->stopModule();
    platform->loadModule(UNIT_TYPE[0] != 0 ? Platform::ModuleWin : Platform::ModuleLose);
#else
    KEYTIMER = 100;
    while (KEYTIMER != 0) {
        platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    }
#endif
    while (platform->getin() == 0 && !platform->quit);
    GOM4();
}

void GOM4()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopModule();
#else
    MUSIC_ON = 0;
#endif
    platform->startFadeScreen(0x000, 15);
    platform->fadeScreen(0, false);
    DISPLAY_ENDGAME_SCREEN();
    DISPLAY_WIN_LOSE();
    platform->renderFrame();
    platform->fadeScreen(15, false);
    while (platform->getin() == 0 && !platform->quit);
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopModule();
#endif
}

uint8_t GAMEOVER1[] = { 0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x6e };
uint8_t GAMEOVER2[] = { 0x5d, 0x07, 0x01, 0x0d, 0x05, 0x20, 0x0f, 0x16, 0x05, 0x12, 0x5d };
uint8_t GAMEOVER3[] = { 0x6d, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7d };

void DISPLAY_WIN_LOSE()
{
    STOP_SONG();
    if (UNIT_TYPE[0] != 0) {
        // WIN MESSAGE
        for (int X = 0; X != 8; X++) {
            writeToScreenMemory(0x088 + X, convertToPETSCII(WIN_MSG[X]));
        }
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->playModule(Platform::ModuleWin);
#else
        PLAY_SOUND(18); // win music
#endif
    } else {
        // LOSE MESSAGE
        for (int X = 0; X != 9; X++) {
            writeToScreenMemory(0x088 + X, convertToPETSCII(LOS_MSG[X]));
        }
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->playModule(Platform::ModuleLose);
#else
        PLAY_SOUND(19); // LOSE music
#endif
    }
}

const char* WIN_MSG = "you win!";
const char* LOS_MSG = "you lose!";

void PRINT_INTRO_MESSAGE()
{
    PRINT_INFO(INTRO_MESSAGE);
}

// This routine will print something to the "information" window
// at the bottom left of the screen.  You must first define the 
// source of the text in $FB. The text should terminate with
// a null character.
void PRINT_INFO(const char *text)
{
    SCROLL_INFO(); // New text always causes a scroll
    PRINTX = 0;
    for (int Y = 0; text[Y] != 0; Y++) {
        if (text[Y] == -1) { // return
            PRINTX = 0;
            SCROLL_INFO();
        } else {
            writeToScreenMemory(0x3C0 + PRINTX, convertToPETSCII(text[Y]));
            PRINTX++;
        }
    }
}

uint8_t PRINTX = 0; // used to store X-cursor location

// This routine scrolls the info screen by one row, clearing
// a new row at the bottom.
void SCROLL_INFO()
{
    /*
    int X;
    for (X = 0; X != 33; X++) {
        writeToScreenMemory(0x370 + X, SCREEN_MEMORY[0x398 + X]);
        writeToScreenMemory(0x398 + X, SCREEN_MEMORY[0x3C0 + X]); // BOTTOM ROW
    }
    // NOW CLEAR BOTTOM ROW
    for (X = 0; X != 33; X++) {
        writeToScreenMemory(0x3C0 + X, 32); // BOTTOM ROW
    }
    */
    platform->copyRect(0, 184, 0, 176, 264, 16);
    // NOW CLEAR BOTTOM ROW
    platform->clearRect(0, 192, 264, 8);
    platform->renderFrame(true);
}

void RESET_KEYS_AMMO()
{
    KEYS = 0;
    AMMO_PISTOL = 0;
    AMMO_PLASMA = 0;
    INV_BOMBS = 0;
    INV_EMP = 0;
    INV_MEDKIT = 0;
    INV_MAGNET = 0;
    SELECTED_WEAPON = 0;
    SELECTED_ITEM = 0;
    MAGNET_ACT = 0;
    PLASMA_ACT = 0;
    BIG_EXP_ACT = 0;
    CYCLES = 0;
    SECONDS = 0;
    MINUTES = 0;
    HOURS = 0;
}

void INTRO_SCREEN()
{
    platform->fadeScreen(0, false);
    DISPLAY_INTRO_SCREEN();
    DISPLAY_MAP_NAME();
    CHANGE_DIFFICULTY_LEVEL();
    START_INTRO_MUSIC();
    platform->show();
    platform->fadeScreen(15, false);
    MENUY = 0;
    REVERSE_MENU_OPTION(true);
    platform->renderFrame();
    bool done = false;
    while (!done && !platform->quit) {
        uint8_t A = platform->getin();
        if (A != 0) {
            if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                if (MENUY != 3) {
                    REVERSE_MENU_OPTION(false);
                    MENUY++;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_CURSOR_UP] || A == KEY_CONFIG[KEY_MOVE_UP]) { // CURSOR UP
                if (MENUY != 0) {
                    REVERSE_MENU_OPTION(false);
                    MENUY--;
                    REVERSE_MENU_OPTION(true);
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == KEY_CONFIG[KEY_SPACE]) { // SPACE
                done = EXEC_COMMAND();
            } else if (A == KEY_CONFIG[KEY_RETURN]) { // RETURN
                PLAY_SOUND(15); // menu beep, SOUND PLAY
                done = EXEC_COMMAND();
            }
            platform->renderFrame();
        }
    }
}

void START_INTRO_MUSIC()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playModule(Platform::ModuleIntro);
#else
    DATA_LINE = 0;
    SOUND_EFFECT = 0xFF;
    CUR_PATTERN = INTRO_MUSIC;
    MUSIC_ON = 1;
#endif
}

bool EXEC_COMMAND()
{
    if (MENUY == 0) { // START GAME
        SET_CONTROLS();
#ifdef PLATFORM_MODULE_BASED_AUDIO
        platform->stopModule();
#else
        MUSIC_ON = 0;
#endif
        platform->stopNote(); // turn off sound
        INIT_GAME();
        return true;
    } else if (MENUY == 2) { // DIFF LEVEL
        DIFF_LEVEL++;
        if (DIFF_LEVEL == 3) {
            DIFF_LEVEL = 0;
        }
        CHANGE_DIFFICULTY_LEVEL();
        PLAY_SOUND(15); // menu beep
    } else if (MENUY == 1) { // cycle map
        PLAY_SOUND(15); // menu beep
        CYCLE_MAP();
    } else if (MENUY == 3) {
        CYCLE_CONTROLS();
        PLAY_SOUND(15); // menu beep
    }
    return false;
}

void CYCLE_CONTROLS()
{
    KEYS_DEFINED = 0;
    CONTROL++;
    if (CONTROL == 3) {
        CONTROL = 0;
    }
    // display control method on screen
    for (int X = 0, Y = CONTROLSTART[CONTROL]; X != 10; X++, Y++) {
#ifdef PLATFORM_IMAGE_SUPPORT
        writeToScreenMemory(0x0A4 + X, convertToPETSCII(CONTROLTEXT[Y]), 14, 5);
#else
        writeToScreenMemory(0x0CC + X, convertToPETSCII(CONTROLTEXT[Y]) | 0x80);
#endif
    }
}

const char* CONTROLTEXT = "keyboard  "
                          "custom key"
                          "snes pad  ";
uint8_t CONTROLSTART[] = { 0, 10, 20 };

void CYCLE_MAP()
{
    SELECTED_MAP++;
    if (SELECTED_MAP == 13) { // Maximum number of maps
        SELECTED_MAP = 0;
    }
    DISPLAY_MAP_NAME();
}

void DISPLAY_MAP_NAME()
{
    const char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
#ifdef PLATFORM_IMAGE_SUPPORT
        writeToScreenMemory(0x119 + Y, convertToPETSCII(name[Y]), 15, 5);
#else
        writeToScreenMemory(0x16A + Y, convertToPETSCII(name[Y]));
#endif
    }
    // now set the mapname for the filesystem load
    MAPNAME[6] = SELECTED_MAP + 65;
}

const char* CALC_MAP_NAME()
{
    // FIND MAP NAME
    return MAP_NAMES + (SELECTED_MAP << 4); // multiply by 16 by shifting 4 times to left.
}

void REVERSE_MENU_OPTION(bool reverse)
{
#ifdef PLATFORM_COLOR_SUPPORT
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART_L[MENUY] + Y, SCREEN_MEMORY[MENU_CHART_L[MENUY] + Y], reverse ? 14 : 15, 5);
    }
#else
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART_L[MENUY] + Y, SCREEN_MEMORY[MENU_CHART_L[MENUY] + Y] ^ 0x80);
    }
#endif
}

uint8_t MENUY = 0; // CURRENT MENU SELECTION
#ifdef PLATFORM_IMAGE_SUPPORT
uint8_t MENU_CHART_L[] = { 0x2C, 0x54, 0x7C, 0xA4 };
#else
uint8_t MENU_CHART_L[] = { 0x54, 0x7C, 0xA4, 0xCC };
#endif

void CHANGE_DIFFICULTY_LEVEL()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    platform->renderFace(DIFF_LEVEL, 234, 75);
#else
    int Y = FACE_LEVEL[DIFF_LEVEL];
    // DO CHARACTERS FIRST
    writeToScreenMemory(0x0DD, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x0DE, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x0DF, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x0E1, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x0E2, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x0E3, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x107, ROBOT_FACE[Y++]);
    writeToScreenMemory(0x109, ROBOT_FACE[Y]);
#endif
}

uint8_t DIFF_LEVEL = 1; // default medium
uint8_t ROBOT_FACE[] = {
    0x3A, 0x43, 0x49, 0x55, 0x43, 0x3A, 0x49, 0x55, // EASY LEVEL
    0x40, 0x40, 0x6E, 0x70, 0x40, 0x40, 0x49, 0x55, // MEDIUM LEVEL
    0x3A, 0x4D, 0x3A, 0x3A, 0x4E, 0x3A, 0x4D, 0x4E  // HARD LEVEL
};
uint8_t FACE_LEVEL[] = { 0, 8, 16 };

// This routine is run after the map is loaded, but before the
// game starts.  If the diffulcty is set to normal, nothing 
// actually happens.  But if it is set to easy or hard, then
// some changes occur accordingly.
void SET_DIFF_LEVEL()
{
    if (DIFF_LEVEL == 0) { // easy
        SET_DIFF_EASY();
    } else if (DIFF_LEVEL == 2) { // hard
        SET_DIFF_HARD();
    }
}

void SET_DIFF_EASY()
{
    // Find all hidden items and double the quantity.
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            UNIT_TYPE[X] != 128) { // KEY
            UNIT_A[X] <<= 1; // item qty
        }
    }
}

void SET_DIFF_HARD()
{
    // Find all hoverbots and change AI
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] == 2 || // hoverbot left/right
            UNIT_TYPE[X] == 3) { // hoverbot up/down
            UNIT_TYPE[X] = 4; // hoverbot attack mode
        }
    }
}

// This chart contains the left-most staring position for each
// row of tiles on the map-editor. 7 Rows.
uint16_t MAP_CHART[] = {
    0x000, 0x078, 0x0F0, 0x168, 0x1E0, 0x258, 0x2D0
};

void EMP_FLASH()
{
#ifdef PLATFORM_IMAGE_SUPPORT
    BORDER_COLOR = 0x00f;
    BORDER = 10;
#else
    for (int Y = 0; Y != 33; Y++) {
        writeToScreenMemory(0x000 + Y, SCREEN_MEMORY[0x000 + Y] ^ 0x80); // screen row 00
        writeToScreenMemory(0x028 + Y, SCREEN_MEMORY[0x028 + Y] ^ 0x80); // screen row 01
        writeToScreenMemory(0x050 + Y, SCREEN_MEMORY[0x050 + Y] ^ 0x80); // screen row 02
        writeToScreenMemory(0x078 + Y, SCREEN_MEMORY[0x078 + Y] ^ 0x80); // screen row 03
        writeToScreenMemory(0x0A0 + Y, SCREEN_MEMORY[0x0A0 + Y] ^ 0x80); // screen row 04
        writeToScreenMemory(0x0C8 + Y, SCREEN_MEMORY[0x0C8 + Y] ^ 0x80); // screen row 05
        writeToScreenMemory(0x0F0 + Y, SCREEN_MEMORY[0x0F0 + Y] ^ 0x80); // screen row 06
        writeToScreenMemory(0x118 + Y, SCREEN_MEMORY[0x118 + Y] ^ 0x80); // screen row 07
        writeToScreenMemory(0x140 + Y, SCREEN_MEMORY[0x140 + Y] ^ 0x80); // screen row 08
        writeToScreenMemory(0x168 + Y, SCREEN_MEMORY[0x168 + Y] ^ 0x80); // screen row 09
        writeToScreenMemory(0x190 + Y, SCREEN_MEMORY[0x190 + Y] ^ 0x80); // screen row 10
        writeToScreenMemory(0x1B8 + Y, SCREEN_MEMORY[0x1B8 + Y] ^ 0x80); // screen row 11
        writeToScreenMemory(0x1E0 + Y, SCREEN_MEMORY[0x1E0 + Y] ^ 0x80); // screen row 12
        writeToScreenMemory(0x208 + Y, SCREEN_MEMORY[0x208 + Y] ^ 0x80); // screen row 13
        writeToScreenMemory(0x230 + Y, SCREEN_MEMORY[0x230 + Y] ^ 0x80); // screen row 14
        writeToScreenMemory(0x258 + Y, SCREEN_MEMORY[0x258 + Y] ^ 0x80); // screen row 15
        writeToScreenMemory(0x280 + Y, SCREEN_MEMORY[0x280 + Y] ^ 0x80); // screen row 16
        writeToScreenMemory(0x2A8 + Y, SCREEN_MEMORY[0x2A8 + Y] ^ 0x80); // screen row 17
        writeToScreenMemory(0x2D0 + Y, SCREEN_MEMORY[0x2D0 + Y] ^ 0x80); // screen row 18
        writeToScreenMemory(0x2F8 + Y, SCREEN_MEMORY[0x2F8 + Y] ^ 0x80); // screen row 19
        writeToScreenMemory(0x320 + Y, SCREEN_MEMORY[0x320 + Y] ^ 0x80); // screen row 20
    }
#endif
}

// This routine animates the tile #204 (water) 
// and also tile 148 (trash compactor)
void ANIMATE_WATER()
{
    if (ANIMATE != 1) {
        return;
    }
    WATER_TIMER++;
    if (WATER_TIMER != 20) {
        return;
    }
    WATER_TIMER = 0;
#ifndef PLATFORM_IMAGE_BASED_TILES
    WATER_TEMP1 = TILE_DATA_BR[204];
    TILE_DATA_BR[204] = TILE_DATA_MM[204];
    TILE_DATA_BR[221] = TILE_DATA_MM[204];
    TILE_DATA_MM[204] = TILE_DATA_TL[204];
    TILE_DATA_TL[204] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BL[204];
    TILE_DATA_BL[204] = TILE_DATA_MR[204];
    TILE_DATA_BL[221] = TILE_DATA_MR[204];
    TILE_DATA_MR[204] = TILE_DATA_TM[204];
    TILE_DATA_TM[204] = WATER_TEMP1;
    TILE_DATA_TM[221] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BM[204];
    TILE_DATA_BM[204] = TILE_DATA_ML[204];
    TILE_DATA_BM[221] = TILE_DATA_ML[204];
    TILE_DATA_ML[204] = TILE_DATA_TR[204];
    TILE_DATA_TR[204] = WATER_TEMP1;
    TILE_DATA_TR[221] = WATER_TEMP1;

    // now do trash compactor
    WATER_TEMP1 = TILE_DATA_TR[148];
    TILE_DATA_TR[148] = TILE_DATA_TM[148];
    TILE_DATA_TM[148] = TILE_DATA_TL[148];
    TILE_DATA_TL[148] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_MR[148];
    TILE_DATA_MR[148] = TILE_DATA_MM[148];
    TILE_DATA_MM[148] = TILE_DATA_ML[148];
    TILE_DATA_ML[148] = WATER_TEMP1;

    WATER_TEMP1 = TILE_DATA_BR[148];
    TILE_DATA_BR[148] = TILE_DATA_BM[148];
    TILE_DATA_BM[148] = TILE_DATA_BL[148];
    TILE_DATA_BL[148] = WATER_TEMP1;
    // Now do HVAC fan
    if (HVAC_STATE != 0) {
        TILE_DATA_MM[196] = 0xCD;
        TILE_DATA_TL[201] = 0xCD;
        TILE_DATA_ML[197] = 0xCE;
        TILE_DATA_TM[200] = 0xCE;
        TILE_DATA_MR[196] = 0xA0;
        TILE_DATA_BM[196] = 0xA0;
        TILE_DATA_BL[197] = 0xA0;
        TILE_DATA_TR[200] = 0xA0;
        HVAC_STATE = 0;
    } else {
        TILE_DATA_MM[196] = 0xA0;
        TILE_DATA_TL[201] = 0xA0;
        TILE_DATA_ML[197] = 0xA0;
        TILE_DATA_TM[200] = 0xA0;
        TILE_DATA_MR[196] = 0xC2;
        TILE_DATA_TR[200] = 0xC2;
        TILE_DATA_BM[196] = 0xC0;
        TILE_DATA_BL[197] = 0xC0;
        HVAC_STATE = 1;
    }
    // now do cinema screen tiles
    // FIRST COPY OLD LETTERS TO THE LEFT.
    TILE_DATA_MM[20] = TILE_DATA_MR[20]; // #2 -> #1
    TILE_DATA_MR[20] = TILE_DATA_ML[21]; // #3 -> #2
    TILE_DATA_ML[21] = TILE_DATA_MM[21]; // #4 -> #3
    TILE_DATA_MM[21] = TILE_DATA_MR[21]; // #5 -> #4
    TILE_DATA_MR[21] = TILE_DATA_ML[22]; // #6 -> #5
    // now insert new character.
    TILE_DATA_ML[22] = convertToPETSCII(CINEMA_MESSAGE[CINEMA_STATE]); // #6

    CINEMA_STATE++;
    if (CINEMA_STATE == 197) {
        CINEMA_STATE = 0;
    }
    // Now animate light on server computers
    if (TILE_DATA_MR[143] == 0xD7) {
        TILE_DATA_MR[143] = 0xD1;
    } else {
        TILE_DATA_MR[143] = 0xD7;
    }
    uint8_t tiles[] = { 204, 221, 148, 196, 197, 200, 201, 20, 21, 22, 143 };
    platform->updateTiles(TILE_DATA_TL, tiles, 11);
#endif
    REDRAW_WINDOW = 1;
}

uint8_t WATER_TIMER = 0;
uint8_t WATER_TEMP1 = 0;
uint8_t HVAC_STATE = 0;
uint8_t CINEMA_STATE = 0;

// This is the routine that allows a person to select
// a level and highlights the selection in the information
// display. It is unique to each computer since it writes
// to the screen directly.
void ELEVATOR_SELECT()
{
    DRAW_MAP_WINDOW();
    ELEVATOR_MAX_FLOOR = UNIT_D[UNIT]; // get max levels
    // Now draw available levels on screen
    for (int Y = 0, A = 0x31; Y != ELEVATOR_MAX_FLOOR; A++, Y++) {
        writeToScreenMemory(0x3C6 + Y, A);
    }
    ELEVATOR_CURRENT_FLOOR = UNIT_C[UNIT]; // what level are we on now?
    // Now highlight current level
    ELEVATOR_INVERT();
    platform->renderFrame();
    // Now get user input
    if (CONTROL != 2) {
        // KEYBOARD INPUT
        while (!platform->quit) {
            uint8_t A = platform->getin();
            if (A != 0) {
                if (A == KEY_CONFIG[KEY_CURSOR_LEFT] || A == KEY_CONFIG[KEY_MOVE_LEFT]) { // CURSOR LEFT
                    ELEVATOR_DEC();
                } else if (A == KEY_CONFIG[KEY_CURSOR_RIGHT] || A == KEY_CONFIG[KEY_MOVE_RIGHT]) { // CURSOR RIGHT
                    ELEVATOR_INC();
                } else if (A == KEY_CONFIG[KEY_CURSOR_DOWN] || A == KEY_CONFIG[KEY_MOVE_DOWN]) { // CURSOR DOWN
                    SCROLL_INFO();
                    SCROLL_INFO();
                    SCROLL_INFO();
                    CLEAR_KEY_BUFFER();
                    return;
                }
            }
        }
    } else {
        // SNES INPUT
        // TODO
    }
}

uint8_t ELEVATOR_MAX_FLOOR = 0;
uint8_t ELEVATOR_CURRENT_FLOOR = 0;

void ELEVATOR_INVERT()
{
    writeToScreenMemory(0x3C5 + ELEVATOR_CURRENT_FLOOR, SCREEN_MEMORY[0x3C5 + ELEVATOR_CURRENT_FLOOR] ^ 0x80); // %10000000
}

void ELEVATOR_INC()
{
    if (ELEVATOR_CURRENT_FLOOR != ELEVATOR_MAX_FLOOR) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR++;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_DEC()
{
    if (ELEVATOR_CURRENT_FLOOR != 1) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR--;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
        platform->renderFrame();
    }
}

void ELEVATOR_FIND_XY()
{
    for (int X = 32; X != 48; X++) { // start of doors
        if (UNIT_TYPE[X] == 19) { // elevator
            if (UNIT_C[X] == ELEVATOR_CURRENT_FLOOR) {
                UNIT_LOC_X[0] = UNIT_LOC_X[X]; // player location = new elevator location
                MAP_WINDOW_X = UNIT_LOC_X[X] - 5;
                UNIT_LOC_Y[0] = UNIT_LOC_Y[X] - 1; // player location = new elevator location
                MAP_WINDOW_Y = UNIT_LOC_Y[X] - 4;
                DRAW_MAP_WINDOW();
                PLAY_SOUND(17); // elevator sound SOUND PLAY
                break;
            }
        }
    }
}

void SET_CONTROLS()
{
    if (CONTROL == 1) { // CUSTOM KEYS
        SET_CUSTOM_KEYS();
    } else {
        // load standard values for key controls
        uint8_t* STANDARD_CONTROLS = platform->standardControls();
        for (int Y = 0; Y != sizeof(KEY_CONFIG); Y++) {
            KEY_CONFIG[Y] = STANDARD_CONTROLS[Y];
        }
    }
}

void SET_CUSTOM_KEYS()
{
    if (KEYS_DEFINED != 0) {
        return;
    }
    platform->fadeScreen(0, false);
#ifdef PLATFORM_IMAGE_SUPPORT
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS, 15);
#else
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS);
#endif
    platform->renderFrame();
    platform->fadeScreen(15, false);
    uint16_t destination = 0x151;
    for (TEMP_A = 0; TEMP_A != 13;) {
        uint8_t A = platform->getin();
        if (A != 0) {
            KEY_CONFIG[TEMP_A] = A;
            DECNUM = A;
#ifdef PLATFORM_IMAGE_SUPPORT
            DECWRITE(destination, 15);
#else
            DECWRITE(destination);
#endif
            destination += 40;
            TEMP_A++;
            platform->renderFrame();
        }
    }
    KEYS_DEFINED = 1;
}

uint8_t KEYS_DEFINED = 0; // DEFAULT 0

void PET_SCREEN_SHAKE()
{
    if (BGTIMER1 != 1) {
        return;
    }
    if (SELECT_TIMEOUT != 0) { // shoehorned this into the screenshake routine
        SELECT_TIMEOUT--; // this is to prevent accidental double-taps
    } // on cycle weapons or items.
    PET_BORDER_FLASH();
    if (SCREEN_SHAKE != 1) {
        return;
    }
#ifndef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    uint16_t source = 1;
    uint16_t destination = 0;
    for (int Y = 0; Y != 21; Y++) {
        for (int X = 0; X != 32; X++) {
            writeToScreenMemory(destination + X, SCREEN_MEMORY[source + X]);
        }
        destination += 40;
        source += 40;
    }
    REDRAW_WINDOW = 1;
    platform->renderFrame();
#endif
}

// So, it doesn't really flash the PET border, instead it
// flashes the health screen.
void PET_BORDER_FLASH()
{
    if (BORDER != 0) {
        // border flash should be active
        if (FLASH_STATE != 1) { // Is it already flashing?
#ifndef PLATFORM_IMAGE_SUPPORT
            // copy flash message to screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(0x2F2 + X, OUCH1[X]);
                writeToScreenMemory(0x31A + X, OUCH2[X]);
                writeToScreenMemory(0x342 + X, OUCH3[X]);
            }
#endif
            platform->startFadeScreen(BORDER_COLOR, 15 - BORDER);
            FLASH_STATE = 1;
        }
    } else {
        if (FLASH_STATE != 0) {
#ifndef PLATFORM_IMAGE_SUPPORT
            // Remove message from screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(0x2F2 + X, 32);
                writeToScreenMemory(0x31A + X, 32);
                writeToScreenMemory(0x342 + X, 32);
            }
#endif
            FLASH_STATE = 0;
            platform->stopFadeScreen();
        }
    }
}

uint8_t FLASH_STATE = 0;
uint8_t OUCH1[] = { 0xCD, 0xA0, 0xA0, 0xA0, 0xA0, 0xCE };
uint8_t OUCH2[] = { 0xA0, 0x8F, 0x95, 0x83, 0x88, 0xA0 };
uint8_t OUCH3[] = { 0xCE, 0xA0, 0xA0, 0xA0, 0xA0, 0xCD };

// This is actually part of a background routine, but it has to be in the main
// source because the screen effects used are unique on each system.
void DEMATERIALIZE()
{
    UNIT_TILE[0] = (UNIT_TIMER_B[UNIT] & 0x01) + 160; // dematerialize tile
    UNIT_TILE[0] += (UNIT_TIMER_B[UNIT] & 0x08) >> 3;
    UNIT_TIMER_B[UNIT]++;
    if (UNIT_TIMER_B[UNIT] != 0x10) { // %00010000
        UNIT_TIMER_A[UNIT] = 1;
        REDRAW_WINDOW = 1;
    } else {
        // TRANSPORT COMPLETE
        if (UNIT_B[UNIT] != 1) { // transport somewhere
            UNIT_TYPE[0] = 2; // this mean game over condition, player type
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
        } else {
            UNIT_TILE[0] = 97;
            UNIT_LOC_X[0] = UNIT_C[UNIT]; // target X coordinates
            UNIT_LOC_Y[0] = UNIT_D[UNIT]; // target Y coordinates
            UNIT_TYPE[UNIT] = 7; // Normal transporter pad
            CACULATE_AND_REDRAW();
        }
    }
}

void ANIMATE_PLAYER()
{
#ifdef PLATFORM_SPRITE_SUPPORT
    UNIT_TILE[0] = 96;
    WALK_FRAME++;
    WALK_FRAME &= 3;
#else
    if (UNIT_TILE[0] == 97) {
        UNIT_TILE[0] = 96;
    } else {
        UNIT_TILE[0] = 97;
    }
#endif
}

void PLAY_SOUND(int sound)
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->playSample(sound);
#else
    // check if music is playing
    if (MUSIC_ON != 0 && SOUND_EFFECT == 0xFF) { // no sound effect currently being played
        PATTERN_TEMP = CUR_PATTERN;
        DATA_LINE_TEMP = DATA_LINE;
        TEMPO_TEMP = TEMPO;
    }
    // check if we should play new effect.
    if (SOUND_EFFECT != 0xFF) { // no sound effect currently being played
        if (sound > SOUND_EFFECT) { // Prioritize sounds that have lower number.
            return;
        }
    }
    // Now process sound effect.
    CUR_PATTERN = SOUND_LIBRARY[sound]; // Get waiting sound# from accumulator
    SOUND_EFFECT = sound;
    DATA_LINE = 0;
#endif
}

#ifndef PLATFORM_MODULE_BASED_AUDIO
uint8_t* PATTERN_TEMP;
uint8_t DATA_LINE_TEMP;
uint8_t TEMPO_TEMP;

uint8_t* SOUND_LIBRARY[] = {
    SND_EXPLOSION,      // sound 0
    SND_EXPLOSION,      // sound 1
    SND_MEDKIT,     // sound 2
    SND_EMP,        // sound 3
    SND_MAGNET,     // sound 4
    SND_SHOCK,      // sound 5
    SND_MOVE_OBJ,       // sound 6
    SND_SHOCK,      // sound 7
    SND_PLASMA,     // sound 8
    SND_PISTOL,     // sound 9
    SND_ITEM_FOUND,     // sound 10
    SND_ERROR,      // sound 11
    SND_CYCLE_WEAPON,   // sound 12
    SND_CYCLE_ITEM,     // sound 13
    SND_DOOR,       // sound 14
    SND_MENU_BEEP,      // sound 15
    SND_SHORT_BEEP,     // sound 16
    SND_SHORT_BEEP,     // sound 17
    WIN_MUSIC,      // sound 18   
    LOSE_MUSIC     // sound 19
};

// 0 explosion
// 1 small explosion
// 2 medkit
// 3 emp
// 4 haywire
// 5 evilbot
// 6 move
// 7 electric shock
// 8 plasma gun
// 9 fire pistol
// 10 item found
// 11 error
// 12 change weapons
// 13 change items
// 14 door
// 15 menu beep
// 16 walk
// 17 sfx (short beep)
// 18 sfx

void MUSIC_ROUTINE()
{
    if (SOUND_EFFECT == 0xFF && MUSIC_ON != 1) {
        return;
    }
    if (TEMPO_TIMER != 0) {
        TEMPO_TIMER--;
        return;
    }
    uint8_t A = CUR_PATTERN[DATA_LINE];
    if (A == 0) { // blank line (do nothing)
        TEMPO_TIMER = TEMPO; // reset timer to wait for next line
        DATA_LINE++;
        return;
    }
    if (A == 37) { // END pattern
        STOP_SONG();
        return;
    }
    if (A == 38) {
        ARP_MODE = 0;
        platform->stopNote(); // RESET SOUND TO ZERO
    }
    if (A > 38 && A <= 49) { // IS IT A TEMPO COMMAND? COMMAND IS BETWEEN 39 AND 49 (TEMPO ADJUST)
        TEMPO = A - 38;
        DATA_LINE++;
        return;
    }
    // PLAY A NOTE
    ARP_MODE = A >> 6;
    CHORD_ROOT = A & 0x3f; // %00111111
    platform->playNote(CHORD_ROOT);
    TEMPO_TIMER = TEMPO; // reset timer to wait for next line
    DATA_LINE++;
}
#endif

void STOP_SONG()
{
#ifdef PLATFORM_MODULE_BASED_AUDIO
    platform->stopSample();
#else
    // actually, stop sound effect.
    platform->stopNote(); // turn off sound;
    SOUND_EFFECT = 0xFF;
    TEMPO_TIMER = TEMPO;
    /// now restore music info for continued play.
    if (MUSIC_ON != 1) {
        return;
    }
    CUR_PATTERN = PATTERN_TEMP;
    DATA_LINE = DATA_LINE_TEMP;
    TEMPO = TEMPO_TEMP;
#endif
}

void BACKGROUND_TASKS()
{
    if (BGTIMER1 == 1) {
#ifdef PLATFORM_LIVE_MAP_SUPPORT
        if (LIVE_MAP_ON) {
            DRAW_LIVE_MAP();
        } else
#endif
        if (REDRAW_WINDOW == 1) {
            REDRAW_WINDOW = 0;
            DRAW_MAP_WINDOW();
        }
        platform->renderFrame();
    }
    // Now check to see if it is time to run background tasks
    if (BGTIMER1 != 1) {
        return;
    }
    BGTIMER1 = 0; // RESET BACKGROUND TIMER
    for (UNIT = 1; UNIT != 64; UNIT++) {
        // ALL AI routines must JMP back to here at the end.
        if (UNIT_TYPE[UNIT] != 0) { // Does unit exist?
            // Unit found to exist, now check it's timer.
            // unit code won't run until timer hits zero.
            if (UNIT_TIMER_A[UNIT] != 0) {
                UNIT_TIMER_A[UNIT]--; // Decrease timer by one.
            } else {
                // Unit exists and timer has triggered
                // The unit type determines which AI routine is run.
                if (UNIT_TYPE[UNIT] < 24) { // MAX DIFFERENT UNIT TYPES IN CHART, ABORT IF GREATER
                    AI_ROUTINE_CHART[UNIT_TYPE[UNIT]]();
                }
            }
        }
    }
}

void (*AI_ROUTINE_CHART[])(void) =
{
    DUMMY_ROUTINE,      // UNIT TYPE 00   ;non-existent unit
    DUMMY_ROUTINE,      // UNIT TYPE 01   ;player unit - can't use
    LEFT_RIGHT_DROID,   // UNIT TYPE 02
    UP_DOWN_DROID,      // UNIT TYPE 03
    HOVER_ATTACK,       // UNIT TYPE 04
    WATER_DROID,        // UNIT TYPE 05
    TIME_BOMB,      // UNIT TYPE 06
    TRANSPORTER_PAD,    // UNIT TYPE 07
    DEAD_ROBOT,     // UNIT TYPE 08
    EVILBOT,        // UNIT TYPE 09 
    AI_DOOR,        // UNIT TYPE 10
    SMALL_EXPLOSION,    // UNIT TYPE 11
    PISTOL_FIRE_UP,     // UNIT TYPE 12
    PISTOL_FIRE_DOWN,   // UNIT TYPE 13
    PISTOL_FIRE_LEFT,   // UNIT TYPE 14
    PISTOL_FIRE_RIGHT,  // UNIT TYPE 15
    TRASH_COMPACTOR,    // UNIT TYPE 16
    UP_DOWN_ROLLERBOT,  // UNIT TYPE 17
    LEFT_RIGHT_ROLLERBOT,   // UNIT TYPE 18
    ELEVATOR,       // UNIT TYPE 19
    MAGNET,         // UNIT TYPE 20
    MAGNETIZED_ROBOT,   // UNIT TYPE 21
    WATER_RAFT_LR,      // UNIT TYPE 22
    DEMATERIALIZE      // UNIT TYPE 23
};

// Dummy routine does nothing, but I need it for development.
void DUMMY_ROUTINE()
{
    return;
}

void WATER_RAFT_LR()
{
    // First check which direction raft is moving.
    if (UNIT_A[UNIT] == 1) {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]++; // player
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            UNIT_LOC_X[UNIT]++; // raft
            RAFT_PLOT();
            // Now check if it has reached its destination
            CHECK_FOR_WINDOW_REDRAW();
        }
        if (UNIT_LOC_X[UNIT] != UNIT_C[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 0;
        }
    } else {
        RAFT_DELETE();
        // Check to see if player is on raft
        if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] &&
            UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
            UNIT_LOC_X[0]--; // player
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            CACULATE_AND_REDRAW();
        } else {
            UNIT_LOC_X[UNIT]--; // raft
            RAFT_PLOT();
            // Now check if it has reached its destination
            CHECK_FOR_WINDOW_REDRAW();
        }
        // Now check if it has reached its destination
        if (UNIT_LOC_X[UNIT] != UNIT_B[UNIT]) {
            UNIT_TIMER_A[UNIT] = 6;
        } else {
            UNIT_TIMER_A[UNIT] = 100;
            UNIT_A[UNIT] = 1;
        }
    }
}

void RAFT_DELETE()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 204; // WATER TILE
    PLOT_TILE_TO_MAP();
}

void RAFT_PLOT()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = 242; // RAFT TILE
    PLOT_TILE_TO_MAP();
}

void MAGNETIZED_ROBOT()
{
    MOVE_TYPE = 0x01; // %00000001
    GENERATE_RANDOM_NUMBER();
    switch (RANDOM & 0x03) { // %00000011
    case 0:
        REQUEST_WALK_UP();
        break;
    case 1:
        REQUEST_WALK_DOWN();
        break;
    case 2:
        REQUEST_WALK_LEFT();
        break;
    case 3:
        REQUEST_WALK_RIGHT();
        break;
    default:
        break;
    }
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 10;
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_TYPE[UNIT] = UNIT_D[UNIT];
    }
}

void GENERATE_RANDOM_NUMBER()
{
    if (RANDOM != 0) { // added this
        if (RANDOM & 0x80) {
            RANDOM = (RANDOM << 1) ^ 0x1D;
        } else {
            RANDOM = (RANDOM << 1);
        }
    } else {
        RANDOM = 0x1D;
    }
}

void MAGNET()
{
MAGNET: 
    // First let's take care of the timers.  This unit runs
    // every cycle so that it can detect contact with another
    // unit.  But it still needs to count down to terminate
    // So, it uses two timers for a 16-bit value.
    UNIT_TIMER_B[UNIT]--;
    if (UNIT_TIMER_B[UNIT] == 0) {
        UNIT_A[UNIT]--;
        if (UNIT_A[UNIT] == 0) {
            // Both timers have reached zero, time to deactivate.
            UNIT_TYPE[UNIT] = 0;
            MAGNET_ACT = 0;
            return;
        }
    }
    // Now let's see if another units walks on the magnet.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    CHECK_FOR_UNIT();
    if (UNIT_FIND == 255) { // no unit found
        return;
    } else if (UNIT_FIND == 0) { // player unit
        INV_MAGNET++;
        DISPLAY_ITEM();
    } else {
        // Collision with robot detected.
        PLAY_SOUND(4); // HAYWIRE SOUND, SOUND PLAY
        UNIT_D[UNIT_FIND] = UNIT_TYPE[UNIT_FIND]; // make backup of unit type
        UNIT_TYPE[UNIT_FIND] = 21; // Crazy robot AI
        UNIT_TIMER_B[UNIT_FIND] = 60;
    }
    UNIT_TYPE[UNIT] = 0;
    MAGNET_ACT = 0;
}

void DEAD_ROBOT()
{
    UNIT_TYPE[UNIT] = 0;
}

void UP_DOWN_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void LEFT_RIGHT_ROLLERBOT()
{
    UNIT_TIMER_A[UNIT] = 7;
    ROLLERBOT_ANIMATE();
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x01; // %00000001
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        ROLLERBOT_FIRE_DETECT();
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void ROLLERBOT_FIRE_DETECT()
{
    int X;
    TEMP_A = UNIT_LOC_X[UNIT];
    TEMP_B = UNIT_LOC_Y[UNIT];
    // See if we're lined up vertically
    if (UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) { // robot, player
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[UNIT] - UNIT_LOC_X[0] >= 6) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 14; // pistol fire left AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_X[0] - UNIT_LOC_X[UNIT] >= 6) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 15; // pistol fire right AI
                    ROLLERBOT_AFTER_FIRE(X, 245); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    } else if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0]) { // robot, player
        // See if we're lined up horizontally
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0] >= 4) { // robot, player
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 12; // pistol fire UP AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        } else {
            // Check to see if distance is less than 5
            if (UNIT_LOC_Y[0] - UNIT_LOC_Y[UNIT] >= 4) { // player, robot
                return;
            }
            for (X = 28; X != 32; X++) {
                if (UNIT_TYPE[X] == 0) {
                    UNIT_TYPE[X] = 13; // pistol fire DOWN AI
                    ROLLERBOT_AFTER_FIRE(X, 244); // tile for horizontal weapons fire
                    return;
                }
            }
        }
    }
}

void ROLLERBOT_AFTER_FIRE(uint8_t unit, uint8_t tile)
{
    UNIT_TILE[unit] = tile;
    UNIT_A[unit] = 5; // travel distance.
    UNIT_B[unit] = 0; // weapon-type = pistol
    UNIT_TIMER_A[unit] = 0;
    UNIT_LOC_X[unit] = TEMP_A;
    UNIT_LOC_Y[unit] = TEMP_B;
    PLAY_SOUND(9); // PISTOL SOUND SOUND PLAY
}

void ROLLERBOT_ANIMATE()
{
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        return;
    }
    UNIT_TIMER_B[UNIT] = 3; // RESET ANIMATE TIMER
    if (UNIT_TILE[UNIT] == 164) {
        UNIT_TILE[UNIT] = 165; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TILE[UNIT] = 164; // ROLLERBOT TILE
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B: 0=completes level 1=send to coordinates
// UNIT_C: X-coordinate
// UNIT_D: Y-coordinate

// The "DEMATERIALIZE" part of this AI routine has to be in the main 
// source for each individual computer, because the screen effects
// are created uniquely for each one.

void TRANSPORTER_PAD()
{
    // first determine if the player is standing here
    if (UNIT_LOC_X[UNIT] == UNIT_LOC_X[0] && UNIT_LOC_Y[UNIT] == UNIT_LOC_Y[0]) {
        TRANS_PLAYER_PRESENT();
    } else {
        // player not present
        if (UNIT_A[UNIT] != 1) {
            TRANS_ACTIVE();
        } else {
            // test if all robots are dead
            for (int X = 1; X != 28; X++) {
                if (UNIT_TYPE[X] != 0) {
                    UNIT_TIMER_A[UNIT] = 30;
                    return;
                }
            }
            UNIT_A[UNIT] = 0; // make unit active
            UNIT_TIMER_A[UNIT] = 30;
        }
    }
}

void TRANS_PLAYER_PRESENT()
{
    if (UNIT_A[UNIT] != 0) { // unit active
        PRINT_INFO(MSG_TRANS1);
        PLAY_SOUND(11); // error-SOUND SOUND PLAY
        UNIT_TIMER_A[UNIT] = 100;
    } else {
        // start transport process
        UNIT_TYPE[UNIT] = 23; // Convert to different AI
        UNIT_TIMER_A[UNIT] = 5;
        UNIT_TIMER_B[UNIT] = 0;
    }
}

void TRANS_ACTIVE()
{
    if (UNIT_TIMER_B[UNIT] != 1) {
        UNIT_TIMER_B[UNIT] = 1;
        TILE = 30;
    } else {
        UNIT_TIMER_B[UNIT] = 0;
        TILE = 31;
    }
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    PLOT_TILE_TO_MAP();
    CHECK_FOR_WINDOW_REDRAW();
    UNIT_TIMER_A[UNIT] = 30;
}

void TIME_BOMB()
{
    if (UNIT_A[UNIT] == 0) {
        BIG_EXP_PHASE1();
    } else if (UNIT_A[UNIT] == 1) {
        BIG_EXP_PHASE2();
    }
}

// This is the large explosion used by the time-bomb
// and plasma gun, and maybe others.  This is the first
// phase of the explosion, which stores the tiles to
// a buffer and then changes each tile to an explosion.
void BIG_EXP_PHASE1()
{
    if (BIG_EXP_ACT != 0) { // Check that no other explosion active.
        UNIT_TIMER_A[UNIT] = 10;
        return; // wait for existing explosion to finish.
    }
    BIG_EXP_ACT = 1; // Set flag so no other explosions can begin until this one ends.
    SCREEN_SHAKE = 1;
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    platform->startShakeScreen();
#endif
    PLAY_SOUND(0); // explosion-sound SOUND PLAY
    BEX_PART1(); // check center piece for unit
    BEXCEN(); // check center piece for unit
    BEX1_NORTH();
    BEX1_SOUTH();
    BEX1_EAST();
    BEX1_WEST();
    BEX1_NE();
    BEX1_NW();
    BEX1_SE();
    BEX1_SW();
    UNIT_TILE[UNIT] = 246; // explosion tile
    UNIT_A[UNIT] = 1; // move to next phase of explosion.
    UNIT_TIMER_A[UNIT] = 12;
    REDRAW_WINDOW = 1;
}

// There are 8 separate subroutines for the large explosion
// with each one handling a specific outward direction of motion.
// The "unit" itself changes tiles to an explosion, so we don't
// need to mess with the center tile.
void BEX1_NORTH()
{
    BEX_PART1();
    // first tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[0] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[1] = TILE;
    BEX_PART3();
}

void BEX1_SOUTH()
{
    BEX_PART1();
    // first tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[2] = TILE;
    BEX_PART3();
    // second tile
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[3] = TILE;
    BEX_PART3();
}

void BEX1_EAST()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[4] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[5] = TILE;
    BEX_PART3();
}

void BEX1_WEST()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[6] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[7] = TILE;
    BEX_PART3();
}

void BEX1_NE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[8] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[9] = TILE;
    BEX_PART3();
}

void BEX1_NW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[10] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y--;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[11] = TILE;
    BEX_PART3();
}

void BEX1_SE()
{
    BEX_PART1();
    // first tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[12] = TILE;
    BEX_PART3();
    // second tile
    MAP_X++;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[13] = TILE;
    BEX_PART3();
}

void BEX1_SW()
{
    BEX_PART1();
    // first tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[14] = TILE;
    BEX_PART3();
    // second tile
    MAP_X--;
    MAP_Y++;
    if (!BEX_PART2()) {
        return;
    }
    EXP_BUFFER[15] = TILE;
    BEX_PART3();
}

void BEX_PART1()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
}

bool BEX_PART2()
{
    GET_TILE_FROM_MAP();
    return (TILE_ATTRIB[TILE] & 0x10) == 0x10; // can see through tile?
}

void BEX_PART3()
{
    MAP_SOURCE[0] = 246;
    BEXCEN();
}

void BEXCEN()
{
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        TEMP_A = 11;
        INFLICT_DAMAGE();
    }
}

void BIG_EXP_PHASE2()
{
    // Do the center tile first.
    BEX_PART1();
    GET_TILE_FROM_MAP();
    MAP_SOURCE[0] = 246;
    TEMP_A = TILE;
    RESTORE_TILE();
    // tile #0 north 1
    BEX_PART1();
    MAP_Y--;
    TEMP_A = EXP_BUFFER[0];
    RESTORE_TILE();
    // tile #1 north 2
    MAP_Y--;
    TEMP_A = EXP_BUFFER[1];
    RESTORE_TILE();
    // tile #2 south 1
    BEX_PART1();
    MAP_Y++;
    TEMP_A = EXP_BUFFER[2];
    RESTORE_TILE();
    // tile #3 south 2
    MAP_Y++;
    TEMP_A = EXP_BUFFER[3];
    RESTORE_TILE();
    // tile #4 east 1
    BEX_PART1();
    MAP_X++;
    TEMP_A = EXP_BUFFER[4];
    RESTORE_TILE();
    // tile #5 east 2
    MAP_X++;
    TEMP_A = EXP_BUFFER[5];
    RESTORE_TILE();
    // tile #6 west 1
    BEX_PART1();
    MAP_X--;
    TEMP_A = EXP_BUFFER[6];
    RESTORE_TILE();
    // tile #7 west 2
    MAP_X--;
    TEMP_A = EXP_BUFFER[7];
    RESTORE_TILE();
    // tile #8 northeast 1
    BEX_PART1();
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[8];
    RESTORE_TILE();
    // tile #9 northeast 2
    MAP_Y--;
    MAP_X++;
    TEMP_A = EXP_BUFFER[9];
    RESTORE_TILE();
    // tile #10 northwest 1
    BEX_PART1();
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[10];
    RESTORE_TILE();
    // tile #11 northwest 2
    MAP_Y--;
    MAP_X--;
    TEMP_A = EXP_BUFFER[11];
    RESTORE_TILE();
    // tile #12 southeast 1
    BEX_PART1();
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[12];
    RESTORE_TILE();
    // tile #13 southeast 2
    MAP_Y++;
    MAP_X++;
    TEMP_A = EXP_BUFFER[13];
    RESTORE_TILE();
    // tile #14 southwest 1
    BEX_PART1();
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[14];
    RESTORE_TILE();
    // tile #15 southwest 2
    MAP_Y++;
    MAP_X--;
    TEMP_A = EXP_BUFFER[15];
    RESTORE_TILE();
    REDRAW_WINDOW = 1;
    UNIT_TYPE[UNIT] = 0; // Deactivate this AI
    BIG_EXP_ACT = 0;
    SCREEN_SHAKE = 0;
#ifdef PLATFORM_HARDWARE_BASED_SHAKE_SCREEN
    platform->stopShakeScreen();
#endif
}

void RESTORE_TILE()
{
    GET_TILE_FROM_MAP();
    if (TILE != 246) {
        return;
    }
    if (TEMP_A != 131) { // Cannister tile
        if ((TILE_ATTRIB[TEMP_A] & 0x08) == 0x08) { // %00001000 can it be destroyed?
            MAP_SOURCE[0] = DESTRUCT_PATH[TEMP_A];
        } else {
            MAP_SOURCE[0] = TEMP_A;
        }
    } else {
        // What to do if we encounter an explosive cannister
        MAP_SOURCE[0] = 135; // Blown cannister
        for (int X = 28; X != 32; X++) { // Start of weapons units
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 6; // bomb AI
                UNIT_TILE[X] = 131; // Cannister tile
                UNIT_LOC_X[X] = MAP_X;
                UNIT_LOC_Y[X] = MAP_Y;
                UNIT_TIMER_A[X] = 10; // How long until exposion?
                UNIT_A[X] = 0;
                return;
            }
        }
        // no slots available right now, abort.
    }
}

void TRASH_COMPACTOR()
{
    if (UNIT_A[UNIT] == 0) { // OPEN
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 148) { // Usual tile for trash compactor danger zone
            MAP_SOURCE[1] = TILE;
            UNIT_TIMER_A[UNIT] = 20;
            // now check for units in the compactor
            MAP_X = UNIT_LOC_X[UNIT];
            MAP_Y = UNIT_LOC_Y[UNIT];
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return; // Nothing found, do nothing.
            }
        }
        // Object has been detected in TC, start closing.
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else if (UNIT_A[UNIT] == 1) { // MID-CLOSING STATE
        TCPIECE1 = 152;
        TCPIECE2 = 153;
        TCPIECE3 = 156;
        TCPIECE4 = 157;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 50;
        // Now check for any live units in the compactor
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        CHECK_FOR_UNIT();
        if (UNIT_FIND == 255) {
            MAP_X++; // check second tile
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                return;
            }
        }
        // Found unit in compactor, kill it.
        PRINT_INFO(MSG_TERMINATED);
        PLAY_SOUND(0); // EXPLOSION sound SOUND PLAY
        UNIT_TYPE[UNIT_FIND] = 0;
        UNIT_HEALTH[UNIT_FIND] = 0;
        for (int X = 28; X != 32; X++) { // start of weapons
            if (UNIT_TYPE[X] == 0) {
                UNIT_TYPE[X] = 11; // SMALL EXPLOSION
                UNIT_TILE[X] = 248; // first tile for explosion
                UNIT_LOC_X[X] = UNIT_LOC_X[UNIT];
                UNIT_LOC_Y[X] = UNIT_LOC_Y[UNIT];
                if (UNIT_FIND == 0) { // is it the player
                    BORDER_COLOR = 0xf00;
                    BORDER = 10;
                }
                break;
            }
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else if (UNIT_A[UNIT] == 2) { // CLOSED STATE
        TCPIECE1 = 146;
        TCPIECE2 = 147;
        TCPIECE3 = 150;
        TCPIECE4 = 151;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT]++;
        UNIT_TIMER_A[UNIT] = 10;
    } else if (UNIT_A[UNIT] == 3) { // MID-OPENING STATE
        TCPIECE1 = 144;
        TCPIECE2 = 145;
        TCPIECE3 = 148;
        TCPIECE4 = 148;
        DRAW_TRASH_COMPACTOR();
        UNIT_A[UNIT] = 0;
        UNIT_TIMER_A[UNIT] = 20;
        PLAY_SOUND(14); // door sound SOUND PLAY
    } else {
        // should never get here.
    }
}

void DRAW_TRASH_COMPACTOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--; // start one tile above
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = TCPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = TCPIECE2;
    MAP_SOURCE[128] = TCPIECE3;
    MAP_SOURCE[129] = TCPIECE4;
    CHECK_FOR_WINDOW_REDRAW();
}
uint8_t TCPIECE1 = 0;
uint8_t TCPIECE2 = 0;
uint8_t TCPIECE3 = 0;
uint8_t TCPIECE4 = 0;

void WATER_DROID()
{
    // first rotate the tiles
    UNIT_TILE[UNIT]++;
    if (UNIT_TILE[UNIT] == 143) {
        UNIT_TILE[UNIT] = 140;
    }
    UNIT_A[UNIT]--;
    if (UNIT_A[UNIT] != 0) {
        CHECK_FOR_WINDOW_REDRAW();
        return;
    }
    // kill unit after countdown reaches zero. 
    UNIT_TYPE[UNIT] = 8; // Dead robot type
    UNIT_TIMER_A[UNIT] = 255;
    UNIT_TILE[UNIT] = 115; // dead robot tile
    CHECK_FOR_WINDOW_REDRAW();
}

void PISTOL_FIRE_UP()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_LOC_Y[UNIT]--; // move it up one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_DOWN()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_LOC_Y[UNIT]++; // move it down one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_LEFT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_LOC_X[UNIT]--; // move it left one.
        PISTOL_AI_COMMON();
    }
}

void PISTOL_FIRE_RIGHT()
{
    // Check if it has reached limits.
    if (UNIT_A[UNIT] == 0) {
        // if it has reached max range, then it vanishes.
        DEACTIVATE_WEAPON();
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_LOC_X[UNIT]++; // move it right one.
        PISTOL_AI_COMMON();
    }
}

void DEACTIVATE_WEAPON()
{
    UNIT_TYPE[UNIT] = 0;
    if (UNIT_B[UNIT] == 1) {
        UNIT_B[UNIT] = 0;
        PLASMA_ACT = 0;
    }
}

void PISTOL_AI_COMMON()
{
    if (UNIT_B[UNIT] == 0) { // is it pistol or plasma?
        UNIT_A[UNIT]--; // reduce range by one
        // Now check what map object it is on.
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE == 131) { // explosive cannister
            // hit an explosive cannister
            MAP_SOURCE[0] = 135; // Blown cannister
            UNIT_TYPE[UNIT] = 6; // bomb AI
            UNIT_TILE[UNIT] = 131; // Cannister tile
            UNIT_LOC_X[UNIT] = MAP_X;
            UNIT_LOC_Y[UNIT] = MAP_Y;
            UNIT_TIMER_A[UNIT] = 5; // How long until exposion?
            UNIT_A[UNIT] = 0;
        } else if ((TILE_ATTRIB[TILE] & 0x10) != 0x10) { // can see through tile?
            // Hit object that can't pass through, convert to explosion
            UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
            UNIT_TILE[UNIT] = 248; // first tile for explosion
            CHECK_FOR_WINDOW_REDRAW();
        } else {
            // check if it encountered a robot/human
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                CHECK_FOR_WINDOW_REDRAW();
            } else {
                // struck a robot/human
                UNIT_TYPE[UNIT] = 11; // SMALL EXPLOSION
                UNIT_TILE[UNIT] = 248; // first tile for explosion
                TEMP_A = 1; // set damage for pistol
                INFLICT_DAMAGE();
                ALTER_AI();
                CHECK_FOR_WINDOW_REDRAW();
            }
        }
    } else {
        UNIT_A[UNIT]--; // reduce range by one
        // find what tile we are over
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if (TILE != 131) { // cannister tile
            if ((TILE_ATTRIB[TILE] & 0x10) == 0x10) {
                // check if it encountered a human/robot
                CHECK_FOR_UNIT();
                if (UNIT_FIND == 255) { // NO UNIT ENCOUNTERED.
                    // no impacts detected:
                    CHECK_FOR_WINDOW_REDRAW();
                    return;
                }
            }
            // impact detected. convert to explosion
            UNIT_TYPE[UNIT] = 6; // bomb AI
            UNIT_TIMER_A[UNIT] = 1; // How long until exposion?
            UNIT_A[UNIT] = 0;
            PLASMA_ACT = 0;
            CHECK_FOR_WINDOW_REDRAW();
        }
    }
}

// This routine checks to see if the robot being shot
// is a hoverbot, if so it will alter it's AI to attack 
// mode.
void ALTER_AI()
{
    if (UNIT_TYPE[UNIT_FIND] == 2 || UNIT_TYPE[UNIT_FIND] == 3) { // hoverbot left/right UP/DOWN
        UNIT_TYPE[UNIT_FIND] = 4; // Attack AI
    }
}

// This routine will inflict damage on whatever is defined in
// UNIT_FIND in the amount set in TEMP_A.  If the damage is more
// than the health of that unit, it will delete the unit.
void INFLICT_DAMAGE()
{
    UNIT_HEALTH[UNIT_FIND] -= TEMP_A;
    if (UNIT_HEALTH[UNIT_FIND] > 0) {
        if (UNIT_FIND == 0) { // IS IT THE PLAYER?
            DISPLAY_PLAYER_HEALTH();
            BORDER_COLOR = 0xf00;
            BORDER = 10;
        }
        return;
    }
    UNIT_HEALTH[UNIT_FIND] = 0;
    if (UNIT_FIND != 0) { // Is it the player that is dead?
        if (UNIT_TYPE[UNIT_FIND] != 8) { // Dead robot type - is it a dead robot already?
            UNIT_TYPE[UNIT_FIND] = 8;
            UNIT_TIMER_A[UNIT_FIND] = 255;
            UNIT_TILE[UNIT_FIND] = 115; // dead robot tile
        }
    } else {
        UNIT_TYPE[UNIT_FIND] = 0;
        DISPLAY_PLAYER_HEALTH();
        BORDER_COLOR = 0xf00;
        BORDER = 10;
    }
}

void SMALL_EXPLOSION()
{
    UNIT_TIMER_A[UNIT] = 0;
    UNIT_TILE[UNIT]++;
    if (UNIT_TILE[UNIT] != 252) {
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TYPE[UNIT] = 0;
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVER_ATTACK()
{
    UNIT_TIMER_B[UNIT] = 0;
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 7;
    MOVE_TYPE = 0x02; // %00000010 HOVER
    // CHECK FOR HORIZONTAL MOVEMENT
    if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
        REQUEST_WALK_LEFT();
    } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
        REQUEST_WALK_RIGHT();
    }
    // NOW CHECK FOR VERITCAL MOVEMENT
    if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
        REQUEST_WALK_UP();
    } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
        REQUEST_WALK_DOWN();
    }
    ROBOT_ATTACK_RANGE();
    if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
        TEMP_A = 1; // amount of damage it will inflict
        UNIT_FIND = 0; // unit to inflict damage on.
        INFLICT_DAMAGE();
        CREATE_PLAYER_EXPLOSION();
        PLAY_SOUND(7); // electric shock SOUND PLAY
        UNIT_TIMER_A[UNIT] = 30; // rate of attack on player.
        // add some code here to create explosion
    }
    CHECK_FOR_WINDOW_REDRAW();
}

void CREATE_PLAYER_EXPLOSION()
{
    int X;
    for (X = 28; X != 32; X++) { // max unit for weaponsfire
        if (UNIT_TYPE[X] == 0) {
            break;
        }
    }
    UNIT_TYPE[X] = 11; // Small explosion AI type
    UNIT_TILE[X] = 248; // first tile for explosion
    UNIT_TIMER_A[X] = 1;
    UNIT_LOC_X[X] = UNIT_LOC_X[0];
    UNIT_LOC_Y[X] = UNIT_LOC_Y[0];
}

void EVILBOT()
{
    UNIT_TIMER_A[UNIT] = 5;
    // first animate evilbot
    if (UNIT_TILE[UNIT] == 100) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 101) {
        UNIT_TILE[UNIT]++;
    } else if (UNIT_TILE[UNIT] == 102) {
        UNIT_TILE[UNIT]++;
    } else {
        UNIT_TILE[UNIT] = 100;
    }
    // now figure out movement
    if (UNIT_TIMER_B[UNIT] != 0) {
        UNIT_TIMER_B[UNIT]--;
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        UNIT_TIMER_B[UNIT] = 1; // Reset timer B
        MOVE_TYPE = 0x01; // %00000001 WALK
        // CHECK FOR HORIZONTAL MOVEMENT
        if (UNIT_LOC_X[UNIT] > UNIT_LOC_X[0]) {
            REQUEST_WALK_LEFT();
        } else if (UNIT_LOC_X[UNIT] < UNIT_LOC_X[0]) {
            REQUEST_WALK_RIGHT();
        }
        // NOW CHECK FOR VERITCAL MOVEMENT
        if (UNIT_LOC_Y[UNIT] > UNIT_LOC_Y[0]) {
            REQUEST_WALK_UP();
        } else if (UNIT_LOC_Y[UNIT] < UNIT_LOC_Y[0]) {
            REQUEST_WALK_DOWN();
        }
        ROBOT_ATTACK_RANGE();
        if (PROX_DETECT == 1) { // 1=Robot next to player 0=not
            TEMP_A = 5; // amount of damage it will inflict
            UNIT_FIND = 0; // unit to inflict damage on.
            INFLICT_DAMAGE();
            CREATE_PLAYER_EXPLOSION();
            PLAY_SOUND(7); // electric shock sound SOUND PLAY
            UNIT_TIMER_A[UNIT] = 15; // rate of attack on player.
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void AI_DOOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        AIDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*AIDB[])(void) = {
    DOOR_OPEN_A,
    DOOR_OPEN_B,
    DOOR_OPEN_FULL,
    DOOR_CLOSE_A,
    DOOR_CLOSE_B,
    DOOR_CLOSE_FULL
};

void DOOR_OPEN_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 17;
        DOORPIECE2 = 9;
        DOORPIECE3 = 91;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 27;
        DOORPIECE2 = 9;
        DOORPIECE3 = 15;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 30;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_OPEN_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 1) {
        UNIT_TIMER_B[UNIT] = 30; // RESET TIMER
        return;
    }
    // if nobody near door, lets close it.
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 88;
        DOORPIECE2 = 89;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 70;
        DOORPIECE2 = 74;
        DOORPIECE3 = 78;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_A()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 84;
        DOORPIECE2 = 85;
        DOORPIECE3 = 86;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 69;
        DOORPIECE2 = 73;
        DOORPIECE3 = 77;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_B()
{
    if (UNIT_A[UNIT] != 1) {
        // HORIZONTAL DOOR
        DOORPIECE1 = 80;
        DOORPIECE2 = 81;
        DOORPIECE3 = 82;
        DRAW_HORIZONTAL_DOOR();
    } else {
        // VERTICAL DOOR
        DOORPIECE1 = 68;
        DOORPIECE2 = 72;
        DOORPIECE3 = 76;
        DRAW_VERTICAL_DOOR();
    }
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void DOOR_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT != 0) {
        // if player near door, lets open it.
        // first check if locked
        if (UNIT_C[UNIT] == 0 || // Lock status
            (UNIT_C[UNIT] == 1 && (KEYS & 0x01) == 0x01) || // SPADE KEY
            (UNIT_C[UNIT] == 2 && (KEYS & 0x02) == 0x02) || // HEART KEY
            (UNIT_C[UNIT] == 3 && (KEYS & 0x04) == 0x04)) { // STAR KEY
            // Start open door process
            PLAY_SOUND(14); // DOOR-SOUND SOUND PLAY
            if (UNIT_A[UNIT] != 1) {
                // HORIZONTAL DOOR
                DOORPIECE1 = 84;
                DOORPIECE2 = 85;
                DOORPIECE3 = 86;
                DRAW_HORIZONTAL_DOOR();
            } else {
                // VERTICAL DOOR
                DOORPIECE1 = 69;
                DOORPIECE2 = 73;
                DOORPIECE3 = 77;
                DRAW_VERTICAL_DOOR();
            }
            UNIT_B[UNIT] = 0;
            UNIT_TIMER_A[UNIT] = 5;
            CHECK_FOR_WINDOW_REDRAW();
            return;
        }
    }
    UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
}

void DRAW_VERTICAL_DOOR()
{
    MAP_Y = UNIT_LOC_Y[UNIT];
    MAP_Y--;
    MAP_X = UNIT_LOC_X[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE2;
    MAP_SOURCE += 128;
    MAP_SOURCE[0] = DOORPIECE3;
}

void DRAW_HORIZONTAL_DOOR()
{
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_X--;
    MAP_Y = UNIT_LOC_Y[UNIT];
    TILE = DOORPIECE1;
    PLOT_TILE_TO_MAP();
    MAP_SOURCE[1] = DOORPIECE2;
    MAP_SOURCE[2] = DOORPIECE3;
}
uint8_t DOORPIECE1 = 0;
uint8_t DOORPIECE2 = 0;
uint8_t DOORPIECE3 = 0;

void ROBOT_ATTACK_RANGE()
{
    /*
    // This is the original code that does not quite work as intended
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // ROBOT UNIT, PLAYER UNIT
    if (A >= 1) { // 1 HORIZONTAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 1) { // 1 VERTICAL TILE FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
    */

    int X = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]);
    int Y = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]);
    PROX_DETECT = (X == 1 && Y == 0) || (X == 0 && Y == 1) ? 1 : 0;
}

void DOOR_CHECK_PROXIMITY()
{
    // First check horizontal proximity to door
    int A = ABS(UNIT_LOC_X[UNIT] - UNIT_LOC_X[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 HORIZONTAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // Now check vertical proximity
    A = ABS(UNIT_LOC_Y[UNIT] - UNIT_LOC_Y[0]); // DOOR UNIT, PLAYER UNIT
    if (A >= 2) { // 2 VERTICAL TILES FROM PLAYER
        PROX_DETECT = 0; // player not detected
        return;
    }
    // PLAYER DETECTED, CHANGE DOOR MODE.
    PROX_DETECT = 1;
}
uint8_t PROX_DETECT = 0; // 0=NO 1=YES

// This routine handles automatic sliding doors.
// UNIT_B register means:
// 0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
void ELEVATOR()
{
    if (UNIT_B[UNIT] < 6) { // make sure number is in bounds
        ELDB[UNIT_B[UNIT]]();
    }
    // -SHOULD NEVER NEED TO HAPPEN
}
void (*ELDB[])(void) = {
    ELEV_OPEN_A,
    ELEV_OPEN_B,
    ELEV_OPEN_FULL,
    ELEV_CLOSE_A,
    ELEV_CLOSE_B,
    ELEV_CLOSE_FULL
};

void ELEV_OPEN_A()
{
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 1;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_B()
{
    DOORPIECE1 = 182;
    DOORPIECE2 = 9;
    DOORPIECE3 = 172;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 2;
    UNIT_TIMER_A[UNIT] = 50;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_OPEN_FULL()
{
    // CLOSE DOOR
    // check for object in the way first.
    MAP_X = UNIT_LOC_X[UNIT];
    MAP_Y = UNIT_LOC_Y[UNIT];
    GET_TILE_FROM_MAP();
    if (TILE != 9) { // FLOOR-TILE
        // SOMETHING IN THE WAY, ABORT
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // check for player or robot in the way
    CHECK_FOR_UNIT();
    if (UNIT_FIND != 255) {
        UNIT_TIMER_A[UNIT] = 35;
        return;
    }
    // START TO CLOSE ELEVATOR DOOR
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 181;
    DOORPIECE2 = 89;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 3;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_A()
{
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 4;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEV_CLOSE_B()
{
    DOORPIECE1 = 80;
    DOORPIECE2 = 81;
    DOORPIECE3 = 174;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 5;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
    ELEVATOR_PANEL();
}

void ELEV_CLOSE_FULL()
{
    DOOR_CHECK_PROXIMITY();
    if (PROX_DETECT == 0) {
        UNIT_TIMER_A[UNIT] = 20; // RESET TIMER
        return;
    }
    // Start open door process
    PLAY_SOUND(14); // DOOR SOUND SOUND PLAY
    DOORPIECE1 = 84;
    DOORPIECE2 = 85;
    DOORPIECE3 = 173;
    DRAW_HORIZONTAL_DOOR();
    UNIT_B[UNIT] = 0;
    UNIT_TIMER_A[UNIT] = 5;
    CHECK_FOR_WINDOW_REDRAW();
}

void ELEVATOR_PANEL()
{
    // Check to see if player is standing in the
    // elevator first.
    if ((UNIT_LOC_X[UNIT] != UNIT_LOC_X[0]) || // elevator X location, player X location
        ((UNIT_LOC_Y[UNIT] - 1) != UNIT_LOC_Y[0])) { // elevator Y location, player Y location
        return;
    }
    // PLAYER DETECTED, START ELEVATOR PANEL
    PRINT_INFO(MSG_ELEVATOR);
    PRINT_INFO(MSG_LEVELS);
    ELEVATOR_SELECT();
}

void PLOT_TILE_TO_MAP()
{
    MAP_SOURCE = MAP + (MAP_Y << 7) + MAP_X;
    MAP_SOURCE[0] = TILE;
}

// This routine will return the tile for a specific X/Y
// on the map.  You must first define MAP_X and MAP-Y.
// The result is stored in TILE.
void GET_TILE_FROM_MAP()
{
    MAP_SOURCE = MAP + ((MAP_Y << 7) + MAP_X);
    TILE = MAP_SOURCE[0];
}

// In this AI routine, the droid simply goes left until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void LEFT_RIGHT_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=LEFT 1=RIGHT
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_LEFT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_RIGHT();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

// In this AI routine, the droid simply goes UP until it
// hits an object, and then reverses direction and does the
// same, bouncing back and forth.
void UP_DOWN_DROID()
{
    HOVERBOT_ANIMATE(UNIT);
    UNIT_TIMER_A[UNIT] = 10; // reset timer to 10
    if (UNIT_A[UNIT] != 1) { // GET DIRECTION 0=UP 1=DOWN
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_UP();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 1; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    } else {
        MOVE_TYPE = 0x02; // %00000010
        REQUEST_WALK_DOWN();
        if (MOVE_RESULT != 1) {
            UNIT_A[UNIT] = 0; // CHANGE DIRECTION
        }
        CHECK_FOR_WINDOW_REDRAW();
    }
}

void HOVERBOT_ANIMATE(uint8_t X)
{
    if (UNIT_TIMER_B[X] != 0) {
        UNIT_TIMER_B[X]--;
    } else {
        UNIT_TIMER_B[X] = 3; // RESET ANIMATE TIMER
        if (UNIT_TILE[X] == 98) {
            UNIT_TILE[X] = 99; // HOVERBOT TILE
        } else {
            UNIT_TILE[X] = 98; // HOVERBOT TILE
        }
    }
}

// The following 4 routines are used by both the player and some
// of the enemy units.  It checks to see if you can walk in a
// specific direction.  It checks for edge of map and also that
// the tile you want to walk onto can allow that.  There is a
// separate routine for hovering robots.
void REQUEST_WALK_RIGHT()
{
    UNIT_DIRECTION[UNIT] = 3;
    if (UNIT_LOC_X[UNIT] != 122) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X++;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_LEFT()
{
    UNIT_DIRECTION[UNIT] = 2;
    if (UNIT_LOC_X[UNIT] != 5) {
        MAP_X = UNIT_LOC_X[UNIT];
        MAP_X--;
        MAP_Y = UNIT_LOC_Y[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_X[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_DOWN()
{
    UNIT_DIRECTION[UNIT] = 1;
    if (UNIT_LOC_Y[UNIT] != 60) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y++;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]++;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

void REQUEST_WALK_UP()
{
    UNIT_DIRECTION[UNIT] = 0;
    if (UNIT_LOC_Y[UNIT] != 3) {
        MAP_Y = UNIT_LOC_Y[UNIT];
        MAP_Y--;
        MAP_X = UNIT_LOC_X[UNIT];
        GET_TILE_FROM_MAP();
        if ((TILE_ATTRIB[TILE] & MOVE_TYPE) == MOVE_TYPE) { // Check, can walk on this tile?
            CHECK_FOR_UNIT();
            if (UNIT_FIND == 255) {
                UNIT_LOC_Y[UNIT]--;
                MOVE_RESULT = 1; // Move success
                return;
            }
        }
    }
    MOVE_RESULT = 0; // Move fail
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a unit present at 
// that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_UNIT()
{
    for (int X = 0; X != 28; X++) {
        if (UNIT_TYPE[X] != 0 && UNIT_LOC_X[X] == MAP_X && UNIT_LOC_Y[X] == MAP_Y) {
            UNIT_FIND = X; // unit found
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

// This routine checks a specific place on the map specified
// in MAP_X and MAP_Y to see if there is a hidden unit present 
// at that spot. If so, the unit# will be stored in UNIT_FIND
// otherwise 255 will be stored. 
void CHECK_FOR_HIDDEN_UNIT()
{
    for (int X = 48; X != 64; X++) {
        if (UNIT_TYPE[X] != 0 &&
            (UNIT_LOC_X[X] == MAP_X || // first compare horizontal position
             (UNIT_LOC_X[X] <= MAP_X && (UNIT_LOC_X[X] + UNIT_C[X]) >= MAP_X)) && // add hidden unit width
            (UNIT_LOC_Y[X] == MAP_Y || // now compare vertical position
             (UNIT_LOC_Y[X] <= MAP_Y && (UNIT_LOC_Y[X] + UNIT_D[X]) >= MAP_Y))) { // add hidden unit HEIGHT
            UNIT_FIND = X;
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

#ifndef PLATFORM_IMAGE_SUPPORT
// These are the included binary files that contain the screen
// image for the main editor.
uint8_t INTRO_TEXT[] = {
    0x60, 0x20, 0x02, 0x4e, 0x60, 0x63, 0x0a, 0x4e, 0x65, 0x60, 0x20, 0x05, 0xe9, 0xce, 0x20, 0x20, 0xe9, 0xce, 0x60, 0x20,
    0x0d, 0xcd, 0x60, 0xa0, 0x09, 0xce, 0x20, 0x65, 0x60, 0x20, 0x05, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x0d,
    0xa0, 0x13, 0x14, 0x01, 0x12, 0x14, 0x20, 0x07, 0x01, 0x0d, 0x05, 0xa0, 0x20, 0x65, 0x60, 0x20, 0x04, 0xe9, 0x66, 0xce,
    0xa0, 0xa0, 0x66, 0xce, 0xce, 0x60, 0x20, 0x0c, 0xa0, 0x13, 0x05, 0x0c, 0x05, 0x03, 0x14, 0x20, 0x0d, 0x01, 0x10, 0xa0,
    0x20, 0x65, 0x60, 0x20, 0x03, 0xe9, 0xa0, 0xe3, 0x60, 0xa0, 0x02, 0xe3, 0x60, 0xce, 0x02, 0x60, 0x20, 0x0b, 0xa0, 0x04,
    0x09, 0x06, 0x06, 0x09, 0x03, 0x15, 0x0c, 0x14, 0x19, 0xa0, 0x20, 0x65, 0x60, 0x20, 0x02, 0xe9, 0x60, 0x66, 0x06, 0xce,
    0xce, 0xa0, 0x60, 0x20, 0x0b, 0xa0, 0x03, 0x0f, 0x0e, 0x14, 0x12, 0x0f, 0x0c, 0x13, 0x20, 0x20, 0xa0, 0x20, 0x65, 0x60,
    0x20, 0x02, 0x66, 0x3a, 0x4d, 0x60, 0x3a, 0x02, 0x4e, 0x3a, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x02, 0xe9, 0xce, 0x20, 0x20,
    0xe9, 0xce, 0x60, 0x20, 0x02, 0xce, 0x60, 0xa0, 0x09, 0xcd, 0x4e, 0x60, 0x20, 0x03, 0x66, 0x55, 0x43, 0x4d, 0x3a, 0x4e,
    0x43, 0x49, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x02, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x13, 0x66, 0x42, 0x51,
    0x48, 0x3a, 0x42, 0x51, 0x48, 0x66, 0xa0, 0x69, 0x60, 0x20, 0x02, 0x66, 0xa0, 0x20, 0x20, 0x66, 0xa0, 0x60, 0x20, 0x02,
    0x70, 0x60, 0x40, 0x02, 0x73, 0x0d, 0x01, 0x10, 0x6b, 0x60, 0x40, 0x02, 0x6e, 0x60, 0x20, 0x03, 0x66, 0x4a, 0x46, 0x4b,
    0x3a, 0x4a, 0x46, 0x4b, 0x66, 0xce, 0x60, 0x20, 0x03, 0x66, 0xce, 0xa0, 0xa0, 0x66, 0xa0, 0x20, 0x20, 0x0b, 0x09, 0x0c,
    0x0c, 0x20, 0x01, 0x0c, 0x0c, 0x20, 0x08, 0x15, 0x0d, 0x01, 0x0e, 0x13, 0x60, 0x20, 0x03, 0x60, 0x66, 0x06, 0xa0, 0xa0,
    0x60, 0x20, 0x03, 0x60, 0x66, 0x04, 0x69, 0x60, 0x20, 0x14, 0x66, 0x60, 0xd0, 0x04, 0x66, 0xa0, 0xa0, 0x60, 0x20, 0x05,
    0x66, 0xa0, 0x20, 0x20, 0x60, 0x43, 0x14, 0x66, 0x60, 0xd0, 0x04, 0x66, 0xa0, 0x69, 0x60, 0x43, 0x05, 0x66, 0xa0, 0x43,
    0x43, 0x60, 0x3a, 0x14, 0x60, 0x66, 0x06, 0xce, 0xa0, 0xa0, 0xce, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x16, 0xe9,
    0xa0, 0xa0, 0xe7, 0xd0, 0xce, 0x60, 0xa0, 0x02, 0xce, 0xa0, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x15, 0xe9, 0x60,
    0xa0, 0x03, 0xe3, 0x60, 0xa0, 0x02, 0xce, 0xa0, 0xa0, 0x60, 0x3a, 0x03, 0x66, 0xa0, 0x60, 0x3a, 0x0b, 0xe9, 0xce, 0xdf,
    0x60, 0x3a, 0x06, 0x60, 0x66, 0x08, 0xd5, 0xc0, 0xc9, 0x60, 0x3a, 0x03, 0x66, 0xce, 0xdf, 0x60, 0x3a, 0x09, 0xe9, 0xe3,
    0xcd, 0xce, 0x60, 0xa0, 0x06, 0x66, 0x51, 0x60, 0x66, 0x04, 0x51, 0x66, 0xdd, 0xce, 0xe3, 0x60, 0xa0, 0x02, 0xce, 0xa0,
    0xcd, 0xce, 0x60, 0x3a, 0x09, 0xa0, 0xd1, 0xe7, 0x60, 0x66, 0x10, 0xdd, 0x60, 0x66, 0x04, 0xa0, 0xd1, 0xe7, 0x69, 0x60,
    0x3a, 0x09, 0x5f, 0xa0, 0xce, 0x60, 0x3a, 0x07, 0x60, 0x66, 0x08, 0xca, 0xc0, 0xcb, 0x60, 0x3a, 0x02, 0x5f, 0xe4, 0x69,
    0x60, 0x3a, 0x0b, 0x66, 0xa0, 0x3a, 0xe9, 0xa0, 0xa0, 0xce, 0x3a, 0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0x66,
    0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0xe9, 0xa0, 0xa0, 0xce, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x60, 0x66,
    0x02, 0xce, 0xce, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x66, 0x02, 0xce, 0xce, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x66, 0x02, 0x69,
    0x60, 0x66, 0x02, 0x69, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0xce, 0xa0, 0x66, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x66,
    0xce, 0xa0, 0x66, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0x66, 0xce, 0xa0, 0xce, 0x60, 0x3a, 0x0a, 0x66,
    0xa0, 0x3a, 0x60, 0x66, 0x02, 0xce, 0xce, 0x66, 0xa0, 0x66, 0xa0, 0x60, 0x66, 0x02, 0xce, 0xce, 0x66, 0xa0, 0x66, 0xa0,
    0x3a, 0x66, 0xa0, 0x3a, 0x60, 0x66, 0x02, 0xa0, 0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x66,
    0xce, 0x66, 0xa0, 0x66, 0xce, 0xa0, 0x66, 0x69, 0x66, 0xce, 0x66, 0xa0, 0x3a, 0x66, 0xa0, 0x3a, 0xe9, 0xa0, 0x66, 0xa0,
    0x60, 0x3a, 0x0a, 0x66, 0xa0, 0x3a, 0x66, 0x69, 0x3a, 0x66, 0x69, 0x60, 0x66, 0x02, 0x69, 0x60, 0x66, 0x02, 0x69, 0x3a,
    0x60, 0x66, 0x02, 0x69, 0x3a, 0x66, 0x69, 0x3a, 0x60, 0x66, 0x02, 0x69, 0x3a
};

uint8_t SCR_TEXT[] = {
    0x60, 0x20, 0x20, 0x5d, 0x17, 0x05, 0x01, 0x10, 0x0f, 0x0e, 0x60, 0x20, 0x20, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20,
    0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20,
    0x5d, 0x20, 0x09, 0x14, 0x05, 0x0d, 0x60, 0x20, 0x21, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20,
    0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20, 0x5d, 0x20, 0x0b, 0x05,
    0x19, 0x13, 0x60, 0x20, 0x21, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x6b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20,
    0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x20, 0x05, 0x73, 0x09, 0x0e, 0x06, 0x0f, 0x12, 0x0d, 0x01,
    0x14, 0x09, 0x0f, 0x0e, 0x6b, 0x60, 0x40, 0x13, 0x5b, 0x60, 0x40, 0x05, 0x60, 0x20, 0x20, 0x5d, 0x08, 0x05, 0x01, 0x0c,
    0x14, 0x08, 0x60, 0x20, 0x20, 0x5d, 0x60, 0x20, 0x26, 0x5d, 0x60, 0x71, 0x05
};

uint8_t SCR_ENDGAME[] = {
    0x55, 0x60, 0x40, 0x03, 0x73, 0x01, 0x14, 0x14, 0x01, 0x03, 0x0B, 0x20, 0x0F, 0x06, 0x20,
    0x14, 0x08, 0x05, 0x20, 0x10, 0x05, 0x14, 0x13, 0x03, 0x09, 0x09, 0x20, 0x12, 0x0F, 0x02,
    0x0F, 0x14, 0x13, 0x6B, 0x60, 0x40, 0x03, 0x49, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x0A, 0x13, 0x03, 0x05,
    0x0E, 0x01, 0x12, 0x09, 0x0F, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x06, 0x05, 0x0C, 0x01, 0x10, 0x13, 0x05, 0x04, 0x20, 0x14, 0x09, 0x0D,
    0x05, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x02,
    0x12, 0x0F, 0x02, 0x0F, 0x14, 0x13, 0x20, 0x12, 0x05, 0x0D, 0x01, 0x09, 0x0E, 0x09, 0x0E,
    0x07, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x20, 0x20, 0x13,
    0x05, 0x03, 0x12, 0x05, 0x14, 0x13, 0x20, 0x12, 0x05, 0x0D, 0x01, 0x09, 0x0E, 0x09, 0x0E,
    0x07, 0x3A, 0x60, 0x20, 0x11, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x08,
    0x04, 0x09, 0x06, 0x06, 0x09, 0x03, 0x15, 0x0C, 0x14, 0x19, 0x3A, 0x60, 0x20, 0x11, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x4A, 0x60, 0x40, 0x25, 0x4B
};
#endif

uint8_t SCR_CUSTOM_KEYS[] = {
    0x55, 0x60, 0x40, 0x03, 0x73, 0x01, 0x14, 0x14, 0x01, 0x03, 0x0B, 0x20, 0x0F, 0x06, 0x20,
    0x14, 0x08, 0x05, 0x20, 0x10, 0x05, 0x14, 0x13, 0x03, 0x09, 0x09, 0x20, 0x12, 0x0F, 0x02,
    0x0F, 0x14, 0x13, 0x6B, 0x60, 0x40, 0x03, 0x49, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60,
    0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x10, 0x12, 0x05, 0x13, 0x13, 0x20, 0x14, 0x08,
    0x05, 0x20, 0x0B, 0x05, 0x19, 0x13, 0x20, 0x19, 0x0F, 0x15, 0x20, 0x17, 0x09, 0x13, 0x08,
    0x20, 0x14, 0x0F, 0x20, 0x15, 0x13, 0x05, 0x60, 0x20, 0x03, 0x5D, 0x5D, 0x60, 0x20, 0x04,
    0x06, 0x0F, 0x12, 0x20, 0x14, 0x08, 0x05, 0x20, 0x06, 0x0F, 0x0C, 0x0C, 0x0F, 0x17, 0x09,
    0x0E, 0x07, 0x20, 0x06, 0x15, 0x0E, 0x03, 0x14, 0x09, 0x0F, 0x0E, 0x13, 0x60, 0x20, 0x05,
    0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25,
    0x5D, 0x5D, 0x60, 0x20, 0x06, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20,
    0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x04, 0x0D, 0x0F, 0x16, 0x05, 0x20, 0x0C,
    0x05, 0x06, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x0D, 0x0F, 0x16,
    0x05, 0x20, 0x12, 0x09, 0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20,
    0x06, 0x06, 0x09, 0x12, 0x05, 0x20, 0x15, 0x10, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x04, 0x0F, 0x17, 0x0E, 0x3A, 0x60, 0x20, 0x16,
    0x5D, 0x5D, 0x60, 0x20, 0x04, 0x06, 0x09, 0x12, 0x05, 0x20, 0x0C, 0x05, 0x06, 0x14, 0x3A,
    0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x03, 0x06, 0x09, 0x12, 0x05, 0x20, 0x12, 0x09,
    0x07, 0x08, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x03, 0x19, 0x03, 0x0C, 0x05,
    0x20, 0x17, 0x05, 0x01, 0x10, 0x0F, 0x0E, 0x13, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60,
    0x20, 0x02, 0x03, 0x19, 0x03, 0x0C, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D, 0x13, 0x3A, 0x60,
    0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x05, 0x15, 0x13, 0x05, 0x20, 0x09, 0x14, 0x05, 0x0D,
    0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x20, 0x13, 0x05, 0x01, 0x12, 0x03, 0x08, 0x20, 0x0F,
    0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D, 0x5D, 0x60, 0x20, 0x02, 0x0D,
    0x0F, 0x16, 0x05, 0x20, 0x0F, 0x02, 0x0A, 0x05, 0x03, 0x14, 0x3A, 0x60, 0x20, 0x16, 0x5D,
    0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D, 0x5D, 0x60, 0x20, 0x25, 0x5D,
    0x4A, 0x60, 0x40, 0x25, 0x4B
};

#ifndef PLATFORM_IMAGE_BASED_TILES
uint8_t CINEMA_MESSAGE[] = {
    "coming soon: space balls 2 - the search for more money, "
    "attack of the paperclips: clippy's revenge, "
    "it came from planet earth, "
    "rocky 5000, all my circuits the movie, "
    "conan the librarian, and more! " 
};
#endif

#ifndef PLATFORM_IMAGE_SUPPORT
uint8_t WEAPON1A[] = {
    0x2c, 0x20, 0x20, 0x20, 0x20, 0x2c
};

uint8_t WEAPON1B[] = {
    0xe2, 0xf9, 0xef, 0xe4, 0x66, 0x66
};

uint8_t WEAPON1C[] = {
    0x20, 0x20, 0x20, 0x20, 0x5f, 0xdf
};

uint8_t WEAPON1D[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t PISTOL1A[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t PISTOL1B[] = {
    0x20, 0x68, 0x62, 0x62, 0x62, 0x20
};

uint8_t PISTOL1C[] = {
    0x20, 0x20, 0x20, 0x5f, 0xdf, 0x20
};

uint8_t PISTOL1D[] = {
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t TBOMB1A[] = {
    0x20, 0x20, 0x55, 0x2a, 0x20, 0x20
};

uint8_t TBOMB1B[] = {
    0x20, 0x55, 0x66, 0x49, 0x20, 0x20
};

uint8_t TBOMB1C[] = {
    0x20, 0x42, 0x20, 0x48, 0x20, 0x20
};

uint8_t TBOMB1D[] = {
    0x20, 0x4a, 0x46, 0x4b, 0x20, 0x20
};

uint8_t EMP1A[] = {
    0x20, 0x55, 0x43, 0x43, 0x49, 0x20
};

uint8_t EMP1B[] = {
    0x66, 0xdf, 0x55, 0x49, 0xe9, 0x66
};

uint8_t EMP1C[] = {
    0x66, 0x69, 0x4a, 0x4b, 0x5f, 0x66
};

uint8_t EMP1D[] = {
    0x20, 0x4a, 0x46, 0x46, 0x4b, 0x20
};

uint8_t MAG1A[] = {
    0x4d, 0x70, 0x6e, 0x70, 0x6e, 0x4e
};

uint8_t MAG1B[] = {
    0x20, 0x42, 0x42, 0x48, 0x48, 0x20
};

uint8_t MAG1C[] = {
    0x63, 0x42, 0x4a, 0x4b, 0x48, 0x63
};

uint8_t MAG1D[] = {
    0x4e, 0x4a, 0x46, 0x46, 0x4b, 0x4d
};

uint8_t MED1A[] = {
    0x20, 0x55, 0x43, 0x43, 0x49, 0x20
};

uint8_t MED1B[] = {
    0x20, 0xA0, 0xA0, 0xA0, 0xA0, 0x20
};

uint8_t MED1C[] = {
    0x20, 0xA0, 0xEB, 0xF3, 0xA0, 0x20
};

uint8_t MED1D[] = {
    0x20, 0xE4, 0xE4, 0xE4, 0xE4, 0x20
};
#endif

#ifndef PLATFORM_MODULE_BASED_AUDIO
uint8_t NOTE_FREQ[] = {
    0,  // placeholder for zero
    251, // Note 01    B   (Lowest note the PET can produce)
    // octave 4
    238, // Note 02    C
    224, // Note 03    C#
    210, // Note 04    D
    199, // Note 05    D#
    188, // Note 06    E
    177, // Note 07    F
    168, // Note 08    F#
    158, // Note 09    G
    149, // Note 10    G#
    140, // Note 11    A
    133, // Note 12    A#
    251, // Note 13    B
    // octave 5
    238, // Note 14    C
    224, // Note 15    C#
    210, // Note 16    D
    199, // Note 17    D#
    188, // Note 18    E
    177, // Note 19    F
    168, // Note 20    F#
    158, // Note 21    G
    149, // Note 22    G#
    140, // Note 23    A
    133, // Note 24    A#
    251, // Note 25    B
    // octave 6
    238, // Note 26    C
    224, // Note 27    C#
    210, // Note 28    D
    199, // Note 29    D#
    188, // Note 30    E
    177, // Note 31    F
    168, // Note 32    F#
    158, // Note 33    G
    149, // Note 34    G#
    140, // Note 35    A
    133 // Note 36    A#  (Highest note the PET can produce)
};

uint8_t NOTE_OCTAVE[] = {
    00,  // placeholder for zero
    15,  // Note 01    B   (Lowest note the PET can produce)
    // octave 4
    15,  // Note 02    C
    15,  // Note 03    C#
    15,  // Note 04    D
    15,  // Note 05    D#
    15,  // Note 06    E
    15,  // Note 07    F
    15,  // Note 08    F#
    15,  // Note 09    G
    15,  // Note 10    G#
    15,  // Note 11    A
    15,  // Note 12    A#
    51,  // Note 13    B
    // octave 5
    51,  // Note 14    C
    51,  // Note 15    C#
    51,  // Note 16    D
    51,  // Note 17    D#
    51,  // Note 18    E
    51,  // Note 19    F
    51,  // Note 20    F#
    51,  // Note 21    G
    51,  // Note 22    G#
    51,  // Note 23    A
    51,  // Note 24    A#
    85,  // Note 25    B
    // octave 6
    85,  // Note 26    C
    85,  // Note 27    C#
    85,  // Note 28    D
    85,  // Note 29    D#
    85,  // Note 30    E
    85,  // Note 31    F
    85,  // Note 32    F#
    85,  // Note 33    G
    85,  // Note 34    G#
    85,  // Note 35    A
    85  // Note 36    A#  (Highest note the PET can produce)
};

uint8_t SND_EXPLOSION[] = {
    0x27, 0x0B, 0x0D, 0x07, 0x08, 0x0B, 0x03, 0x07, 0x03, 0x05, 0x06, 0x0C, 0x0E, 0x02, 0x01,
    0x06, 0x08, 0x0D, 0x0B, 0x01, 0x05, 0x06, 0x07, 0x0A, 0x0A, 0x03, 0x06, 0x25
};

uint8_t SND_MEDKIT[] = {
    0x29, 0x09, 0x04, 0x01, 0x25
};

uint8_t SND_EMP[] = {
    0x27, 0x02, 0x07, 0x0C, 0x11, 0x16, 0x1B, 0x20, 0x1B, 0x16, 0x11, 0x0C, 0x07, 0x02, 0x25
};

uint8_t SND_MAGNET[] = {
    0x2A, 0x17, 0x0D, 0x12, 0x23, 0x0F, 0x15, 0x09, 0x17, 0x03, 0x25
};

uint8_t SND_SHOCK[] = {
    0x27, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x0E, 0x26, 0x04,
    0x26, 0x04, 0x26, 0x04, 0x26, 0x04, 0x25
};

uint8_t SND_MOVE_OBJ[] = {
    0x28, 0x02, 0x26, 0x0E, 0x26, 0x25
};

uint8_t SND_PLASMA[] = {
    0x27, 0x07, 0x13, 0x06, 0x12, 0x07, 0x13, 0x06, 0x12,
    0x07, 0x13, 0x06, 0x12, 0x07, 0x13, 0x25 
};

uint8_t SND_PISTOL[] = {
    0x27, 0x24, 0x23, 0x22, 0x21, 0x20, 0x1F, 0x25
};

uint8_t SND_ITEM_FOUND[] = {
    0x28, 0x09, 0x15, 0x0B, 0x17, 0x0D, 0x19,
    0x0E, 0x02, 0x0E, 0x02, 0x0E, 0x25
};

uint8_t SND_ERROR[] = {
    0x27, 0x01, 0x26, 0x01, 0x26, 0x01, 0x26, 0x01, 0x25
};

uint8_t SND_CYCLE_WEAPON[] = {
    0x27, 0x15, 0x13, 0x11, 0x0F, 0x0D, 0x15, 0x25
};

uint8_t SND_CYCLE_ITEM[] = {
    0x27, 0x0D, 0x0C, 0x0B, 0x0A, 0x0B, 0x0C, 0x0D, 0x25
};

uint8_t SND_DOOR[] = {
    0x28, 0x0B, 0x0D, 0x0E, 0x10, 0x12, 0x13, 0x15, 0x25
};

uint8_t SND_MENU_BEEP[] = {
    0x28, 0x0E, 0x04, 0x25
};

uint8_t SND_SHORT_BEEP[] = {
    0x28, 0x11, 0x25
};

uint8_t INTRO_MUSIC[] = {
    0x2D, 0x06, 0x12, 0x0F, 0x03, 0x0F, 0x0D, 0x08, 0x12, 0x03, 0x06, 0x08, 0x0A, 0x03, 0x12,
    0x03, 0x09, 0x2D, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00, 0x01, 0x26,
    0x03, 0x01, 0x00, 0x00, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00, 0x01,
    0x26, 0x08, 0x06, 0x00, 0x00, 0x03, 0x00, 0x01, 0x01, 0x03, 0x26, 0x06, 0x26, 0x00, 0x00,
    0x01, 0x26, 0x03, 0x01, 0x00, 0x00, 0x08, 0x0A, 0x08, 0x00, 0x06, 0x00, 0x03, 0x01, 0x03,
    0x26, 0x06, 0x26, 0x03, 0x26, 0x00, 0x00, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x0F, 0x26, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00, 0x12,
    0x00, 0x00, 0x00, 0x0D, 0x0F, 0x14, 0x00, 0x12, 0x00, 0x0F, 0x00, 0x0D, 0x0F, 0x26, 0x00,
    0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x14, 0x00, 0x0F, 0x00, 0x14, 0x16, 0x14, 0x00, 0x12,
    0x00, 0x0F, 0x0D, 0x0F, 0x26, 0x12, 0x26, 0x0F, 0x26, 0x00, 0x00, 0x0F, 0x00, 0x0D, 0x0F,
    0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x26, 0x0F, 0x26, 0x0F, 0x00, 0x0D,
    0x0F, 0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x14, 0x00, 0x0F, 0x00,
    0x0D, 0x0F, 0x26, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0D, 0x0F, 0x12, 0x00, 0x0F, 0x00, 0x14,
    0x16, 0x14, 0x00, 0x12, 0x14, 0x12, 0x00, 0x0F, 0x12, 0x0D, 0x00, 0x0F, 0x08, 0x0A, 0x0B,
    0x0F, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0F, 0x04, 0x01, 0x04, 0x12, 0x04, 0x01,
    0x04, 0x0D, 0x04, 0x0F, 0x04, 0x12, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0B, 0x0A,
    0x08, 0x06, 0x08, 0x04, 0x01, 0x04, 0x0F, 0x04, 0x01, 0x04, 0x0D, 0x04, 0x01, 0x04, 0x0B,
    0x00
};

uint8_t WIN_MUSIC[] = {
    0x2C, 0x07, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x10, 0x00,
    0x11, 0x00, 0x00, 0x10, 0x00, 0x00, 0x11, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25,
};

uint8_t LOSE_MUSIC[] = {
    0x2F, 0x0D, 0x00, 0x0B, 0x00, 0x09, 0x00, 0x08, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x04,
    0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25
};

uint8_t IN_GAME_MUSIC1[] = {
    0x30, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26,
    0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x04, 0x03, 0x04, 0x26, 0x09, 0x26, 0x08,
    0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26,
    0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0F, 0x26, 0x0B, 0x26, 0x09, 0x08, 0x09, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x14, 0x00, 0x15, 0x00, 0x0B, 0x09, 0x08, 0x00,
    0x12, 0x00, 0x14, 0x00, 0x09, 0x08, 0x06, 0x26, 0x12, 0x00, 0x14, 0x00, 0x02, 0x04, 0x08,
    0x09, 0x08, 0x06, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x14, 0x00, 0x15, 0x00, 0x0B, 0x09,
    0x08, 0x00, 0x12, 0x00, 0x14, 0x00, 0x09, 0x08, 0x06, 0x00, 0x12, 0x00, 0x14, 0x00, 0x02,
    0x06, 0x0D, 0x00, 0x0B, 0x09, 0x0B, 0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06, 0x08, 0x00,
    0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09,
    0x26, 0x04, 0x03, 0x04, 0x26, 0x09, 0x26, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06,
    0x08, 0x00, 0x06, 0x04, 0x06, 0x26, 0x0B, 0x26, 0x09, 0x26, 0x06, 0x04, 0x06, 0x26, 0x0F,
    0x26, 0x0B, 0x26, 0x09, 0x08, 0x09, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x26, 0x08, 0x00,
    0x13, 0x00, 0x14, 0x00, 0x16, 0x14, 0x13, 0x00, 0x05, 0x00, 0x07, 0x00, 0x14, 0x13, 0x11,
    0x00, 0x05, 0x00, 0x07, 0x00, 0x01, 0x03, 0x07, 0x08, 0x07, 0x05, 0x07, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x13, 0x00, 0x14, 0x00, 0x0A, 0x08, 0x07, 0x00, 0x11, 0x00, 0x13, 0x00, 0x08,
    0x07, 0x05, 0x00, 0x11, 0x00, 0x13, 0x00, 0x03, 0x04, 0x09, 0x00, 0x06, 0x00, 0x08, 0x00,
    0x00
};

uint8_t IN_GAME_MUSIC2[] = {
    0x2E, 0x03, 0x00, 0x26, 0x03, 0x0F, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0F, 0x00, 0x01, 0x0D,
    0x02, 0x0E, 0x03, 0x00, 0x26, 0x03, 0x0F, 0x00, 0x00, 0x03, 0x00, 0x00, 0x0F, 0x00, 0x01,
    0x0D, 0x02, 0x0E, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12,
    0x00, 0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x14,
    0x00, 0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12,
    0x00, 0x0F, 0x00, 0x0D, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x26, 0x00,
    0x00, 0x2E, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12, 0x00,
    0x0F, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x14, 0x00,
    0x0F, 0x26, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x26, 0x00, 0x0F, 0x00, 0x12, 0x00,
    0x0F, 0x00, 0x0D, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x0D,
    0x00, 0x0C, 0x26, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00, 0x00, 0x00, 0x26,
    0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x11, 0x30, 0x00, 0x00, 0x00,
    0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x13, 0x00, 0x11, 0x00, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F, 0x30, 0x00,
    0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x11, 0x30,
    0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x2E, 0x0C, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x13,
    0x00, 0x00, 0x00, 0x14, 0x00, 0x13, 0x00, 0x11, 0x00, 0x0F, 0x00, 0x0E, 0x00, 0x0D, 0x2E,
    0x00
};

uint8_t IN_GAME_MUSIC3[] = {
    0x2C, 0x02, 0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x02, 0x00,
    0x09, 0x00, 0x0C, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x09, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0E,
    0x00, 0x09, 0x00, 0x0E, 0x00, 0x09, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x09, 0x00,
    0x0C, 0x00, 0x09, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00, 0x00, 0x00,
    0x09, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x00,
    0x00, 0x09, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00, 0x09,
    0x00, 0x26, 0x00, 0x09, 0x00, 0x26, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x10, 0x00,
    0x11, 0x00, 0x10, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x0C, 0x00,
    0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x00, 0x0C,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x2F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x05,
    0x00, 0x07, 0x00, 0x09, 0x00, 0x07, 0x00, 0x0B, 0x00, 0x2F, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x2C, 0x0E, 0x00, 0x0C, 0x00, 0x26, 0x00, 0x0C, 0x00, 0x26, 0x00, 0x0E, 0x00, 0x26, 0x00,
    0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x0B, 0x00, 0x10, 0x00, 0x26, 0x00, 0x10,
    0x00, 0x12, 0x00, 0x26, 0x00, 0x17, 0x00, 0x26, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
#endif

char convertToPETSCII(char value)
{
    return value >= 96 ? (value - 96) : value;
}

void writeToScreenMemory(uint16_t address, uint8_t value, uint8_t color, uint8_t yOffset)
{
    SCREEN_MEMORY[address] = value;
#ifdef PLATFORM_COLOR_SUPPORT
    platform->writeToScreenMemory(address, value, color, yOffset);
#else
    platform->writeToScreenMemory(address, value);
#endif
}

// NOTES ABOUT UNIT TYPES
// ----------------------
// 000=no unit (does not exist)
// 001=player unit
// 002=hoverbot left-to-right
// 003=hoverbot up-down
// 004=hoverbot attack mode
// 005=hoverbot chase player
// 006=
// 007=transporter
// 008=
// 009=evilbot chase player
// 010=door
// 011=small explosion
// 012=pistol fire up
// 013=pistol fire down
// 014=pistol fire left
// 015=pistol fire right
// 016=trash compactor
// 017=
// 018=
// 019=
// 020=

// NOTES ABOUT UNIT NUMBERING SCHEME
// ---------------------------------
// 0 = player unit
// 1-27 = enemy robots    (max 28 units)
// 28-31 = weapons fire
// 32-47 = doors and other units that don't have sprites (max 16 units)
// 48-63 = hidden objects to be found (max 16 units)

// NOTES ABOUT DOORS.
// -------------------
// A-0=horitzonal 1=vertical
// B-0=opening-A 1=opening-B 2=OPEN / 3=closing-A 4=closing-B 5-CLOSED
// C-0=unlocked / 1=locked spade 2=locked heart 3=locked star
// D-0=automatic / 0=manual

// HIDDEN OBJECTS
// --------------
// UNIT_TYPE:128=key UNIT_A: 0=SPADE 1=HEART 2=STAR
// UNIT_TYPE:129=time bomb
// UNIT_TYPE:130=EMP
// UNIT_TYPE:131=pistol
// UNIT_TYPE:132=charged plasma gun
// UNIT_TYPE:133=medkit
// UNIT_TYPE:134=magnet

// NOTES ABOUT TRANSPORTER
// ----------------------
// UNIT_A: 0=always active    1=only active when all robots are dead
// UNIT_B:    0=completes level 1=send to coordinates
// UNIT_C:    X-coordinate
// UNIT_D:    Y-coordinate

// Sound Effects
// ----------------------
// 0 explosion
// 1 small explosion
// 2 medkit
// 3 emp
// 4 haywire
// 5 evilbot
// 6 move
// 7 electric shock
// 8 plasma gun
// 9 fire pistol
// 10 item found
// 11 error
// 12 change weapons
// 13 change items
// 14 door
// 15 menu beep
// 16 walk
// 17 sfx (short beep)
// 18 sfx
