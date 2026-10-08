#define WIN_HALFWORM                     0x0100

#define IDS_APPTITLE                     0x1000
#define IDS_LEFTWELCOME                  0x1001
#define IDS_RIGHTWELCOME                 0x1002

#define IDS_WONPREVGAME                  0x1003
#define IDS_LOSTPREVGAME                 0x1004
#define IDS_DRAWPREVGAME                 0x1005

#define IDS_PLAYER1                      0x101a
#define IDS_PLAYER2                      0x101b
#define IDS_ANTICLOCKROT                 0x101c
#define IDS_CLOCKROT                     0x101d
#define IDS_FIRE                         0x101e

#define IDS_PROFILE_NAME                 0x1030
#define IDS_PRFAPP                       0x1031
#define IDS_PRFKEY_PLAYER_FORMAT         0x1032
#define IDS_PRFKEY_CONTROL_COUNTERCLOCKROT  0x1033
#define IDS_PRFKEY_CONTROL_CLOCKROT      0x1034
#define IDS_PRFKEY_CONTROL_FIRE          0x1035
#define IDS_PRFKEY_FIRST_RUN             0x1036
#define IDS_PRKKEY_SOUNDFX               0x1037
#define IDS_PRFKEY_SOUNDFX_VOLUME        0x1038
#define IDS_PRFKEY_BOARD_WIDTH           0x1039
#define IDS_PRFKEY_BOARD_HEIGHT          0x103a
#define IDS_PRFKEY_DOUBLE_SIZE           0x103b
#define IDS_PRFKEY_FRAMEPOS              0x103c

#define IDS_NO_MODIFICATION_APPLE        0x1100

#define IDS_APPLE_LOWER_SPEED            0x1101
#define IDS_APPLE_RAISE_SPEED            0x1102
#define IDS_APPLE_RESET                  0x1103
#define IDS_APPLE_AUTOFIRE               0x1104
#define IDS_APPLE_LOWER_FIRERATE         0x1105
#define IDS_APPLE_RAISE_FIRERATE         0x1106
#define IDS_APPLE_SLOWER_BULLETS         0x1107
#define IDS_APPLE_FASTER_BULLETS         0x1108
#define IDS_APPLE_PACIFIST               0x1109
#define IDS_APPLE_SUMMON_FRENZY          0x110a
#define IDS_APPLE_SUMMON_WRIGGLY         0x110b
#define IDS_APPLE_MORE_BULLETS           0x110c
#define IDS_APPLE_LESS_BULLETS           0x110d
#define IDS_APPLE_WRIGGLE                0x110e
#define IDS_APPLE_FATAL_ATTRACTION       0x110f
#define IDS_APPLE_HOMING_BULLETS         0x1110
#define IDS_APPLE_BIGGER_EXPLOSIONS      0x1111
#define IDS_APPLE_SMALLER_EXPLOSIONS     0x1112
#define IDS_APPLE_BOUNCE_MORE            0x1113
#define IDS_APPLE_BOUNCE_LESS            0x1114
#define IDS_APPLE_WARPINGBULLETS         0x1115
#define IDS_APPLE_DRUNKEN_MISSILES       0x1116

#define IDS_WORM_DIED                    0x1120
#define IDS_NO_MORE_PACIFIST             0x1121


#define IDM_GAME                         0x4000
#define IDM_GAME_START                   0x4001
#define IDM_GAME_PAUSE                   0x4002
#define IDM_GAME_EXIT                    0x4003
#define IDM_SETTINGS                     0x4004
#define IDM_SET_BOARDSIZE                0x4005
#define IDM_SET_KEYS                     0x4006
#define IDM_SET_DOUBLE                   0x4007
#define IDM_SET_SOUND                    0x4008
#define IDM_SET_SOUNDFX                  0x4009
#define IDM_SET_SOUNDVOL                 0x400a

#define DLG_CONFIG_KEYS                  0xa000
#define ST_PLAYER                        0xa001
#define ST_KEYDEF                        0xa002


#define DLG_BOARD_SIZE                   0xa020
#define SPBN_WIDTH                       0xa021
#define SPBN_HEIGHT                      0xa022
#define BN_MAX_SIZE                      0xa023
#define BN_MAX_SIZE_DOUBLE               0xa024

#define DLG_WELCOME                      0xc000

#define DLG_SFXVOLUME                    0xc005
#define SPBN_VOLUME                      0xc006

#ifdef DEBUG
#define IDM_DBG                          0xd000
#define IDM_DBG_APPLE                    0xd001
#define IDM_DBGW1                        0xd101
#define IDM_DBGW1_GROW                   0xd102
#define IDM_DBGW1_FASTER                 0xd103
#define IDM_DBGW1_SLOWER                 0xd104
#define IDM_DBGW1_FIRERATE_UP            0xd105
#define IDM_DBGW1_FIRERATE_DOWN          0xd106
#define IDM_DBGW1_FASTER_BULLETS         0xd107
#define IDM_DBGW1_SLOWER_BULLETS         0xd108
#define IDM_DBGW1_BIGGER_EXPLOSIONS      0xd109
#define IDM_DBGW1_SMALLER_EXPLOSIONS     0xd10a
#define IDM_DBGW1_MORE_BULLETS           0xd10b
#define IDM_DBGW1_LESS_BULLETS           0xd10c
#define IDM_DBGW1_BOUNCE_MORE            0xd10d
#define IDM_DBGW1_BOUNCE_LESS            0xd10e
#define IDM_DBGW1_HOMING_BULLETS         0xd10f


#define IDM_DBGW2                        0xd201
#define IDM_DBGW2_GROW                   0xd202
#define IDM_DBGW2_FASTER                 0xd203
#define IDM_DBGW2_SLOWER                 0xd204
#define IDM_DBGW2_FIRERATE_UP            0xd205
#define IDM_DBGW2_FIRERATE_DOWN          0xd206
#define IDM_DBGW2_FASTER_BULLETS         0xd207
#define IDM_DBGW2_SLOWER_BULLETS         0xd208
#define IDM_DBGW2_BIGGER_EXPLOSIONS      0xd209
#define IDM_DBGW2_SMALLER_EXPLOSIONS     0xd20a
#define IDM_DBGW2_MORE_BULLETS           0xd20b
#define IDM_DBGW2_LESS_BULLETS           0xd20c
#define IDM_DBGW2_BOUNCE_MORE            0xd20d
#define IDM_DBGW2_BOUNCE_LESS            0xd20e
#define IDM_DBGW2_HOMING_BULLETS         0xd20f
#endif

