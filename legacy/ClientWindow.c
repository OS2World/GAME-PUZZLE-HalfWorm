#pragma strings(readonly)

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINMENUS
#define INCL_WINMESSAGEMGR
#define INCL_WININPUT
#define INCL_WINSTDSPIN
#define INCL_WINSTDSLIDER
#define INCL_WINDIALOGS
#define INCL_WINSHELLDATA
#define INCL_DOSSEMAPHORES
#define INCL_GPIREGIONS
#define INCL_GPILOGCOLORTABLE
#define INCL_OS2MM

#include <os2.h>
#include <os2me.h>

#include <dive.h>
#include <fourcc.h>

#include <memory.h>
#include <malloc.h>
#include <process.h>
#include <stdlib.h>

#include "ClientWindow.h"
#include "GameThread.h"
#include "resources.h"
#include "configuration.h"
#include "pmx.h"

#include "debug.h"



#define DIVE_MAX_RECT                    50


typedef struct _WINDOWDATA
{
   HWND hwndFrame;
   HWND hwndMenu;
   SIZEL sizlWindow;
   HDIVE hDive;
   SETUP_BLITTER SetupBlitter;
   SIZEL sizlGameBitmap;
   int tidGameEngine;
   char abPal[256];
   HMQ hmqGameThread;
   BYTE abScanCodes[256];
}WINDOWDATA, *PWINDOWDATA;


typedef struct _BOARDSIZEINFO
{
   USHORT cb;
   SIZEL sizlBoard;
   HWND hwndFrame;
}BOARDSIZEINFO, *PBOARDSIZEINFO;


typedef struct _SETKEYSDATA
{
   USHORT cb;
   int iPlayer;
   int iKey;
   int cKeysSelected;
   BYTE used_keys[6];
   PLAYERKEYS keys[2];
   BOOL fNewPlayer;
   BYTE prev_key;
}SETKEYSDATA, *PSETKEYSDATA;

typedef struct _SETVOLUMEDATA
{
   USHORT cb;
   LONG volume;
}SETVOLUMEDATA, *PSETVOLUMEDATA;


/*
 * Function prototypes - external functions
 */
extern void _Optlink GameThread(void *param);


/*
 * Function Prototypes - local functions
 */
static MRESULT EXPENTRY WindowProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static BOOL _Optlink processCreateMessage(HWND hwnd, MPARAM mp1);
static void _Optlink resetWindow(HWND hwnd, LONG cxGameBitmap, LONG cyGameBitmap);

static MRESULT EXPENTRY WelcomeDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY KeysDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void _Optlink processKeysDialogCharMsg(HWND hwnd, MPARAM mp1, PSETKEYSDATA setKeyData);
static BOOL _Optlink saveControls(HWND hwnd, PPLAYERKEYS playerKeys);

static void _Optlink processSetBoardSizeMenuItemMessage(HWND hwnd);
static MRESULT EXPENTRY BoardSizeDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static MRESULT EXPENTRY SFXVolDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static void _Optlink centerDialogWindow(HWND hwndReference, HWND hwndDialog);



BOOL _Optlink registerClientClass(HAB hab)
{
   return WinRegisterClass(hab, WC_WORMCLIENT, WindowProcedure, 0UL, sizeof(PWINDOWDATA));
}


static MRESULT EXPENTRY WindowProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   PWINDOWDATA wd = (PWINDOWDATA)WinQueryWindowPtr(hwnd, 0UL);
   HPS hps;
   RECTL rect;
   SHORT scxold;
   SHORT scyold;
   SHORT scxnew;
   SHORT scynew;

   switch(msg)
   {
      case WM_CREATE:
         mReturn = (MRESULT)TRUE;
         if(processCreateMessage(hwnd, mp1))
            mReturn = (MRESULT)FALSE;
         break;

      /*
       * NOTE: I tried scanning for more keys in the game thread, but it failed due
       *       to some unexplained reason. I got the tip (hello Marty!) to manage my
       *       own scan codes though PM, but the response time was too slow. Probably
       *       Due to the fact that I'm using the HRT. Anyway, I scrapped the
       *       "quick rotation" idea, so I don't need this any longer.
       */
#ifdef PM_CONTROLS_KEYS
      case WM_CHAR:
         {
            USHORT fsflags = SHORT1FROMMP(mp1);
/*            BYTE ucrepeat = LOBYTE(SHORT2FROMMP(mp1)); */
            BYTE ucscancode = HIBYTE(SHORT2FROMMP(mp1));
/*            USHORT usch = SHORT1FROMMP(mp2);
            USHORT usvk = SHORT2FROMMP(mp2); */
/*
            if(fsflags & KC_SCANCODE)
            {
               if((fsflags & KC_PREVDOWN) == 0 && wd->abScanCodes[ucscancode] == 0x00)
               {
                  wd->abScanCodes[ucscancode] = 0x80;
                  break;
               }

               if(fsflags & KC_KEYUP)
               {
                  wd->abScanCodes[ucscancode] = 0x00;
                  break;
               }
            }
*/
            fHandled = FALSE;
         }

      #ifdef DEBUG_KEYS
/*
         if( (SHORT1FROMMP(mp1) & KC_VIRTUALKEY) && (SHORT2FROMMP(mp2) == VK_PAUSE) )
         {
            WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_PAUSE, MPVOID, MPVOID);
         }
*/
         {
            USHORT fsflags = SHORT1FROMMP(mp1);
            BYTE ucrepeat = LOBYTE(SHORT2FROMMP(mp1));
            BYTE ucscancode = HIBYTE(SHORT2FROMMP(mp1));
            USHORT usch = SHORT1FROMMP(mp2);
            USHORT usvk = SHORT2FROMMP(mp2);

            dprintf(("New keypress\n------------------\n"));
            dprintf(("fsflags:"));
            if(fsflags & KC_CHAR) dprintf((" KC_CHAR"));
            if(fsflags & KC_SCANCODE) dprintf((" KC_SCANCODE"));
            if(fsflags & KC_VIRTUALKEY) dprintf((" KC_VIRTUALKEY"));
            if(fsflags & KC_KEYUP) dprintf((" KC_KEYUP"));
            if(fsflags & KC_PREVDOWN) dprintf((" KC_PREVDOWN"));
            if(fsflags & KC_DEADKEY) dprintf((" KC_DEADKEY"));
            if(fsflags & KC_COMPOSITE) dprintf((" KC_COMPOSITE"));
            if(fsflags & KC_INVALIDCOMP) dprintf((" KC_INVALIDCOMP"));
            if(fsflags & KC_LONEKEY) dprintf((" KC_LONEKEY"));
            if(fsflags & KC_SHIFT) dprintf((" KC_SHIFT"));
            if(fsflags & KC_ALT) dprintf((" KC_ALT"));
            if(fsflags & KC_CTRL) dprintf((" KC_CTRL"));
            dprintf(("\n"));

            if(fsflags & KC_SCANCODE)   dprintf(("usscancode: %02x (%u)\n", ucscancode, ucscancode));
            if(fsflags & KC_CHAR)       dprintf(("      usch: %02x (%u) '%c'\n", usch, usch, (char)usch));
            if(fsflags & KC_VIRTUALKEY) dprintf(("      usvk: %02x (%u)\n", usvk, usvk));
         }
         break;
      #endif
#endif

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDM_GAME_START:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_START_GAME, MPVOID, MPVOID);
               break;

            case IDM_GAME_EXIT:
               WinPostMsg(hwnd, WM_CLOSE, MPVOID, MPVOID);
               break;

            case IDM_SET_BOARDSIZE:
               processSetBoardSizeMenuItemMessage(hwnd);
               break;

            case IDM_SET_KEYS:
               {
                  SETKEYSDATA wndData = { sizeof(wndData) };
                  if(WinDlgBox(HWND_DESKTOP, hwnd, KeysDialogProc, (HMODULE)NULLHANDLE, DLG_CONFIG_KEYS, &wndData) == DID_OK)
                  {
                     saveControls(hwnd, wndData.keys);
                  }
               }
               break;

            case IDM_SET_SOUNDFX:
               /*
                * NOTE: GameThread is responsible for (un)checking menuitem and storing profile data
                */
               if(WinIsMenuItemChecked(wd->hwndMenu, IDM_SET_SOUNDFX))
               {
                  WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_ENABLE_SOUNDFX, MPFROMLONG(FALSE), MPVOID);
               }
               else
               {
                  WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_ENABLE_SOUNDFX, MPFROMLONG(TRUE), MPVOID);
               }
               break;

            case IDM_SET_SOUNDVOL:
               /*
                * Since the GameThread keeps all data, this thread needs to tell the GameThread
                * to send this window the volume.
                * Better Solution: Query the volume from the mixer module. Nah, too easy. :-)
                */
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_SOUNDVOL_DLG, MPVOID, MPVOID);
               break;

            case IDM_SET_DOUBLE:
               if(WinIsMenuItemChecked(wd->hwndMenu, IDM_SET_DOUBLE))
               {
                  WinCheckMenuItem(wd->hwndMenu, IDM_SET_DOUBLE, FALSE);
               }
               else
               {
                  WinCheckMenuItem(wd->hwndMenu, IDM_SET_DOUBLE, TRUE);
               }
               WinSendMsg(hwnd, WMU_RESET_WINDOW, MPVOID, MPVOID);
               break;

            #ifdef DEBUG
            case IDM_DBGW1_GROW:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_GROW, MPFROM2SHORT(0, 0), MPVOID);
               break;
            case IDM_DBGW1_FASTER:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_SPEED, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_SLOWER:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_SPEED, MPFROM2SHORT(0, -1), MPVOID);
               break;
            case IDM_DBGW1_FASTER_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLET_SPEED, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_SLOWER_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLET_SPEED, MPFROM2SHORT(0, -1), MPVOID);
               break;
            case IDM_DBGW1_FIRERATE_UP:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_FIRERATE, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_FIRERATE_DOWN:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_FIRERATE, MPFROM2SHORT(0, -1), MPVOID);
               break;
            case IDM_DBGW1_BIGGER_EXPLOSIONS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_EXPLOSION_SIZE, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_SMALLER_EXPLOSIONS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_EXPLOSION_SIZE, MPFROM2SHORT(0, -1), MPVOID);
               break;
            case IDM_DBGW1_MORE_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_PER_SHOT, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_LESS_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_PER_SHOT, MPFROM2SHORT(0, -1), MPVOID);
               break;
            case IDM_DBGW1_BOUNCE_MORE:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_BOUNCE, MPFROM2SHORT(0, 1), MPVOID);
               break;
            case IDM_DBGW1_BOUNCE_LESS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_BOUNCE, MPFROM2SHORT(0, -1), MPVOID);
               break;


            case IDM_DBGW2_GROW:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_GROW, MPFROM2SHORT(1, 0), MPVOID);
               break;
            case IDM_DBGW2_FASTER:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_SPEED, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_SLOWER:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_SPEED, MPFROM2SHORT(1, -1), MPVOID);
               break;
            case IDM_DBGW2_FASTER_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLET_SPEED, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_SLOWER_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLET_SPEED, MPFROM2SHORT(1, -1), MPVOID);
               break;
            case IDM_DBGW2_FIRERATE_UP:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_FIRERATE, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_FIRERATE_DOWN:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_WORM_FIRERATE, MPFROM2SHORT(1, -1), MPVOID);
               break;
            case IDM_DBGW2_BIGGER_EXPLOSIONS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_EXPLOSION_SIZE, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_SMALLER_EXPLOSIONS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_EXPLOSION_SIZE, MPFROM2SHORT(1, -1), MPVOID);
               break;
            case IDM_DBGW2_MORE_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_PER_SHOT, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_LESS_BULLETS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_PER_SHOT, MPFROM2SHORT(1, -1), MPVOID);
               break;
            case IDM_DBGW2_BOUNCE_MORE:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_BOUNCE, MPFROM2SHORT(1, 1), MPVOID);
               break;
            case IDM_DBGW2_BOUNCE_LESS:
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_BULLETS_BOUNCE, MPFROM2SHORT(1, -1), MPVOID);
               break;
            #endif

            default:
               fHandled = FALSE;
               break;
         }
         break;

      case WM_PAINT:
         WinPostQueueMsg(wd->hmqGameThread, WM_PAINT, MPVOID, MPVOID);
         fHandled = FALSE;
         break;

      case WM_REALIZEPALETTE:
         dprintf(("WM_REALIZEPALETTE\n"));
         if((hps = WinGetPS(hwnd)) != NULLHANDLE)
         {
            GpiQueryRealColors(hps, 0, 0, 256, (PLONG)wd->abPal);
            DiveSetDestinationPalette(wd->hDive, 0, 256, (PBYTE)wd->abPal);
            WinReleasePS(hps);
         }
         break;

      case WM_SIZE:
         scxold = SHORT1FROMMP(mp1);
         scyold = SHORT2FROMMP(mp1);
         scxnew = SHORT1FROMMP(mp2);
         scynew = SHORT2FROMMP(mp2);

         if(scxnew != scxold)
         {
            wd->sizlWindow.cx = scxnew;
         }

         if(scynew != scyold)
         {
            wd->sizlWindow.cy = scynew;
         }
         break;

      case WM_VRNENABLED:
         dived_puts(("WM_VRNENABLED"));
         if((hps = WinGetPS(hwnd)) != NULLHANDLE)
         {
            HRGN hrgnVisible = GpiCreateRegion(hps, 0L, NULL);
            if(hrgnVisible)
            {
               if(WinQueryVisibleRegion(hwnd, hrgnVisible) != RGN_ERROR)
               {
                  RGNRECT rgnCtl = { 0 };
                  POINTL ptl = { 0, 0 };
                  ULONG rc = 0;

                  rgnCtl.ircStart = 0;
                  rgnCtl.crc = DIVE_MAX_RECT;
                  rgnCtl.ulDirection = RECTDIR_LFRT_TOPBOT;
                  GpiQueryRegionRects(hps, hrgnVisible, NULL, &rgnCtl, wd->SetupBlitter.pVisDstRects);

                  WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1);

                  wd->SetupBlitter.ulStructLen = sizeof(SETUP_BLITTER);

                  wd->SetupBlitter.fInvert = 0x00000000;
                  wd->SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
                  wd->SetupBlitter.ulDitherType = 0UL;
                  wd->SetupBlitter.fccDstColorFormat = FOURCC_SCRN;

                  wd->SetupBlitter.ulSrcWidth = wd->sizlGameBitmap.cx;
                  wd->SetupBlitter.ulSrcHeight = wd->sizlGameBitmap.cy;
                  wd->SetupBlitter.ulSrcPosX = 0;
                  wd->SetupBlitter.ulSrcPosY = 0;

                  wd->SetupBlitter.ulDstWidth = wd->sizlWindow.cx;
                  wd->SetupBlitter.ulDstHeight = wd->sizlWindow.cy;
                  wd->SetupBlitter.lDstPosX = 0;
                  wd->SetupBlitter.lDstPosY = 0;
                  wd->SetupBlitter.lScreenPosX = ptl.x;
                  wd->SetupBlitter.lScreenPosY = ptl.y;
                  wd->SetupBlitter.ulNumDstRects = rgnCtl.crcReturned;

                  if((rc = DiveSetupBlitter(wd->hDive, &wd->SetupBlitter)) != DIVE_SUCCESS)
                  {
                     dprintf(("e DiveSetupBlitter() returned %08x for DiveSetupBlitter(%08x, 0).\n", rc, wd->hDive));
                  }
                  GpiDestroyRegion(hps, hrgnVisible);
               }
               WinReleasePS(hps);
            }
         }
         WinInvalidateRect(hwnd, NULL, FALSE);
         break;

      case WM_VRNDISABLED:
         dived_puts(("WM_VRNDISABLED"));
         {
            ULONG rc = DIVE_SUCCESS;
            if((rc = DiveSetupBlitter(wd->hDive, 0)) != DIVE_SUCCESS)
            {
               dprintf(("e DiveSetupBlitter() returned %08x for DiveSetupBlitter(%08x, 0).\n", rc, wd->hDive));
            }
         }
         break;
/*
      case WMU_ENABLE_DIVE:
         //
          * mp1 - Enable TRUE/FALSE
          * mp2 - PHDIVE
         //
         if(LONGFROMMP(mp1))
         {

            // Initialize DIVE data which will not (read: should not) change
            wd->SetupBlitter.ulStructLen = sizeof(SETUP_BLITTER);
            wd->SetupBlitter.fInvert = 0x00000000;
            wd->SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
            wd->SetupBlitter.ulDitherType = 0UL;
            wd->SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
            wd->SetupBlitter.ulNumDstRects = 0;

            if((rc = DiveOpen(&wd->hDive, FALSE, NULL)) == DIVE_SUCCESS)
            {
               HPS hps = NULLHANDLE;

               *(PHDIVE)mp2 = wd->hDive;

               dprintf(("i wd->hDive = %08x\n", wd->hDive));

               if((wd->SetupBlitter.pVisDstRects = (PRECTL)calloc(DIVE_MAX_RECT, sizeof(RECTL))) != NULL)
               {
                  memset(wd->SetupBlitter.pVisDstRects, 0, DIVE_MAX_RECT*sizeof(RECTL));
                  WinSetVisibleRegionNotify(hwnd, TRUE);
               }
               else
               {
                  if((rc = DiveClose(wd->hDive)) != DIVE_SUCCESS)
                  {
                     dprintf(("e DiveClose() returned %08x in CanvasWindow\n", rc));
                  }
                  wd->hDive = NULLHANDLE;
               }

               if((hps = WinGetPS(hwnd)) != NULLHANDLE)
               {
                  GpiQueryRealColors(hps, 0, 0, 256, (PLONG)wd->abPal);
                  DiveSetDestinationPalette(wd->hDive, 0, 256, (PBYTE)wd->abPal);
                  WinReleasePS(hps);
               }
            }
         }
         else
         {
            WinSetVisibleRegionNotify(hwnd, FALSE);
            if(wd->hDive)
            {
               ULONG rc = DIVE_SUCCESS;
               if((rc = DiveClose(wd->hDive)) == DIVE_SUCCESS)
               {
                  wd->hDive = NULLHANDLE;
               }
            }
         }
         break;
*/

      case WMU_RESET_WINDOW:
         resetWindow(hwnd, wd->sizlGameBitmap.cx, wd->sizlGameBitmap.cy);
         break;

      case WM_CLOSE:
         dputs(("ClientWindow got a WM_CLOSE message"));
         if(wd->hmqGameThread)
         {
            WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_TERMINATE, MPVOID, MPVOID);
         }
         else
         {
            WinPostMsg(hwnd, WM_QUIT, MPVOID, MPVOID);
         }
         break;

      case WM_DESTROY:
         free(wd);
         break;

      case WMU_FIRST_RUN:
         WinDlgBox(HWND_DESKTOP, hwnd, WelcomeDialogProc, (HMODULE)NULLHANDLE, DLG_WELCOME, NULL);
         WinSendMsg(hwnd, WM_COMMAND, MPFROM2SHORT(IDM_SET_KEYS, 0), MPVOID);
         WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_INITGAME_COMPLETE, MPVOID, MPVOID);
         break;

      case WMU_INFORM:
         switch(LONGFROMMP(mp1))
         {
            case GAMETHREAD_HMQ:
               wd->hmqGameThread = (HMQ)LONGFROMMP(mp2);
               dprintf(("Client Window got hmqGameThread=%08x\n", wd->hmqGameThread));
               break;

            case GAMETHREAD_TERMINATED:
               WinSetVisibleRegionNotify(hwnd, FALSE);
               if(wd->hDive)
               {
                  ULONG rc = DIVE_SUCCESS;
                  if((rc = DiveClose(wd->hDive)) == DIVE_SUCCESS)
                  {
                     wd->hDive = NULLHANDLE;
                  }
               }
               WinPostMsg(hwnd, WM_QUIT, MPVOID, MPVOID);
               break;
         }
         break;

      case WMU_VOLUME_DIALOG:
         {
            SETVOLUMEDATA wndData = { sizeof(wndData) };
            wndData.volume = LONGFROMMP(mp1);
            if(WinDlgBox(HWND_DESKTOP, hwnd, SFXVolDialogProc, (HMODULE)NULLHANDLE, DLG_SFXVOLUME, &wndData) == DID_OK)
            {
               WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_SET_SFX_VOLUME, MPFROMLONG(wndData.volume), MPVOID);
            }
         }
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = WinDefWindowProc(hwnd, msg, mp1, mp2);
   }
   return mReturn;
}


static BOOL _Optlink processCreateMessage(HWND hwnd, MPARAM mp1)
{
   HAB hab = WinQueryAnchorBlock(hwnd);
   PWINDOWDATA wd = NULL;
   SWP swpDesktop = { 0 };
   BOOL fSuccess = FALSE;

   WinQueryWindowPos(HWND_DESKTOP, &swpDesktop);

   if((wd = malloc(sizeof(WINDOWDATA))) != NULL)
   {
      PWORMWNDCDATA ctldata = (PWORMWNDCDATA)mp1;
      ULONG rc = DIVE_SUCCESS;
      SWP swp = { 0 };
      HPS hps = NULLHANDLE;
      SIZEL sizlGameBoard = { 320, 240 };

      memset(wd, 0, sizeof(WINDOWDATA));

      wd->hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
      wd->hwndMenu = WinWindowFromID(wd->hwndFrame, FID_MENU);

      memcpy(&wd->sizlGameBitmap, &ctldata->sizlBoard, sizeof(SIZEL));

      WinSetWindowPtr(hwnd, 0, wd);

      /* Initialize DIVE data which will not (read: should not) change */
      wd->SetupBlitter.ulStructLen = sizeof(SETUP_BLITTER);
      wd->SetupBlitter.fInvert = 0x00000000;
      wd->SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
      wd->SetupBlitter.ulDitherType = 0UL;
      wd->SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
      wd->SetupBlitter.ulNumDstRects = 0;

      if((rc = DiveOpen(&wd->hDive, FALSE, NULL)) == DIVE_SUCCESS)
      {
         dprintf(("i wd->hDive = %08x\n", wd->hDive));

         if((wd->SetupBlitter.pVisDstRects = (PRECTL)calloc(DIVE_MAX_RECT, sizeof(RECTL))) != NULL)
         {
            memset(wd->SetupBlitter.pVisDstRects, 0, DIVE_MAX_RECT*sizeof(RECTL));
            WinSetVisibleRegionNotify(hwnd, TRUE);
         }
         else
         {
            if((rc = DiveClose(wd->hDive)) != DIVE_SUCCESS)
            {
               dprintf(("e DiveClose() returned %08x in CanvasWindow\n", rc));
            }
            wd->hDive = NULLHANDLE;
         }
      }
   }

   if(wd->hDive)
   {
      HPS hps = NULLHANDLE;
      if((hps = WinGetPS(hwnd)) != NULLHANDLE)
      {
         GpiQueryRealColors(hps, 0, 0, 256, (PLONG)wd->abPal);
         DiveSetDestinationPalette(wd->hDive, 0, 256, (PBYTE)wd->abPal);
         WinReleasePS(hps);
      }
   }


   if(wd->hDive)
   {
      GAMETHREADPARAMS threadParams = { 0 };
      LONG lLength = 0;
      APIRET rc = NO_ERROR;
      PAPPPRF prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
      if(prf)
      {
         RECTL rclDesktop = { 0, 0, swpDesktop.cx, swpDesktop.cy };
         SIZEL sizlClient = { 320, 240 };
         BOOL fDouble = TRUE;

         fDouble = readProfileBoolean(prf, IDS_PRFKEY_DOUBLE_SIZE, TRUE);
         WinCheckMenuItem(wd->hwndMenu, IDM_SET_DOUBLE, fDouble);

         WinCalcFrameRect(wd->hwndFrame, &rclDesktop, TRUE);
         sizlClient.cx = rclDesktop.xRight-rclDesktop.xLeft;
         sizlClient.cy = rclDesktop.yTop-rclDesktop.yBottom;
         if(fDouble)
         {
            sizlClient.cx /= 2;
            sizlClient.cy /= 2;
         }

         /* Just in case ... */
         sizlClient.cx = max(sizlClient.cx, 320);
         sizlClient.cy = max(sizlClient.cy, 240);

         /*
          * Query default size from profile, default to maximum size
          */
         wd->sizlGameBitmap.cx = readProfileLong(prf, IDS_PRFKEY_BOARD_WIDTH, sizlClient.cx);
         wd->sizlGameBitmap.cy = readProfileLong(prf, IDS_PRFKEY_BOARD_HEIGHT, sizlClient.cy);

         closeAppProfile(prf);
      }


      if((rc = DosCreateEventSem(NULL, &threadParams.hevInitCompleted, 0UL, FALSE)) == NO_ERROR)
      {
         threadParams.hwnd = hwnd;
         threadParams.hDive = wd->hDive;
         threadParams.sizlGameBitmap.cx = wd->sizlGameBitmap.cx;
         threadParams.sizlGameBitmap.cy = wd->sizlGameBitmap.cy;
         threadParams.abKeyStates = wd->abScanCodes;

         if((wd->tidGameEngine = _beginthread(GameThread, NULL, 16384, (PVOID)&threadParams)) != -1)
         {
            if((rc = DosWaitEventSem(threadParams.hevInitCompleted, 1000)) != NO_ERROR)
            {
               dprintf(("e DosWaitEventSem() returned %08x (%u)\n", rc, rc));
            }
            if(wd->tidGameEngine != -1)
            {
               fSuccess = TRUE;
            }
         }
         if((rc = DosCloseEventSem(threadParams.hevInitCompleted)) != NO_ERROR)
         {
            dprintf(("e DosCloseEventSem() returned %08x (%u)\n", rc, rc));
         }
      }
   }
   return fSuccess;
}



static void _Optlink resetWindow(HWND hwnd, LONG cxGameBitmap, LONG cyGameBitmap)
{
   HWND hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
   HWND hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
   RECTL rclClient = { 0, 0, cxGameBitmap, cyGameBitmap };
   SWP swpDesk = { 0 };
   SWP swp = { 0 };

   if(WinIsMenuItemChecked(hwndMenu, IDM_SET_DOUBLE))
   {
      rclClient.xRight *= 2;
      rclClient.yTop *= 2;
   }

   WinCalcFrameRect(hwndFrame, &rclClient, FALSE);

   WinQueryWindowPos(HWND_DESKTOP, &swpDesk);

   swp.cx = rclClient.xRight-rclClient.xLeft;
   swp.cy = rclClient.yTop-rclClient.yBottom;
   swp.x = (swpDesk.cx/2)-(swp.cx/2);
   swp.y = (swpDesk.cy/2)-(swp.cy/2);
   WinSetWindowPos(hwndFrame, HWND_TOP, swp.x, swp.y, swp.cx, swp.cy, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_SHOW | SWP_ACTIVATE);
}





static MRESULT EXPENTRY WelcomeDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   SWP swpParent;
   SWP swpThis;

   switch(msg)
   {
      case WM_INITDLG:
         centerDialogWindow(WinQueryWindow(hwnd, QW_PARENT), hwnd);
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
   return mReturn;
}


static MRESULT EXPENTRY KeysDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   char tmp[256] = "";
   LONG lLength;
   PSETKEYSDATA wd = (PSETKEYSDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   HAB hab;
   SWP swpParent;
   SWP swpThis;

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PSETKEYSDATA)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);

         centerDialogWindow(WinQueryWindow(hwnd, QW_PARENT), hwnd);

         hab = WinQueryAnchorBlock(hwnd);

         if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PLAYER1, sizeof(tmp), tmp)) != 0L)
         {
            WinSetDlgItemText(hwnd, ST_PLAYER, tmp);
         }
         if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_ANTICLOCKROT, sizeof(tmp), tmp)) != 0L)
         {
            WinSetDlgItemText(hwnd, ST_KEYDEF, tmp);
         }
         break;

      case WM_CHAR:
         processKeysDialogCharMsg(hwnd, mp1, wd);
         if(wd->fNewPlayer && wd->iPlayer < 2)
         {
            wd->fNewPlayer = FALSE;
            if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PLAYER1+wd->iPlayer, sizeof(tmp), tmp)) != 0L)
               WinSetDlgItemText(hwnd, ST_PLAYER, tmp);
         }
         switch(wd->iKey)
         {
            case 0:
               if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_ANTICLOCKROT, sizeof(tmp), tmp)) != 0L)
                  WinSetDlgItemText(hwnd, ST_KEYDEF, tmp);
               break;
            case 1:
               if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_CLOCKROT, sizeof(tmp), tmp)) != 0L)
                  WinSetDlgItemText(hwnd, ST_KEYDEF, tmp);
               break;
            case 2:
               if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_FIRE, sizeof(tmp), tmp)) != 0L)
                  WinSetDlgItemText(hwnd, ST_KEYDEF, tmp);
               break;
         }
         if(wd->cKeysSelected == 6)
         {
            WinDismissDlg(hwnd, DID_OK);
         }
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
   return mReturn;
}

static void _Optlink processKeysDialogCharMsg(HWND hwnd, MPARAM mp1, PSETKEYSDATA setKeyData)
{
   USHORT fsflags = SHORT1FROMMP(mp1);
   BYTE ucscancode = HIBYTE(SHORT2FROMMP(mp1));

   if((fsflags & (KC_SCANCODE|KC_LONEKEY)) == (KC_SCANCODE|KC_LONEKEY) && (ucscancode == setKeyData->prev_key))
   {
      int i = 0;

      /*
       * Check if button is used already
       */
      for(i = 0; i < setKeyData->cKeysSelected; i++)
      {
         if(setKeyData->used_keys[i] == ucscancode)
            return;
      }

      /*
       * Key is unique!
       */
      switch(setKeyData->iKey)
      {
         case 0:
            setKeyData->keys[setKeyData->iPlayer].bCounterClockwise = ucscancode;
            break;
         case 1:
            setKeyData->keys[setKeyData->iPlayer].bClockwise = ucscancode;
            break;
         case 2:
            setKeyData->keys[setKeyData->iPlayer++].bFire = ucscancode;
            setKeyData->fNewPlayer = TRUE;
            break;
      }
      setKeyData->iKey = (setKeyData->iKey+1) % 3;
      setKeyData->used_keys[setKeyData->cKeysSelected++] = ucscancode;
   }

   if((fsflags & KC_PREVDOWN) == 0)
   {
      setKeyData->prev_key = ucscancode;
      return;
   }
}

static BOOL _Optlink saveControls(HWND hwnd, PPLAYERKEYS playerKeys)
{
   HAB hab = WinQueryAnchorBlock(hwnd);
   BOOL fSuccess = FALSE;
   PAPPPRF prf = NULL;

   prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
   if(prf)
   {
      int i = 0;
      fSuccess = TRUE;
      for(; i < 2; i++)
      {
         savePlayerKeys(hab, prf, i, &playerKeys[i]);
      }
      closeAppProfile(prf);
   }

   return fSuccess;
}


static void _Optlink processSetBoardSizeMenuItemMessage(HWND hwnd)
{
   BOARDSIZEINFO wnddata = { sizeof(wnddata) };
   PWINDOWDATA wd = (PWINDOWDATA)WinQueryWindowPtr(hwnd, 0);

   wnddata.hwndFrame = WinQueryWindow(hwnd, QW_PARENT);
   memcpy(&wnddata.sizlBoard, &wd->sizlGameBitmap, sizeof(SIZEL));
   if(WinDlgBox(HWND_DESKTOP, hwnd, BoardSizeDialogProc, (HMODULE)NULLHANDLE, DLG_BOARD_SIZE, &wnddata) == DID_OK)
   {
      dputs(("Returned from WinDlgBox"));
      if((wnddata.sizlBoard.cx != wd->sizlGameBitmap.cx) || (wnddata.sizlBoard.cy != wd->sizlGameBitmap.cy))
      {
         HAB hab = WinQueryAnchorBlock(hwnd);
         PAPPPRF prf = NULL;

         dputs(("New size!"));
         memcpy(&wd->sizlGameBitmap, &wnddata.sizlBoard, sizeof(SIZEL));

         prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
         if(prf)
         {
            writeProfileLong(prf, IDS_PRFKEY_BOARD_WIDTH, wnddata.sizlBoard.cx);
            writeProfileLong(prf, IDS_PRFKEY_BOARD_HEIGHT, wnddata.sizlBoard.cy);
            closeAppProfile(prf);
         }
         WinPostQueueMsg(wd->hmqGameThread, GTHRDMSG_NEW_BITMAP_SIZE, MPFROMLONG(wd->sizlGameBitmap.cx), MPFROMLONG(wd->sizlGameBitmap.cy));
      }
   }
}



static MRESULT EXPENTRY BoardSizeDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   PBOARDSIZEINFO wd = (PBOARDSIZEINFO)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   RECTL rect;
   SWP swp;

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PBOARDSIZEINFO)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);

         WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETLIMITS, MPFROMLONG(2048), MPFROMLONG(320));
         WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETLIMITS, MPFROMLONG(2048), MPFROMLONG(240));

         WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETCURRENTVALUE, MPFROMLONG(wd->sizlBoard.cx), MPVOID);
         WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETCURRENTVALUE, MPFROMLONG(wd->sizlBoard.cy), MPVOID);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case BN_MAX_SIZE:
               WinQueryWindowPos(HWND_DESKTOP, &swp);
               rect.xLeft = rect.yBottom = 0;
               rect.xRight = swp.cx;
               rect.yTop = swp.cy;
               WinCalcFrameRect(wd->hwndFrame, &rect, TRUE);
               WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETCURRENTVALUE, MPFROMLONG(rect.xRight-rect.xLeft), MPVOID);
               WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETCURRENTVALUE, MPFROMLONG(rect.yTop-rect.yBottom), MPVOID);
               break;

            case BN_MAX_SIZE_DOUBLE:
               WinQueryWindowPos(HWND_DESKTOP, &swp);
               rect.xLeft = rect.yBottom = 0;
               rect.xRight = swp.cx;
               rect.yTop = swp.cy;
               WinCalcFrameRect(wd->hwndFrame, &rect, TRUE);
               WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETCURRENTVALUE, MPFROMLONG((rect.xRight-rect.xLeft)/2), MPVOID);
               WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETCURRENTVALUE, MPFROMLONG((rect.yTop-rect.yBottom)/2), MPVOID);
               break;

            case DID_OK:
               WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_QUERYVALUE, (MPARAM)&wd->sizlBoard.cx, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
               WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_QUERYVALUE, (MPARAM)&wd->sizlBoard.cy, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
            case DID_CANCEL:
               WinDismissDlg(hwnd, SHORT1FROMMP(mp1));
               break;

            default:
               fHandled = FALSE;
               break;
         }
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
   return mReturn;
}


static MRESULT EXPENTRY SFXVolDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   SWP swpParent;
   SWP swpThis;
   PSETVOLUMEDATA wd = (PSETVOLUMEDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PSETVOLUMEDATA)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);

         WinSendDlgItemMsg(hwnd, SPBN_VOLUME, SPBM_SETLIMITS, MPFROMLONG(100), MPFROMLONG(0));
         WinSendDlgItemMsg(hwnd, SPBN_VOLUME, SPBM_SETCURRENTVALUE, MPFROMLONG(wd->volume), MPVOID);

         centerDialogWindow(WinQueryWindow(hwnd, QW_PARENT), hwnd);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               WinSendDlgItemMsg(hwnd, SPBN_VOLUME, SPBM_QUERYVALUE, (MPARAM)&wd->volume, MPFROM2SHORT(0, SPBQ_UPDATEIFVALID));
               WinDismissDlg(hwnd, SHORT1FROMMP(mp1));
               break;

            default:
               fHandled = FALSE;
               break;
         }
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = WinDefDlgProc(hwnd, msg, mp1, mp2);
   }
   return mReturn;
}




static void _Optlink centerDialogWindow(HWND hwndReference, HWND hwndDialog)
{
   SWP swpParent = { 0 };
   SWP swpDialog = { 0 };

   WinQueryWindowPos(hwndDialog, &swpDialog);
   WinQueryWindowPos(hwndReference, &swpParent);
   WinSetWindowPos(hwndDialog, HWND_TOP, swpParent.cx/2-swpDialog.cx/2, swpParent.cy/2-swpDialog.cy/2, 0, 0, SWP_MOVE);
}
