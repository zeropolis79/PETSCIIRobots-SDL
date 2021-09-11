/**
 * PETSCII Robots (Portable version)
 * by David Murray 2020
 * dfwgreencars@gmail.com
 * ported by Vesa Halttunen 2021
 * vesuri@jormas.com
 */

#ifdef AMIGA
#include "PlatformAmiga.h"
#else
#include "PlatformSDL.h"
#endif
#include <string.h>
#include "petrobots.h"

uint8_t DESTRUCT_PATH[256]; // Destruct path array (256 bytes)
uint8_t TILE_ATTRIB[256];   // Tile attrib array (256 bytes)
uint8_t TILE_DATA_TL[256];  // Tile character top-left (256 bytes)
uint8_t TILE_DATA_TM[256];  // Tile character top-middle (256 bytes)
uint8_t TILE_DATA_TR[256];  // Tile character top-right (256 bytes)
uint8_t TILE_DATA_ML[256];  // Tile character middle-left (256 bytes)
uint8_t TILE_DATA_MM[256];  // Tile character middle-middle (256 bytes)
uint8_t TILE_DATA_MR[256];  // Tile character middle-right (256 bytes)
uint8_t TILE_DATA_BL[256];  // Tile character bottom-left (256 bytes)
uint8_t TILE_DATA_BM[256];  // Tile character bottom-middle (256 bytes)
uint8_t TILE_DATA_BR[256];  // Tile character bottom-right (256 bytes)

// These arrays can go anywhere in RAM
uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
uint8_t MAP_PRECALC[77];    // Stores pre-calculated objects for map window (77 bytes)

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
uint8_t KEY_CONFIG[13];
uint8_t* KEY_MOVE_UP = KEY_CONFIG;
uint8_t* KEY_MOVE_DOWN = KEY_CONFIG + 1;
uint8_t* KEY_MOVE_LEFT = KEY_CONFIG + 2;
uint8_t* KEY_MOVE_RIGHT = KEY_CONFIG + 3;
uint8_t* KEY_FIRE_UP = KEY_CONFIG + 4;
uint8_t* KEY_FIRE_DOWN = KEY_CONFIG + 5;
uint8_t* KEY_FIRE_LEFT = KEY_CONFIG + 6;
uint8_t* KEY_FIRE_RIGHT = KEY_CONFIG + 7;
uint8_t* KEY_CYCLE_WEAPONS = KEY_CONFIG + 8;
uint8_t* KEY_CYCLE_ITEMS = KEY_CONFIG + 9;
uint8_t* KEY_USE = KEY_CONFIG + 10;
uint8_t* KEY_SEARCH = KEY_CONFIG + 11;
uint8_t* KEY_MOVE = KEY_CONFIG + 12;

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
uint8_t UNIT_TYPE[64];  // Unit type 0=none (64 bytes)	
uint8_t UNIT_LOC_X[64]; // Unit X location (64 bytes)
uint8_t UNIT_LOC_Y[64]; // Unit X location (64 bytes)
uint8_t UNIT_A[64];
uint8_t UNIT_B[64];
uint8_t UNIT_C[64];
uint8_t UNIT_D[64];
uint8_t UNIT_HEALTH[64];    // Unit health (0 to 11) (64 bytes)
uint8_t MAP_UNUSED[256];
uint8_t MAP[8 * 1024];      // Location of MAP (8K)
// END OF MAP FILE

uint8_t TILE;           // The tile number to be plotted
uint8_t TEMP_X;         // Temporarily used for loops
uint8_t TEMP_Y;         // Temporarily used for loops
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
uint8_t PRECALC_COUNT;  // part of screen draw routine
uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

void (*CINV)(void);     // $90-$91 Vector: Hardware Interrupt
uint8_t LSTX;           // $97 Current Key Pressed: 255 = No Key
uint8_t* MAP_SOURCE;    // $FD
uint8_t SCREEN_MEMORY[40 * 25]; // $8000

int main(int argc, char *argv[])
{
#ifdef AMIGA
    PlatformAmiga platformAmiga;
#else
    PlatformSDL platformSDL;
#endif

    if (!platform) {
        return 1;
    }

    strncpy(MAPNAME, "level-a", sizeof(MAPNAME));
    strncpy(LOAD_MSG2, "loading map:", sizeof(LOAD_MSG2));

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
    RESET_KEYS_AMMO();
    DISPLAY_GAME_SCREEN();
    DISPLAY_LOAD_MESSAGE2();
    MAP_LOAD_ROUTINE();
    SET_DIFF_LEVEL();
    ANIMATE_PLAYER();
    CACULATE_AND_REDRAW();
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
char MAPNAME[8];
const char* SNDNAME = "pdrv-pet";
const char* LOADMSG1 = "loading tiles...\x0d";
const char* LOADMSG2 = "\x93loading map...\x0d";
uint8_t KEYS = 0; // bit0=spade bit2=heart bit3=star
uint16_t AMMO_PISTOL = 0; // how much ammo for the pistol
uint8_t AMMO_PLASMA = 0; // how many shots of the plasmagun
uint8_t INV_BOMBS = 0; // How many bombs do we have
uint8_t INV_EMP = 0; // How many EMPs do we have
uint8_t INV_MEDKIT = 0; // How many medkits do we have?
uint8_t INV_MAGNET = 0; // How many magnets do we have?
uint8_t SELECTED_WEAPON = 0; // 0=none 1=pistol 2=plasmagun
uint8_t SELECTED_ITEM = 0; // 0=none 1=bomb 2=emp 3=medkit 4=magnet
uint8_t SELECT_TIMEOUT = 0; // can only change weapons once it hits zero
uint8_t ANIMATE	 = 1; // 0=DISABLED 1=ENABLED
uint8_t BIG_EXP_ACT = 0; // 0=No explosion active 1=big explosion active
uint8_t MAGNET_ACT = 0; // 0=no magnet active 1=magnet active
uint8_t PLASMA_ACT = 0; // 0=No plasma fire active 1=plasma fire active
uint8_t RANDOM = 0; // used for random number generation
uint8_t BORDER = 0; // Used for border flash timing
uint8_t SCREEN_SHAKE = 0; // 1=shake 0=no shake
uint8_t CONTROL = 0; // 0=keyboard 1=custom keys 2=snes
#ifdef AMIGA
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
                        "10-death tower  ";
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
    for (Y = 0; Y != 12; Y++) {
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
    MUSIC_ROUTINE();
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
    }
    platform->renderFrame();
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
                if (A == 0x1D || A == *KEY_MOVE_RIGHT) { // CURSOR RIGHT
                    UNIT = 0;
                    MOVE_TYPE = 1; // %00000001
                    REQUEST_WALK_RIGHT();
                    AFTER_MOVE();   // TODO jmp
                } else if (A == 0x9D || A == *KEY_MOVE_LEFT) { // CURSOR LEFT
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_LEFT();
                    AFTER_MOVE();   // TODO jmp
                } else if (A == 0x11 || A == *KEY_MOVE_DOWN) { // CURSOR DOWN
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_DOWN();
                    AFTER_MOVE();   // TODO jmp
                } else if (A == 0x91 || A == *KEY_MOVE_UP) { // CURSOR UP
                    UNIT = 0;
                    MOVE_TYPE = 1;  // %00000001
                    REQUEST_WALK_UP();
                    AFTER_MOVE();   // TODO jmp
                } else if (A == *KEY_CYCLE_WEAPONS) {
                    CYCLE_WEAPON();
                    CLEAR_KEY_BUFFER();
                } else if (A == *KEY_CYCLE_ITEMS) {
                    CYCLE_ITEM();
                    CLEAR_KEY_BUFFER();
                } else if (A == *KEY_MOVE) {
                    MOVE_OBJECT();
                    CLEAR_KEY_BUFFER();
                } else if (A == *KEY_SEARCH) {
                    SEARCH_OBJECT();
                    CLEAR_KEY_BUFFER();
                } else if (A == *KEY_USE) {
                    USE_ITEM();
                    CLEAR_KEY_BUFFER();
                } else if (A == *KEY_FIRE_UP) {
                    FIRE_UP();
                    KEYTIMER = 20;
                } else if (A == *KEY_FIRE_LEFT) {
                    FIRE_LEFT();
                    KEYTIMER = 20;
                } else if (A == *KEY_FIRE_DOWN) {
                    FIRE_DOWN();
                    KEYTIMER = 20;
                } else if (A == *KEY_FIRE_RIGHT) {
                    FIRE_RIGHT();
                    KEYTIMER = 20;
                } else if (A == 0x03) { // RUN/STOP
                    done = PAUSE_GAME();
                } else if (A == 195) { // SHIFT-C
                    CHEATER();
                } else if (A == 205) { // SHIFT-M
                    TOGGLE_MUSIC();
                    CLEAR_KEY_BUFFER();
                }
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
        platform->stopNote(); // turn off sound
    } else {
        PRINT_INFO(MSG_MUSICON);
        MUSIC_ON = 1;
        START_IN_GAME_MUSIC();
    }
}

void START_IN_GAME_MUSIC()
{
    MUSIC_ON = 1;
    if (SOUND_EFFECT == 0xFF) { // FF=NO sound effect in progress
        DATA_LINE = 0;
        CUR_PATTERN = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    } else {
        // apparently a sound-effect is active, so we do things differently.
        DATA_LINE_TEMP = 0;
        PATTERN_TEMP = IN_GAME_MUSIC1 + (LEVEL_MUSIC[SELECTED_MAP] << 8);
    }
}

uint8_t LEVEL_MUSIC[] = { 0,1,2,0,1,2,0,1,2,0 };

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
    for (BGTIMER1 = 0; BGTIMER1 != 1;); // to prevent double-tap of run/stop
    CLEAR_KEY_BUFFER();
    while (true) {
        switch (platform->getin()) {
        case 03: // RUN/STOP
        case 78: // N-KEY
            SCROLL_INFO();
            SCROLL_INFO();
            SCROLL_INFO();
            CLEAR_KEY_BUFFER();
            CLOCK_ACTIVE = 1;
            PLAY_SOUND(15);
            return false;
        case 89: // Y-KEY
            UNIT_TYPE[0] = 0; // make player dead
            PLAY_SOUND(15);
            GOM4();
            return true;
        default:
            break;
        }
    }
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
    if (BOMB_MAGNET_COMMON1() == 0) {
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
    if (BOMB_MAGNET_COMMON1() == 0) {
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
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
    MAP_X = CURSOR_X + MAP_WINDOW_X;
    MOVTEMP_UX = MAP_X;
    MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
    MOVTEMP_UY = MAP_Y;
    GET_TILE_FROM_MAP();
    return TILE_ATTRIB[TILE] & 1; // is that spot available for something to move onto it?
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
            UNIT_LOC_X[X] < (MAP_WINDOW_X + 10) &&  // NOW CHECK VERTICAL
            UNIT_LOC_Y[X] >= MAP_WINDOW_Y &&
            UNIT_LOC_Y[X] < (MAP_WINDOW_Y + 6)) {
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
        CACULATE_AND_REDRAW();
    }
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
        CURSOR_ON = 0;
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
        CURSOR_ON = 0;
        DRAW_MAP_WINDOW(); // ERASE THE CURSOR
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
            AMMO_PISTOL += TEMP_B;
            if (AMMO_PISTOL > 255) { // If we rolled over past 255
                AMMO_PISTOL = 255; // set it to 255.
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
    CURSOR_ON = 1;
    REVERSE_TILE();
    // First ask user which object to move
    while (!platform->quit) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
            CURSOR_ON = 0;
            return;
        }
        if (CONTROL != 2) {
            uint8_t A = platform->getin();
            if (A == 0x1D || A == *KEY_MOVE_RIGHT) { // CURSOR RIGHT
                CURSOR_X++;
                return;
            } else if (A == 0x9D || A == *KEY_MOVE_LEFT) { // CURSOR LEFT
                CURSOR_X--;
                return;
            } else if (A == 0x11 || A == *KEY_MOVE_DOWN) { // CURSOR DOWN
                CURSOR_Y++;
                return;
            } else if (A == 0x91 || A == *KEY_MOVE_UP) { // CURSOR UP
                CURSOR_Y--;
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
    CURSOR_ON = 0;
    DRAW_MAP_WINDOW(); // ERASE THE CURSOR
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
    CURSOR_ON = 1;
    REVERSE_TILE();
    // NOW ASK THE USER WHICH DIRECTION TO MOVE IT TO
    while (true) {
        PET_SCREEN_SHAKE();
        BACKGROUND_TASKS();
        if (UNIT_TYPE[0] == 0) { // Did player die wile moving something?
            CURSOR_ON = 0;
            return;
        }
        if (CONTROL != 2) { // which controller are we using?
            // keyboard control
            uint8_t A = platform->getin();
            if (A == 0) {
                continue;
            } else if (A == 0x1D || A == *KEY_MOVE_RIGHT) { // CURSOR RIGHT
                CURSOR_X++;
            } else if (A == 0x9D || A == *KEY_MOVE_LEFT) { // CURSOR LEFT
                CURSOR_X--;
            } else if (A == 0x11 || A == *KEY_MOVE_DOWN) { // CURSOR DOWN
                CURSOR_Y++;
            } else if (A == 0x91 || A == *KEY_MOVE_UP) { // CURSOR UP
                CURSOR_Y--;
            }
        } else {
            // SNES controls
            // TODO
        }
        // NOW TEST TO SEE IF THAT SPOT IS OPEN
        CURSOR_ON = 0;
        DRAW_MAP_WINDOW(); // ERASE THE CURSOR
        MAP_X = CURSOR_X + MAP_WINDOW_X;
        MOVTEMP_UX = MAP_X;
        MAP_Y = CURSOR_Y + MAP_WINDOW_Y;
        MOVTEMP_UY = MAP_Y;
        GET_TILE_FROM_MAP();
        if (TILE_ATTRIB[TILE] & 0x20) { // %00100000 is that spot available for something to move onto it?
            // Now scan for any units at that location:
            CHECK_FOR_UNIT();
            if (UNIT_FIND != 255) { // 255 means no unit found.
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
        }
        // continue search
    }
}

uint8_t PRECALC_ROWS[] = { 0,11,22,33,44,55,66 };

// This routine is where the MAP is displayed on the screen
// This is a temporary routine, taken from the map editor.
void DRAW_MAP_WINDOW()
{
    MAP_PRE_CALCULATE();
    REDRAW_WINDOW = 0;
    PRECALC_COUNT = 0;
    for (TEMP_Y = 0; TEMP_Y != 7; TEMP_Y++) {
        for (TEMP_X = 0; TEMP_X != 11; TEMP_X++) {
            // FIRST CALCULATE WHERE THE BYTE IS STORED IN THE MAP
            MAP_SOURCE = MAP + (((MAP_WINDOW_Y + TEMP_Y) << 7) + TEMP_X + MAP_WINDOW_X);
            TILE = MAP_SOURCE[0];
            // NOW FIGURE OUT WHERE TO PLACE IT ON SCREEN.
            PLOT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
            // now check for sprites in this location
            if (MAP_PRECALC[PRECALC_COUNT] != 0) {
                TILE = MAP_PRECALC[PRECALC_COUNT];
                PLOT_TRANSPARENT_TILE(MAP_CHART[TEMP_Y] + TEMP_X + TEMP_X + TEMP_X);
            }
            PRECALC_COUNT++;
        }
        // CHECK FOR CURSOR
        if (CURSOR_ON == 1) { // Is cursor even on?
            if (TEMP_Y == CURSOR_Y) { // is cursor on the same row that were drawing?
                REVERSE_TILE();
            }
        }
    }
}

// This routine plots a 3x3 tile from the tile database anywhere
// on screen.  But first you must define the tile number in the
// TILE variable, as well as the starting screen address must
// be defined in $FB.
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

// This routine plots a transparent tile from the tile database
// anywhere on screen.  But first you must define the tile number
// in the TILE variable, as well as the starting screen address must
// be defined in $FB.  Also, this routine is slower than the usual
// tile routine, so is only used for sprites.  The ":" character ($3A)
// is not drawn.
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
        UNIT_LOC_X[UNIT] < (MAP_WINDOW_X + 10) &&
        UNIT_LOC_Y[UNIT] >= MAP_WINDOW_Y && // NOW CHECK VERTICAL
        UNIT_LOC_Y[UNIT] < (MAP_WINDOW_Y + 6)) {
        REDRAW_WINDOW = 1;
    }
}

void DECWRITE(uint16_t destination)
{
    for (int X = 2; X >= 0; X--) {
        writeToScreenMemory(destination + X, 0x30 + (DECNUM % 10));
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
    platform->load(TILENAME, DESTRUCT_PATH, 2816);
}

// The following routine loads the map from disk
void MAP_LOAD_ROUTINE()
{
    platform->load(MAPNAME, UNIT_TYPE, 8960);
}

void DISPLAY_GAME_SCREEN()
{
    DECOMPRESS_SCREEN(SCR_TEXT);
}

void DISPLAY_INTRO_SCREEN()
{
    DECOMPRESS_SCREEN(INTRO_TEXT);
}

void DISPLAY_ENDGAME_SCREEN()
{
    int X;
    DECOMPRESS_SCREEN(SCR_ENDGAME);
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

void DECOMPRESS_SCREEN(uint8_t* source)
{
    uint16_t destination = 0;

    while (true) {
        if (*source != 96) { // REPEAT FLAG
            writeToScreenMemory(destination, *source);
        } else {
            // REPEAT CODE
            source++;
            RPT = *source;
            source++;
            for (int X = *source; X >= 0; X--) {
                writeToScreenMemory(destination++, RPT);
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
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, TBOMB1A[Y]);
        writeToScreenMemory(0x18A + Y, TBOMB1B[Y]);
        writeToScreenMemory(0x1B2 + Y, TBOMB1C[Y]);
        writeToScreenMemory(0x1DA + Y, TBOMB1D[Y]);
    }
    DECNUM = INV_BOMBS;
    DECWRITE(0x205);
}

void DISPLAY_EMP()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, EMP1A[Y]);
        writeToScreenMemory(0x18A + Y, EMP1B[Y]);
        writeToScreenMemory(0x1B2 + Y, EMP1C[Y]);
        writeToScreenMemory(0x1DA + Y, EMP1D[Y]);
    }
    DECNUM = INV_EMP;
    DECWRITE(0x205);
}

void DISPLAY_MEDKIT()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, MED1A[Y]);
        writeToScreenMemory(0x18A + Y, MED1B[Y]);
        writeToScreenMemory(0x1B2 + Y, MED1C[Y]);
        writeToScreenMemory(0x1DA + Y, MED1D[Y]);
    }
    DECNUM = INV_MEDKIT;
    DECWRITE(0x205);
}

void DISPLAY_MAGNET()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, MAG1A[Y]);
        writeToScreenMemory(0x18A + Y, MAG1B[Y]);
        writeToScreenMemory(0x1B2 + Y, MAG1C[Y]);
        writeToScreenMemory(0x1DA + Y, MAG1D[Y]);
    }
    DECNUM = INV_MAGNET;
    DECWRITE(0x205);
}

void DISPLAY_BLANK_ITEM()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x162 + Y, 32);
        writeToScreenMemory(0x18A + Y, 32);
        writeToScreenMemory(0x1B2 + Y, 32);
        writeToScreenMemory(0x1DA + Y, 32);
        writeToScreenMemory(0x202 + Y, 32);
    }
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
    if (SELECTED_WEAPON == 2) {
        SELECTED_WEAPON = 0;
    }
    DISPLAY_WEAPON();
}

void DISPLAY_WEAPON()
{
    while (true) {
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
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y, WEAPON1A[Y]);
        writeToScreenMemory(0x072 + Y, WEAPON1B[Y]);
        writeToScreenMemory(0x09A + Y, WEAPON1C[Y]);
        writeToScreenMemory(0x0C2 + Y, WEAPON1D[Y]);
    }
    DECNUM = AMMO_PLASMA;
    DECWRITE(0x0ED);
}

void DISPLAY_PISTOL()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y,PISTOL1A[Y]);
        writeToScreenMemory(0x072 + Y, PISTOL1B[Y]);
        writeToScreenMemory(0x09A + Y, PISTOL1C[Y]);
        writeToScreenMemory(0x0C2 + Y, PISTOL1D[Y]);
    }
    DECNUM = AMMO_PISTOL;
    DECWRITE(0x0ED);

}

void DISPLAY_BLANK_WEAPON()
{
    for (int Y = 0; Y != 6; Y++) {
        writeToScreenMemory(0x04A + Y, 32);
        writeToScreenMemory(0x072 + Y, 32);
        writeToScreenMemory(0x09A + Y, 32);
        writeToScreenMemory(0x0C2 + Y, 32);
        writeToScreenMemory(0x0EA + Y, 32);
    }
}

void DISPLAY_KEYS()
{
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
}

void GAME_OVER()
{
    // stop game clock
    CLOCK_ACTIVE = 0;
    // disable music
    MUSIC_ON = 0;
    platform->stopNote(); // turn off sound
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
    KEYTIMER = 100;
    while (KEYTIMER != 0);
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    while (platform->getin() == 0);
    GOM4();
}

void GOM4()
{
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
    MUSIC_ON = 0;
    DISPLAY_ENDGAME_SCREEN();
    DISPLAY_WIN_LOSE();
    while (platform->getin() == 0);
    platform->clearKeyBuffer(); // CLEAR KEYBOARD BUFFER
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
        PLAY_SOUND(18); // win music
    } else {
        // LOSE MESSAGE
        for (int X = 0; X != 9; X++) {
            writeToScreenMemory(0x088 + X, convertToPETSCII(LOS_MSG[X]));
        }
        PLAY_SOUND(19); // LOSE music
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
    int X;
    for (X = 0; X != 33; X++) {
        writeToScreenMemory(0x370 + X, SCREEN_MEMORY[0x398 + X]);
        writeToScreenMemory(0x398 + X, SCREEN_MEMORY[0x3C0 + X]); // BOTTOM ROW
    }
    // NOW CLEAR BOTTOM ROW
    for (X = 0; X != 33; X++) {
        writeToScreenMemory(0x3C0 + X, 32); // BOTTOM ROW
    }
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
    DISPLAY_INTRO_SCREEN();
    DISPLAY_MAP_NAME();
    CHANGE_DIFFICULTY_LEVEL();
    START_INTRO_MUSIC();
    MENUY = 0;
    REVERSE_MENU_OPTION();
    bool done = false;
    while (!done && !platform->quit) {
        uint8_t A = platform->getin();
        if (A != 0) {
            if (A == 0x11 || A == *KEY_MOVE_DOWN) { // CURSOR DOWN
                if (MENUY != 3) {
                    REVERSE_MENU_OPTION();
                    MENUY++;
                    REVERSE_MENU_OPTION();
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == 0x91 || A == *KEY_MOVE_UP) { // CURSOR UP
                if (MENUY != 0) {
                    REVERSE_MENU_OPTION();
                    MENUY--;
                    REVERSE_MENU_OPTION();
                    PLAY_SOUND(15); // menu beep
                }
            } else if (A == 32) { // SPACE
                done = EXEC_COMMAND();
            } else if (A == 13) { // RETURN
                PLAY_SOUND(15); // menu beep, SOUND PLAY
                done = EXEC_COMMAND();
            }
        }
    }
}

void START_INTRO_MUSIC()
{
    DATA_LINE = 0;
    SOUND_EFFECT = 0xFF;
    CUR_PATTERN = INTRO_MUSIC;
    MUSIC_ON = 1;
}

bool EXEC_COMMAND()
{
    if (MENUY == 0) { // START GAME
        SET_CONTROLS();
        MUSIC_ON = 0;
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
        writeToScreenMemory(0x0CC + X, convertToPETSCII(CONTROLTEXT[Y]) | 0x80);
    }
}

const char* CONTROLTEXT = "keyboard  "
                          "custom key"
                          "snes pad  ";
uint8_t CONTROLSTART[] = { 0, 10, 20 };

void CYCLE_MAP()
{
    SELECTED_MAP++;
    if (SELECTED_MAP == 10) { // Maximum number of maps
        SELECTED_MAP = 0;
    }
    DISPLAY_MAP_NAME();
}

void DISPLAY_MAP_NAME()
{
    const char* name = CALC_MAP_NAME();
    for (int Y = 0; Y != 16; Y++) {
        writeToScreenMemory(0x16A + Y, convertToPETSCII(name[Y]));
    }
    // now set the mapname for the filesystem load
    MAPNAME[6] = SELECTED_MAP + 65;
}

const char* CALC_MAP_NAME()
{
    // FIND MAP NAME
    return MAP_NAMES + (SELECTED_MAP << 4); // multiply by 16 by shifting 4 times to left.
}

void REVERSE_MENU_OPTION()
{
    for (int Y = 0; Y != 10; Y++) {
        writeToScreenMemory(MENU_CHART_L[MENUY] + Y, SCREEN_MEMORY[MENU_CHART_L[MENUY] + Y] ^ 0x80);
    }
}

uint8_t MENUY = 0; // CURRENT MENU SELECTION
uint8_t MENU_CHART_L[] = {  0x54, 0x7C, 0xA4, 0xCC };

void CHANGE_DIFFICULTY_LEVEL()
{
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
    // Now get user input
    if (CONTROL != 2) {
        // KEYBOARD INPUT
        while (!platform->quit) {
            uint8_t A = platform->getin();
            if (A != 0) {
                if (A == 0x9D || A == *KEY_MOVE_LEFT) { // CURSOR LEFT
                    ELEVATOR_DEC();
                } else if (A == 0x1D || A == *KEY_MOVE_RIGHT) { // CURSOR RIGHT
                    ELEVATOR_INC();
                } else if (A == 0x11 || A == *KEY_MOVE_DOWN) { // CURSOR DOWN
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
    writeToScreenMemory(0x3C5 + ELEVATOR_CURRENT_FLOOR, SCREEN_MEMORY[0x3C5 + ELEVATOR_CURRENT_FLOOR]);
}

void ELEVATOR_INC()
{
    if (ELEVATOR_CURRENT_FLOOR != ELEVATOR_MAX_FLOOR) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR++;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
    }
}

void ELEVATOR_DEC()
{
    if (ELEVATOR_CURRENT_FLOOR != 1) {
        ELEVATOR_INVERT();
        ELEVATOR_CURRENT_FLOOR--;
        ELEVATOR_INVERT();
        ELEVATOR_FIND_XY();
    }
}

void ELEVATOR_FIND_XY()
{
    for (int X = 32; X != 48; X++) { // start of doors
        if (UNIT_TYPE[X] == 19) { // elevator
            if (UNIT_C[X] == ELEVATOR_CURRENT_FLOOR) {
                UNIT_LOC_X[0] = UNIT_LOC_X[X]; // player location = new elevator location
                MAP_WINDOW_X = UNIT_LOC_X[0] - 5;
                UNIT_LOC_Y[0] = UNIT_LOC_Y[X] - 1; // player location = new elevator location
                MAP_WINDOW_Y = UNIT_LOC_Y[0] - 4;
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
        for (int Y = 0; Y != 13; Y++) {
            KEY_MOVE_UP[Y] = STANDARD_CONTROLS[Y];
        }
    }
}

uint8_t STANDARD_CONTROLS[] = {
    73, // MOVE UP orig: 56 (8)
    75, // MOVE DOWN orig: 50 (2)
    74, // MOVE LEFT orig: 52 (4)
    76, // MOVE RIGHT orig: 54 (6)
    87, // FIRE UP
    83, // FIRE DOWN
    65, // FIRE LEFT
    68, // FIRE RIGHT
    44, // CYCLE WEAPONS orig: 60 (<)
    46, // CYCLE ITEMS orig: 62 (>)
    32, // USE ITEM
    90, // SEARCH OBEJCT
    77  // MOVE OBJECT
};

void SET_CUSTOM_KEYS()
{
    if (KEYS_DEFINED != 0) {
        return;
    }
    DECOMPRESS_SCREEN(SCR_CUSTOM_KEYS);
    uint16_t destination = 0x151;
    for (TEMP_A = 0; TEMP_A != 13;) {
        uint8_t A = platform->getin();
        if (A != 0) {
            KEY_MOVE_UP[TEMP_A] = A;
            DECNUM = A;
            DECWRITE(destination);
            destination += 40;
            TEMP_A++;
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
}

// So, it doesn't really flash the PET border, instead it
// flashes the health screen.
void PET_BORDER_FLASH()
{
    if (BORDER != 0) {
        // border flash should be active
        if (FLASH_STATE != 1) { // Is it already flashing?
            // copy flash message to screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(0x2F2 + X, OUCH1[X]);
                writeToScreenMemory(0x31A + X, OUCH2[X]);
                writeToScreenMemory(0x342 + X, OUCH3[X]);
            }
            FLASH_STATE = 1;
        }
    } else {
        if (FLASH_STATE != 0) {
            // Remove message from screen
            for (int X = 0; X != 6; X++) {
                writeToScreenMemory(0x2F2 + X, 0);
                writeToScreenMemory(0x31A + X, 0);
                writeToScreenMemory(0x342 + X, 0);
            }
            FLASH_STATE = 0;
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
    if (UNIT_TILE[0] == 97) {
        UNIT_TILE[0] = 96;
    } else {
        UNIT_TILE[0] = 97;
    }
}

void PLAY_SOUND(int sound)
{
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
}

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

void STOP_SONG()
{
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
}

void BACKGROUND_TASKS()
{
    if (REDRAW_WINDOW == 1 && BGTIMER1 == 1) {
        REDRAW_WINDOW = 0;
        DRAW_MAP_WINDOW();
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
    // TODO
}

void TRANS_PLAYER_PRESENT()
{
    // TODO
}

void TRANS_ACTIVE()
{
    // TODO
}

void TIME_BOMB()
{
    // TODO
}

void BIG_EXP_PHASE1()
{
    // TODO
}

void BEX1_NORTH()
{
    // TODO
}

void BEX1_SOUTH()
{
    // TODO
}

void BEX1_EAST()
{
    // TODO
}

void BEX1_WEST()
{
    // TODO
}

void BEX1_NE()
{
    // TODO
}

void BEX1_NW()
{
    // TODO
}

void BEX1_SE()
{
    // TODO
}

void BEX1_SW()
{
    // TODO
}

void BEX_PART1()
{
    // TODO
}

void BEX_PART2()
{
    // TODO
}

void BEX_PART3()
{
    // TODO
}

void BIG_EXP_PHASE2()
{
    // TODO
}

void RESTORE_TILE()
{
    // TODO
}

void TRASH_COMPACTOR()
{
    // TODO
}

void DRAW_TRASH_COMPACTOR()
{
    // TODO
}

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
    // TODO
}

void CREATE_PLAYER_EXPLOSION()
{
    // TODO
}

void EVILBOT()
{
    // TODO
}

void AI_DOOR()
{
    // TODO
}

void DRAW_VERTICAL_DOOR()
{
    // TODO
}

void DRAW_HORIZONTAL_DOOR()
{
    // TODO
}

void ROBOT_ATTACK_RANGE()
{
    // TODO
}

void DOOR_CHECK_PROXIMITY()
{
    // TODO
}

void ELEVATOR()
{
    // TODO
}

void ELEV_CLOSE_FULL()
{
    // TODO
}

void ELEVATOR_PANEL()
{
    // TODO
}

void PLOT_TILE_TO_MAP()
{
    MAP[(MAP_Y << 7) + MAP_X] = TILE;
}

// This routine will return the tile for a specific X/Y
// on the map.  You must first define MAP_X and MAP-Y.
// The result is stored in TILE.
void GET_TILE_FROM_MAP()
{
    TILE = MAP[(MAP_Y << 7) + MAP_X];
}

void LEFT_RIGHT_DROID()
{
    // TODO
}

void UP_DOWN_DROID()
{
    // TODO
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
             (UNIT_LOC_X[X] >= MAP_X && UNIT_LOC_X[X] < (MAP_X + UNIT_C[X]))) && // add hidden unit width
            (UNIT_LOC_Y[X] == MAP_Y || // now compare vertical position
             (UNIT_LOC_Y[X] >= MAP_Y && UNIT_LOC_Y[X] < (MAP_Y + UNIT_D[X])))) { // add hidden unit HEIGHT
            UNIT_FIND = X;
            return;
        }
    }
    UNIT_FIND = 255; // no units found
}

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

uint8_t CINEMA_MESSAGE[] = {
    "coming soon: space balls 2 - the search for more money, "
    "attack of the paperclips: clippy's revenge, "
    "it came from planet earth, "
    "rocky 5000, all my circuits the movie, "
    "conan the librarian, and more! " 
};

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

char convertToPETSCII(char value)
{
    return value >= 96 ? (value - 96) : value;
}

void writeToScreenMemory(uint16_t address, uint8_t value)
{
    SCREEN_MEMORY[address] = value;
    platform->writeToScreenMemory(address, value);
}

/* 6502 TODO

TRANSPORTER_PAD:
    ;first determine if the player is standing here
    LDX UNIT
    LDA UNIT_LOC_X,X
    CMP UNIT_LOC_X
    BNE TRP01
    LDA UNIT_LOC_Y,X
    CMP UNIT_LOC_Y
    BNE TRP01
    JMP TRANS_PLAYER_PRESENT    
TRP01:  ;player not present
    LDA UNIT_A,X
    CMP #1
    BEQ TRP02
    JMP TRANS_ACTIVE
TRP02:  ;test if all robots are dead
    LDX #1
TRP03:  LDA UNIT_TYPE,X
    CMP #0
    BNE TRP04
    INX
    CPX #28
    BNE TRP03
    LDX UNIT
    LDA #0
    STA UNIT_A,X    ;make unit active
TRP04:  LDX UNIT
    LDA #30
    STA UNIT_TIMER_A,X
    JMP AILP        

TRANS_PLAYER_PRESENT:
    LDX UNIT
    LDA UNIT_A,X
    CMP #0  ;unit active
    BEQ TRPL1
    LDA #<MSG_TRANS1
    STA $FB
    LDA #>MSG_TRANS1
    STA $FC
    JSR PRINT_INFO
    LDA #11     ;error-SOUND
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA #100
    STA UNIT_TIMER_A,X
    JMP AILP
TRPL1:  ;start transport process
    LDA #23 ;Convert to different AI
    STA UNIT_TYPE,X
    LDA #5
    STA UNIT_TIMER_A,X
    LDA #0
    STA UNIT_TIMER_B,X
    JMP AILP

TRANS_ACTIVE:
    LDA UNIT_TIMER_B,X
    CMP #1
    BEQ TRAC1
    LDA #1
    STA UNIT_TIMER_B,X
    LDA #30
    STA TILE
    JMP TRAC2
TRAC1:  LDA #0
    STA UNIT_TIMER_B,X
    LDA #31
    STA TILE
TRAC2:  LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR PLOT_TILE_TO_MAP
    JSR CHECK_FOR_WINDOW_REDRAW
    LDA #30
    STA UNIT_TIMER_A,X
    JMP AILP

TIME_BOMB:
    LDX UNIT
    LDA UNIT_A,X
    CMP #0
    BNE TB01
    JMP BIG_EXP_PHASE1
TB01:   CMP #1
    BNE TB02
    JMP BIG_EXP_PHASE2
TB02:   JMP AILP

;This is the large explosion used by the time-bomb
;and plasma gun, and maybe others.  This is the first
;phase of the explosion, which stores the tiles to
;a buffer and then changes each tile to an explosion.
BIG_EXP_PHASE1:
    LDA BIG_EXP_ACT
    CMP #0  ;Check that no other explosion active.
    BEQ BEX001
    LDX UNIT
    LDA #10
    STA UNIT_TIMER_A,X
    JMP AILP    ;wait for existing explosion to finish.
BEX001:
    LDA #1      ;Set flag so no other explosions
    STA BIG_EXP_ACT ;can begin until this one ends.
    STA SCREEN_SHAKE
    LDA #0  ;explosion-sound
    JSR PLAY_SOUND  ;SOUND PLAY
    JSR BEX_PART1   ;check center piece for unit
    JSR BEXCEN      ;check center piece for unit
    JSR BEX1_NORTH
    JSR BEX1_SOUTH
    JSR BEX1_EAST
    JSR BEX1_WEST
    JSR BEX1_NE
    JSR BEX1_NW
    JSR BEX1_SE
    JSR BEX1_SW
    LDX UNIT
    LDA #246        ;explosion tile
    STA UNIT_TILE,X
    LDA #1      ;move to next phase of explosion.
    STA UNIT_A,X
    LDA #12
    STA UNIT_TIMER_A,X
    LDA #1
    STA REDRAW_WINDOW
    JMP AILP

;There are 8 separate subroutines for the large explosion
;with each one handling a specific outward direction of motion.
;The "unit" itself changes tiles to an explosion, so we don't
;need to mess with the center tile.
BEX1_NORTH:
    JSR BEX_PART1
    ;first tile
    DEC MAP_Y
    JSR BEX_PART2
    BEQ BEX1N1
    RTS
BEX1N1: LDA TILE
    STA EXP_BUFFER+0
    JSR BEX_PART3
    ;second tile
    DEC MAP_Y
JSR BEX_PART2
    BEQ BEX1N2
    RTS
BEX1N2: LDA TILE
    STA EXP_BUFFER+1
    JSR BEX_PART3
    RTS

BEX1_SOUTH:
    JSR BEX_PART1
    ;first tile
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1S1
    RTS
BEX1S1: LDA TILE
    STA EXP_BUFFER+2
    JSR BEX_PART3
    ;second tile
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1S2
    RTS
BEX1S2: LDA TILE
    STA EXP_BUFFER+3
    JSR BEX_PART3
    RTS

BEX1_EAST:
    JSR BEX_PART1
    ;first tile
    INC MAP_X
    JSR BEX_PART2
    BEQ BEX1E1
    RTS
BEX1E1: LDA TILE
    STA EXP_BUFFER+4
    JSR BEX_PART3
    ;second tile
    INC MAP_X
    JSR BEX_PART2
    BEQ BEX1E2
    RTS
BEX1E2: LDA TILE
    STA EXP_BUFFER+5
    JSR BEX_PART3
    RTS

BEX1_WEST:
    JSR BEX_PART1
    ;first tile
    DEC MAP_X
    JSR BEX_PART2
    BEQ BEX1W1
    RTS
BEX1W1: LDA TILE
    STA EXP_BUFFER+6
    JSR BEX_PART3
    ;second tile
    DEC MAP_X
    JSR BEX_PART2
    BEQ BEX1W2
    RTS
BEX1W2: LDA TILE
    STA EXP_BUFFER+7
    JSR BEX_PART3
    RTS

BEX1_NE:
    JSR BEX_PART1
    ;first tile
    INC MAP_X
    DEC MAP_Y
    JSR BEX_PART2
    BEQ BEX1NE1
    RTS
BEX1NE1:LDA TILE
    STA EXP_BUFFER+8
    JSR BEX_PART3
    ;second tile
    INC MAP_X
    DEC MAP_Y
    JSR BEX_PART2
    BEQ BEX1NE2
    RTS
BEX1NE2:LDA TILE
    STA EXP_BUFFER+9
    JSR BEX_PART3
    RTS

BEX1_NW:
    JSR BEX_PART1
    ;first tile
    DEC MAP_X
    DEC MAP_Y
    JSR BEX_PART2
    BEQ BEX1NW1
    RTS
BEX1NW1:LDA TILE
    STA EXP_BUFFER+10
    JSR BEX_PART3
    ;second tile
    DEC MAP_X
    DEC MAP_Y
    JSR BEX_PART2
    BEQ BEX1NW2
    RTS
BEX1NW2:LDA TILE
    STA EXP_BUFFER+11
    JSR BEX_PART3
    RTS

BEX1_SE:
    JSR BEX_PART1
    ;first tile
    INC MAP_X
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1SE1
    RTS
BEX1SE1:LDA TILE
    STA EXP_BUFFER+12
    JSR BEX_PART3
    ;second tile
    INC MAP_X
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1SE2
    RTS
BEX1SE2:LDA TILE
    STA EXP_BUFFER+13
    JSR BEX_PART3
    RTS

BEX1_SW:
    JSR BEX_PART1
    ;first tile
    DEC MAP_X
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1SW1
    RTS
BEX1SW1:LDA TILE
    STA EXP_BUFFER+14
    JSR BEX_PART3
    ;second tile
    DEC MAP_X
    INC MAP_Y
    JSR BEX_PART2
    BEQ BEX1SW2
    RTS
BEX1SW2:LDA TILE
    STA EXP_BUFFER+15
    JSR BEX_PART3
    RTS

BEX_PART1:
    LDX UNIT
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    RTS

BEX_PART2:
    JSR GET_TILE_FROM_MAP
    LDY TILE
    LDA TILE_ATTRIB,Y
    AND #%00010000  ;can see through tile?
    CMP #%00010000
    RTS

BEX_PART3:
    LDA #246
    LDY #0
    STA ($FD),Y
BEXCEN: JSR CHECK_FOR_UNIT
    LDA UNIT_FIND
    CMP #255
    BEQ EPT2    
    LDA #11
    STA TEMP_A
    JSR INFLICT_DAMAGE
EPT2:   RTS

BIG_EXP_PHASE2:
    ;Do the center tile first.
    JSR BEX_PART1
    JSR GET_TILE_FROM_MAP
    LDA #246
    STA ($FD),Y
    LDA TILE
    STA TEMP_A
    JSR RESTORE_TILE    
    ;tile #0 north 1
    JSR BEX_PART1
    DEC MAP_Y
    LDA EXP_BUFFER+0
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #1 north 2
    DEC MAP_Y
    LDA EXP_BUFFER+1
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #2 south 1
    JSR BEX_PART1
    INC MAP_Y
    LDA EXP_BUFFER+2
    STA TEMP_A
    JSR RESTORE_TILE    
    ;tile #3 south 2
    INC MAP_Y
    LDA EXP_BUFFER+3
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #4 east 1
    JSR BEX_PART1
    INC MAP_X
    LDA EXP_BUFFER+4
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #5 east 2
    INC MAP_X
    LDA EXP_BUFFER+5
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #6 west 1
    JSR BEX_PART1
    DEC MAP_X
    LDA EXP_BUFFER+6
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #7 west 2
    DEC MAP_X
    LDA EXP_BUFFER+7
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #8 northeast 1
    JSR BEX_PART1
    DEC MAP_Y
    INC MAP_X
    LDA EXP_BUFFER+8
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #9 northeast 2
    DEC MAP_Y
    INC MAP_X
    LDA EXP_BUFFER+9
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #10 northwest 1
    JSR BEX_PART1
    DEC MAP_Y
    DEC MAP_X
    LDA EXP_BUFFER+10
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #11 northwest 2
    DEC MAP_Y
    DEC MAP_X
    LDA EXP_BUFFER+11
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #12 southeast 1
    JSR BEX_PART1
    INC MAP_Y
    INC MAP_X
    LDA EXP_BUFFER+12
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #13 southeast 2
    INC MAP_Y
    INC MAP_X
    LDA EXP_BUFFER+13
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #14 southwest 1
    JSR BEX_PART1
    INC MAP_Y
    DEC MAP_X
    LDA EXP_BUFFER+14
    STA TEMP_A
    JSR RESTORE_TILE
    ;tile #15 southwest 2
    INC MAP_Y
    DEC MAP_X
    LDA EXP_BUFFER+15
    STA TEMP_A
    JSR RESTORE_TILE
    LDA #1
    STA REDRAW_WINDOW
    LDX UNIT
    LDA #0
    STA UNIT_TYPE,X ;Deactivate this AI
    STA BIG_EXP_ACT
    STA SCREEN_SHAKE
    JMP AILP

RESTORE_TILE:
    JSR GET_TILE_FROM_MAP
    LDA TILE
    CMP #246
    BEQ REST0
    RTS
    
REST0:  LDY TEMP_A
    CPY #131    ;Cannister tile
    BEQ REST3
    LDA TILE_ATTRIB,Y
    AND #%00001000  ;can it be destroyed?
    CMP #%00001000
    BNE REST2
    LDA DESTRUCT_PATH,Y
    LDY #0
    STA ($FD),Y
    RTS
REST2:  LDA TEMP_A
    LDY #0
    STA ($FD),Y
    RTS
REST3:  ;What to do if we encounter an explosive cannister
    LDA #135    ;Blown cannister
    LDY #0
    STA ($FD),Y
    LDX #28 ;Start of weapons units
REST4:  LDA UNIT_TYPE,X
    CMP #0
    BEQ REST5
    INX
    CPX #32
    BNE REST4
    RTS ;no slots available right now, abort.
REST5:  LDA #6  ;bomb AI
    STA UNIT_TYPE,X
    LDA #131    ;Cannister tile
    STA UNIT_TILE,X
    LDA MAP_X
    STA UNIT_LOC_X,X
    LDA MAP_Y
    STA UNIT_LOC_Y,X
    LDA #10     ;How long until exposion?
    STA UNIT_TIMER_A,X
    LDA #0
    STA UNIT_A,X
    RTS

TRASH_COMPACTOR:
    LDX UNIT    
    LDA UNIT_A,X
    CMP #0  ;OPEN
    BNE TRS01
    JMP TC_OPEN_STATE
TRS01:  CMP #1  ;MID-CLOSING STATE
    BNE TRS02
    JMP TC_MID_CLOSING
TRS02:  CMP #2  ;CLOSED STATE
    BNE TRS03
    JMP TC_CLOSED_STATE
TRS03:  CMP #3  ;MID-OPENING STATE
    BNE TRS04
    JMP TC_MID_OPENING
TRS04:  JMP AILP    ;should never get here. 
    
TC_OPEN_STATE:
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR GET_TILE_FROM_MAP
    CMP #148    ;Usual tile for trash compactor danger zone
    BNE TRS15
TRS10:  INY 
    LDA ($FD),Y
    CMP #148    ;Usual tile for trash compactor danger zone
    BNE TRS15
    LDA #20
    STA UNIT_TIMER_A,X
    ;now check for units in the compactor
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR CHECK_FOR_UNIT
    LDA UNIT_FIND
    CMP #255
    BNE TRS15
    JMP AILP    ;Nothing found, do nothing.
TRS15:  ;Object has been detected in TC, start closing.
    LDA #146
    STA TCPIECE1
    LDA #147
    STA TCPIECE2
    LDA #150
    STA TCPIECE3
    LDA #151
    STA TCPIECE4
    JSR DRAW_TRASH_COMPACTOR
    INC UNIT_A,X
    LDA #10
    STA UNIT_TIMER_A,X  
    LDA #14     ;door sound
    JSR PLAY_SOUND  ;SOUND PLAY
    JMP AILP

TC_MID_CLOSING:
    LDA #152
    STA TCPIECE1
    LDA #153
    STA TCPIECE2
    LDA #156
    STA TCPIECE3
    LDA #157
    STA TCPIECE4
    JSR DRAW_TRASH_COMPACTOR
    INC UNIT_A,X
    LDA #50
    STA UNIT_TIMER_A,X
    ;Now check for any live units in the compactor
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR CHECK_FOR_UNIT
    LDA UNIT_FIND
    CMP #255
    BNE TCMC1
    INC MAP_X   ;check second tile
    JSR CHECK_FOR_UNIT
    LDA UNIT_FIND
    CMP #255
    BNE TCMC1
    JMP AILP
TCMC1:  ;Found unit in compactor, kill it.
    LDA #<MSG_TERMINATED
    STA $FB
    LDA #>MSG_TERMINATED
    STA $FC
    JSR PRINT_INFO
    LDA #0  ;EXPLOSION sound
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT_FIND
    LDA #0
    STA UNIT_TYPE,X
    STA UNIT_HEALTH,X
    LDX #28 ;start of weapons
TCMC2:  LDA UNIT_TYPE,X
    CMP #0
    BEQ TCMC3
    INX
    CPX #32
    BNE TCMC2
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP
TCMC3:  LDA #11 ;SMALL EXPLOSION
    STA UNIT_TYPE,X
    LDA #248    ;first tile for explosion
    STA UNIT_TILE,X
    LDY UNIT
    LDA UNIT_LOC_X,Y
    STA UNIT_LOC_X,X
    LDA UNIT_LOC_Y,Y
    STA UNIT_LOC_Y,X
    LDA UNIT_FIND
    CMP #0  ;is it the player?
    BNE TCMC4
    LDA #10
    STA BORDER
TCMC4:  JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

TC_CLOSED_STATE:
    LDA #146
    STA TCPIECE1
    LDA #147
    STA TCPIECE2
    LDA #150
    STA TCPIECE3
    LDA #151
    STA TCPIECE4
    JSR DRAW_TRASH_COMPACTOR
    INC UNIT_A,X
    LDA #10
    STA UNIT_TIMER_A,X  
    JMP AILP

TC_MID_OPENING:
    LDA #144
    STA TCPIECE1
    LDA #145
    STA TCPIECE2
    LDA #148
    STA TCPIECE3
    LDA #148
    STA TCPIECE4
    JSR DRAW_TRASH_COMPACTOR    
    LDA #0
    STA UNIT_A,X
    LDA #20
    STA UNIT_TIMER_A,X
    LDA #14     ;door sound
    JSR PLAY_SOUND  ;SOUND PLAY 
    JMP AILP

DRAW_TRASH_COMPACTOR:
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    DEC MAP_Y   ;start one tile above
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA TCPIECE1
    STA TILE
    JSR PLOT_TILE_TO_MAP
    INY
    LDA TCPIECE2
    STA ($FD),Y 
    TYA
    CLC
    ADC #127
    TAY
    LDA TCPIECE3
    STA ($FD),Y
    LDA TCPIECE4
    INY 
    STA ($FD),Y
    JSR CHECK_FOR_WINDOW_REDRAW 
    RTS
TCPIECE1:   !BYTE 00
TCPIECE2:   !BYTE 00
TCPIECE3:   !BYTE 00
TCPIECE4:   !BYTE 00



HOVER_ATTACK:
    LDX UNIT
    LDA #0
    STA UNIT_TIMER_B,X
    JSR HOVERBOT_ANIMATE
    LDA #7
    STA UNIT_TIMER_A,X
    LDA #%00000010  ;HOVER
    STA MOVE_TYPE
    ;CHECK FOR HORIZONTAL MOVEMENT
    LDA UNIT_LOC_X,X
    CMP UNIT_LOC_X
    BEQ HOAT13
    BCC HOAT12
    JSR REQUEST_WALK_LEFT
    JMP HOAT13
HOAT12: JSR REQUEST_WALK_RIGHT
HOAT13: ;NOW CHECK FOR VERITCAL MOVEMENT
    LDA UNIT_LOC_Y,X
    CMP UNIT_LOC_Y
    BEQ HOAT20
    BCC HOAT14
    JSR REQUEST_WALK_UP
    JMP HOAT20
HOAT14: JSR REQUEST_WALK_DOWN   
HOAT20: JSR ROBOT_ATTACK_RANGE
    LDA PROX_DETECT
    CMP #1  ;1=Robot next to player 0=not
    BNE HOAT21  
    LDA #1  ;amount of damage it will inflict
    STA TEMP_A  
    LDA #0  ;unit to inflict damage on.
    STA UNIT_FIND
    JSR INFLICT_DAMAGE
    JSR CREATE_PLAYER_EXPLOSION
    LDA #07     ;electric shock
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA #30     ;rate of attack on player.
    STA UNIT_TIMER_A,X
    ;add some code here to create explosion
HOAT21: JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

CREATE_PLAYER_EXPLOSION:
    LDX #28
TE01:   LDA UNIT_TYPE,X
    CMP #0
    BEQ TE02
    INX
    CPX #32 ;max unit for weaponsfire
    BNE TE01    
TE02:   LDA #11 ;Small explosion AI type
    STA UNIT_TYPE,X
    LDA #248    ;first tile for explosion
    STA UNIT_TILE,X
    LDA #1
    STA UNIT_TIMER_A,X
    LDA UNIT_LOC_X
    STA UNIT_LOC_X,X
    LDA UNIT_LOC_Y
    STA UNIT_LOC_Y,X
    RTS 

EVILBOT:
    LDX UNIT
    LDA #5
    STA UNIT_TIMER_A,X
    ;first animate evilbot
    LDA UNIT_TILE,X
    CMP #100
    BNE EVIL1
    INC UNIT_TILE,X
    JMP EVIL10
EVIL1:  CMP #101
    BNE EVIL2
    INC UNIT_TILE,X
    JMP EVIL10
EVIL2:  CMP #102
    BNE EVIL3
    INC UNIT_TILE,X
    JMP EVIL10
EVIL3:  LDA #100
    STA UNIT_TILE,X
EVIL10: ;now figure out movement
    LDA UNIT_TIMER_B,X
    CMP #0
    BEQ EVIL11
    DEC UNIT_TIMER_B,X
    JSR CHECK_FOR_WINDOW_REDRAW
    JMP AILP
EVIL11: LDA #1  ;Reset timer B
    STA UNIT_TIMER_B,X
    LDA #%00000001  ;WALK
    STA MOVE_TYPE
    ;CHECK FOR HORIZONTAL MOVEMENT
    LDA UNIT_LOC_X,X
    CMP UNIT_LOC_X
    BEQ EVIL13
    BCC EVIL12
    JSR REQUEST_WALK_LEFT
    JMP EVIL13
EVIL12: JSR REQUEST_WALK_RIGHT
EVIL13: ;NOW CHECK FOR VERITCAL MOVEMENT
    LDA UNIT_LOC_Y,X
    CMP UNIT_LOC_Y
    BEQ EVIL20
    BCC EVIL14
    JSR REQUEST_WALK_UP
    JMP EVIL20
EVIL14: JSR REQUEST_WALK_DOWN   
EVIL20: JSR ROBOT_ATTACK_RANGE
    LDA PROX_DETECT
    CMP #1  ;1=Robot next to player 0=not
    BNE EVIL21  
    LDA #5  ;amount of damage it will inflict
    STA TEMP_A  
    LDA #0  ;unit to inflict damage on.
    STA UNIT_FIND
    JSR INFLICT_DAMAGE
    JSR CREATE_PLAYER_EXPLOSION
    LDA #07     ;electric shock sound
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA #15     ;rate of attack on player.
    STA UNIT_TIMER_A,X
EVIL21: JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

;This routine handles automatic sliding doors.
;UNIT_B register means:
;0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
AI_DOOR:
    LDX UNIT
    LDA UNIT_B,X
    CMP #06 ;make sure number is in bounds
    BCS DOORA
    TAY
    LDA AIDB_L,Y
    STA DOORJ+1
    LDA AIDB_H,Y
    STA DOORJ+2
DOORJ:  JMP $0000   ;self modifying code
DOORA:  JMP AILP    ;-SHOULD NEVER NEED TO HAPPEN
AIDB_L: 
    <DOOR_OPEN_A
    <DOOR_OPEN_B
    <DOOR_OPEN_FULL
    <DOOR_CLOSE_A
    <DOOR_CLOSE_B
    <DOOR_CLOSE_FULL
AIDB_H: 
    >DOOR_OPEN_A
    >DOOR_OPEN_B
    >DOOR_OPEN_FULL
    >DOOR_CLOSE_A
    >DOOR_CLOSE_B
    >DOOR_CLOSE_FULL

DOOR_OPEN_A:
    LDA UNIT_A,X
    CMP #1
    BEQ DOA1
    ;HORIZONTAL DOOR
    LDA #88
    STA DOORPIECE1
    LDA #89
    STA DOORPIECE2
    LDA #86
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DOA2
DOA1:   ;VERTICAL DOOR
    LDA #70
    STA DOORPIECE1
    LDA #74
    STA DOORPIECE2
    LDA #78
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DOA2:   LDY UNIT
    LDA #1
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

DOOR_OPEN_B:
    LDA UNIT_A,X
    CMP #1
    BEQ DOB1
    ;HORIZONTAL DOOR
    LDA #17
    STA DOORPIECE1
    LDA #09
    STA DOORPIECE2
    LDA #91
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DOB2
DOB1:   ;VERTICAL DOOR
    LDA #27
    STA DOORPIECE1
    LDA #09
    STA DOORPIECE2
    LDA #15
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DOB2:   LDX UNIT
    LDA #2
    STA UNIT_B,X
    LDA #30
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP
DOOR_OPEN_FULL:
    LDX UNIT
    JSR DOOR_CHECK_PROXIMITY    
    LDA PROX_DETECT
    CMP #1
    BNE DOF1
    LDA #30
    STA UNIT_TIMER_B,X  ;RESET TIMER
    JMP AILP
DOF1:   ;if nobody near door, lets close it.
    ;check for object in the way first.
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR GET_TILE_FROM_MAP
    LDA TILE
    CMP #09 ;FLOOR-TILE
    BEQ DOFB
    ;SOMETHING IN THE WAY, ABORT
    LDA #35
    STA UNIT_TIMER_A,X
    JMP AILP
DOFB:   LDA #14     ;DOOR-SOUND
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA UNIT_A,X
    CMP #1
    BEQ DOF2
    ;HORIZONTAL_DOOR
    LDA #88
    STA DOORPIECE1
    LDA #89
    STA DOORPIECE2
    LDA #86
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DOF3
DOF2:   ;VERTICAL DOOR
    LDA #70
    STA DOORPIECE1
    LDA #74
    STA DOORPIECE2
    LDA #78
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DOF3:   LDY UNIT
    LDA #3
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

DOOR_CLOSE_A:
    LDA UNIT_A,X
    CMP #1
    BEQ DCA2
    ;HORIZONTAL DOOR
    LDA #84
    STA DOORPIECE1
    LDA #85
    STA DOORPIECE2
    LDA #86
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DCA3
DCA2:   ;VERTICAL DOOR
    LDA #69
    STA DOORPIECE1
    LDA #73
    STA DOORPIECE2
    LDA #77
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DCA3:   LDY UNIT
    LDA #4
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

DOOR_CLOSE_B:
    LDA UNIT_A,X
    CMP #1
    BEQ DCB2
    ;HORIZONTAL DOOR
    LDA #80
    STA DOORPIECE1
    LDA #81
    STA DOORPIECE2
    LDA #82
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DCB3
DCB2:   ;VERTICAL DOOR
    LDA #68
    STA DOORPIECE1
    LDA #72
    STA DOORPIECE2
    LDA #76
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DCB3:   LDY UNIT
    LDA #5
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

DOOR_CLOSE_FULL:
    LDX UNIT
    JSR DOOR_CHECK_PROXIMITY    
    LDA PROX_DETECT
    CMP #0
    BNE DCF1
DCF0:   LDA #20
    STA UNIT_TIMER_A,X  ;RESET TIMER
    JMP AILP
DCF1:   ;if player near door, lets open it.
    ;first check if locked
    LDA UNIT_C,X    ;Lock status
    CMP #0  ;UNLOCKED
    BEQ DCFZ
    CMP #1  ;SPADE KEY
    BNE DCFB        
    LDA KEYS
    AND #%00000001  ;CHECK FOR SPADE KEY
    CMP #%00000001
    BEQ DCFZ
    JMP DCF0
DCFB:   CMP #2  ;HEART KEY
    BNE DCFC        
    LDA KEYS
    AND #%00000010  ;CHECK FOR HEART KEY
    CMP #%00000010
    BEQ DCFZ
    JMP DCF0
DCFC:   CMP #3  ;STAR KEY
    BNE DCF0    ;SHOULD NEVER HAPPEN    
    LDA KEYS
    AND #%00000100  ;CHECK FOR STAR KEY
    CMP #%00000100
    BEQ DCFZ
    JMP DCF0
DCFZ:   ;Start open door process
    LDA #14     ;DOOR-SOUND
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA UNIT_A,X
    CMP #1
    BEQ DCF2
    ;HORIZONTAL DOOR
    LDA #84
    STA DOORPIECE1
    LDA #85
    STA DOORPIECE2
    LDA #86
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    JMP DCF3
    ;VERTICAL DOOR
DCF2:   LDA #69
    STA DOORPIECE1
    LDA #73
    STA DOORPIECE2
    LDA #77
    STA DOORPIECE3
    JSR DRAW_VERTICAL_DOOR
DCF3:   LDY UNIT
    LDA #0
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

DRAW_VERTICAL_DOOR:
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    DEC MAP_Y
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA DOORPIECE1
    STA TILE
    JSR PLOT_TILE_TO_MAP
    LDA $FD
    CLC
    ADC #128
    STA $FD
    LDA $FE
    ADC #$00
    STA $FE
    LDA DOORPIECE2
    STA ($FD),Y
    LDA $FD
    CLC
    ADC #128
    STA $FD
    LDA $FE
    ADC #$00
    STA $FE
    LDA DOORPIECE3
    STA ($FD),Y 
    RTS

DRAW_HORIZONTAL_DOOR:
    LDA UNIT_LOC_X,X
    STA MAP_X
    DEC MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    LDA DOORPIECE1
    STA TILE
    JSR PLOT_TILE_TO_MAP
    INY
    LDA DOORPIECE2
    STA ($FD),Y
    INY
    LDA DOORPIECE3
    STA ($FD),Y
    RTS
DOORPIECE1  00
DOORPIECE2  00
DOORPIECE3  00

ROBOT_ATTACK_RANGE:
    ;First check horizontal proximity to door
    LDA UNIT_LOC_X,X    ;ROBOT UNIT
    SEC     ;always set carry before subtraction
    SBC     UNIT_LOC_X  ;PLAYER UNIT
    BCC     RAR1 ;if carry cleared then its negative
    JMP RAR2
RAR1:   EOR     #$FF ;convert two's comp back to positive
    ADC     #$01 ;no need for CLC here, already cleared
RAR2:   CMP #1  ;1 HORIZONTAL TILE FROM PLAYER
    BCC RAR3
    LDA #0  ;player not detected
    STA PROX_DETECT
    RTS 
RAR3:   ;Now check vertical proximity
    LDA UNIT_LOC_Y,X    ;DOOR UNIT
    SEC     ;always set carry before subtraction
    SBC     UNIT_LOC_Y  ;PLAYER UNIT
    BCC     RAR4 ;if carry cleared then its negative
    JMP RAR5
RAR4:   EOR     #$FF ;convert two's comp back to positive
    ADC     #$01 ;no need for CLC here, already cleared
RAR5:   CMP #1  ;1 VERTICAL TILE FROM PLAYER
    BCC RAR6
    LDA #0  ;player not detected
    STA PROX_DETECT
    RTS     
RAR6:   ;PLAYER DETECTED, CHANGE DOOR MODE.
    LDA #1
    STA PROX_DETECT
    RTS     

DOOR_CHECK_PROXIMITY:
    ;First check horizontal proximity to door
    LDA UNIT_LOC_X,X    ;DOOR UNIT
    SEC     ;always set carry before subtraction
    SBC     UNIT_LOC_X  ;PLAYER UNIT
    BCC     PRD1 ;if carry cleared then its negative
    JMP PRD2
PRD1:   EOR     #$FF ;convert two's comp back to positive
    ADC     #$01 ;no need for CLC here, already cleared
PRD2:   CMP #2  ;2 HORIZONTAL TILES FROM PLAYER
    BCC PRD3
    LDA #0  ;player not detected
    STA PROX_DETECT
    RTS 
PRD3:   ;Now check vertical proximity
    LDA UNIT_LOC_Y,X    ;DOOR UNIT
    SEC     ;always set carry before subtraction
    SBC     UNIT_LOC_Y  ;PLAYER UNIT
    BCC     PRD4 ;if carry cleared then its negative
    JMP PRD5
PRD4:   EOR     #$FF ;convert two's comp back to positive
    ADC     #$01 ;no need for CLC here, already cleared
PRD5:   CMP #2  ;2 VERTICAL TILES FROM PLAYER
    BCC PRD6
    LDA #0  ;player not detected
    STA PROX_DETECT
    RTS     
PRD6:   ;PLAYER DETECTED, CHANGE DOOR MODE.
    LDA #1
    STA PROX_DETECT
    RTS 
PROX_DETECT 00  ;0=NO 1=YES

;This routine handles automatic sliding doors.
;UNIT_B register means:
;0=opening-A 1=opening-B 2=OPEN 3=closing-A 4=closing-B 5-CLOSED
ELEVATOR:
    LDX UNIT
    LDA UNIT_B,X
    CMP #06 ;make sure number is in bounds
    BCS ELEVA
    TAY
    LDA ELDB_L,Y
    STA ELEVJ+1
    LDA ELDB_H,Y
    STA ELEVJ+2
ELEVJ:  JMP $0000   ;self modifying code
ELEVA:  JMP AILP    ;-SHOULD NEVER NEED TO HAPPEN
ELDB_L: 
    <ELEV_OPEN_A
    <ELEV_OPEN_B
    <ELEV_OPEN_FULL
    <ELEV_CLOSE_A
    <ELEV_CLOSE_B
    <ELEV_CLOSE_FULL
ELDB_H: 
    >ELEV_OPEN_A
    >ELEV_OPEN_B
    >ELEV_OPEN_FULL
    >ELEV_CLOSE_A
    >ELEV_CLOSE_B
    >ELEV_CLOSE_FULL

ELEV_OPEN_A:
    LDA #181
    STA DOORPIECE1
    LDA #89
    STA DOORPIECE2
    LDA #173
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDY UNIT
    LDA #1
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP
    
ELEV_OPEN_B:
    LDA #182
    STA DOORPIECE1
    LDA #09
    STA DOORPIECE2
    LDA #172
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDX UNIT
    LDA #2
    STA UNIT_B,X
    LDA #50
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

ELEV_OPEN_FULL:
    LDX UNIT
EVOF1:  ;CLOSE DOOR
    ;check for object in the way first.
    LDA UNIT_LOC_X,X
    STA MAP_X
    LDA UNIT_LOC_Y,X
    STA MAP_Y
    JSR GET_TILE_FROM_MAP
    LDA TILE
    CMP #09 ;FLOOR-TILE
    BEQ EVOF3
    ;SOMETHING IN THE WAY, ABORT
EVOF2:  LDA #35
    STA UNIT_TIMER_A,X
    JMP AILP
EVOF3:  ;check for player or robot in the way
    JSR CHECK_FOR_UNIT
    LDX UNIT
    LDA UNIT_FIND   
    CMP #255
    BNE EVOF2
EVOFB:  ;START TO CLOSE ELEVATOR DOOR
    LDA #14     ;DOOR SOUND
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA #181
    STA DOORPIECE1
    LDA #89
    STA DOORPIECE2
    LDA #173
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDX UNIT
    LDA #3
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

ELEV_CLOSE_A:
    LDA #84
    STA DOORPIECE1
    LDA #85
    STA DOORPIECE2
    LDA #173
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDY UNIT
    LDA #4
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

ELEV_CLOSE_B:
    LDA #80
    STA DOORPIECE1
    LDA #81
    STA DOORPIECE2
    LDA #174
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDY UNIT
    LDA #5
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW
    JSR ELEVATOR_PANEL
    JMP AILP

ELEV_CLOSE_FULL:
    LDX UNIT
    JSR DOOR_CHECK_PROXIMITY    
    LDA PROX_DETECT
    CMP #0
    BNE EVF1
    LDA #20
    STA UNIT_TIMER_A,X  ;RESET TIMER
    JMP AILP
EVF1:   ;Start open door process
    LDA #14     ;DOOR SOUND
    JSR PLAY_SOUND  ;SOUND PLAY
    LDX UNIT
    LDA #84
    STA DOORPIECE1
    LDA #85
    STA DOORPIECE2
    LDA #173
    STA DOORPIECE3
    JSR DRAW_HORIZONTAL_DOOR
    LDY UNIT
    LDA #0
    STA UNIT_B,X
    LDA #5
    STA UNIT_TIMER_A,X
    JSR CHECK_FOR_WINDOW_REDRAW 
    JMP AILP

ELEVATOR_PANEL:
    ;Check to see if player is standing in the
    ;elevator first.
    LDX UNIT
    LDA UNIT_LOC_X,X    ;elevator X location
    CMP UNIT_LOC_X  ;player X location
    BEQ ELPN1
    RTS
ELPN1:  LDA UNIT_LOC_Y,X    ;elevator Y location
    SEC
    SBC #01
    CMP UNIT_LOC_Y  ;player Y location
    BEQ ELPN2
    RTS
ELPN2:  ;PLAYER DETECTED, START ELEVATOR PANEL
    LDA #<MSG_ELEVATOR
    STA $FB
    LDA #>MSG_ELEVATOR
    STA $FC
    JSR PRINT_INFO
    LDA #<MSG_LEVELS
    STA $FB
    LDA #>MSG_LEVELS
    STA $FC
    JSR PRINT_INFO
    JSR ELEVATOR_SELECT
    RTS

;In this AI routine, the droid simply goes left until it
;hits an object, and then reverses direction and does the
;same, bouncing back and forth.
LEFT_RIGHT_DROID:
    LDX UNIT
    JSR HOVERBOT_ANIMATE
    LDA #10     ;reset timer to 10
    STA UNIT_TIMER_A,X  
    LDA UNIT_A,X        ;GET DIRECTION
    CMP #1  ;0=LEFT 1=RIGHT
    BEQ LRD01
    LDA #%00000010
    STA MOVE_TYPE
    JSR REQUEST_WALK_LEFT
    LDA MOVE_RESULT
    CMP #1
    BEQ LRD02
    LDA #1
    LDX UNIT
    STA UNIT_A,X    ;CHANGE DIRECTION
    JSR CHECK_FOR_WINDOW_REDRAW
    JMP AILP
LRD01:  LDA #%00000010
    STA MOVE_TYPE
    JSR REQUEST_WALK_RIGHT
    LDA MOVE_RESULT
    CMP #1
    BEQ LRD02
    LDA #0
    LDX UNIT
    STA UNIT_A,X    ;CHANGE DIRECTION
LRD02:  JSR CHECK_FOR_WINDOW_REDRAW
    JMP AILP

;In this AI routine, the droid simply goes UP until it
;hits an object, and then reverses direction and does the
;same, bouncing back and forth.
UP_DOWN_DROID:
    LDX UNIT
    JSR HOVERBOT_ANIMATE
    LDA #10     ;reset timer to 10
    STA UNIT_TIMER_A,X  
    LDA UNIT_A,X        ;GET DIRECTION
    CMP #1  ;0=UP 1=DOWN
    BEQ UDD01
    LDA #%00000010
    STA MOVE_TYPE
    JSR REQUEST_WALK_UP
    LDA MOVE_RESULT
    CMP #1
    BEQ UDD02
    LDA #1
    LDX UNIT
    STA UNIT_A,X    ;CHANGE DIRECTION
    JSR CHECK_FOR_WINDOW_REDRAW
    JMP AILP
UDD01:  LDA #%00000010
    STA MOVE_TYPE
    JSR REQUEST_WALK_DOWN
    LDA MOVE_RESULT
    CMP #1
    BEQ UDD02
    LDA #0
    LDX UNIT
    STA UNIT_A,X    ;CHANGE DIRECTION
UDD02:  JSR CHECK_FOR_WINDOW_REDRAW
    JMP AILP
*/

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
