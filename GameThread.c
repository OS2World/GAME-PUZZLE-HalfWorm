#pragma strings(readonly)

#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_WINMESSAGEMGR
#define INCL_WINSHELLDATA
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_OS2MM
#define INCL_MM_OS2                      /* Needed for DiveBlitImageLines() */

#include <os2.h>
#include <os2me.h>

#include <dive.h>
#include <fourcc.h>

#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ClientWindow.h"
#include "GameThread.h"
#include "AppleTypes.h"
#include "FrameSubProc.h"
#include "StatusbarWindow.h"
#include "resources.h"

#include "configuration.h"

#include "graphlib.h"
#include "SoundEngine.h"
#include "pmx.h"

#include "debug.h"


/* #define MAX_BLOCKING */

#ifdef MAX_BLOCKING
#define MINBLOCKTIME(max_block,this_block) max_block = min(max_block,this_block)
#else
#define MINBLOCKTIME(max_block,this_block)
#endif


/*
 * notes:
 * * For a 5 degree turning angle, set the rotate_delay to 10
 */

/*
 * Limits/other definitions
 */
#define MIN_WORM_LENGTH                  8           /* The length of the worm core body */
#define MAX_WORM_LENGTH                  1024
#define MAX_BULLETS                      128
#define MAX_EXPLOSIONS                   MAX_BULLETS*2
#define MAX_WORMS                        2
#define MAX_PLAYERS                      2

#define MIN_WORM_GROW                    4
#define MAX_WORM_GROW                    16

#define MAX_APPLES                       8

#define MAX_WRIGGLERS                    0
#define MAX_WRIGGLY_LENGTH               512

#define MAX_FRENZIES                     0
#define MAX_FRENZY_LENGTH                256

#define MAX_TRONS                        4

#define MIN_NEW_APPLES_WAIT              20000
#define MAX_NEW_APPLES_WAIT              72000

/*
 * table sizes
 */
#define WORM_SPEEDS                      5
#define WORM_FIRE_RATES                  5
#define BULLET_SPEEDS                    4
#define EXPLOSION_SIZES                  5


/* How many seconds does the game continue after a worm has died? */
#define DEAD_WORM_AFTERPLAY              2500

/*
 * Game state flags
 */
#define GAMEFLG_WORMDEAD                 0x00000001  /* Game is over */


/*
   Use a resolution of 512 - can be just about anything (the higher the better)
   NOTA BENE: INTTRIG_PIVOT MUST BE EQUAL TO TWO RAISED TO THE POWER OF INTTRIG_PIVOT_SHIFT!!!!
 */
#define INTTRIG_PIVOT                    512
#define INTTRIG_PIVOT_SHIFT              9

#define KEYSCAN_DELAY                    2


/* Colors */
#define BACKGROUND_COLOR                 0
#define BULLET_COLOR                     1
#define WORM_COLORS                      3
#define WORM1BASECOLOR                   2
#define WORM2BASECOLOR                   5
#define WORM3BASECOLOR                   8

#define TRON_HEAD                        20
#define TRON_TRAIL                       21

#define APPLE_COLOR                      50

#ifndef PI
#define PI 3.14159265358979323846264338327950288419716939937510
#define PI2 (PI*2)
#endif


/*
 * Game options/mostly flashyness
 */
#define OPT_MULTICOLOR_WORMS             0x00000002
#define OPT_DELAY                        0x00000004


/*
 * Blitter thread paramters block
 */
typedef struct _BLITTHREADPARAMS
{
   HEV hevInitCompleted;
   HDIVE hDive;
   PPIXELBUFFER pb;
   HEV hevPaint;
   HEV hevTermPainter;
   HEV hevPainterTerminated;
}BLITTHREADPARAMS, *PBLITTHREADPARAMS;


/*
 * Variable limits structure
 */
typedef struct _BOARDLIMITS
{
   SIZEL sizlBoard;                      /* Size of game field in pels */
   SIZEL sizlBigBoard;                   /* sizlBoard * INTTRIG_PIVOT */
   POINTL maxPos;                        /* Maximum position for any pixel */
   POINTL maxBigPos;                     /* Maximum position in the "big" board */
}BOARDLIMITS, *PBOARDLIMITS;


typedef struct _FPOINT
{
   float x;
   float y;
}FPOINT, *PFPOINT;


typedef struct _POINTHEADING
{
   int deg;
   POINTL dir;
   POINTL pos;
   POINTL vecPos;
}POINTHEADING, *PPOINTHEADING;

/*
 * WORM data
 */
typedef struct _WORMINFO
{
   int iHead;
   int iTail;
   int length;
   int iBaseColor;

   POINTL bigPos;                        /* multiplied by INTTRIG_PIVOT */
   POINTL dir;                           /* Direction (dx and dy) */
   int deg;                              /* Heading index 0 - 359, unaffected by wriggle */
   int adjust_deg;                       /* degree adjustment */
   int virtDeg;                          /* Heading index 0 - 359, affected by wriggle */

   ULONG next_movement;                  /* Timer: When worm can move next */
   ULONG next_shoot;                     /* Timer: When next shot can be issued */
   ULONG next_rotate;
   ULONG next_attaction;

   int iSpeed;                           /* Speed index (0 - WORM_SPEEDS) */
   int iFireRate;                        /* Fire rate index (0 - WORM_FIRE_RATES) */
   int iBulletSpeed;                     /* New bullet speed index (0 - BULLET_SPEEDS) */

   ULONG move_delay;                     /* How many milliseconds to wait before worm moves - taken from a global table using iSpeed */
   ULONG shoot_delay;                    /* Time which must elapse before worm can shoot again - taken from a global table using iFireRate */
   ULONG rotate_delay;

   POINTL aptl[MAX_WORM_LENGTH];         /* List of worm segment positions */
   int adeg[MAX_WORM_LENGTH];            /* Degree/direction of each worm segment (shouldn't be required!) */

   int rotFastKey;                       /* Rotate fast key */
   int rotLeftKey;                       /* Scancode for rotate left key */
   int rotRightKey;                      /* Scancode for ritate right key */
   int shootKey;                         /* Scancode for fire key */

   int bullets_per_shot;                 /* number of bullets fired for each shot (1, 3 or 5) */
   int bullet_bounces;
   int bullet_power;                     /* 0 - 5, in effect, the size of the resulting explosion */

   int grow;                             /* How much worm should grow */

   int iWriggleState;                    /* 0 - WRIGGLE_ANGLES */
   int iWriggleWait;

   ULONG flags;
}WORMINFO, *PWORMINFO;
#define WORMFLG_DEAD                     0x00000001
#define WORMFLG_AUTOFIRE                 0x00000002  /* Player can not stop firing */
#define WORMFLG_HOMING_BULLETS           0x00000004  /* Homing missiles! */
#define WORMFLG_PACIFIST                 0x00000008  /* Player can not fire shots */
#define WORMFLG_WRIGGLE                  0x00000010  /* Worm wriggles */
#define WORMFLG_FATAL_ATTRACTION         0x00000020
#define WORMFLG_WARPINGBULLETS           0x00000040  /* Bullets will wrap around borders */
#define WORMFLG_DRUNKEN_MISSILES         0x00000080


/*
 * Unimplemented feature
 */
typedef struct _FRENZY
{
   int iHead;
   int iTail;

   POINTL vecPos;

   POINTL aptl[MAX_FRENZY_LENGTH];
}FRENZY, *PFRENZY;

/*
 * Unimplemented feature
 */
typedef struct _WRIGGLY
{
   int iHead;
   int iTail;

   int baseDeg;
   int deg;

   POINTL vecPos;

   POINTL aptl[MAX_WRIGGLY_LENGTH];
}WRIGGLY, *PWRIGGLY;


typedef struct _TRONSTRATEGY
{
   int dist;                             /* How many pels to travel before switching direction */
   int next_scan;                        /* ToDo: randomize a number between 4 and 20. Count down each turn, and when it reaches zero
                                                  check if tron will crash into something if it continues. Makes trons survive longer. */
}TRONSTRATEGY, *PTRONSTRATEGY;

typedef struct _TRON
{
   BOOL fActive;
   POINTL pos;                           /* Current position */
   int dir;                              /* 0 - up, 1 - left, 2 - down, 3 - right */
   POINTL vector;                        /* heading dx/dy */
   ULONG tmNextMove;
   TRONSTRATEGY strategy;
}TRON, *PTRON;


typedef struct _BULLET
{
   BOOL fActive;                         /* Is slot used ? */
   int deg;                              /* 0 - 360 */
   POINTL bigPos;                        /* Position in "big world" */
   POINTL dir;                           /* Copied from inttrig_table[deg] */
   POINTL pos;                           /* Position on screen map */
   int delay;                            /* delay between movements */
   ULONG next_move;                      /* next move */
   int iCycle;
   int bounces;                          /* counts down each bounce, explodes at 0 */
   int power;                            /* indicated the size of the explosion, 0 - EXPLOSION_SIZES */
   int iTarget;                          /* Which worm fired the shot? Used to make sure homing bullets don't turn on self */
   ULONG flags;
}BULLET, *PBULLET;
#define BULFLG_HOMING                    0x00000001  /* Bullet homes in on enemies */
#define BULFLG_WARP                      0x00000002  /* Bullet warps */
#define BULFLG_SELFDEST                  0x00000004  /* Bullet's selfdestruct after a specified time */
#define BULFLG_DRUNKEN                   0x00000008


typedef struct _EXPLOSION
{
   BOOL fActive;
   POINTL center;                        /* Center of explosion */
   ULONG ulStartTime;                    /* What time explosion started */
   ULONG ulExpireTime;                   /* when explosion should expire */
   int grow;                             /* For each passed millisecond, grow this much */
   int power;                            /* 0 - 5 */
   int iColor;
   ULONG next_color;
   ULONG flags;
}EXPLOSION, *PEXPLOSION;
#define EXPFLG_PREPROCESSED              0x00000001


typedef struct _APPLECHANCES
{
   int max_rand_value;
   int chance[MaxAppleTypes];
}APPLECHANCES, *PAPPLECHANCES;




typedef struct _APPLE
{
   BOOL fActive;
   ULONG ulExpire;                       /* Apples become old and wither away */
   POINTL pos;
   enum AppleType type;
}APPLE, *PAPPLE;


enum {
   OutOfBoundLeft   = 0x00000001,
   OutOfBoundRight  = 0x00000002,
   OutOfBoundTop    = 0x00000004,
   OutOfBoundBottom = 0x00000008
};


enum PlayGameRet
{
   Worm1Died    = 0x00000001,
   Worm2Died    = 0x00000002,
   TerminateApp = 0x00000004
};

/*
 * Global variables
 */
static PPOINTL dir_table = NULL;
static PAPPLECHANCES appletype_randomization_data = NULL;
static PRLECIRCLE rle_explosion_data[EXPLOSION_SIZES] = { NULL, NULL, NULL, NULL, NULL };

const ULONG worm_speed_delays[WORM_SPEEDS] = { 20, 18, 14, 10, 6 };
const ULONG worm_shoot_delays[WORM_FIRE_RATES] = { 2000, 1000, 500, 400, 250 };
const ULONG bullet_speed_delays[BULLET_SPEEDS] = { 5, 4, 3, 2 };

#define WRIGGLE_ANGLES                   20
const int wriggle_deg[WRIGGLE_ANGLES] = { -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 16, 12, 8, 4, 0, -4, -8, -12, -16 };

const POINTL tron_dir[4] = { { 0, -1 }, { -1, 0 }, { 0, 1 }, { 1, 0 } };

static volatile PBYTE abKeyStates = NULL;


#define WORM_DIED_SAMPLES                5
#define WORM_ATE_APPLE_SAMPLES           5
typedef struct _SAMPLES
{
   PSAMPLE worm_died;                    /* "worm_die%d.wav"; when a worm croaks */
   PSAMPLE both_worms_died[2];           /* "both_worms_died%1.wav" when both worms croaked */
   PSAMPLE homing_in;                    /* "homing_in.wav"; when bullet is homing in */
   PSAMPLE worm_shoot;                   /* "worm_shoot.wav"; TODO: three different samples for three different launchers */
   PSAMPLE explosion[EXPLOSION_SIZES];   /* "explosion%d.wav"; whenever an explosion is created */
   PSAMPLE bullet_bounce_wall;
   PSAMPLE bullet_bounce_worm;
   PSAMPLE bullet_bounce_tron;
   PSAMPLE upgraded_weaponary;
   PSAMPLE worm_eating_apple[WORM_ATE_APPLE_SAMPLES];
   PSAMPLE ate_apple;
   PSAMPLE bullet_homing;
   PSAMPLE fatal_attraction;
}SAMPLES;
SAMPLES samples = { NULL };


/*
 * Function prototypes
 */
static BOOL _Optlink isInitialized(HAB hab);
static void _Optlink initFrameWindow(PAPPPRF prf, HWND hwndFrame, PSIZEL psizlGameBoard);
static void _Optlink initCompleted(HAB hab);
static BOOL _Optlink loadPlayersControls(HAB hab, PPLAYERKEYS playerKeys);


void _Inline draw_apple(PPIXELBUFFER pb, PPOINTL p);
void _Optlink clear_apple(PPIXELBUFFER pb, PPOINTL p);

static void init_worms(PWORMINFO worms, PBOARDLIMITS limits);
static void init_worm(PWORMINFO worm, PPOINTL pp, int deg);




long _Inline flround(float f);
long _Inline dlround(double d);

double _Inline deg_to_rad(LONG deg);
int _Inline rad_to_deg(double r);

int _Inline irandom(int minimum, int maximum);

BOOL _Inline is_within_radius(PPOINTL center, PPOINTL p, LONG radius);


void _Inline calc_new_pos(PPOINTL newpos, PPOINTL vecPos, PPOINTL dir);



int _Inline pointsequ(PPOINTL p1, PPOINTL p2);
int _Inline bigpointsequ(PPOINTL p1, PPOINTL p2);


void _Inline reflect_bullet(PBULLET bullet, int deg);


int _Inline add_deg(int deg, int a);
int _Inline sub_deg(int deg, int a);
int _Inline adjust_deg(int deg, int a);


void setup_limits(PBOARDLIMITS limits, PSIZEL sizlGameBitmap);



/****************************************************************************\
 *                      W O R M  A C T I O N                                *
 * todo: prefix with worm_*                                                 *
\****************************************************************************/
static void worm_shoot(PPIXELBUFFER pb, PWORMINFO worm, int iWorm, BULLET bullets[], ULONG thisTime, PULONG pcActiveBullets);
BOOL _Inline is_worm_body_within_explosion(PWORMINFO worm, PEXPLOSION explosion);
void _Inline check_tail_explosion(PWORMINFO worm, PEXPLOSION explosion);


/****************************************************************************\
 *             B U L L E T S  A N D  E X P L O S I O N S                    *
\****************************************************************************/
PEXPLOSION _Inline bullet2explosion(PBULLET bullet, EXPLOSION explosions[], ULONG thisTime, PULONG pcBullets, PULONG pcExplosions, int iFirstExplosion);

void _Inline alloc_bullet(BULLET bullets[], PPOINTL virtPos, int deg, int iBulletSpeed, ULONG thisTime, int bounces, int power, PULONG pcBullets, ULONG flags, int iTarget);
void _Inline init_bullet_2(PBULLET bullet, PPOINTL virtPos, int deg, int iBulletSpeed, ULONG thisTime, int bounces, int power, ULONG flags, int iTarget);
void _Inline init_bullet(PBULLET bullet, PPOINTL pos, int deg, int iBulletDelay, ULONG thisTime, int bounce, int power);

PEXPLOSION _Inline alloc_explosion(PEXPLOSION explosions, PPOINTL center, ULONG thisTime, ULONG power, PULONG pcExplosions, int i);
void _Inline init_explosion(PEXPLOSION explosion, PPOINTL center, ULONG thisTime, ULONG size);


/****************************************************************************\
 *           G A M E  A C T I O N  D A T A  G E N E R A T I O N             *
\****************************************************************************/
enum AppleType _Optlink random_apple_type(void);
static BOOL _Optlink create_tron(TRON trons[], PPOINTL pos, int dir, PULONG cActiveTrons, ULONG thisTime);

/****************************************************************************\
 *                E X P L O S I O N  D R A W I N G                          *
\****************************************************************************/
void _Inline draw_explosion(PPIXELBUFFER pb, PPOINTL center, PRLECIRCLE prc, BYTE color, PBOARDLIMITS limits);
void _Inline clear_explosion(PPIXELBUFFER pb, PPOINTL center, PRLECIRCLE prc, BYTE color, PBOARDLIMITS limits);


/****************************************************************************\
 *              T H R E A D  I N I T I A L I Z A T I O N                    *
 *                    A N D  T E R M I N A T I O N                          *
\****************************************************************************/
static BOOL _Optlink generate_deg_vec_table(void);
static void _Optlink destroy_deg_vec_table(void);
static BOOL _Optlink initialize_dive_palette(HDIVE hDive);
static BOOL _Optlink generate_apple_randomization_data(void);
static void _Optlink destroy_apple_randomization_data(void);
static BOOL _Optlink generate_rle_explosion_data(void);
static void _Optlink destroy_rle_explosion_data(void);
static int _Optlink startBlitterThread(HDIVE hDive, PPIXELBUFFER pb, HEV hevPaint, HEV hevTermPainer, HEV hevPainterTerminated);
static BOOL _Optlink terminateBlitterThread(int tidBlitter, HEV hevPaint, HEV hevTermPainter, HEV hevPainterTerminated);


/****************************************************************************\
 *             H I G H  R E S O L U T I O N  T I M E R                      *
\****************************************************************************/
static PULONG _Optlink hrtInitialize(PHFILE phTimer);
static BOOL _Optlink hrtTerminate(HFILE hTimer, PULONG pulTimer);
void _Inline hrtBlock(HFILE hTimer, ULONG ulDelay);


/****************************************************************************\
 *            W O R M  A T T R I B U T E  M O D I F I E R S                 *
\****************************************************************************/
int _Inline inc_worm_speed(PWORMINFO worm);
int _Inline dec_worm_speed(PWORMINFO worm);
int _Inline inc_worm_firerate(PWORMINFO worm);
int _Inline dec_worm_firerate(PWORMINFO worm);
int _Inline inc_worm_bullet_speed(PWORMINFO worm);
int _Inline dec_worm_bullet_speed(PWORMINFO worm);
int _Inline inc_worm_bulletpower(PWORMINFO worm);
int _Inline dec_worm_bulletpower(PWORMINFO worm);
int _Inline inc_worm_bullets_per_shot(PWORMINFO worm);
int _Inline dec_worm_bullets_per_shot(PWORMINFO worm);
int _Inline inc_worm_bullets_bounce(PWORMINFO worm);
int _Inline dec_worm_bullets_bounce(PWORMINFO worm);


/****************************************************************************\
 *                     B L I T T E R  T H R E A D                           *
\****************************************************************************/
static void _Optlink BlitThread(void *param);


/****************************************************************************\
 *                S T A T U S  B A R  F U N C T I O N S                     *
\****************************************************************************/
static void _Optlink showAppleStatus(HWND hwndStatus, enum AppleType apple);


/****************************************************************************\
 *                 S A M P L E S   I N I T / T E R M                        *
\****************************************************************************/
static void _Optlink loadSamples(void*);
static void _Optlink releaseSamples(void);


/****************************************************************************\
 *                G A M E  T H R E A D  W A T C H  D O G                    *
\****************************************************************************/
#ifdef DEBUG
ULONG GameThreadState = 0;
HEV hevHeartBeat = NULLHANDLE;
static void _Optlink WatchDog(void*);
#endif




static ULONG _Optlink PlayGame(HAB hab, PPIXELBUFFER pb, HWND hwnd, HFILE hTimer, PULONG pHRT, HEV hevPaint);


void _Optlink GameThread(void *param)
{
   APIRET rc = NO_ERROR;
   PGAMETHREADPARAMS threadParams = (PGAMETHREADPARAMS)param;

   /* Thread parameters */
   HWND hwnd = threadParams->hwnd;
   HDIVE hDive = threadParams->hDive;
   SIZEL sizlGameBitmap = { threadParams->sizlGameBitmap.cx, threadParams->sizlGameBitmap.cy };

   /* OS/2 PM variables */
   HAB hab = NULLHANDLE;
   HMQ hmq = NULLHANDLE;

   HWND hwndFrame = NULLHANDLE;
   HWND hwndMenu = NULLHANDLE;
   HWND hwndStatus[2] = { NULLHANDLE, NULLHANDLE };

   /* Bitmap and blitter variables */
   PPIXELBUFFER pb = NULLHANDLE;
   int tidBlitter = -1;
   HEV hevPaint = NULLHANDLE;
   HEV hevTermPainter = NULLHANDLE;
   HEV hevPainterTerminated = NULLHANDLE;

   BOOL fTerminate = FALSE;
   BOOL fSuccess = FALSE;

   /* High resolution timer */
   HFILE hTimer = NULLHANDLE;
   PULONG pulTimer = NULL;

   BOARDLIMITS limits = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

   /*
    * Intro data
    */
   BOOL fIntroRunning = FALSE;
   PBULLET bullets = NULL;
   PWORMINFO worms = NULL;
   PEXPLOSION explosions = NULL;
   ULONG cActiveBullets = 0;
   ULONG cActiveExplosions = 0;
   int iCurrentDeg = 0;
   ULONG tmNextBullet = 0;
   POINTL center = { 0, 0 };
   ULONG acSurvived[MAX_WORMS] = { 0, 0 };
   ULONG acDied[MAX_WORMS] = { 0, 0 };
   ULONG acWon[MAX_WORMS] = { 0, 0 };
   ULONG cDraw = 0;
   ULONG cPlayedGames = 0;

   BOOL fMixerActive = FALSE;
   LONG lMixerVolume = 85;

   PAPPPRF prf = NULL;

   /*
    * Statusbar stuff
    */
   ULONG tmNextStatusMessage = 0;
   ULONG iCurrentMessage = 0;

   abKeyStates = threadParams->abKeyStates;

   memset(&samples, 0x00, sizeof(samples));

   /*
    * Tell creator that we've done the preliminary initialization and no longer require the threadparams buffer.
    */
   if((rc = DosPostEventSem(threadParams->hevInitCompleted)) == NO_ERROR)
   {
      threadParams = NULL;
   }
   else
   {
      _endthread();
   }

   /*
    * Don't allocate any resources before this point!
    */

   #ifdef DEBUG
   GameThreadState = 0x00000000;
   DosCreateEventSem(NULL, &hevHeartBeat, 0UL, FALSE);
   _beginthread(WatchDog, NULL, 8192, NULL);
   #endif

   /*
    * Initialize PM for this thread
    */
   if(threadParams == NULL)
   {
      hab = WinInitialize(0UL);
   }
   if(hab)
   {
      hmq = WinCreateMsgQueue(hab, 0);
   }
   if(hmq == NULLHANDLE)
   {
      if(hab)
      {
         WinTerminate(hab);
      }
      _endthread();
   }

   /*
    * Tell the application client window about this thread's message queue
    */
   WinSendMsg(hwnd, WMU_INFORM, MPFROMLONG(GAMETHREAD_HMQ), MPFROMLONG(hmq));


   /*
    * Get frame and frame control handles
    */
   hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
   if(hwndFrame)
   {
      hwndMenu =  WinWindowFromID(hwndFrame, FID_MENU);
      hwndStatus[0] = WinWindowFromID(hwndFrame, FID_LEFTSTATUSBAR);
      hwndStatus[1] = WinWindowFromID(hwndFrame, FID_RIGHTSTATUSBAR);
   }


   /*
    * Allocate some global static data
    */
   fSuccess = FALSE;
   if(generate_deg_vec_table())
   {
      if(generate_apple_randomization_data())
      {
         if(generate_rle_explosion_data())
         {
            fSuccess = TRUE;
         }
      }
   }

   /*
    * Do a preliminary check if sound is available
    */
   if(isMixerAvailable())
   {
      /* Sound is available -- enable menuitems for controlling sound! */
      EnableMenuItems(hwndMenu, IDM_SET_SOUND, IDM_SET_SOUNDVOL, TRUE);
   }

   /*
    * Read settings from profile
    */
   prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
   if(prf)
   {
      initFrameWindow(prf, hwndFrame, &sizlGameBitmap);

      if(WinIsMenuItemEnabled(hwnd, IDM_SET_SOUND))
      {
         /*
          * The sound effects menu is enabled, which means that sound can be used
          */
         if(readProfileBoolean(prf, IDS_PRKKEY_SOUNDFX, TRUE))
         {
            WinPostQueueMsg(hmq, GTHRDMSG_ENABLE_SOUNDFX, MPFROMLONG(TRUE), MPVOID);
         }
         lMixerVolume = readProfileLong(prf, IDS_PRFKEY_SOUNDFX_VOLUME, lMixerVolume);
      }
      closeAppProfile(prf);
   }


   if(fSuccess)
   {
      pulTimer = hrtInitialize(&hTimer);
   }

   if(pulTimer)
   {
      srand(*pulTimer);
   }

   /*
    * Initialize graphics
    *    - Initialize palette
    *    - Create blitter event notification semaphores
    *    - Launch the blitter thread
    */
   if(pulTimer)
   {
      if(initialize_dive_palette(hDive))
      {
         /*
          * Create blitter thread sempahores
          */
         if((rc = DosCreateEventSem(NULL, &hevPaint, 0UL, FALSE)) == NO_ERROR)
         {
            if((rc = DosCreateEventSem(NULL, &hevTermPainter, 0UL, FALSE)) == NO_ERROR)
            {
               rc = DosCreateEventSem(NULL, &hevPainterTerminated, 0UL, FALSE);
            }
         }
         if(rc == NO_ERROR)
         {
            if((pb = CreateDivePixelBuffer(hDive, sizlGameBitmap.cx, sizlGameBitmap.cy)) != NULL)
            {
               tidBlitter = startBlitterThread(hDive, pb, hevPaint, hevTermPainter, hevPainterTerminated);
            }
         }
      }
   }

   setup_limits(&limits, &sizlGameBitmap);

   if((bullets = calloc(MAX_BULLETS, sizeof(BULLET))) != NULL)
   {
      memset(bullets, 0, MAX_BULLETS*sizeof(BULLET));
   }
   WinPostQueueMsg(hmq, GTHRDMSG_INIT_INTRO, MPVOID, MPVOID);

   if(!isInitialized(hab))
   {
      WinPostMsg(hwnd, WMU_FIRST_RUN, MPVOID, MPVOID);
   }
   else
   {
      WinEnableMenuItem(hwndMenu, IDM_GAME_START, TRUE);
   }

   /*
    * Initialize intro data
    */
   while(!fTerminate)
   {
      ULONG thisTime = *pulTimer;
      QMSG qmsg;
      BOOL fBlit = FALSE;

      DosSleep(0);

      if(WinPeekMsg(hab, &qmsg, NULLHANDLE, 0UL, 0UL, PM_REMOVE))
      {
         ULONG playRet;
         BOOL fMixerAvailable;

         switch(qmsg.msg)
         {
            case WM_PAINT:
               memset(pb->afLineMask, 0xff, pb->cy);
               fBlit = TRUE;
               break;

            case GTHRDMSG_INITGAME_COMPLETE:
               initCompleted(hab);
               break;

            case GTHRDMSG_START_GAME:
               fMixerAvailable = FALSE;
               if(WinIsMenuItemEnabled(hwndMenu, IDM_SET_SOUND))
               {
                  fMixerAvailable = TRUE;
               }

               /* ToDo: Don't disable sound options unless sound is available */
               EnableMenuItems(hwndMenu, IDM_SET_BOARDSIZE, IDM_SET_SOUNDVOL, FALSE);

               /*
                * New statusbar window message: SBMSG_FADEOUT?
                */
               WinSetWindowText(hwndStatus[0], "");
               WinSetWindowText(hwndStatus[1], "");

               WinSendMsg(hwndStatus[0], SBMSG_SETAUTODIM, MPFROMLONG(TRUE), MPVOID);
               WinSendMsg(hwndStatus[1], SBMSG_SETAUTODIM, MPFROMLONG(TRUE), MPVOID);

               playRet = PlayGame(hab, pb, hwnd, hTimer, pulTimer, hevPaint);
               cPlayedGames++;
               if(playRet & Worm1Died)
               {
                  acDied[0]++;
               }
               else
               {
                  acSurvived[0]++;
               }
               if(playRet & Worm2Died)
               {
                  acDied[1]++;
               }
               else
               {
                  acSurvived[1]++;
               }

               if((playRet & (Worm1Died|Worm2Died)) == (Worm1Died|Worm2Died))
               {
                  cDraw++;
               }

               if(!(playRet & Worm1Died) && (playRet & Worm2Died))
               {
                  acWon[0]++;
               }
               if((playRet & Worm1Died) && !(playRet & Worm2Died))
               {
                  acWon[1]++;
               }

               fIntroRunning = FALSE;

               if(playRet & TerminateApp)
               {
                  fTerminate = TRUE;
               }
               else
               {
                  /*
                   * Reactivate intro
                   */
                  WinPostQueueMsg(hmq, GTHRDMSG_INIT_INTRO, MPVOID, MPVOID);

                  /* ToDo: Don't enable sound options unless sound is available */
                  if(fMixerAvailable)
                  {
                     EnableMenuItems(hwndMenu, IDM_SET_BOARDSIZE, IDM_SET_SOUNDVOL, TRUE);
                  }
                  else
                  {
                     EnableMenuItems(hwndMenu, IDM_SET_BOARDSIZE, IDM_SET_DOUBLE, TRUE);
                  }
               }
               break;

            case GTHRDMSG_NEW_BITMAP_SIZE:
               dputs(("GTHRDMSG_NEW_BITMAP_SIZE"));
               /* Stop blitter thread before attempting to destroy the data it's blitting */
               terminateBlitterThread(tidBlitter, hevPaint, hevTermPainter, hevPainterTerminated);

               /* Destroy the pixel buffer */
               DestroyDivePixelBuffer(pb);

               /* Reinitialize graphics (including blitter) */
               sizlGameBitmap.cx = LONGFROMMP(qmsg.mp1);
               sizlGameBitmap.cy = LONGFROMMP(qmsg.mp2);

               pb = CreateDivePixelBuffer(hDive, sizlGameBitmap.cx, sizlGameBitmap.cy);
               tidBlitter = startBlitterThread(hDive, pb, hevPaint, hevTermPainter, hevPainterTerminated);
               setup_limits(&limits, &sizlGameBitmap);

               fIntroRunning = FALSE;

               WinPostQueueMsg(hmq, GTHRDMSG_INIT_INTRO, MPVOID, MPVOID);
               WinPostMsg(hwnd, WMU_RESET_WINDOW, MPVOID, MPVOID);
               break;

            case GTHRDMSG_INIT_INTRO:
               memset(pb->data, 0, pb->cbBitmap);
               memset(pb->afLineMask, 0xff, pb->cy);
               fBlit = TRUE;
               center.x = sizlGameBitmap.cx/2;
               center.y = sizlGameBitmap.cy/2;
               tmNextBullet = thisTime;
               memset(bullets, 0, MAX_BULLETS*sizeof(BULLET));
               cActiveBullets = 0;
               fIntroRunning = TRUE;

               EnableMenuItems(hwndMenu, IDM_GAME, IDM_GAME_START, TRUE);
               EnableMenuItems(hwndMenu, IDM_SETTINGS, IDM_SET_DOUBLE, TRUE);

               tmNextStatusMessage = thisTime;

               WinSendMsg(hwndStatus[0], SBMSG_SETAUTODIM, MPFROMLONG(FALSE), MPVOID);
               WinSendMsg(hwndStatus[1], SBMSG_SETAUTODIM, MPFROMLONG(FALSE), MPVOID);
               break;

            case GTHRDMSG_ENABLE_SOUNDFX:
               if(LONGFROMMP(qmsg.mp1))
               {
                  dputs(("enable sound"));
                  if(startMixer() && !fMixerActive)
                  {
                     mixerSetVolume(lMixerVolume);
                     _beginthread(loadSamples, NULL, 8192, NULL);
                     dputs(("mixer active!"));
                     fMixerActive = TRUE;

                     EnableMenuItems(hwndMenu, IDM_SET_SOUND, IDM_SET_SOUNDVOL, TRUE);
                     WinCheckMenuItem(hwndMenu, IDM_SET_SOUNDFX, TRUE);
                  }
               }
               else
               {
                  if(stopMixer())
                  {
                     fMixerActive = FALSE;
                     WinCheckMenuItem(hwndMenu, IDM_SET_SOUNDFX, FALSE);
                     releaseSamples();
                     memset(&samples, 0x00, sizeof(samples));
                  }
               }
               writeProfileBoolean(prf, IDS_PRKKEY_SOUNDFX, WinIsMenuItemChecked(hwndMenu, IDM_SET_SOUNDFX));
               break;

            case GTHRDMSG_SOUNDVOL_DLG:
               WinPostMsg(hwnd, WMU_VOLUME_DIALOG, MPFROMLONG(lMixerVolume), MPVOID);
               break;

            case GTHRDMSG_SET_SFX_VOLUME:
               lMixerVolume = LONGFROMMP(qmsg.mp1);
               if(fMixerActive)
               {
                  mixerSetVolume(lMixerVolume);
               }
               if(prf)
               {
                  writeProfileLong(prf, IDS_PRFKEY_SOUNDFX_VOLUME, lMixerVolume);
               }
               break;

            case GTHRDMSG_TERMINATE:
               dputs(("GameThread received GTHRDMSG_TERMINATE message. setting fTerminate=TRUE"));
               fTerminate = TRUE;
               continue;
         } /* switch(qmsg.msg) */
      }  /* if(WinPeekMsg(hab, &qmsg, NULLHANDLE, 0UL, 0UL, PM_REMOVE)) */

      if(fIntroRunning)
      {
         int iBullet = 0;
         ULONG cProcessed = 0;

         if(thisTime >= tmNextBullet)
         {
            /* Launch new bullet */
            for(iBullet = 0; iBullet < MAX_BULLETS; iBullet++)
            {
               if(!bullets[iBullet].fActive)
               {
                  init_bullet(&bullets[iBullet], &center, iCurrentDeg, 4, thisTime, 0, 0);
                  cActiveBullets++;
                  break;
               }
            }

            /* Rotate launcher */
            iCurrentDeg = sub_deg(iCurrentDeg, 5);

            tmNextBullet = thisTime + 50;
         }

         if(thisTime >= tmNextStatusMessage)
         {
            int i = 0;

            char msg[256] = "";
            switch(iCurrentMessage)
            {
               case 0:
                  if(cPlayedGames > 0)
                  {
                     sprintf(msg, "%d game%s played this session", cPlayedGames, cPlayedGames != 1 ? "s" : "");
                     WinSetWindowText(hwndStatus[0], msg);
                     sprintf(msg, "%d game%s ended in a draw", cDraw, cDraw != 1 ? "s" : "");
                     WinSetWindowText(hwndStatus[1], msg);
                     iCurrentMessage++;
                     tmNextStatusMessage = thisTime + 3000;
                  }
                  break;

               case 1:
                  for(i = 0; i < MAX_PLAYERS; i++)
                  {
                     sprintf(msg, "Worm %d has won %d game%s", i+1, acWon[i], acWon[i] != 1 ? "s" : "");
                     WinSetWindowText(hwndStatus[i], msg);
                  }
                  tmNextStatusMessage = thisTime + 3000;
                  iCurrentMessage = 0;
                  break;
            }
         }

         /* Process active bullets */
         for(iBullet = 0, cProcessed = 0; iBullet < MAX_BULLETS; iBullet++)
         {
            PBULLET bullet = &bullets[iBullet];
            POINTL ptlNew;
            POINTL newVirtPos;
            BYTE pix;
            ULONG flBound;

            if(cProcessed == cActiveBullets)
               break;
            if(!bullet->fActive || thisTime < bullet->next_move)
               continue;

            /* Clear previous bullet */
            setPixel(pb, &bullet->pos, 0);

recheck_bullet:
            flBound = 0;

            /*
             * Calculate new virtual position
             */
            newVirtPos.x = bullet->bigPos.x + bullet->dir.x;
            newVirtPos.y = bullet->bigPos.y + bullet->dir.y;

            /*
             * Check if bullet is out of bound (using temporary virtual point)
             */
            if(newVirtPos.x < 0)
               flBound |= OutOfBoundLeft;
            if(newVirtPos.x > limits.maxBigPos.x)
               flBound |= OutOfBoundRight;
            if(newVirtPos.y < 0)
               flBound |= OutOfBoundTop;
            if(newVirtPos.y > limits.maxBigPos.y)
               flBound |= OutOfBoundBottom;

            if(flBound)
            {
               /*
                * Bullet would be out of bound if it continues on it's present course
                */
               if(bullet->bounces)
               {
                  /*
                   * Bounce bullet off wall(s)
                   */
                  if(flBound & (OutOfBoundLeft|OutOfBoundRight))
                  {
                     bullet->dir.x = -bullet->dir.x;
                  }
                  if(flBound & (OutOfBoundBottom|OutOfBoundTop))
                  {
                     bullet->dir.y = -bullet->dir.y;
                  }

                  /*
                   * Decrease number of bounces remaining and reevaluate bullet
                   * position.
                   */
                  bullet->bounces--;
                  goto recheck_bullet;
               }
               /* bullet2explosion(bullet, explosions, thisTime, &cActiveBullets, &cActiveExplosions); */
               bullet->fActive = FALSE;
               cActiveBullets--;
               continue;
            }
            ptlNew.x = newVirtPos.x>>INTTRIG_PIVOT_SHIFT;      /* ptlNew.x = newVirtPos.x/INTTRIG_PIVOT; */
            ptlNew.y = newVirtPos.y>>INTTRIG_PIVOT_SHIFT;      /* ptlNew.y = newVirtPos.y/INTTRIG_PIVOT; */

            /*
             * Store new position in the bullet's data structure
             */
            bullet->bigPos.x = newVirtPos.x;
            bullet->bigPos.y = newVirtPos.y;
            bullet->pos.x = ptlNew.x;
            bullet->pos.y = ptlNew.y;

            setPixel(pb, &bullet->pos, BULLET_COLOR);

            bullet->next_move = thisTime + bullet->delay;

            fBlit = TRUE;

            cProcessed++;
         }

         /*
          * Draw Logo ?
          */
      }

      /* Tell blitter to blit any changes */
      if(fBlit)
      {
         DosPostEventSem(hevPaint);
      }

      #ifdef DEBUG
      DosPostEventSem(hevHeartBeat);
      #endif

      hrtBlock(hTimer, 4);

   } /* while(!fTerminate) */

   dputs(("GameThread is no longer in the while(!fTerminate) loop!"));

   /*
    * Store application settings
    */
   prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
   if(prf)
   {
      SWP swp = { 0 };

      if(WinQueryWindowPos(hwndFrame, &swp))
      {
         POINTL p = { swp.x, swp.y };
         writeProfileData(prf, IDS_PRFKEY_FRAMEPOS, &p, sizeof(p));
      }

      writeProfileBoolean(prf, IDS_PRFKEY_DOUBLE_SIZE, WinIsMenuItemChecked(hwndMenu, IDM_SET_DOUBLE));

      if(WinIsMenuItemEnabled(hwnd, IDM_SET_SOUND))
      {
         writeProfileBoolean(prf, IDS_PRKKEY_SOUNDFX, WinIsMenuItemChecked(hwndMenu, IDM_SET_SOUNDFX));
         writeProfileLong(prf, IDS_PRFKEY_SOUNDFX_VOLUME, lMixerVolume);
      }

      closeAppProfile(prf);
   }

   if(pulTimer)
   {
      hrtTerminate(hTimer, pulTimer);
   }

   if(bullets)
   {
      free(bullets);
   }

   terminateBlitterThread(tidBlitter, hevPaint, hevTermPainter, hevPainterTerminated);
   rc = DosCloseEventSem(hevPainterTerminated);
   rc = DosCloseEventSem(hevTermPainter);
   rc = DosCloseEventSem(hevPaint);

   if(pb)
   {
      DestroyDivePixelBuffer(pb);
   }

   destroy_rle_explosion_data();
   destroy_apple_randomization_data();
   destroy_deg_vec_table();

   if(hab)
   {
      if(hmq)
      {
         WinDestroyMsgQueue(hmq);
      }
      WinTerminate(hab);
   }

   if(fMixerActive)
   {
      if(stopMixer())
      {
         releaseSamples();
      }
   }

   dputs(("GameThread is about to inform client window of it's termination state"));

   WinPostMsg(hwnd, WMU_INFORM, MPFROMLONG(GAMETHREAD_TERMINATED), MPVOID);

   _endthread();
}


static BOOL _Optlink isInitialized(HAB hab)
{
   LONG lLength = 0;
   char tmp[256] = "";
   HINI hIni = NULLHANDLE;
   BOOL fInitialized = FALSE;
   PAPPPRF prf = NULL;

   if((prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP)) != NULL)
   {
      if(readProfileBoolean(prf, IDS_PRFKEY_FIRST_RUN, FALSE))
      {
         fInitialized = TRUE;
      }
      closeAppProfile(prf);
   }
   return fInitialized;
}


static void _Optlink initFrameWindow(PAPPPRF prf, HWND hwndFrame, PSIZEL psizlGameBoard)
{
   SWP swp = { 0 };
   RECTL rclClient = { 0, 0, 0, 0 };
   HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
   POINTL p = { 0, 0 };
   ULONG cbData = sizeof(p);
   SWP swpDesktop = { 0 };
   SIZEL sizlClientDefault = { 320, 240 };

   if(readProfileBoolean(prf, IDS_PRFKEY_DOUBLE_SIZE, TRUE))
      WinCheckMenuItem(hwndMenu, IDM_SET_DOUBLE, TRUE);

   /*
    * Get default size of frame window (size of desktop)
    */
   if(WinQueryWindowPos(HWND_DESKTOP, &swpDesktop))
   {
      rclClient.xRight = swpDesktop.cx;
      rclClient.yTop = swpDesktop.cy;

      /* Calculate client size from a maximum frame size */
      WinCalcFrameRect(hwndFrame, &rclClient, TRUE);

      sizlClientDefault.cx = rclClient.xRight - rclClient.xLeft;
      sizlClientDefault.cy = rclClient.yTop - rclClient.yBottom;
      if(WinIsMenuItemChecked(hwndMenu, IDM_SET_DOUBLE))
      {
         sizlClientDefault.cx /= 2;
         sizlClientDefault.cy /= 2;
      }
   }
   else
   {
      swpDesktop.cx = 640;
      swpDesktop.cy = 480;
   }

   psizlGameBoard->cx = readProfileLong(prf, IDS_PRFKEY_BOARD_WIDTH, sizlClientDefault.cx);
   psizlGameBoard->cy = readProfileLong(prf, IDS_PRFKEY_BOARD_HEIGHT, sizlClientDefault.cy);

   memset(&rclClient, 0x00, sizeof(rclClient));

   rclClient.xRight = psizlGameBoard->cx;
   rclClient.yTop = psizlGameBoard->cy;

   if(WinIsMenuItemChecked(hwndMenu, IDM_SET_DOUBLE))
   {
      rclClient.xRight *= 2;
      rclClient.yTop *= 2;
   }

   WinCalcFrameRect(hwndFrame, &rclClient, FALSE);

   swp.cx = rclClient.xRight-rclClient.xLeft;
   swp.cy = rclClient.yTop-rclClient.yBottom;

   if(readProfileData(prf, IDS_PRFKEY_FRAMEPOS, &p, &cbData))
   {
      swp.x = p.x;
      swp.y = p.y;
   }
   else
   {
      swp.x = (swpDesktop.cx/2)-(swp.cx/2);
      swp.y = (swpDesktop.cy/2)-(swp.cy/2);
   }

   WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE);
}


static void _Optlink initCompleted(HAB hab)
{
   PAPPPRF prf = NULL;

   if((prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP)) != NULL)
   {
      writeProfileBoolean(prf, IDS_PRFKEY_FIRST_RUN, TRUE);
      closeAppProfile(prf);
   }
}


static BOOL _Optlink loadPlayersControls(HAB hab, PPLAYERKEYS playerKeys)
{
   BOOL fSuccess = FALSE;
   PAPPPRF prf = NULL;

   if((prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP)) != NULL)
   {
      int i = 0;
      fSuccess = TRUE;
      for(; i < MAX_WORMS; i++)
      {
         if(!loadPlayerKeys(hab, prf, i, &playerKeys[i]))
         {
            fSuccess = FALSE;
            break;
         }
      }
   }
   return fSuccess;
}


/*
static void _Optlink last_match_status(ULONG flLastGame, HWND hwndStatus[])
{
   const char achText[128] = "";

   // If no worm craoked, then last game was either interrupted or none have been played yet
   if(flLastGame & (Worm1Died|Worm2Died) == 0)
      return;

   if(lastGame & (Worm1Died|Worm2Died) == (Worm1Died|Worm2Died))
   {
      strcpy(achText, "Last game ended in draw");
      WinSetWindowText(hwndStatus[0], achText);
      WinSetWindowText(hwndStatus[1], achText);
   }
   else
   {
      if(lastGame & Worm1Died)
      {
         strcpy(achText, "Worm 1 was killed in previous game");
      }
      else
      {
         strcpy(achText, "Worm 1 survived last game");
      }
      WinSetWindowText(hwndStatus[0], achText);

      if(lastGame & Worm2Died)
      {
         strcpy(achText, "Worm 2 was killed in previous game");
      }
      else
      {
         strcpy(achText, "Worm 2 survived last game");
      }
      WinSetWindowText(hwndStatus[1], achText);
   }
}

static void _Optlink app_session_match_status(ULONG acWins[], HWND hwndStatus[])
{
   const char achText[128] = "";

   for(i = 0; i < MAX_WORMS; i++)
   {
      sprintf(achText, "Worm has survived %d of %d played matches", acWins);
      WinSetWindowText(hwndStatus[i], achText);
   }
}
*/




static ULONG _Optlink PlayGame(HAB hab, PPIXELBUFFER pb, HWND hwnd, HFILE hTimer, PULONG pHRT, HEV hevPaint)
{
   PBULLET bullets = NULL;
   PWORMINFO defaultWorm = NULL;
   PWORMINFO worms = NULL;
   PEXPLOSION explosions = NULL;
   PTRON trons = NULL;

   ULONG cActiveBullets = 0;
   ULONG cActiveExplosions = 0;
   ULONG cActiveTrons = 0;

   ULONG flReturn = 0;

   BOOL fGameOver = FALSE;

   ULONG flGameState = 0;

   BOARDLIMITS limits;
   SIZEL sizlGameBitmap = { pb->cx, pb->cy };

   /* PM variables */
   HWND hwndFrame = NULLHANDLE;
   HWND hwndMenu = NULLHANDLE;
   HWND hwndStatus[2] = { NULLHANDLE, NULLHANDLE };

   /* Various timers */
   ULONG tmNextApple = 0;
   ULONG tmNextKeyScan = 0;
   ULONG tmEndGame = 0;

   PLAYERKEYS keys[MAX_PLAYERS];

   int i = 0;

   POINTL ptl = { 0, 0 };

   USHORT **deg_map;

   /*
    * Clear bitmap
    */
   memset(pb->data, 0, pb->cbBitmap);
   memset(pb->afLineMask, 0xff, pb->cy);

   /*
    * Get frame and frame control handles
    */
   hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
   if(hwndFrame)
   {
      hwndMenu =  WinWindowFromID(hwndFrame, FID_MENU);
      hwndStatus[0] = WinWindowFromID(hwndFrame, FID_LEFTSTATUSBAR);
      hwndStatus[1] = WinWindowFromID(hwndFrame, FID_RIGHTSTATUSBAR);
   }

   /*
    * Initialize worms
    */
   worms = calloc(MAX_WORMS, sizeof(WORMINFO));
   if(worms)
   {
      memset(worms, 0, MAX_WORMS*sizeof(WORMINFO));

      bullets = calloc(MAX_BULLETS, sizeof(BULLET));
   }

   if(bullets)
   {
      memset(bullets, 0, MAX_BULLETS*sizeof(BULLET));

      explosions = calloc(MAX_EXPLOSIONS, sizeof(EXPLOSION));
   }

   if(explosions)
   {
      memset(explosions, 0, MAX_EXPLOSIONS*sizeof(EXPLOSION));

      trons = calloc(MAX_TRONS, sizeof(TRON));
   }

   if(trons)
   {
      memset(trons, 0, MAX_TRONS*sizeof(TRON));
   }

   setup_limits(&limits, &sizlGameBitmap);

   /*
    * Allocate a map which will be used for keeping track of angles.
    */
   deg_map = calloc(limits.sizlBoard.cy, sizeof(USHORT*));
   if(deg_map)
   {
      memset(deg_map, 0x00, limits.sizlBoard.cy*sizeof(USHORT*));
      for(i = 0; i < limits.sizlBoard.cy; i++)
      {
         deg_map[i] = calloc(limits.sizlBoard.cx, sizeof(USHORT));
         memset(deg_map[i], 0x00, limits.sizlBoard.cx*sizeof(USHORT));
      }
   }


   init_worms(worms, &limits);

   for(i = 0; i < MAX_WORMS; i++)
   {
      worms[i].next_rotate = *pHRT;
      worms[i].next_movement = *pHRT;
      worms[i].next_shoot = *pHRT;
   }
   tmNextApple = *pHRT + irandom(2000, 10000);
   tmNextKeyScan = *pHRT;
   srand(*pHRT);

   /*
    * Load key configuration
    */
   loadPlayersControls(hab, &keys);

   for(i = 0; i < MAX_PLAYERS; i++)
   {
      worms[i].rotLeftKey = keys[i].bCounterClockwise;
      worms[i].rotRightKey = keys[i].bClockwise;
      worms[i].shootKey = keys[i].bFire;
   }
   worms[0].iBaseColor = WORM1BASECOLOR;
   worms[1].iBaseColor = WORM2BASECOLOR;

   /*
    * Initialize two trons
    */
   ptl.x = sizlGameBitmap.cx/2-sizlGameBitmap.cx/8;
   ptl.y = sizlGameBitmap.cy/2-sizlGameBitmap.cx/8;
   create_tron(trons, &ptl, 0, &cActiveTrons, *pHRT);

   ptl.x = sizlGameBitmap.cx/2+sizlGameBitmap.cx/8;
   ptl.y = sizlGameBitmap.cy/2+sizlGameBitmap.cx/8;
   create_tron(trons, &ptl, 2, &cActiveTrons, *pHRT);


   while(!fGameOver)
   {
      ULONG thisTime = *pHRT;
      int iBullet;
      int iWorm;
      int iExplosion;
      int iApple;
      int iWriggler;
      int iFrenzy;
      int iTron;
      int cProcessed;
      BOOL fBlit = FALSE;
      QMSG qmsg;

      #ifdef DEBUG
      GameThreadState = 0x10000000;
      #endif

      if(WinPeekMsg(hab, &qmsg, NULLHANDLE, 0UL, 0UL, PM_REMOVE))
      {
         switch(qmsg.msg)
         {
            #ifdef DEBUG
            case GTHRDMSG_GROW:
               worms[SHORT1FROMMP(qmsg.mp1)].grow += irandom(MIN_WORM_GROW, MAX_WORM_GROW);
               break;

            case GTHRDMSG_WORM_SPEED:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_speed(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_speed(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d speed is %d\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].iSpeed));
               break;

            case GTHRDMSG_BULLET_SPEED:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_bullet_speed(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_bullet_speed(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d bullet speed is %d\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].iBulletSpeed));
               break;

            case GTHRDMSG_WORM_FIRERATE:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_firerate(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_firerate(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d fire rate is %d\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].iFireRate));
               break;

            case GTHRDMSG_EXPLOSION_SIZE:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_bulletpower(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_bulletpower(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d explosion size is %d\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].bullet_power));
               break;

            case GTHRDMSG_BULLETS_PER_SHOT:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_bullets_per_shot(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_bullets_per_shot(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d fires %d bullets per shot\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].bullets_per_shot));
               break;

            case GTHRDMSG_BULLETS_BOUNCE:
               if((SHORT)SHORT2FROMMP(qmsg.mp1) > 0)
                  inc_worm_bullets_bounce(&worms[SHORT1FROMMP(qmsg.mp1)]);
               else
                  dec_worm_bullets_bounce(&worms[SHORT1FROMMP(qmsg.mp1)]);
               dprintf(("Worm%d's bullets will bounce %d times before exploding\n", SHORT1FROMMP(qmsg.mp1)+1, worms[SHORT1FROMMP(qmsg.mp1)].bullet_bounces));
               break;
            #endif

            case GTHRDMSG_TERMINATE:
               flReturn |= TerminateApp;
               fGameOver = TRUE;
               continue;

            case WM_PAINT:
               memset(pb->afLineMask, 0xff, pb->cy);
               fBlit = TRUE;
               break;

         } /* switch(qmsg.msg) */
      }  /* if(WinPeekMsg(hab, &qmsg, NULLHANDLE, 0UL, 0UL, PM_REMOVE)) */

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x20000000;
      #endif

      if(flGameState)
      {
         if(flGameState & GAMEFLG_WORMDEAD && thisTime >= tmEndGame)
         {
            fGameOver = TRUE;
            continue;
         }
      }

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x30000000;
      #endif

      if(thisTime >= tmNextApple)
      {
         /* 1, 2, 4, 8, 16, 32, 64 */
         int check = 64;
         int num = 1 + rand() % check;
         dprintf(("Randomize apples: check=%d  num=%d\n", check, num));
         while(check >= num)
         {
            POINTL p = { (rand()%(limits.sizlBoard.cx-8))/8, (rand()%(limits.sizlBoard.cy-8))/8 };
            POINTL p2 = { p.x*8, p.y*8 };

            draw_apple(pb, &p2);
            for(i = 0; i < 8; i++)
            {
               pb->afLineMask[p2.y] = 0xff;
               p2.y++;
            }
            check >>= 1;
            dprintf(("Randomize apples: new check=%d\n", check));
         }

         fBlit = TRUE;

         tmNextApple = thisTime + irandom(20000, 72000);
         /* tmNextApple = thisTime + 4000;  */ /* temp! */
         tmNextApple = thisTime + irandom(5000, 20000); /* gooder!! */
         tmNextApple = thisTime + irandom(5000, 10000); /* gooder!! */
      }

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x40000000;
      #endif

      /*
       * Process user input
       */
      if(thisTime >= tmNextKeyScan)
      {
         for(iWorm = 0; iWorm < MAX_WORMS; iWorm++)
         {
            BOOL fNewDirection = FALSE;
            PWORMINFO worm = &worms[iWorm];

            if(worm->flags & WORMFLG_DEAD)
               continue;

            if(thisTime >= worm->next_rotate)
            {
               LONG lKeyState;

               /* if(abKeyStates[worm->rotLeftKey] == 0x80) */
               lKeyState = WinGetPhysKeyState(HWND_DESKTOP, worm->rotLeftKey);
               if(lKeyState & 0x8000)
               {
                  worm->deg = worm->virtDeg = sub_deg(worm->deg, 1);

                  fNewDirection = TRUE;
               }
               /* if(abKeyStates[worm->rotRightKey] == 0x80) */
               lKeyState = WinGetPhysKeyState(HWND_DESKTOP, worm->rotRightKey);
               if(lKeyState & 0x8000)
               {
                  worm->deg = worm->virtDeg = add_deg(worm->deg, 1);

                  fNewDirection = TRUE;
               }

               if(fNewDirection)
               {
                  worm->virtDeg = adjust_deg(worm->deg, worm->adjust_deg);
                  memcpy(&worm->dir, &dir_table[worm->virtDeg], sizeof(POINTL));

                  /* Only set next roatete time if a rotatation was processed */

/*                  lKeyState = WinGetPhysKeyState(HWND_DESKTOP, worm->rotFastKey);
                  if(lKeyState & 0x8000)
                  {
*/
                     worm->next_rotate = thisTime + worm->rotate_delay;
/*
                  }
                  else
                  {
                     worm->next_rotate = thisTime + worm->rotate_delay+10;
                     MINBLOCKTIME(ulMaxBlock, worm->rotate_delay+10);
                  } */
               }
            }

            /*
             * Worms can not fire if they are pacifists or if the other worm(s) are dead. Also,firing is disabled
             * if the other worm has croaked
             */
            if((thisTime >= worm->next_shoot && !(worm->flags & WORMFLG_PACIFIST)) && !(flGameState & GAMEFLG_WORMDEAD))
            {
               LONG lKeyState;
               lKeyState = WinGetPhysKeyState(HWND_DESKTOP, worm->shootKey);
               if(lKeyState & 0x8000 || (worm->flags & WORMFLG_AUTOFIRE))
               {
                  worm_shoot(pb, worm, iWorm, bullets, thisTime, &cActiveBullets);

                  /* Make sure worm doesn't fire too soon */
                  worm->next_shoot = thisTime + worm->shoot_delay;
               }
            }
         }
         tmNextKeyScan = thisTime + KEYSCAN_DELAY;
      }

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x50000000;
      #endif

      /*
       * Process bullets
       *   Note: This MUST be done before processing worm movement or worms will run into their own
       *         bullets just after firing.
       */
      for(iBullet = 0, cProcessed = 0; iBullet < MAX_BULLETS; iBullet++)
      {
         PBULLET bullet = &bullets[iBullet];
         POINTL ptlNew;
         POINTL newVirtPos;
         BYTE pix;
         ULONG flBound;

         if(cProcessed == cActiveBullets)
            break;
         if(!bullet->fActive)
            continue;
         if(thisTime < bullet->next_move)
         {
            cProcessed++;
            continue;
         }

         /*
          * Process "homing bullets"
          */
         if(bullet->flags & BULFLG_HOMING)
         {
            PWORMINFO worm = &worms[bullet->iTarget];
            double dxBullet = (double)(bullet->bigPos.x-worms[bullet->iTarget].bigPos.x);
            double dyBullet = (double)(bullet->bigPos.y-worms[bullet->iTarget].bigPos.y);
            LONG deg;

            /* Homing bullets */
            deg = rad_to_deg(atan2(dyBullet, dxBullet)+PI);
            deg = 359 - deg;
            if(add_deg(deg, bullet->deg) < 180)
            {
               bullet->deg = sub_deg(bullet->deg, 1);
               memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
            }
            else if(add_deg(deg, bullet->deg) > 180)
            {
               bullet->deg = add_deg(bullet->deg, 1);
               memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
            }

            /*
             * Play homing sample, short intervals when bullet gets closer to
             * target.
             */
         }

         /*
          * Process "drunken missiles"
          */
         if(bullet->flags & BULFLG_DRUNKEN)
         {
            if(rand()%2)
            {
               bullet->deg = add_deg(bullet->deg, 1+rand()%5);
            }
            else
            {
               bullet->deg = sub_deg(bullet->deg, 1+rand()%5);
            }
            memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
         }

         /* Clear previous bullet */
         setPixel(pb, &bullet->pos, 0);

recheck_bullet:
         flBound = 0;

         /*
          * Calculate new virtual position
          */
         newVirtPos.x = bullet->bigPos.x + bullet->dir.x;
         newVirtPos.y = bullet->bigPos.y + bullet->dir.y;

         /*
          * Check if bullet is out of bound (using temporary virtual point)
          */
         if(newVirtPos.x < 0)
         {
            if(bullet->flags & BULFLG_WARP)
            {
               newVirtPos.x = limits.maxBigPos.x + newVirtPos.x;
            }
            else
            {
               flBound |= OutOfBoundLeft;
            }
         }
         if(newVirtPos.x > limits.maxBigPos.x)
         {
            if(bullet->flags & BULFLG_WARP)
            {
               newVirtPos.x = newVirtPos.x - limits.maxBigPos.x;
            }
            else
            {
               flBound |= OutOfBoundRight;
            }
         }
         if(newVirtPos.y < 0)
         {
            if(bullet->flags & BULFLG_WARP)
            {
               newVirtPos.y = limits.maxBigPos.y + newVirtPos.y;
            }
            else
            {
               flBound |= OutOfBoundTop;
            }
         }
         if(newVirtPos.y > limits.maxBigPos.y)
         {
            if(bullet->flags & BULFLG_WARP)
            {
               newVirtPos.y = newVirtPos.y - limits.maxBigPos.y;
            }
            else
            {
               flBound |= OutOfBoundBottom;
            }
         }

         if(flBound)
         {
            /*
             * Bullet would be out of bound if it continues on it's present course
             */
            if(bullet->bounces)
            {
               /*
                * Bounce bullet off wall(s)
                */
               if(flBound & (OutOfBoundLeft|OutOfBoundRight))
               {
                  bullet->dir.x = -bullet->dir.x;
               }
               if(flBound & (OutOfBoundBottom|OutOfBoundTop))
               {
                  bullet->dir.y = -bullet->dir.y;
               }

               /*
                * Vectors and degree variable must always match.
                */
               bullet->deg = add_deg(rad_to_deg(atan2((double)bullet->dir.y, (double)bullet->dir.x)+PI), 180);

               playSample(samples.bullet_bounce_wall, thisTime);

               /*
                * Decrease number of bounces remaining and reevaluate bullet
                * position.
                */
               bullet->bounces--;
               goto recheck_bullet;
            }
            /*
             * Bullet did hit wall, but bullet does not have any (remaining) bounces, so
             * replace with an explosion, and process next bullet.
             */
            bullet2explosion(bullet, explosions, thisTime, &cActiveBullets, &cActiveExplosions, 0);
            continue;
         }

         /*
          * Once the bounding test has been done, calculate the bitmap position (but do so in a temporary
          * variable).
          */
         ptlNew.x = newVirtPos.x>>INTTRIG_PIVOT_SHIFT;
         ptlNew.y = newVirtPos.y>>INTTRIG_PIVOT_SHIFT;

         pix = getPixel(pb, &ptlNew);
         if(pix)
         {
            if(bullet->bounces)
            {
               enum HitObject {
                  Unknown = 0,
                  Worm
               };
               enum HitObject objHit = Unknown;
               PWORMINFO w = NULL;
               int iWormSeg;
               switch(pix)
               {
                  case APPLE_COLOR:
                  case APPLE_COLOR+1:
                  case BULLET_COLOR:
                     bullet2explosion(bullet, explosions, thisTime, &cActiveBullets, &cActiveExplosions, 0);
                     continue;

                  case TRON_TRAIL:
                  case TRON_HEAD:
                     playSample(samples.bullet_bounce_tron, thisTime);
                     reflect_bullet(bullet, deg_map[ptlNew.y][ptlNew.x]);
                     break;

                  default:
                     reflect_bullet(bullet, deg_map[ptlNew.y][ptlNew.x]);
                     break;
               }
               bullet->bounces--;
               goto recheck_bullet;
            }
            else
            {
               /* No bounce! */
               bullet2explosion(bullet, explosions, thisTime, &cActiveBullets, &cActiveExplosions, 0);
               continue;
            }
         }

         /*
          * Store new position in the bullet's data structure
          */
         bullet->bigPos.x = newVirtPos.x;
         bullet->bigPos.y = newVirtPos.y;
         bullet->pos.x = ptlNew.x;
         bullet->pos.y = ptlNew.y;

         setPixel(pb, &bullet->pos, BULLET_COLOR);

         bullet->next_move = thisTime + bullet->delay;

         fBlit = TRUE;

         cProcessed++;
      }  /* for(iBullet = 0, cProcessed = 0; iBullet < MAX_BULLETS; iBullet++) */

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x60000000;
      #endif

      /*
       * Process worm movement
       */
      for(iWorm = 0; iWorm < MAX_WORMS; iWorm++)
      {
         PWORMINFO worm = &worms[iWorm];
         BYTE pix;
         PPOINTL pptlNewHead;
         int i;
         int iOffs;

         if((thisTime < worm->next_movement) || (worm->flags & WORMFLG_DEAD))
            continue;

         /*
          * Fatal attraction modifier -- Worm is drawn toward foes
          */
         if((worm->flags & WORMFLG_FATAL_ATTRACTION) && thisTime >= worm->next_attaction)
         {
            /*
             * Find out which worm is nearest!
             */
            PWORMINFO targetWorm = &worms[(iWorm+1)%MAX_WORMS];
            double dxWorms = (double)(worm->bigPos.x-targetWorm->bigPos.x);
            double dyWorms = (double)(worm->bigPos.y-targetWorm->bigPos.y);
            LONG deg;

            deg = rad_to_deg(atan2(dyWorms, dxWorms)+PI);
            deg = 359 - deg;
            if(add_deg(deg, worm->deg) < 180)
            {
               worm->deg = sub_deg(worm->deg, 1);
               memcpy(&worm->dir, &dir_table[worm->deg], sizeof(POINTL));
            }
            else if(add_deg(deg, worm->deg) > 180)
            {
               worm->deg = add_deg(worm->deg, 1);
               memcpy(&worm->dir, &dir_table[worm->deg], sizeof(POINTL));
            }
            worm->virtDeg = worm->deg;
            worm->next_attaction = thisTime + 10;
         }

         /* Remove the tail and increase tail index */
         if(worm->grow)
         {
            worm->grow--;
            worm->length++;
         }
         else
         {
            /* Only clear tail if worm isn't growing */
            setPixel(pb, &worm->aptl[worm->iTail], 0);
            worm->iTail = (worm->iTail + 1) % MAX_WORM_LENGTH;
         }

         if(worm->flags & WORMFLG_WRIGGLE)
         {
            /* Wriggle worm */
            worm->iWriggleState = (worm->iWriggleState+1) % WRIGGLE_ANGLES;
            worm->adjust_deg = wriggle_deg[worm->iWriggleState];

            worm->virtDeg = adjust_deg(worm->deg, worm->adjust_deg);

            worm->dir.x = dir_table[worm->virtDeg].x;
            worm->dir.y = dir_table[worm->virtDeg].y;
         }

         /* Move worm on the vector board */
         worm->bigPos.x += worm->dir.x;
         worm->bigPos.y += worm->dir.y;

         if(worm->bigPos.x > limits.maxBigPos.x)
         {
            worm->bigPos.x = worm->bigPos.x % limits.sizlBigBoard.cx;
         }
         else if(worm->bigPos.x < 0)
         {
            LONG tmp = abs(worm->bigPos.x);
            worm->bigPos.x = limits.sizlBigBoard.cx - tmp;
         }

         if(worm->bigPos.y > limits.maxBigPos.y)
         {
            worm->bigPos.y = worm->bigPos.y % limits.sizlBigBoard.cy;
         }
         else if(worm->bigPos.y < 0)
         {
            LONG tmp = abs(worm->bigPos.y);
            worm->bigPos.y = limits.sizlBigBoard.cy - tmp;
         }

         worm->iHead = (worm->iHead + 1) % MAX_WORM_LENGTH;
         pptlNewHead = &worm->aptl[worm->iHead];
         pptlNewHead->x = (unsigned)worm->bigPos.x>>INTTRIG_PIVOT_SHIFT;
         pptlNewHead->y = (unsigned)worm->bigPos.y>>INTTRIG_PIVOT_SHIFT;
         worm->adeg[worm->iHead] = worm->virtDeg;

         pix = getPixel(pb, pptlNewHead);
         if(pix && pix != worm->iBaseColor)
         {
            int iLine = 0;
            POINTL tmp = { pptlNewHead->x-(pptlNewHead->x%8), pptlNewHead->y-(pptlNewHead->y%8) };
            enum AppleType appleType = random_apple_type();
            BOOL fDidModify = FALSE;
            enum Taunts { None, UpgradedWeapons };
            enum Taunts play_taunt = None;

            switch(pix)
            {
               case APPLE_COLOR:
               case APPLE_COLOR+1:
                  for(iLine = 0; iLine < 8; iLine++)
                     pb->afLineMask[tmp.y+iLine] = 0xff;
                  clear_apple(pb, &tmp);
                  playSample(samples.ate_apple, thisTime);

                  /* Always grow worm after eating an apple */
                  worm->grow += irandom(MIN_WORM_GROW, MAX_WORM_GROW);

                  switch(appleType)
                  {
                     case LowerSpeed:
                        fDidModify = dec_worm_speed(worm);
                        break;

                     case RaiseSpeed:
                        fDidModify = inc_worm_speed(worm);
                        break;

                     case Reset:
                        if(worm->flags & (WORMFLG_AUTOFIRE|WORMFLG_WRIGGLE|WORMFLG_PACIFIST|WORMFLG_FATAL_ATTRACTION|WORMFLG_DRUNKEN_MISSILES))
                        {
                           if(worm->flags & WORMFLG_PACIFIST)
                           {
                              WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           }
                           worm->flags &=  ~(WORMFLG_AUTOFIRE|WORMFLG_WRIGGLE|WORMFLG_PACIFIST|WORMFLG_FATAL_ATTRACTION|WORMFLG_DRUNKEN_MISSILES);
                           fDidModify = TRUE;
                        }
                        break;

                     case AutoFire:
                        if(!(worm->flags & WORMFLG_AUTOFIRE))
                        {
                           worm->flags &= ~WORMFLG_PACIFIST; /* Don't bother notifying */
                           worm->flags |= WORMFLG_AUTOFIRE;
                           fDidModify = TRUE;
                        }
                        break;

                     case LowerFireRate:
                        fDidModify = dec_worm_firerate(worm);
                        break;

                     case RaiseFireRate:
                        fDidModify = inc_worm_firerate(worm);
                        if(fDidModify)
                           play_taunt = UpgradedWeapons;
                        break;

                     case LowerBulletSpeed:
                        fDidModify = dec_worm_bullet_speed(worm);
                        break;

                     case RaiseBulletSpeed:
                        fDidModify = inc_worm_bullet_speed(worm);
                        if(fDidModify)
                           play_taunt = UpgradedWeapons;
                        break;

                     case Pacifist:
                        if(!(worm->flags & WORMFLG_PACIFIST))
                        {
                           worm->flags &= ~WORMFLG_AUTOFIRE;
                           worm->flags |= WORMFLG_PACIFIST;
                           fDidModify = TRUE;
                        }
                        break;

                     case SummonFrenzy:
                        /*
                         * Worm tail becomes a frenzy (which can never be destroyed?)
                         */
                        fDidModify = TRUE;
                        break;

                     case SummonWriggly:
                        /*
                         * Wriggly targets any nearby worms or apples, and shoots!
                         */
                        fDidModify = TRUE;
                        break;

                     case SpreadMore:
                        if(worm->flags & WORMFLG_PACIFIST)
                        {
                           WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           worm->flags &= ~WORMFLG_PACIFIST;
                        }
                        fDidModify = inc_worm_bullets_per_shot(worm);
                        if(fDidModify)
                           play_taunt = UpgradedWeapons;
                        break;

                     case SpreadLess:
                        fDidModify = dec_worm_bullets_per_shot(worm);
                        break;

                     case Wriggle:
                        if(!(worm->flags & WORMFLG_WRIGGLE))
                        {
                           worm->flags |= WORMFLG_WRIGGLE;
                           fDidModify = TRUE;
                        }
                        break;

                     case FatalAttraction:
                        if(!(worm->flags & WORMFLG_FATAL_ATTRACTION))
                        {
                           worm->flags |= WORMFLG_FATAL_ATTRACTION;
                           worm->next_attaction = thisTime;
                           fDidModify = TRUE;
                        }
                        break;

                     case HomingBullets:
                        if(worm->flags & WORMFLG_PACIFIST)
                        {
                           WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           worm->flags &= ~WORMFLG_PACIFIST;
                        }
                        if(!(worm->flags & WORMFLG_HOMING_BULLETS))
                        {
                           worm->flags |= WORMFLG_HOMING_BULLETS;
                           fDidModify = TRUE;
                           play_taunt = UpgradedWeapons;
                        }
                        break;

                     case MoreExplosive:
                        fDidModify = inc_worm_bulletpower(worm);
                        if(worm->flags & WORMFLG_PACIFIST)
                        {
                           WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           worm->flags &= ~WORMFLG_PACIFIST;
                        }
                        if(fDidModify)
                           play_taunt = UpgradedWeapons;
                        break;

                     case LessExplosive:
                        fDidModify = dec_worm_bulletpower(worm);
                        break;

                     case BounceMore:
                        fDidModify = inc_worm_bullets_bounce(worm);
                        break;

                     case BounceLess:
                        fDidModify = dec_worm_bullets_bounce(worm);
                        break;

                     case WarpingBullets:
                        if(worm->flags & WORMFLG_PACIFIST)
                        {
                           WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           worm->flags &= ~WORMFLG_PACIFIST;
                        }
                        if(!(worm->flags & WORMFLG_WARPINGBULLETS))
                        {
                           worm->flags |= WORMFLG_WARPINGBULLETS;
                           fDidModify = TRUE;
                        }
                        break;

                     case DrunkenMissiles:
                        if(worm->flags & WORMFLG_PACIFIST)
                        {
                           WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MORE_PACIFIST));
                           worm->flags &= ~WORMFLG_PACIFIST;
                        }
                        if(!(worm->flags & WORMFLG_DRUNKEN_MISSILES))
                        {
                           worm->flags |= WORMFLG_DRUNKEN_MISSILES;
                           fDidModify = TRUE;
                        }
                        break;

                     default:
                        dputs(("Unhandled appletype"));
                        break;
                  }
                  if(fDidModify)
                  {
                     showAppleStatus(hwndStatus[iWorm], appleType);
                  }
                  else
                  {
                     WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_NO_MODIFICATION_APPLE));
                  }
                  switch(play_taunt)
                  {
                     case UpgradedWeapons:
                        playSample(samples.upgraded_weaponary, thisTime);
                        break;
                  }
                  break;

               default:
                  alloc_explosion(explosions, pptlNewHead, thisTime, worm->bullet_power, &cActiveExplosions, 0);
                  continue;
            }  /* switch(pix) */
         }

         setPixel(pb, pptlNewHead, worm->iBaseColor);

         deg_map[pptlNewHead->y][pptlNewHead->x] = worm->virtDeg;

         iOffs = (worm->iTail+(worm->length-1)) % MAX_WORM_LENGTH;
         setPixel(pb, &worm->aptl[iOffs], worm->iBaseColor+1);

         iOffs = (worm->iTail+(worm->length-8)) % MAX_WORM_LENGTH;
         setPixel(pb, &worm->aptl[iOffs], worm->iBaseColor+2);

         fBlit = TRUE;
         worm->next_movement = thisTime + worm->move_delay;
      }

      DosSleep(0);

      for(iTron = 0, cProcessed = 0; iTron < MAX_TRONS; iTron++)
      {
         PTRON tron = &trons[iTron];
         POINTL newPos;
         if(cProcessed == cActiveTrons)
            break;
         if(!tron->fActive)
            continue;
         if(thisTime < tron->tmNextMove)
            continue;

         fBlit = TRUE;

         setPixel(pb, &tron->pos, TRON_TRAIL);

         tron->strategy.dist--;
         if(tron->strategy.dist == 0)
         {
            int rev_dir = (tron->dir+2)%4;
            do {
               tron->dir = rand()%4;
            }while(tron->dir == rev_dir);
            memcpy(&tron->vector, &tron_dir[tron->dir], sizeof(POINTL));
            /* tron->strategy.dist = irandom(42, 200); */
            tron->strategy.dist = irandom(10, 100);
         }

         tron->pos.x += tron->vector.x;
         if(tron->pos.x < 0)
            tron->pos.x = limits.maxPos.x;
         else if(tron->pos.x > limits.maxPos.x)
            tron->pos.x = 0;
         tron->pos.y += tron->vector.y;
         if(tron->pos.y < 0)
            tron->pos.y = limits.maxPos.y;
         else if(tron->pos.y > limits.maxPos.y)
            tron->pos.y = 0;

         if(getPixel(pb, &tron->pos) != 0)
         {
            tron->fActive = FALSE;
            cActiveTrons--;
            alloc_explosion(explosions, &tron->pos, thisTime, EXPLOSION_SIZES-1, &cActiveExplosions, 0);
         }
         else
         {
            setPixel(pb, &tron->pos, TRON_HEAD);
            /*
             * TODO: If tron has changed direction, make the corner pixel the median of the two angles.
             *
             *                ^
             *                |
             *                |  <- 90
             *                |
             *     -----------+ <- 45
             *       ^
             *       |
             *       +-- 0
             */
            switch(tron->dir)
            {
               case 0:                   /* up */
                  deg_map[tron->pos.y][tron->pos.x] = 90;
                  break;
               case 1:                   /* left */
                  deg_map[tron->pos.y][tron->pos.x] = 180;
                  break;
               case 2:                   /* down */
                  deg_map[tron->pos.y][tron->pos.x] = 180+90;
                  break;
               case 3:                   /* right */
                  deg_map[tron->pos.y][tron->pos.x] = 0;
                  break;
            }

            tron->tmNextMove = thisTime + 20;
            /* tron->tmNextMove = thisTime + 10; */
         }
         cProcessed++;
      }

      DosSleep(0);

      #ifdef DEBUG
      GameThreadState = 0x70000000;
      #endif

      /*
       * Process explosions
       */
      for(iExplosion = 0, cProcessed = 0; iExplosion < MAX_EXPLOSIONS; iExplosion++)
      {
         PEXPLOSION explosion = &explosions[iExplosion];
         if(cProcessed == cActiveExplosions)
            break;
         if(!explosion->fActive)
            continue;

         #ifdef DEBUG
         GameThreadState = 0x71000000;
         #endif

         /*
          * First time an explosion is processed, check is it intersects with anything, of it does,
          * take appropriate action.
          */
         if(!(explosion->flags & EXPFLG_PREPROCESSED))
         {
            LONG radius = rle_explosion_data[explosion->power]->radius;
            int cProcessedBullets = 0;

            #ifdef DEBUG
            GameThreadState = 0x71100000;
            #endif

            playSample(samples.explosion[explosion->power], thisTime);

            /*
             * Check if any of the worms is within explosion
             */
            for(iWorm = 0; iWorm < MAX_WORMS; iWorm++)
            {
               PWORMINFO worm = &worms[iWorm];
               if(worm->flags & WORMFLG_DEAD)
                  continue;
               #ifdef DEBUG
               GameThreadState = 0x71110000;
               #endif

               /* Check worm body */
               if(is_worm_body_within_explosion(worm, explosion))
               {
                  playSample(samples.worm_died, thisTime);

                  worm->flags |= WORMFLG_DEAD;
                  flGameState |= GAMEFLG_WORMDEAD;
                  tmEndGame = thisTime + DEAD_WORM_AFTERPLAY;
                  WinPostMsg(hwndStatus[iWorm], SBMSG_QUEUE_RES_STRING, LONGFROMMP(NULLHANDLE), LONGFROMMP(IDS_WORM_DIED));

                  /* check next worm, no need to proceed since worm is dead */
                  continue;
               }
               #ifdef DEBUG
               GameThreadState = 0x71120000;
               #endif

               /* Check worm tail -- this should be done *after* the body check. Note, this function will cut off worm tail! */
               check_tail_explosion(worm, explosion);

               #ifdef DEBUG
               GameThreadState = 0x71130000;
               #endif

            }

            #ifdef DEBUG
            GameThreadState = 0x71200000;
            #endif

            /* Explode any bullets within reach of the explosion */
            for(iBullet = 0, cProcessedBullets = 0; iBullet < MAX_BULLETS; iBullet++)
            {
               PBULLET bullet = &bullets[iBullet];
               if(cProcessedBullets == cActiveBullets)
                  break;
               if(!bullet->fActive)
                  continue;
               if(is_within_radius(&explosion->center, &bullet->pos, radius))
               {
                  /* Create a new explosion, must index it above current explosion! */
                  bullet2explosion(bullet, explosions, thisTime, &cActiveBullets, &cActiveExplosions, iExplosion+1);
               }
               cProcessedBullets++;
            }

            #ifdef DEBUG
            GameThreadState = 0x71300000;
            #endif

            explosion->flags |= EXPFLG_PREPROCESSED;
         }

         #ifdef DEBUG
         GameThreadState = 0x72000000;
         #endif

         if(thisTime >= explosion->ulExpireTime)
         {
            clear_explosion(pb, &explosion->center, rle_explosion_data[explosion->power], 0x00, &limits);
            explosion->fActive = FALSE;
            cActiveExplosions--;
            fBlit = TRUE;
            continue;
         }
         #ifdef DEBUG
         GameThreadState = 0x73000000;
         #endif

         if(thisTime >= explosion->next_color)
         {
            draw_explosion(pb, &explosion->center, rle_explosion_data[explosion->power], 60+explosion->iColor, &limits);
            explosion->iColor = (explosion->iColor + 1) % 32;
            explosion->next_color = thisTime + 5;
            fBlit = TRUE;
         }
         #ifdef DEBUG
         GameThreadState = 0x74000000;
         #endif

         cProcessed++;
      }  /* for(iExplosion = 0, cProcessed = 0; iExplosion < MAX_EXPLOSIONS; iExplosion++) */


      #ifdef DEBUG
      GameThreadState = 0x80000000;
      #endif

      /* Tell blitter to blit any changes */
      if(fBlit)
      {
         DosPostEventSem(hevPaint);
      }

      #ifdef DEBUG
      GameThreadState = 0x90000000;
      #endif

      #ifdef DEBUG
      DosPostEventSem(hevHeartBeat);
      #endif

      hrtBlock(hTimer, 1);
   }  /* while(!fGameOver) */

   if(worms[0].flags & WORMFLG_DEAD)
      flReturn |= Worm1Died;
   if(worms[1].flags & WORMFLG_DEAD)
      flReturn |= Worm2Died;


   if(deg_map)
   {
      for(i = 0; i < limits.sizlBoard.cy; i++)
      {
         free(deg_map[i]);
      }
      free(deg_map);
   }


   /*
    * Terminate game data
    */
   if(trons)
      free(trons);
   if(explosions)
      free(explosions);
   if(bullets)
      free(bullets);
   if(worms)
      free(worms);

   _heapmin();

   return flReturn;
}

void _Inline draw_apple(PPIXELBUFFER pb, PPOINTL p)
{
   POINTL tmp = { p->x+4, p->y };

   setPixel(pb, &tmp, 51);

   tmp.y++;
   tmp.x = p->x+3;
   memset(getPixelP(pb, &tmp), APPLE_COLOR+1, 2);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 2);
   tmp.x = p->x+3;
   setPixel(pb, &tmp, APPLE_COLOR+1);
   tmp.x = p->x+4;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 2);

   tmp.y++;
   tmp.x = p->x;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 7);

   tmp.y++;
   tmp.x = p->x;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 7);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 6);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 5);

   tmp.y++;
   tmp.x = p->x+2;
   setPixel(pb, &tmp, APPLE_COLOR);
   tmp.x = p->x+4;
   memset(getPixelP(pb, &tmp), APPLE_COLOR, 2);
}

void _Optlink clear_apple(PPIXELBUFFER pb, PPOINTL p)
{
   POINTL tmp = { p->x+4, p->y };

   setPixel(pb, &tmp, 0);

   tmp.y++;
   tmp.x = p->x+3;
   memset(getPixelP(pb, &tmp), 0, 2);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), 0, 2);
   tmp.x = p->x+3;
   setPixel(pb, &tmp, 0);
   tmp.x = p->x+4;
   memset(getPixelP(pb, &tmp), 0, 2);

   tmp.y++;
   tmp.x = p->x;
   memset(getPixelP(pb, &tmp), 0, 7);

   tmp.y++;
   tmp.x = p->x;
   memset(getPixelP(pb, &tmp), 0, 7);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), 0, 6);

   tmp.y++;
   tmp.x = p->x+1;
   memset(getPixelP(pb, &tmp), 0, 5);

   tmp.y++;
   tmp.x = p->x+2;
   setPixel(pb, &tmp, 0);
   tmp.x = p->x+4;
   memset(getPixelP(pb, &tmp), 0, 2);
}


static void init_worms(PWORMINFO worms, PBOARDLIMITS limits)
{
   POINTL p = { 42, limits->sizlBoard.cy-42 };
   init_worm(&worms[0], &p, 270);

   p.x = limits->sizlBoard.cx-42;
   p.y = 42;
   init_worm(&worms[1], &p, 90);
}

static void init_worm(PWORMINFO worm, PPOINTL pp, int deg)
{
   POINTL p = { pp->x, pp->y };
   int i = 0;

   worm->iTail = 0;
   worm->iHead = 0;
   worm->length = 0;

   worm->deg = worm->virtDeg = deg;
   memcpy(&worm->dir, &dir_table[worm->deg], sizeof(POINTL));

   worm->bigPos.x = (unsigned)p.x<<INTTRIG_PIVOT_SHIFT;
   worm->bigPos.y = (unsigned)p.y<<INTTRIG_PIVOT_SHIFT;

   /*
    * Make sure worm is at least 8 units long!
    * This is done to eliminate the need for two extra if() blocks in the main game loop.
    */
   for(i = 0; i < 8; i++)
   {
      worm->aptl[worm->iHead].x = (unsigned)worm->bigPos.x>>INTTRIG_PIVOT_SHIFT;
      worm->aptl[worm->iHead].y = (unsigned)worm->bigPos.y>>INTTRIG_PIVOT_SHIFT;

      /*
       * The worm needs to be drawn here. Worms look funny when game starts
       * otherwise..
       */
      /* setPixel(pb,  */
      worm->bigPos.x += worm->dir.x;
      worm->bigPos.y += worm->dir.y;
      worm->iHead++;
      worm->length++;
   }

   worm->iSpeed = 2;
   worm->iFireRate = 1;
   worm->iBulletSpeed = 0;

   worm->move_delay = worm_speed_delays[worm->iSpeed];
   worm->shoot_delay = worm_shoot_delays[worm->iFireRate];
   worm->rotate_delay = 4;               /* 2 */

   worm->bullet_bounces = 0;
   worm->bullet_power = 1;
   worm->bullets_per_shot = 1;

   worm->grow = 32-8;

   worm->flags = 0;
}


void _Inline init_bullet(PBULLET bullet, PPOINTL pos, int deg, int iBulletSpeed, ULONG thisTime, int bounces, int power)
{
   bullet->fActive = TRUE;
   memcpy(&bullet->pos, pos, sizeof(POINTL));
   bullet->bigPos.x = (unsigned)bullet->pos.x<<INTTRIG_PIVOT_SHIFT;
   bullet->bigPos.y = (unsigned)bullet->pos.y<<INTTRIG_PIVOT_SHIFT;
   bullet->deg = deg;
   memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
   bullet->delay = bullet_speed_delays[iBulletSpeed];
   bullet->iCycle = 0;
   bullet->bounces = bounces;
   bullet->power = power;

   bullet->next_move = thisTime;
}


void _Inline alloc_bullet(BULLET bullets[], PPOINTL virtPos, int deg, int iBulletSpeed, ULONG thisTime, int bounces, int power, PULONG pcBullets, ULONG flags, int iTarget)
{
   int i = 0;
   for(; i < MAX_BULLETS; i++)
   {
      if(!bullets[i].fActive)
      {
         init_bullet_2(&bullets[i], virtPos, deg, iBulletSpeed, thisTime, bounces, power, flags, iTarget);
         (*pcBullets)++;
         return;
      }
   }
}

void _Inline init_bullet_2(PBULLET bullet, PPOINTL virtPos, int deg, int iBulletSpeed, ULONG thisTime, int bounces, int power, ULONG flags, int iTarget)
{
   bullet->fActive = TRUE;
   bullet->bigPos.x = virtPos->x;
   bullet->bigPos.y = virtPos->y;
   bullet->pos.x = (unsigned)bullet->bigPos.x>>INTTRIG_PIVOT_SHIFT;
   bullet->pos.y = (unsigned)bullet->bigPos.y>>INTTRIG_PIVOT_SHIFT;
   bullet->deg = deg;
   memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
   bullet->delay = bullet_speed_delays[iBulletSpeed];
   bullet->iCycle = 0;
   bullet->bounces = bounces;
   bullet->power = power;
   bullet->flags = flags;
   bullet->iTarget = iTarget;

   bullet->next_move = thisTime;
}

PEXPLOSION _Inline bullet2explosion(PBULLET bullet, EXPLOSION explosions[], ULONG thisTime, PULONG pcBullets, PULONG pcExplosions, int iFirstExplosion)
{
   bullet->fActive = FALSE;
   (*pcBullets)--;
   return alloc_explosion(explosions, &bullet->pos, thisTime, bullet->power, pcExplosions, iFirstExplosion);
}


PEXPLOSION _Inline alloc_explosion(PEXPLOSION explosions, PPOINTL center, ULONG thisTime, ULONG power, PULONG pcExplosions, int i)
{
   for(; i < MAX_EXPLOSIONS; i++)
   {
      if(!explosions[i].fActive)
      {
         init_explosion(&explosions[i], center, thisTime, power);
         (*pcExplosions)++;
         return &explosions[i];
      }
   }
   return NULL;
}

void _Inline init_explosion(PEXPLOSION explosion, PPOINTL center, ULONG thisTime, ULONG power)
{
   explosion->fActive = TRUE;
   memcpy(&explosion->center, center, sizeof(POINTL));
   explosion->ulStartTime = thisTime;
   explosion->ulExpireTime = thisTime+160;
   explosion->power = power;
   explosion->next_color = thisTime;
   explosion->iColor = 0;
   explosion->flags = 0UL;
}




long _Inline flround(float f)
{
   return f - (long)f < 0.5 ? (long)f : (long)f + 1;
}

long _Inline dlround(double d)
{
   return d - (long)d < 0.5 ? (long)d : (long)d + 1;
}

double _Inline deg_to_rad(LONG deg)
{
   double ddeg = (float)deg;
   return (ddeg/360)*(2*PI);
}

int _Inline rad_to_deg(double r)
{
   return dlround(360.0*(r/PI2));
}

int _Inline irandom(int minimum, int maximum)
{
   return minimum + (rand()*rand()) % (maximum-minimum);
}

BOOL _Inline is_within_radius(PPOINTL center, PPOINTL p, LONG radius)
{
   LONG dx = center->x - p->x;
   LONG dy = center->y - p->y;

   if( ((dx*dx) + (dy*dy)) <= (radius*radius) )
   {
      return TRUE;
   }
   return FALSE;
}


void _Inline calc_new_pos(PPOINTL newpos, PPOINTL vecPos, PPOINTL dir)
{
   POINTL p = { (vecPos->x+dir->x)>>INTTRIG_PIVOT_SHIFT,
                (vecPos->y+dir->y)>>INTTRIG_PIVOT_SHIFT };
   newpos->x = p.x;
   newpos->y = p.y;
}



BOOL _Inline is_worm_body_within_explosion(PWORMINFO worm, PEXPLOSION explosion)
{
   int i = (worm->iTail + worm->length-MIN_WORM_LENGTH) % MAX_WORM_LENGTH;
   int iMax = (worm->iHead+1) % MAX_WORM_LENGTH;
   LONG radius = rle_explosion_data[explosion->power]->radius;
   for(; i != iMax; i = (i + 1) % MAX_WORM_LENGTH)
   {
      if(is_within_radius(&explosion->center, &worm->aptl[i], radius))
      {
         return TRUE;
      }
   }
   return FALSE;
}

void _Inline check_tail_explosion(PWORMINFO worm, PEXPLOSION explosion)
{
   int i = worm->iTail;
   int iMax = (worm->iTail + worm->length-MIN_WORM_LENGTH) % MAX_WORM_LENGTH;
   LONG radius = rle_explosion_data[explosion->power]->radius;
   int cStepped = 0;
   for(; i != iMax; i = (i + 1) % MAX_WORM_LENGTH, cStepped++)
   {
      /* First first point along the tail which is within the area of the explosion */
      if(is_within_radius(&explosion->center, &worm->aptl[i], radius))
      {
         /* Ieeek! Worm's tail was was hit! Find out how much is left of worm! */
         for(; i != iMax; i = (i + 1) % MAX_WORM_LENGTH, cStepped++)
         {
            /* First first point which is NOT within the area of the explosion */
            if(!is_within_radius(&explosion->center, &worm->aptl[i], radius))
            {
               /* We've found the point which is the index nearest the head which is still intact, break out of loop */
               break;
            }
         }
         /* i should now contain the index nearest the head which is still intact, now we must adjust
            the length and tail variables of the worm */
         worm->length -= cStepped;
         worm->iTail = i;
         cStepped = 0;                   /* Reset */
         /* Note, do not break out of loop, it must run to completion since the explosion could (well, in theory at least)
            intersect with two portions of the tail */
      }
   }
}




int _Inline pointsequ(PPOINTL p1, PPOINTL p2)
{
   if((p1->x == p2->x) && (p1->y == p2->y))
      return TRUE;
   return FALSE;
}

int _Inline bigpointsequ(PPOINTL p1, PPOINTL p2)
{
   if(( (p1->x>>INTTRIG_PIVOT_SHIFT) == (p2->x>>INTTRIG_PIVOT_SHIFT) ) && ( (p1->y>>INTTRIG_PIVOT_SHIFT) == (p2->y>>INTTRIG_PIVOT_SHIFT) ))
      return TRUE;
   return FALSE;
}



int same_heading(int deg1, int deg2)
{
   int diff = 90 - deg1;
   int tmp_deg1 = deg1 + diff;
   int tmp_deg2 = add_deg(deg2, diff);
   if(tmp_deg2 >= 0 && tmp_deg2 < 180)
      return 1;
   return 0;
}


/*
 * bullet - bullet object to modify
 * deg - degree of surface to reflect against
 */
void _Inline reflect_bullet(PBULLET bullet, int ref_deg)
{
   int normal = 0;
   int alpha = 0;
   int tmp_bul = add_deg(bullet->deg, 180);

   /* Calculate a normal */
   normal = add_deg(ref_deg, 90);
   /* If normal has the same heading (within 180 degrees) as the bullet, then
      the normal is on the wrong side of the reflection */
   if(same_heading(normal, bullet->deg))
   {
      normal = add_deg(normal, 180);
   }

   alpha = normal - tmp_bul;

   bullet->deg = normal + alpha;
   while(bullet->deg < 0)
      bullet->deg += 360;
   bullet->deg %= 360;

   memcpy(&bullet->dir, &dir_table[bullet->deg], sizeof(POINTL));
}


int _Inline add_deg(int deg, int a)
{
   return (deg+a) % 360;
}
int _Inline sub_deg(int deg, int a)
{
   int tmp = deg - a;
   return tmp >= 0 ? tmp : 360 + tmp;
}

int _Inline adjust_deg(int deg, int a)
{
   if(a < 0)
      return sub_deg(deg, abs(a));
   return add_deg(deg, a);
}


void setup_limits(PBOARDLIMITS limits, PSIZEL sizlGameBitmap)
{
   memset(limits, 0, sizeof(BOARDLIMITS));
   limits->sizlBoard.cx = sizlGameBitmap->cx;
   limits->sizlBoard.cy = sizlGameBitmap->cy;
   limits->sizlBigBoard.cx = (unsigned)limits->sizlBoard.cx<<INTTRIG_PIVOT_SHIFT;
   limits->sizlBigBoard.cy = (unsigned)limits->sizlBoard.cy<<INTTRIG_PIVOT_SHIFT;
   limits->maxPos.x = limits->sizlBoard.cx-1;
   limits->maxPos.y = limits->sizlBoard.cy-1;
   limits->maxBigPos.x = (unsigned)limits->maxPos.x<<INTTRIG_PIVOT_SHIFT;
   limits->maxBigPos.y = (unsigned)limits->maxPos.y<<INTTRIG_PIVOT_SHIFT;
}



/****************************************************************************\
 *                                                                          *
 *                      W O R M  A C T I O N                                *
 *                                                                          *
\****************************************************************************/
static void worm_shoot(PPIXELBUFFER pb, PWORMINFO worm, int iWorm, BULLET bullets[], ULONG thisTime, PULONG pcActiveBullets)
{
   int iTaget = iWorm == 1 ? 0 : 1;
   int iBullet = 0;
   int iShot = worm->bullets_per_shot;
   ULONG flBullet = 0;

   if(worm->flags & WORMFLG_HOMING_BULLETS)
   {
      flBullet |= BULFLG_HOMING;
   }
   if(worm->flags & WORMFLG_DRUNKEN_MISSILES)
   {
      flBullet |= BULFLG_DRUNKEN;
   }
   if(worm->flags & WORMFLG_WARPINGBULLETS)
   {
      flBullet |= BULFLG_WARP;
   }

   while(iShot)
   {
      int deg = worm->virtDeg;
      PBULLET bullet = NULL;

      for(; iBullet < MAX_BULLETS; iBullet++)
      {
         if(!bullets[iBullet].fActive)
         {
            bullet = &bullets[iBullet++];
            break;
         }
      }
      if(bullet)
      {
         BYTE pix;
         POINTL ptlNextWormPos = { worm->bigPos.x+worm->dir.x, worm->bigPos.y+worm->dir.y };
         switch(iShot)
         {
            case 2:                         /* +45 degrees */
               deg = add_deg(deg, 45);
               break;

            case 3:                         /* -45 degrees */
               deg = sub_deg(deg, 45);
               break;

            case 4:                         /* +20 degrees */
               deg = add_deg(deg, 20);
               break;

            case 5:                         /* -20 degrees */
               deg = sub_deg(deg, 20);
               break;
         }
         /* init_bullet(bullet, &worm->aptl[worm->iHead], deg, worm->iBulletSpeed, thisTime, worm->bullet_bounces, worm->bullet_power); */
         init_bullet_2(bullet, &worm->bigPos, deg, worm->iBulletSpeed, thisTime, worm->bullet_bounces, worm->bullet_power, flBullet, iTaget);
         (*pcActiveBullets)++;

         /*
          * Need to make sure worm won't bump into bullet next turn
          */
         bullet->bigPos.x += bullet->dir.x;
         bullet->bigPos.y += bullet->dir.y;
         bullet->pos.x = (unsigned)bullet->bigPos.x>>INTTRIG_PIVOT_SHIFT;
         bullet->pos.y = (unsigned)bullet->bigPos.y>>INTTRIG_PIVOT_SHIFT;

         pix = getPixel(pb, &bullet->pos);
         while(bigpointsequ(&bullet->bigPos, &ptlNextWormPos) || pix == worm->iBaseColor || pix == BULLET_COLOR)
         {
            /* this doesn't seem to work correctly -- I had an incident when the bullets
               blew up in the worm's face. It happened when I had the 5 bullet launcher
               and fired in a 45 degreeish angle. Investigare! */
            bullet->bigPos.x += bullet->dir.x;
            bullet->bigPos.y += bullet->dir.y;
            bullet->pos.x = (unsigned)bullet->bigPos.x>>INTTRIG_PIVOT_SHIFT;
            bullet->pos.y = (unsigned)bullet->bigPos.y>>INTTRIG_PIVOT_SHIFT;
            pix = getPixel(pb, &bullet->pos);
         }
         iShot--;

         playSample(samples.worm_shoot, thisTime);
      }
   }
}



/****************************************************************************\
 *                                                                          *
 *           G A M E  A C T I O N  D A T A  G E N E R A T I O N             *
 *                                                                          *
\****************************************************************************/
enum AppleType _Inline random_apple_type(void)
{
   int r = rand() % appletype_randomization_data->max_rand_value;
   enum AppleType i = 0;

   while(r > appletype_randomization_data->chance[i])
      i++;
   return i;
}


static BOOL _Optlink create_tron(TRON trons[], PPOINTL pos, int dir, PULONG pcActiveTrons, ULONG thisTime)
{
   int iTron = 0;
   for(; iTron < MAX_TRONS; iTron++)
   {
      PTRON tron = &trons[iTron];
      if(tron->fActive)
         continue;

      tron->fActive = TRUE;
      tron->pos.x = pos->x;
      tron->pos.y = pos->y;
      tron->dir = dir;
      memcpy(&tron->vector, &tron_dir[tron->dir], sizeof(POINTL));

      (*pcActiveTrons)++;

      tron->strategy.dist = irandom(42, 200);

      tron->tmNextMove = thisTime;
      return TRUE;
   }
   return FALSE;
}


/****************************************************************************\
 *                                                                          *
 *                E X P L O S I O N  D R A W I N G                          *
 *                                                                          *
\****************************************************************************/
void _Inline draw_explosion(PPIXELBUFFER pb, PPOINTL center, PRLECIRCLE prc, BYTE color, PBOARDLIMITS limits)
{
   int iLine = 0;
   enum {
      NoClip = 0x00000000,
      ClipX = 0x00000001,
      ClipY = 0x00000002
   };
   ULONG flClip = NoClip;
   POINTL p;

   if(center->x+prc->xMin < 0 || center->x+prc->xMax > limits->maxPos.x)
      flClip |= ClipX;
   if(center->y+prc->yMin < 0 || center->y+prc->yMax > limits->maxPos.y)
      flClip |= ClipY;

   p.y = center->y + prc->line[iLine].y;

   switch(flClip)
   {
      case NoClip:
         memset(&pb->afLineMask[p.y], 0xff, prc->lines);
         for(; iLine < prc->lines; iLine++)
         {
            p.x = center->x + prc->line[iLine].xLeft;
            memset(getPixelP(pb, &p), color, prc->line[iLine].cb);
            p.y++;
         }
         break;

      case ClipY:
         if(p.y < 0)
         {
            iLine += abs(p.y);
            p.y = 0;
         }
         for(; iLine < prc->lines; iLine++)
         {
            if(p.y <= limits->maxPos.y)
            {
               p.x = center->x + prc->line[iLine].xLeft;
               memset(getPixelP(pb, &p), color, prc->line[iLine].cb);
               pb->afLineMask[p.y] = 0xff;
               p.y++;
               continue;
            }
            break;
         }
         break;

      case ClipX:
         memset(&pb->afLineMask[p.y], 0xff, prc->lines);
         for(; iLine < prc->lines; iLine++)
         {
            ULONG cb = prc->line[iLine].cb;
            LONG xMax;
            p.x = center->x + prc->line[iLine].xLeft;
            if(p.x < 0)
            {
               cb += p.x;
               p.x = 0;
            }
            xMax = p.x + cb;
            if(xMax > limits->maxPos.x)
            {
               cb -= (xMax - limits->maxPos.x);
            }
            memset(getPixelP(pb, &p), color, cb);
            p.y++;
         }
         break;

      case ClipX|ClipY:
         if(p.y < 0)
         {
            iLine += abs(p.y);
            p.y = 0;
         }
         for(; iLine < prc->lines; iLine++)
         {
            if(p.y <= limits->maxPos.y)
            {
               ULONG cb = prc->line[iLine].cb;
               LONG xMax;
               p.x = center->x + prc->line[iLine].xLeft;
               if(p.x < 0)
               {
                  cb += p.x;
                  p.x = 0;
               }
               xMax = p.x + cb;
               if(xMax > limits->maxPos.x)
               {
                  cb -= (xMax - limits->maxPos.x);
               }
               memset(getPixelP(pb, &p), color, cb);
               pb->afLineMask[p.y] = 0xff;
               p.y++;
               continue;
            }
            break;
         }
         break;
   }
}

void _Inline clear_explosion(PPIXELBUFFER pb, PPOINTL center, PRLECIRCLE prc, BYTE color, PBOARDLIMITS limits)
{
   draw_explosion(pb, center, prc, 0, limits);
}

/*
void _Inline clear_explosion_new(PPIXELBUFFER pb, PEXPLOSION explosion, PBOARDLIMITS limits)
{
   draw_explosion(pb, &explosion->center, rle_explosion_data[explosion->power], 0, limits);
}
*/



/****************************************************************************\
 *                                                                          *
 *              T H R E A D  I N I T I A L I Z A T I O N                    *
 *                    A N D  T E R M I N A T I O N                          *
 *                                                                          *
\****************************************************************************/
static BOOL _Optlink generate_deg_vec_table(void)
{
   PPOINTL aptl = calloc(360, sizeof(POINTL));
   if(aptl)
   {
      int i = 0;
      for(; i < 360; i++)
      {
         aptl[i].x = dlround((double)INTTRIG_PIVOT*cos(deg_to_rad(i)));
         aptl[i].y = dlround((double)INTTRIG_PIVOT*sin(deg_to_rad(i)));
      }
      dir_table = aptl;
      return TRUE;
   }
   return FALSE;
}


static void _Optlink destroy_deg_vec_table(void)
{
   if(dir_table)
   {
      free(dir_table);
      _heapmin();
   }
}

static BOOL _Optlink initialize_dive_palette(HDIVE hDive)
{
   PLONG plPalette = calloc(256, sizeof(LONG));
   if(plPalette != NULL)
   {
      int i = 0;
      memset(plPalette, 0, 256*sizeof(LONG));

      /* The game area color */
      /* plPalette[BACKGROUND_COLOR] = 0x00cccccc; */
      /* plPalette[BACKGROUND_COLOR] = 0x00af7964; */
      plPalette[BACKGROUND_COLOR] = 0x007e5648;

      /* Worm Colors 1 */
      plPalette[WORM1BASECOLOR] = 0x00ffeeee;
      plPalette[WORM1BASECOLOR+1] = 0x00ff8888;
      plPalette[WORM1BASECOLOR+2] = 0x00ff0000;

      /* Worm Colors 2 */
      plPalette[WORM2BASECOLOR] = 0x00eeeeff;
      plPalette[WORM2BASECOLOR+1] = 0x008888ff;
      plPalette[WORM2BASECOLOR+2] = 0x000000ff;

      /* Worm Colors 3 */
      plPalette[WORM3BASECOLOR] = 0x00eeffee;
      plPalette[WORM3BASECOLOR+1] = 0x0088ff88;
      plPalette[WORM3BASECOLOR+2] = 0x0000ff00;

      plPalette[TRON_HEAD] = 0x00ffffff;
      plPalette[TRON_TRAIL] = 0x000000aa;

      /* Bullet color */
      /* plPalette[BULLET_COLOR] = 0x00000000; */
      plPalette[BULLET_COLOR] = 0x00ffffff;

      plPalette[42] = 0x00ffbb00;

      plPalette[APPLE_COLOR] = 0x00ff4444;
      plPalette[APPLE_COLOR+1] = 0x00bbcc22;

      for(i = 0; i < 16; i++)
      {
         plPalette[60+i] = i*16;
      }
      for(i = 0; i < 16; i++)
      {
         plPalette[60+16+i] = ((i*16)<<16) | ((i*16)<<8) | 0x000000ff;
      }

      DiveSetSourcePalette(hDive, 0, 256, (PBYTE)plPalette);
      free(plPalette);
      _heapmin();
      return TRUE;
   }
   return FALSE;
}

static BOOL _Optlink generate_apple_randomization_data(void)
{
   PAPPLECHANCES ac = malloc(sizeof(APPLECHANCES));
   if(ac)
   {
      int tmp = 0;
      memset(ac, 0, sizeof(APPLECHANCES));

      /*
       * This looks odd, but it's just a way of making some apple types to appear more
       * often than others. The higher the number, the bigger chance you'll get it when
       * you take an apple.
       */
      tmp =  9;  ac->chance[LowerSpeed]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp = 10;  ac->chance[RaiseSpeed]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp = 10;  ac->chance[Reset]            = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  7;  ac->chance[AutoFire]         = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  9;  ac->chance[LowerFireRate]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp = 10;  ac->chance[RaiseFireRate]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  9;  ac->chance[LowerBulletSpeed] = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp = 10;  ac->chance[RaiseBulletSpeed] = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  8;  ac->chance[Pacifist]         = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  1;  ac->chance[SummonFrenzy]     = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  1;  ac->chance[SummonWriggly]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  6;  ac->chance[SpreadMore]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  6;  ac->chance[SpreadLess]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  3;  ac->chance[Wriggle]          = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  2;  ac->chance[FatalAttraction]  = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  4;  ac->chance[HomingBullets]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  6;  ac->chance[MoreExplosive]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  5;  ac->chance[LessExplosive]    = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  7;  ac->chance[BounceMore]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  6;  ac->chance[BounceLess]       = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  3;  ac->chance[WarpingBullets]   = ac->max_rand_value + tmp; ac->max_rand_value += tmp;
      tmp =  2;  ac->chance[DrunkenMissiles]  = ac->max_rand_value + tmp; ac->max_rand_value += tmp;


      ac->max_rand_value += 1;

      appletype_randomization_data = ac;
      return TRUE;
   }
   return FALSE;
}


static void _Optlink destroy_apple_randomization_data(void)
{
   if(appletype_randomization_data)
   {
      free(appletype_randomization_data);
      appletype_randomization_data = NULL;
      _heapmin();
   }
}

static BOOL _Optlink generate_rle_explosion_data(void)
{
   int i = 0;
   for(; i < EXPLOSION_SIZES; i++)
   {
      /* if((rle_explosion_data[i] = generate_rlecircle_data(3 + i*5)) == NULL) */
      if((rle_explosion_data[i] = generate_rlecircle_data(3 + i*10)) == NULL)
      {
         for(; i >= 0; i--)
         {
            free(rle_explosion_data[i]);
            rle_explosion_data[i] = NULL;
         }
         return FALSE;
      }
   }
   return TRUE;
}

static void _Optlink destroy_rle_explosion_data(void)
{
   int i = 0;
   for(; i < EXPLOSION_SIZES; i++)
   {
      free(rle_explosion_data[i]);
      rle_explosion_data[i] = NULL;
   }
   _heapmin();
}


static int _Optlink startBlitterThread(HDIVE hDive, PPIXELBUFFER pb, HEV hevPaint, HEV hevTermPainter, HEV hevPainterTerminated)
{
   int tidBlitter = -1;
   APIRET rc = NO_ERROR;
   BLITTHREADPARAMS threadParams = { 0 };

   if((rc = DosCreateEventSem(NULL, &threadParams.hevInitCompleted, 0UL, FALSE)) == NO_ERROR)
   {
      threadParams.hDive = hDive;
      threadParams.pb = pb;
      threadParams.hevPaint = hevPaint;
      threadParams.hevTermPainter = hevTermPainter;
      threadParams.hevPainterTerminated = hevPainterTerminated;
      if((tidBlitter = _beginthread(BlitThread, NULL, 8192, &threadParams)) != -1)
      {
         if((rc = DosWaitEventSem(threadParams.hevInitCompleted, (ULONG)SEM_INDEFINITE_WAIT)) != NO_ERROR)
         {
            dprintf(("e %s: DosWaitEventSem() returned %08x (%u)\n", __FUNCTION__, rc, rc));
         }
      }
      if((rc = DosCloseEventSem(threadParams.hevInitCompleted)) != NO_ERROR)
      {
         dprintf(("e %s: DosCloseEventSem() returned %08x (%u)\n", __FUNCTION__, rc, rc));
      }
   }
   else
   {
      dprintf(("e %s: DosCloseEventSem() returned %08x (%u)\n", __FUNCTION__, rc, rc));
   }
   return tidBlitter;
}

static BOOL _Optlink terminateBlitterThread(int tidBlitter, HEV hevPaint, HEV hevTermPainter, HEV hevPainterTerminated)
{
   BOOL fSuccess = TRUE;
   if(tidBlitter != -1)
   {
      APIRET rc = NO_ERROR;
      ULONG cPosts = 0;

      /* Post blitter termination event */
      if((rc = DosPostEventSem(hevTermPainter)) == NO_ERROR)
      {
         /* Make blitter take another turn so it will recieve it's termination event */
         rc = DosPostEventSem(hevPaint);
      }

      /* Give the painter thread three seconds to report back that it's terminating correctly */
      if((rc = DosWaitEventSem(hevPainterTerminated, 3000)) == ERROR_TIMEOUT)
      {
         /* The blitter thread hasn't reported in, so let's just kill it and hope for the best */
         DosKillThread((TID)tidBlitter);

         DosResetEventSem(hevPaint, &cPosts);
         DosResetEventSem(hevTermPainter, &cPosts);
         fSuccess = FALSE;
      }
      DosResetEventSem(hevPainterTerminated, &cPosts);
   }
   return fSuccess;
}

/****************************************************************************\
 *                                                                          *
 *             H I G H  R E S O L U T I O N  T I M E R                      *
 *                                                                          *
\****************************************************************************/
static PULONG _Optlink hrtInitialize(PHFILE phTimer)
{
   APIRET rc = NO_ERROR;
   HFILE hTimer = NULLHANDLE;
   ULONG ulAction = 0;
   ULONG *pulTimer = NULL;

   rc = DosOpen( "TIMER0$ ", &hTimer, &ulAction, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
      OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
   if(rc == NO_ERROR)
   {
      ULONG * _Seg16 pulTimer16;
      ULONG ulSize2 = sizeof(pulTimer16);
      ULONG ulResolution = 0;
      ULONG cbData = sizeof(ulResolution);

      *phTimer = hTimer;

      dprintf(("TIMER0$ opened. File Handle is %lu\n", hTimer));

      rc = DosDevIOCtl(hTimer, 0x80, 0x01, NULL, 0, NULL, &ulResolution, sizeof(ulResolution), &cbData);
      if(rc == NO_ERROR)
      {
         dprintf(("Timer resolution: %ld\n", ulResolution));
      }

      if(ulResolution != 0)
      {
         ULONG  ulResolution = 1;
         ULONG  ulSize1 = sizeof(ulResolution);

         if((rc = DosDevIOCtl(hTimer, 0x80, 0x02, &ulResolution, ulSize1, &ulSize1, NULL, 0, NULL)) != NO_ERROR)
         {
            dprintf(("e Couldn't set timer resolution! DosDevIOCtl() returned %08x (%u)\n", rc, rc));
         }
      }

      if((rc = DosDevIOCtl(hTimer, 0x80, 0x03, NULL, 0, NULL, &pulTimer16, ulSize2, &ulSize2)) == NO_ERROR)
      {
         pulTimer = pulTimer16;    /* converts a 16:16 pointer to a 0:32 pointer */
         dprintf(("Got HRT pointer: %08x\n", pulTimer));
      }
      else
      {
         dprintf(("e Couldn't query timer resolution! DosDevIOCtl() returned %08x (%u)\n", rc, rc));
      }
   }
   else
   {
      dprintf(("e Couldn't open TIMER0$! DosOpen() returned %08x (%u)\n", rc, rc));
   }
   return pulTimer;
}

static BOOL _Optlink hrtTerminate(HFILE hTimer, PULONG pulTimer)
{
   APIRET rc = NO_ERROR;
   if(pulTimer)
   {
      ULONG cbData = sizeof(pulTimer);
      if((rc = DosDevIOCtl(hTimer, 0x80, 0x04, pulTimer, cbData, &cbData, NULL, 0, NULL)) != NO_ERROR)
      {
         dprintf(("e Couldn't release HRT pointer! DosDevIOCtl() returned %08x (%u)\n", rc, rc));
      }
   }
   DosClose(hTimer);
   return TRUE;
}

void _Inline hrtBlock(HFILE hTimer, ULONG ulDelay)
{
   APIRET rc = NO_ERROR;
   ULONG cbData = sizeof(ulDelay);
   if((rc = DosDevIOCtl(hTimer, 0x80, 0x05, &ulDelay, cbData, &cbData, NULL, 0, NULL)) != NO_ERROR)
   {
      dprintf(("e HRT delay error! DovDevIOCtl() returned %08x (%u)\n", rc, rc));
   }
}


/****************************************************************************\
 *                                                                          *
 *            W O R M  A T T R I B U T E  M O D I F I E R S                 *
 *                                                                          *
\****************************************************************************/
int _Inline inc_worm_speed(PWORMINFO worm)
{
   int prev = worm->iSpeed;
   worm->iSpeed = min(worm->iSpeed+1, WORM_SPEEDS-1);
   worm->move_delay = worm_speed_delays[worm->iSpeed];
   return prev != worm->iSpeed;
}
int _Inline dec_worm_speed(PWORMINFO worm)
{
   int prev = worm->iSpeed;
   worm->iSpeed = max(worm->iSpeed-1, 0);
   worm->move_delay = worm_speed_delays[worm->iSpeed];
   return prev != worm->iSpeed;
}

int _Inline inc_worm_firerate(PWORMINFO worm)
{
   int prev = worm->iFireRate;
   worm->iFireRate = min(worm->iFireRate+1, WORM_FIRE_RATES-1);
   worm->shoot_delay = worm_shoot_delays[worm->iFireRate];
   return prev != worm->iFireRate;
}
int _Inline dec_worm_firerate(PWORMINFO worm)
{
   int prev = worm->iFireRate;
   worm->iFireRate = max(worm->iFireRate-1, 0);
   worm->shoot_delay = worm_shoot_delays[worm->iFireRate];
   return prev != worm->iFireRate;
}

int _Inline inc_worm_bullet_speed(PWORMINFO worm)
{
   int prev = worm->iBulletSpeed;
   worm->iBulletSpeed = min(worm->iBulletSpeed+1, BULLET_SPEEDS-1);
   return prev != worm->iBulletSpeed;
}
int _Inline dec_worm_bullet_speed(PWORMINFO worm)
{
   int prev = worm->iBulletSpeed;
   worm->iBulletSpeed = max(worm->iBulletSpeed-1, 0);
   return prev != worm->iBulletSpeed;
}

int _Inline inc_worm_bulletpower(PWORMINFO worm)
{
   int prev = worm->bullet_power;
   worm->bullet_power = min(worm->bullet_power+1, EXPLOSION_SIZES-1);
   return prev != worm->bullet_power;
}
int _Inline dec_worm_bulletpower(PWORMINFO worm)
{
   int prev = worm->bullet_power;
   worm->bullet_power = max(worm->bullet_power-1, 0);
   return prev != worm->bullet_power;
}

int _Inline inc_worm_bullets_per_shot(PWORMINFO worm)
{
   int prev = worm->bullets_per_shot;
   worm->bullets_per_shot = min(worm->bullets_per_shot+2, 5);
   return prev != worm->bullets_per_shot;
}

int _Inline dec_worm_bullets_per_shot(PWORMINFO worm)
{
   int prev = worm->bullets_per_shot;
   worm->bullets_per_shot = max(worm->bullets_per_shot-2, 1);
   return prev != worm->bullets_per_shot;
}

int _Inline inc_worm_bullets_bounce(PWORMINFO worm)
{
   int prev = worm->bullet_bounces;
   worm->bullet_bounces = min(worm->bullet_bounces+3, 7);
   return prev != worm->bullet_bounces;
}

int _Inline dec_worm_bullets_bounce(PWORMINFO worm)
{
   int prev = worm->bullet_bounces;
   worm->bullet_bounces = max(worm->bullet_bounces-1, 0);
   return prev != worm->bullet_bounces;
}




/****************************************************************************\
 *                                                                          *
 *                     B L I T T E R  T H R E A D                           *
 *                                                                          *
\****************************************************************************/
static void _Optlink BlitThread(void *param)
{
   PBLITTHREADPARAMS threadParams = (PBLITTHREADPARAMS)param;
   APIRET rc = NO_ERROR;
   HDIVE hDive = threadParams->hDive;
   PPIXELBUFFER pb = threadParams->pb;
   HEV hevPaint = threadParams->hevPaint;
   HEV hevTermPainter = threadParams->hevTermPainter;
   HEV hevPainterTerminated = threadParams->hevPainterTerminated;
   PBYTE pbLineMask = calloc(pb->cy, sizeof(BYTE));

   if((rc = DosPostEventSem(threadParams->hevInitCompleted)) != NO_ERROR)
   {
      _endthread();
   }

   memset(pbLineMask, 0x00, pb->cy*sizeof(BYTE));

   /* Wait for a paint event */
   while((rc = DosWaitEventSem(hevPaint, (ULONG)SEM_INDEFINITE_WAIT)) == NO_ERROR)
   {
      ULONG cPosts;
      int i;

      /* Reset the paint event -- this should be done *before* the DiveBlitImageLines() call! */
      DosResetEventSem(hevPaint, &cPosts);

      /*
       * Access to pb->afLineMask should be serialized with mutex semaphores..
       */
      for(i = 0; i < pb->cy; i++)
      {
         if(pb->afLineMask[i])
         {
            pbLineMask[i] = 0xff;
            pb->afLineMask[i] = 0x00;
         }
      }

      /* Check for a termination event */
      if((rc = DosWaitEventSem(hevTermPainter, SEM_IMMEDIATE_RETURN)) == NO_ERROR)
      {
         /* Reset the event semaphore before breaking out of loop */
         DosResetEventSem(hevTermPainter, &cPosts);

         /* A termination event was posted, break out of loop */
         break;
      }

      /* Blit! */
      if((rc = DiveBlitImageLines(hDive, pb->ulBuffer, DIVE_BUFFER_SCREEN, pbLineMask)) == DIVE_SUCCESS)
      {
         /* memset(pb->afLineMask, 0x00, pb->cy); */
         memset(pbLineMask, 0x00, pb->cy);
      }
      else
      {
         dprintf(("e DiveBlitImageLines() returned %08x (%u).\n", rc, rc));
      }
      DosSleep(0);
   }

   free(pbLineMask);

   DosPostEventSem(hevPainterTerminated);

   _endthread();
}



/****************************************************************************\
 *                                                                          *
 *                S T A T U S  B A R  F U N C T I O N S                     *
 *                                                                          *
\****************************************************************************/
static void _Optlink showAppleStatus(HWND hwndStatus, enum AppleType apple)
{
   switch(apple)
   {
      case LowerSpeed:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_LOWER_SPEED));
         break;
      case RaiseSpeed:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_RAISE_SPEED));
         break;
      case Reset:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_RESET));
         break;
      case AutoFire:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_AUTOFIRE));
         break;
      case LowerFireRate:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_LOWER_FIRERATE));
         break;
      case RaiseFireRate:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_RAISE_FIRERATE));
         break;
      case LowerBulletSpeed:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_SLOWER_BULLETS));
         break;
      case RaiseBulletSpeed:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_FASTER_BULLETS));
         break;
      case Pacifist:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_PACIFIST));
         break;
      case SummonFrenzy:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_SUMMON_FRENZY));
         break;
      case SummonWriggly:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_SUMMON_WRIGGLY));
         break;
      case SpreadMore:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_MORE_BULLETS));
         break;
      case SpreadLess:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_LESS_BULLETS));
         break;
      case Wriggle:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_WRIGGLE));
         break;
      case FatalAttraction:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_FATAL_ATTRACTION));
         break;
      case HomingBullets:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_HOMING_BULLETS));
         break;
      case MoreExplosive:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_BIGGER_EXPLOSIONS));
         break;
      case LessExplosive:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_SMALLER_EXPLOSIONS));
         break;
      case BounceMore:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_BOUNCE_MORE));
         break;
      case BounceLess:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_BOUNCE_LESS));
         break;
      case WarpingBullets:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_WARPINGBULLETS));
         break;
      case DrunkenMissiles:
         WinPostMsg(hwndStatus, SBMSG_QUEUE_RES_STRING, MPFROMLONG(NULLHANDLE), MPFROMLONG(IDS_APPLE_DRUNKEN_MISSILES));
         break;
   }
}


/****************************************************************************\
 *                                                                          *
 *                 S A M P L E S   I N I T / T E R M                        *
 *                                                                          *
\****************************************************************************/
static void _Optlink loadSamples(void* param)
{
   int i = 0;
   APIRET rc = NO_ERROR;
   PTIB ptib = NULL;
   PPIB ppib = NULL;
   char path[256] = "";
   char *p = NULL;

   DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MINIMUM, 0);

   if((rc = DosGetInfoBlocks(&ptib, &ppib)) == NO_ERROR)
   {
      if((rc = DosQueryModuleName(ppib->pib_hmte, sizeof(path), path)) == NO_ERROR)
      {
         p = path;

         while(*p)
            p++;
         while(*p != '\\')
            p--;
         p++;
         strcpy(p, "sounds\\");
         while(*p)
            p++;
      }
   }
   #ifdef DEBUG
   strcpy(path, "c:\\sound\\halfworm\\");
   p = path;
   while(*p)
      p++;
   #endif
   if(p == NULL)
      return;

   for(i = 0; i < EXPLOSION_SIZES; i++)
   {
      sprintf(p, "explosion%d.wav", i);
      samples.explosion[i] = loadSample(path);
      if(samples.explosion[i])
         dputs((path));
   }

   strcpy(p, "bullet_bounce_wall.wav");
   samples.bullet_bounce_wall = loadSample(path);
   if(samples.bullet_bounce_wall)
      dputs(path);

   strcpy(p, "bullet_bounce_worm.wav");
   samples.bullet_bounce_worm = loadSample(path);
   if(samples.bullet_bounce_worm)
      dputs((path));

   strcpy(p, "worm_shoot.wav");
   samples.worm_shoot = loadSample(path);
   if(samples.worm_shoot)
      dputs((path));

   strcpy(p, "worm_died.wav");
   samples.worm_died = loadSample(path);
   if(samples.worm_died)
      dputs((path));

   strcpy(p, "upgraded_weaponary.wav");
   samples.upgraded_weaponary = loadSample(path);
   if(samples.upgraded_weaponary)
      dputs((path));

   strcpy(p, "ate_apple.wav");
   samples.ate_apple = loadSample(path);
   if(samples.ate_apple)
      dputs((path));

   strcpy(p, "bullet_bounce_tron.wav");
   samples.bullet_bounce_tron = loadSample(path);
   if(samples.bullet_bounce_tron)
      dputs((path));

   _endthread();
}

static void _Optlink releaseSamples(void)
{
   int i = 0;

   destroySample(samples.bullet_bounce_tron);

   destroySample(samples.worm_shoot);

   destroySample(samples.bullet_bounce_worm);

   destroySample(samples.bullet_bounce_wall);

   for(i = 0; i < EXPLOSION_SIZES; i++)
      destroySample(samples.explosion[i]);
}






/****************************************************************************\
 *                                                                          *
 *                G A M E  T H R E A D  W A T C H  D O G                    *
 *                                                                          *
\****************************************************************************/
#ifdef DEBUG
/*
 * If game thread doesn't post a heartbeat event at least once a second, this
 * thread tells us where game thread locked up.
 */
static void _Optlink WatchDog(void *param)
{
   APIRET rc = NO_ERROR;
   while((rc = DosWaitEventSem(hevHeartBeat, 1000)) == NO_ERROR)
   {
      ULONG cPosts = 0;
      rc = DosResetEventSem(hevHeartBeat, &cPosts);
   }
   dprintf(("GameThreadState=%08x\n", GameThreadState));
   DosBeep(10000, 250);
   _endthread();
}
#endif
