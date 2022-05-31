#ifndef _PETROBOTS_H
#define _PETROBOTS_H

#include "Platform.h"

#define MAP_WINDOW_SIZE (PLATFORM_MAP_WINDOW_TILES_WIDTH * PLATFORM_MAP_WINDOW_TILES_HEIGHT)
#define MAP_WINDOW_WIDTH (PLATFORM_MAP_WINDOW_TILES_WIDTH * 24)
#define MAP_WINDOW_HEIGHT (PLATFORM_MAP_WINDOW_TILES_HEIGHT * 24)

// MAP FILES CONSIST OF EVERYTHING FROM THIS POINT ON
extern uint8_t MAP_DATA[8960];
#define UNIT_TYPE MAP_DATA  // Unit type 0=none (64 bytes)
#define UNIT_LOC_X (MAP_DATA + 1 * 64) // Unit X location (64 bytes)
#define UNIT_LOC_Y (MAP_DATA + 2 * 64) // Unit X location (64 bytes)
#define UNIT_A (MAP_DATA + 3 * 64)
#define UNIT_B (MAP_DATA + 4 * 64)
#define UNIT_C (MAP_DATA + 5 * 64)
#define UNIT_D (MAP_DATA + 6 * 64)
#define UNIT_HEALTH ((int8_t*)(MAP_DATA + 7 * 64)) // Unit health (0 to 11) (64 bytes)
#define MAP (MAP_DATA + 8 * 64 + 256)        // Location of MAP (8K)
// END OF MAP FILE

extern uint8_t* DESTRUCT_PATH; // Destruct path array (256 bytes)
extern uint8_t* TILE_ATTRIB;   // Tile attrib array (256 bytes)
#ifndef PLATFORM_SPRITE_SUPPORT
extern uint8_t* TILE_DATA_TL;  // Tile character top-left (256 bytes)
extern uint8_t* TILE_DATA_TM;  // Tile character top-middle (256 bytes)
extern uint8_t* TILE_DATA_TR;  // Tile character top-right (256 bytes)
extern uint8_t* TILE_DATA_ML;  // Tile character middle-left (256 bytes)
extern uint8_t* TILE_DATA_MM;  // Tile character middle-middle (256 bytes)
extern uint8_t* TILE_DATA_MR;  // Tile character middle-right (256 bytes)
extern uint8_t* TILE_DATA_BL;  // Tile character bottom-left (256 bytes)
extern uint8_t* TILE_DATA_BM;  // Tile character bottom-middle (256 bytes)
extern uint8_t* TILE_DATA_BR;  // Tile character bottom-right (256 bytes)
#endif

// These arrays can go anywhere in RAM
extern uint8_t UNIT_TIMER_A[64];   // Primary timer for units (64 bytes)
extern uint8_t UNIT_TIMER_B[64];   // Secondary timer for units (64 bytes)
extern uint8_t UNIT_TILE[32];      // Current tile assigned to unit (32 bytes)
extern uint8_t EXP_BUFFER[16];     // Explosion Buffer (16 bytes)
extern uint8_t MAP_PRECALC[MAP_WINDOW_SIZE];    // Stores pre-calculated objects for map window (77 bytes)
extern uint8_t MAP_PRECALC_DIRECTION[MAP_WINDOW_SIZE];    // Stores pre-calculated object directions for map window (77 bytes)
extern uint8_t MAP_PRECALC_TYPE[MAP_WINDOW_SIZE];    // Stores pre-calculated object types for map window (77 bytes)
#ifdef OPTIMIZED_MAP_RENDERING
extern uint8_t PREVIOUS_MAP_BACKGROUND[MAP_WINDOW_SIZE];
extern uint8_t PREVIOUS_MAP_FOREGROUND[MAP_WINDOW_SIZE];
extern uint8_t PREVIOUS_MAP_FOREGROUND_VARIANT[MAP_WINDOW_SIZE];
#endif

// The following are the locations where the current
// key controls are stored.  These must be set before
// the game can start.
extern uint8_t KEY_CONFIG[26];

extern uint8_t TILE;           // The tile number to be plotted
extern uint8_t DIRECTION;      // The direction of the tile to be plotted
extern uint8_t WALK_FRAME;     // Player walking animation frame
extern uint8_t DEMATERIALIZE_FRAME; // Dematerialize animation frame
extern uint8_t MAP_X;          // Current X location on map
extern uint8_t MAP_Y;          // Current Y location on map
extern uint8_t MAP_WINDOW_X;   // Top left location of what is displayed in map window
extern uint8_t MAP_WINDOW_Y;   // Top left location of what is displayed in map window
extern uint8_t DECNUM;         // a decimal number to be displayed onscreen as 3 digits.
extern uint8_t ATTRIB;         // Tile attribute value
extern uint8_t UNIT;           // Current unit being processed
extern uint8_t TEMP_A;         // used within some routines
extern uint8_t TEMP_B;         // used within some routines
extern uint8_t TEMP_C;         // used within some routines
extern uint8_t TEMP_D;         // used within some routines
extern uint8_t CURSOR_X;       // For on-screen cursor
extern uint8_t CURSOR_Y;       // For on-screen cursor
extern uint8_t CURSOR_ON;      // Is cursor active or not? 1=yes 0=no
extern uint8_t REDRAW_WINDOW;  // 1=yes 0=no
extern uint8_t MOVE_RESULT;    // 1=Move request success, 0=fail.
extern uint8_t UNIT_FIND;      // 255=no unit present.
extern uint8_t MOVE_TYPE;      // %00000001=WALK %00000010=HOVER
extern uint8_t* CUR_PATTERN;   // stores the memory location of the current musical pattern being played.

//extern uint8_t LSTX;           // $97 Current Key Pressed: 255 = No Key
extern uint8_t NDX;            // $9E No. of Chars. in Keyboard Buffer (Queue)
extern uint8_t* MAP_SOURCE;    // $FD
extern uint8_t SCREEN_MEMORY[SCREEN_WIDTH_IN_CHARACTERS * SCREEN_HEIGHT_IN_CHARACTERS]; // $8000
extern bool quit;

void INIT_GAME();

//extern char MAPNAME[];
extern const char* LOADMSG1;
extern uint8_t KEYS; // bit0=spade bit2=heart bit3=star
extern uint8_t AMMO_PISTOL; // how much ammo for the pistol
extern uint8_t AMMO_PLASMA; // how many shots of the plasmagun
extern uint8_t INV_BOMBS; // How many bombs do we have
extern uint8_t INV_EMP; // How many EMPs do we have
extern uint8_t INV_MEDKIT; // How many medkits do we have?
extern uint8_t INV_MAGNET; // How many magnets do we have?
extern uint8_t SELECTED_WEAPON; // 0=none 1=pistol 2=plasmagun
extern uint8_t SELECTED_ITEM; // 0=none 1=bomb 2=emp 3=medkit 4=magnet
extern uint8_t SELECT_TIMEOUT; // can only change weapons once it hits zero
extern uint8_t ANIMATE; // 0=DISABLED 1=ENABLED
extern uint8_t BIG_EXP_ACT; // 0=No explosion active 1=big explosion active
extern uint8_t MAGNET_ACT; // 0=no magnet active 1=magnet active
extern uint8_t PLASMA_ACT; // 0=No plasma fire active 1=plasma fire active
extern uint8_t RANDOM; // used for random number generation
extern uint8_t BORDER; // Used for border flash timing
extern uint8_t SCREEN_SHAKE; // 1=shake 0=no shake
extern uint8_t CONTROL; // 0=keyboard 1=custom keys 2=snes
extern uint16_t BORDER_COLOR; // Used for border flash coloring
extern char INTRO_MESSAGE[];
extern char MSG_CANTMOVE[];
extern char MSG_BLOCKED[];
extern char MSG_SEARCHING[];
extern char MSG_NOTFOUND[];
extern char MSG_FOUNDKEY[];
extern char MSG_FOUNDGUN[];
extern char MSG_FOUNDEMP[];
extern char MSG_FOUNDBOMB[];
extern char MSG_FOUNDPLAS[];
extern char MSG_FOUNDMED[];
extern char MSG_FOUNDMAG[];
extern char MSG_MUCHBET[];
extern char MSG_EMPUSED[];
extern char MSG_TERMINATED[];
extern char MSG_TRANS1[];
extern char MSG_ELEVATOR[];
extern char MSG_LEVELS[];
extern char MSG_PAUSED[];
extern char MSG_MUSICON[];
extern char MSG_MUSICOFF[];
extern uint8_t SELECTED_MAP;
extern char MAP_NAMES[];
#ifndef PLATFORM_MODULE_BASED_AUDIO
// THE FOLLOWING ARE USED BY THE SOUND SYSTEM*
extern uint8_t TEMPO_TIMER; // used for counting down to the next tick
extern uint8_t TEMPO; // How many IRQs between ticks
extern uint8_t DATA_LINE; // used for playback to keep track of which line we are executing.
extern uint8_t ARP_MODE; // 0=no 1=major 2=minor 3=sus4
extern uint8_t CHORD_ROOT; // root note of the chord
extern uint8_t SOUND_EFFECT; // FF=OFF or number of effect in progress
#endif
extern uint8_t MUSIC_ON; // 0=off 1=on

void DISPLAY_LOAD_MESSAGE1();
void DISPLAY_LOAD_MESSAGE2();

extern char LOAD_MSG2[];

void SETUP_INTERRUPT();
void RUNIRQ();

extern uint8_t BGTIMER1;
extern uint8_t BGTIMER2;
extern uint8_t KEYTIMER; // Used for repeat of movement

void UPDATE_GAME_CLOCK();

extern uint8_t HOURS;
extern uint8_t MINUTES;
extern uint8_t SECONDS;
extern uint8_t CYCLES;
extern uint8_t CLOCK_ACTIVE;
#ifdef INACTIVITY_TIMEOUT_GAME
extern uint8_t INACTIVE_SECONDS;
#endif

void SET_INITIAL_TIMERS();
void MAIN_GAME_LOOP();
void AFTER_MOVE_SNES();
void TOGGLE_MUSIC();
void START_IN_GAME_MUSIC();

#ifdef PLATFORM_MODULE_BASED_AUDIO
extern Platform::Module LEVEL_MUSIC[];
#else
extern uint8_t LEVEL_MUSIC[];
#endif

void CHEATER();
bool PAUSE_GAME();
void CLEAR_KEY_BUFFER();
void USE_ITEM();
void USE_BOMB();
void USE_MAGNET();
bool BOMB_MAGNET_COMMON1();
void BOMB_MAGNET_COMMON2();
void USE_EMP();
void USE_MEDKIT();
void FIRE_UP();
void FIRE_UP_PISTOL();
void FIRE_UP_PLASMA();
void FIRE_DOWN();
void FIRE_DOWN_PISTOL();
void FIRE_DOWN_PLASMA();
void FIRE_LEFT();
void FIRE_LEFT_PISTOL();
void FIRE_LEFT_PLASMA();
void FIRE_RIGHT();
void FIRE_RIGHT_PISTOL();
void FIRE_RIGHT_PLASMA();
void AFTER_FIRE(int X);
void KEY_REPEAT(bool keyDown);
void AFTER_MOVE();

extern uint8_t KEY_FAST; // 0=DEFAULT STATE

void SEARCH_OBJECT();

extern uint8_t SEARCHBAR; // to count how many periods to display.

void CALC_COORDINATES();
void USER_SELECT_OBJECT();
void MOVE_OBJECT();

extern uint8_t MOVTEMP_O; // origin tile
extern uint8_t MOVTEMP_D; // destination tile
extern uint8_t MOVTEMP_X; // x-coordinate
extern uint8_t MOVTEMP_Y; // y-coordinate
extern uint8_t MOVTEMP_U; // unit number (255=none)
extern uint8_t MOVTEMP_UX;
extern uint8_t MOVTEMP_UY;

void CACULATE_AND_REDRAW();
void MAP_PRE_CALCULATE();

#ifdef OPTIMIZED_MAP_RENDERING
void INVALIDATE_PREVIOUS_MAP();
#endif
void DRAW_MAP_WINDOW();

#ifdef PLATFORM_LIVE_MAP_SUPPORT
void TOGGLE_LIVE_MAP();
void TOGGLE_LIVE_MAP_ROBOTS();
void DRAW_LIVE_MAP();

extern uint8_t LIVE_MAP_ON;
extern uint8_t LIVE_MAP_ROBOTS_ON;
extern uint8_t LIVE_MAP_PLAYER_BLINK;
#endif

#ifdef PLATFORM_TILE_BASED_RENDERING
void PLOT_TILE(uint16_t destination, uint16_t x, uint16_t y);
void PLOT_TRANSPARENT_TILE(uint16_t destination, uint16_t x, uint16_t y);
#else
void PLOT_TILE(uint16_t destination);
void PLOT_TRANSPARENT_TILE(uint16_t destination);
#endif
#ifndef PLATFORM_CURSOR_SUPPORT
void REVERSE_TILE();
#endif
void CHECK_FOR_WINDOW_REDRAW();
void DECWRITE(uint16_t destination, uint8_t color = 10);

void TILE_LOAD_ROUTINE();
void MAP_LOAD_ROUTINE();
void DISPLAY_GAME_SCREEN();

extern char INTRO_OPTIONS[];

void DISPLAY_INTRO_SCREEN();
void DISPLAY_ENDGAME_SCREEN();

extern char DIFF_LEVEL_WORDS[];

void DECOMPRESS_SCREEN(uint8_t* source, uint8_t color = 10);

extern uint8_t RPT; // repeat value

void DISPLAY_PLAYER_HEALTH();
void CYCLE_ITEM();
void DISPLAY_ITEM();
void PRESELECT_ITEM();
void DISPLAY_TIMEBOMB();
void DISPLAY_EMP();
void DISPLAY_MEDKIT();
void DISPLAY_MAGNET();
void DISPLAY_BLANK_ITEM();
void CYCLE_WEAPON();
void DISPLAY_WEAPON();
void PRESELECT_WEAPON();
void DISPLAY_PLASMA_GUN();
void DISPLAY_PISTOL();
void DISPLAY_BLANK_WEAPON();
void DISPLAY_KEYS();
void GAME_OVER();
void GOM4();

extern uint8_t GAMEOVER1[];
extern uint8_t GAMEOVER2[];
extern uint8_t GAMEOVER3[];

void DISPLAY_WIN_LOSE();

extern char WIN_MSG[];
extern char LOS_MSG[];

void PRINT_INTRO_MESSAGE();
void PRINT_INFO(const char*);

extern uint8_t PRINTX;

void SCROLL_INFO();
void RESET_KEYS_AMMO();
void INTRO_SCREEN();
void START_INTRO_MUSIC();
bool EXEC_COMMAND();
void CYCLE_CONTROLS();

extern char CONTROLTEXT[];
extern uint8_t CONTROLSTART[];

void CYCLE_MAP();
void DISPLAY_MAP_NAME();
char* CALC_MAP_NAME();
void REVERSE_MENU_OPTION(bool reverse);

extern uint8_t MENUY; // CURRENT MENU SELECTION
typedef uint16_t menu_chart_t;
extern menu_chart_t MENU_CHART[];

void CHANGE_DIFFICULTY_LEVEL();

extern uint8_t DIFF_LEVEL; // default medium
extern uint8_t ROBOT_FACE[];
extern uint8_t FACE_LEVEL[];

void SET_DIFF_LEVEL();
void SET_DIFF_EASY();
void SET_DIFF_HARD();

extern uint16_t MAP_CHART[PLATFORM_MAP_WINDOW_TILES_HEIGHT];

void EMP_FLASH();
void ANIMATE_WATER();

extern uint8_t WATER_TIMER;
#ifdef PLATFORM_IMAGE_BASED_TILES
extern uint8_t ANIM_STATE;
#else
extern uint8_t WATER_TEMP1;
extern uint8_t HVAC_STATE;
#endif
extern uint8_t CINEMA_STATE;

void ELEVATOR_SELECT();

extern uint8_t ELEVATOR_MAX_FLOOR;
extern uint8_t ELEVATOR_CURRENT_FLOOR;

void ELEVATOR_INVERT();
void ELEVATOR_INC();
void ELEVATOR_DEC();
void ELEVATOR_FIND_XY();
void SET_CONTROLS();

extern uint8_t STANDARD_CONTROLS[];

void SET_CUSTOM_KEYS();

extern uint8_t KEYS_DEFINED; // DEFAULT 0

void PET_SCREEN_SHAKE();
void PET_BORDER_FLASH();

extern uint8_t FLASH_STATE;
#ifndef PLATFORM_IMAGE_SUPPORT
extern uint8_t OUCH1[];
extern uint8_t OUCH2[];
extern uint8_t OUCH3[];
#endif

void DEMATERIALIZE();
void ANIMATE_PLAYER();
void PLAY_SOUND(int);

#ifndef PLATFORM_MODULE_BASED_AUDIO
extern uint8_t* PATTERN_TEMP;
extern uint8_t DATA_LINE_TEMP;
extern uint8_t TEMPO_TEMP;

extern uint8_t* SOUND_LIBRARY[];

void MUSIC_ROUTINE();
#endif
void STOP_SONG();
void BACKGROUND_TASKS();

extern void (*AI_ROUTINE_CHART[])(void);

void DUMMY_ROUTINE();
void WATER_RAFT_LR();
void RAFT_DELETE();
void RAFT_PLOT();
void MAGNETIZED_ROBOT();
void GENERATE_RANDOM_NUMBER();
void MAGNET();
void DEAD_ROBOT();
void UP_DOWN_ROLLERBOT();
void LEFT_RIGHT_ROLLERBOT();
void ROLLERBOT_FIRE_DETECT();
void ROLLERBOT_AFTER_FIRE(uint8_t unit, uint8_t tile);
void ROLLERBOT_ANIMATE();
void TRANSPORTER_PAD();
void TRANS_PLAYER_PRESENT();
void TRANS_ACTIVE();
void TIME_BOMB();
void BIG_EXP_PHASE1();
void BEX1_NORTH();
void BEX1_SOUTH();
void BEX1_EAST();
void BEX1_WEST();
void BEX1_NE();
void BEX1_NW();
void BEX1_SE();
void BEX1_SW();
void BEX_PART1();
bool BEX_PART2();
void BEX_PART3();
void BEXCEN();
void BIG_EXP_PHASE2();
void RESTORE_TILE();
void TRASH_COMPACTOR();
void DRAW_TRASH_COMPACTOR();
extern uint8_t TCPIECE1;
extern uint8_t TCPIECE2;
extern uint8_t TCPIECE3;
extern uint8_t TCPIECE4;
void WATER_DROID();
void PISTOL_FIRE_UP();
void PISTOL_FIRE_DOWN();
void PISTOL_FIRE_LEFT();
void PISTOL_FIRE_RIGHT();
void DEACTIVATE_WEAPON();
void PISTOL_AI_COMMON();
void ALTER_AI();
void INFLICT_DAMAGE();
void SMALL_EXPLOSION();
void HOVER_ATTACK();
void CREATE_PLAYER_EXPLOSION();
void EVILBOT();
void AI_DOOR();
extern void (*AIDB[])(void);
void DOOR_OPEN_A();
void DOOR_OPEN_B();
void DOOR_OPEN_FULL();
void DOOR_CLOSE_A();
void DOOR_CLOSE_B();
void DOOR_CLOSE_FULL();
void DRAW_VERTICAL_DOOR();
void DRAW_HORIZONTAL_DOOR();
extern uint8_t DOORPIECE1;
extern uint8_t DOORPIECE2;
extern uint8_t DOORPIECE3;
void ROBOT_ATTACK_RANGE();
void DOOR_CHECK_PROXIMITY();
extern uint8_t PROX_DETECT;
void ELEVATOR();
extern void (*ELDB[])(void);
void ELEV_OPEN_A();
void ELEV_OPEN_B();
void ELEV_OPEN_FULL();
void ELEV_CLOSE_A();
void ELEV_CLOSE_B();
void ELEV_CLOSE_FULL();
void ELEVATOR_PANEL();
void PLOT_TILE_TO_MAP();
void GET_TILE_FROM_MAP();
void LEFT_RIGHT_DROID();
void UP_DOWN_DROID();
void HOVERBOT_ANIMATE(uint8_t X);
void REQUEST_WALK_RIGHT();
void REQUEST_WALK_LEFT();
void REQUEST_WALK_DOWN();
void REQUEST_WALK_UP();
void CHECK_FOR_UNIT();
void CHECK_FOR_HIDDEN_UNIT();

#ifndef PLATFORM_IMAGE_SUPPORT
extern uint8_t INTRO_TEXT[];
extern uint8_t SCR_TEXT[];
extern uint8_t SCR_ENDGAME[];
#endif
extern uint8_t SCR_CUSTOM_KEYS[];
extern char CINEMA_MESSAGE[];
#ifndef PLATFORM_IMAGE_SUPPORT
extern uint8_t WEAPON1A[];
extern uint8_t WEAPON1B[];
extern uint8_t WEAPON1C[];
extern uint8_t WEAPON1D[];
extern uint8_t PISTOL1A[];
extern uint8_t PISTOL1B[];
extern uint8_t PISTOL1C[];
extern uint8_t PISTOL1D[];
extern uint8_t TBOMB1A[];
extern uint8_t TBOMB1B[];
extern uint8_t TBOMB1C[];
extern uint8_t TBOMB1D[];
extern uint8_t EMP1A[];
extern uint8_t EMP1B[];
extern uint8_t EMP1C[];
extern uint8_t EMP1D[];
extern uint8_t MAG1A[];
extern uint8_t MAG1B[];
extern uint8_t MAG1C[];
extern uint8_t MAG1D[];
extern uint8_t MED1A[];
extern uint8_t MED1B[];
extern uint8_t MED1C[];
extern uint8_t MED1D[];
#endif
#ifndef PLATFORM_MODULE_BASED_AUDIO
extern uint8_t NOTE_FREQ[];
extern uint8_t NOTE_OCTAVE[];
extern uint8_t SND_EXPLOSION[];
extern uint8_t SND_MEDKIT[];
extern uint8_t SND_EMP[];
extern uint8_t SND_MAGNET[];
extern uint8_t SND_SHOCK[];
extern uint8_t SND_MOVE_OBJ[];
extern uint8_t SND_PLASMA[];
extern uint8_t SND_PISTOL[];
extern uint8_t SND_ITEM_FOUND[];
extern uint8_t SND_ERROR[];
extern uint8_t SND_CYCLE_WEAPON[];
extern uint8_t SND_CYCLE_ITEM[];
extern uint8_t SND_DOOR[];
extern uint8_t SND_MENU_BEEP[];
extern uint8_t SND_SHORT_BEEP[];
extern uint8_t INTRO_MUSIC[];
extern uint8_t WIN_MUSIC[];
extern uint8_t LOSE_MUSIC[];
extern uint8_t IN_GAME_MUSIC1[];
extern uint8_t IN_GAME_MUSIC2[];
extern uint8_t IN_GAME_MUSIC3[];
#endif

void convertToPETSCII(char* string);
void writeToScreenMemory(address_t address, uint8_t value, uint8_t color = 10, uint8_t yOffset = 0);

#endif
