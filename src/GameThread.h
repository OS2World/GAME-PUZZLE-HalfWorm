typedef struct _GAMETHREADPARAMS
{
   HEV hevInitCompleted;
   HWND hwnd;
   HDIVE hDive;
   SIZEL sizlGameBitmap;
   PBYTE abKeyStates;
}GAMETHREADPARAMS, *PGAMETHREADPARAMS;

#define GTHRDMSG_INITGAME_COMPLETE       WM_USER+1
#define GTHRDMSG_TERMINATE               WM_USER+2
#define GTHRDMSG_START_GAME              WM_USER+3
#define GTHRDMSG_NEW_BITMAP_SIZE         WM_USER+5

#define GTHRDMSG_INIT_INTRO              WM_USER+6

#define GTHRDMSG_ENABLE_SOUNDFX          WM_USER+7
#define GTHRDMSG_SOUNDVOL_DLG            WM_USER+8
#define GTHRDMSG_SET_SFX_VOLUME          WM_USER+9
#define GTHRDMSG_QUIT_GAME               WM_USER+10


#ifdef DEBUG
#define GTHRDMSG_WORM_SPEED              WM_USER+1000
#define GTHRDMSG_BULLET_SPEED            WM_USER+1001
#define GTHRDMSG_WORM_FIRERATE           WM_USER+1002
#define GTHRDMSG_EXPLOSION_SIZE          WM_USER+1003
#define GTHRDMSG_GROW                    WM_USER+1004
#define GTHRDMSG_BULLETS_PER_SHOT        WM_USER+1005
#define GTHRDMSG_BULLETS_BOUNCE          WM_USER+1006
#endif

/* Set by the client window: non zero while the game must be paused (window lost the focus and
   Background Run is off).  The game thread stops the game clock while it is set. */
#define GAMEPAUSE_FOCUS  0x01      /* window has no focus and Background Run is off */
#define GAMEPAUSE_USER   0x02      /* Game - Pause Game */
extern volatile ULONG ulGamePause;

