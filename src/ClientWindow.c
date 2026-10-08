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
#define INCL_DOSPROCESS
#define INCL_DOSMODULEMGR
#define INCL_WINHELP

#include <os2.h>
#include <os2me.h>

#include <dive.h>
#include <fourcc.h>

#include <memory.h>
#include <malloc.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ClientWindow.h"
#include "GameThread.h"
#include "resources.h"
#include "configuration.h"
#include "pmx.h"

#include "debug.h"


#define DIVE_MAX_RECT                    50

#define LANG_EN  0
#define LANG_ES  1
#define LANG_NL  2
#define LANG_DE  3
#define LANG_FR  4
#define LANG_IT  5
#define LANG_COUNT 6

enum {
   LS_GAME=0, LS_NEWGAME, LS_PAUSE, LS_QUIT, LS_EXIT,
   LS_OPTIONS, LS_BOARDSIZE, LS_KEYS, LS_DOUBLE, LS_SOUND, LS_SOUNDEN, LS_SOUNDVOL,
   LS_LANGUAGE, LS_BACKGRND, LS_FRAME, LS_SAVEONEXIT,
   LS_HELP, LS_ABOUT, LS_HELPINDEX, LS_HELPGEN, LS_HELPUSING,
   LS_COUNT
};

static const char *lang_strings[LANG_COUNT][LS_COUNT] = {
   /* EN */ {
      "~Game", "~New Game\tCtrl+N", "~Pause Game\tCtrl+P", "~Quit Game\tCtrl+Q", "E~xit\tCtrl+X",
      "~Options", "~Board Size..", "~Keys..", "~Double window\tCtrl+D",
      "~Sound effects", "~Enabled", "~Volume..",
      "~Language", "~Background Run\tCtrl+B", "~Frame Controls\tCtrl+F",
      "~Save settings on exit", "~Help", "~About...",
      "Help ~index", "~General help", "~Using help"
   },
   /* ES */ {
      "~Juego", "~Nuevo Juego\tCtrl+N", "~Pausar Juego\tCtrl+P", "~Salir Juego\tCtrl+Q", "~Salir\tCtrl+X",
      "~Opciones", "Tama~no tablero..", "~Teclas..", "~Doble ventana\tCtrl+D",
      "Efectos de ~sonido", "~Activado", "~Volumen..",
      "~Idioma", "Fondo ~Activo\tCtrl+B", "Controles ~Marco\tCtrl+F",
      "~Guardar al salir", "A~yuda", "~Acerca de...",
      "~Indice de ayuda", "Ayuda ~general", "~Usar la ayuda"
   },
   /* NL */ {
      "~Spel", "~Nieuw Spel\tCtrl+N", "~Pauze Spel\tCtrl+P", "~Stop Spel\tCtrl+Q", "~Afsluiten\tCtrl+X",
      "~Opties", "Spelbord ~grootte..", "~Toetsen..", "~Dubbel venster\tCtrl+D",
      "~Geluidseffecten", "~Ingeschakeld", "~Volume..",
      "~Taal", "Achtergrond ~Actief\tCtrl+B", "~Frame Controls\tCtrl+F",
      "~Sla op bij sluiten", "~Help", "~Over...",
      "Help-~index", "~Algemene help", "~Help gebruiken"
   },
   /* DE */ {
      "~Spiel", "~Neues Spiel\tCtrl+N", "Spiel ~Pause\tCtrl+P", "Spiel ~Beenden\tCtrl+Q", "~Beenden\tCtrl+X",
      "~Optionen", "Brett~groesse..", "~Tasten..", "~Doppeltes Fenster\tCtrl+D",
      "~Soundeffekte", "~Aktiviert", "~Lautstaerke..",
      "~Sprache", "~Hintergrundlauf\tCtrl+B", "Rahmen-~Steuerung\tCtrl+F",
      "~Einstellungen speichern", "~Hilfe", "~Ueber...",
      "Hilfe-~Index", "~Allgemeine Hilfe", "Hilfe ~verwenden"
   },
   /* FR */ {
      "~Jeu", "~Nouveau Jeu\tCtrl+N", "~Pause Jeu\tCtrl+P", "~Quitter Jeu\tCtrl+Q", "~Quitter\tCtrl+X",
      "~Options", "Taille ~plateau..", "~Touches..", "~Double fenetre\tCtrl+D",
      "Effets ~sonores", "~Active", "~Volume..",
      "~Langue", "Arriere-plan ~Actif\tCtrl+B", "Controles ~Cadre\tCtrl+F",
      "~Sauver a la fermeture", "~Aide", "~A propos...",
      "~Index de l'aide", "Aide ~generale", "~Utiliser l'aide"
   },
   /* IT */ {
      "~Gioco", "~Nuovo Gioco\tCtrl+N", "~Pausa Gioco\tCtrl+P", "~Finire Gioco\tCtrl+Q", "~Esci\tCtrl+X",
      "~Opzioni", "Dimensione ~campo..", "~Tasti..", "Finestra ~doppia\tCtrl+D",
      "Effetti ~sonori", "~Attivato", "~Volume..",
      "~Lingua", "Sfondo ~Attivo\tCtrl+B", "Controlli ~Cornice\tCtrl+F",
      "Salva all ~uscita", "~Aiuto", "~Info...",
      "~Indice della guida", "Guida ~generale", "~Uso della guida"
   }
};

static int current_lang = LANG_EN;


typedef struct _WINDOWDATA
{
   HWND hwndFrame;
   HWND hwndMenu;
   SIZEL sizlWindow;
   HDIVE hDive;
   SETUP_BLITTER SetupBlitter;
   SIZEL sizlGameBitmap;
   int tidGameEngine;
   LONG alPal[256];    /* 256 RGB2 entries: GpiQueryRealColors writes 4 bytes per color (this was a
                          256 byte buffer, which overran WINDOWDATA and the heap) */
   HMQ hmqGameThread;
   BYTE abScanCodes[256];

   HWND hwndObject;
   HWND hwndTitleBar;
   HWND hwndSysMenu;
   HWND hwndMinMax;
   HWND hwndMenuBar;
   BOOL bFrameHidden;

   BOOL bBackgrndRun;
   BOOL bSaveOnexit;
   BOOL bFocusPaused;
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

typedef struct _KEYMAPDATA
{
   USHORT cb;
   PLAYERKEYS keys[2];
   int iCapturing;   /* -1=none, 0-5=slot being captured */
   BYTE prev_key;
}KEYMAPDATA, *PKEYMAPDATA;


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
static MRESULT EXPENTRY KeyMappingDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static void _Optlink scancodeToName(BYTE sc, char *buf, int cBuf);
static void _Optlink kmUpdateButton(HWND hwnd, USHORT btnId, BYTE sc);
static BYTE kmGetKey(PKEYMAPDATA wd, int iSlot);
static void kmSetKey(PKEYMAPDATA wd, int iSlot, BYTE v);

static void _Optlink processSetBoardSizeMenuItemMessage(HWND hwnd);
static MRESULT EXPENTRY BoardSizeDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static MRESULT EXPENTRY SFXVolDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static void _Optlink centerDialogWindow(HWND hwndReference, HWND hwndDialog);
static void _Optlink set_menu_lang(HWND hwndMenu, int lang);
static void _Optlink HelpInit(HWND hwndFrame);
static void _Optlink HelpDrop(HWND hwndFrame);
static void _Optlink HelpMessage(ULONG ulMsg);
static void _Optlink frame_show(PWINDOWDATA wd, BOOL show);
static void _Optlink load_hw_settings(HWND hwnd, PWINDOWDATA wd);
static void _Optlink save_hw_settings(HWND hwnd, PWINDOWDATA wd);



BOOL _Optlink registerClientClass(HAB hab)
{
   return WinRegisterClass(hab, WC_WORMCLIENT, WindowProcedure, 0UL, sizeof(PWINDOWDATA));
}


/*
 * Online help: IPF files help\HalfWorm_xx.hlp next to the exe, one per language
 * (English is the fallback).  The instance is recreated whenever the language changes.
 */
static const char *help_files[LANG_COUNT] = {
   "HalfWorm_en.hlp", "HalfWorm_es.hlp", "HalfWorm_nl.hlp",
   "HalfWorm_de.hlp", "HalfWorm_fr.hlp", "HalfWorm_it.hlp"
};
static const char *help_missing[LANG_COUNT] = {
   "The help file %s could not be loaded.",
   "No se pudo cargar el archivo de ayuda %s.",
   "Het helpbestand %s kon niet worden geladen.",
   "Die Hilfedatei %s konnte nicht geladen werden.",
   "Le fichier d'aide %s n'a pas pu etre charge.",
   "Impossibile caricare il file della guida %s."
};
static const char *help_title[LANG_COUNT] = {
   "HalfWorm Help", "Ayuda de HalfWorm", "HalfWorm Help",
   "HalfWorm Hilfe", "Aide de HalfWorm", "Guida di HalfWorm"
};

static HWND hwndHelp = NULLHANDLE;
static char szHelpLib[CCHMAXPATH + 20];

static void _Optlink HelpDrop(HWND hwndFrame)
{
   if(hwndHelp != NULLHANDLE) {
      WinAssociateHelpInstance(NULLHANDLE, hwndFrame);
      WinDestroyHelpInstance(hwndHelp);
      hwndHelp = NULLHANDLE;
   }
}

static void _Optlink HelpInit(HWND hwndFrame)
{
   HELPINIT hi;
   PTIB ptib;
   PPIB ppib;
   char szDir[CCHMAXPATH];
   char szMsg[CCHMAXPATH + 100];
   const char *pszFile = help_files[current_lang];
   char *p;
   FILE *fp;

   HelpDrop(hwndFrame);

   szDir[0] = '\0';
   if(DosGetInfoBlocks(&ptib, &ppib) == 0 &&
      DosQueryModuleName(ppib->pib_hmte, sizeof(szDir), szDir) == 0) {
      p = strrchr(szDir, '\\');
      if(p) p[1] = '\0'; else szDir[0] = '\0';
   }

   sprintf(szHelpLib, "%shelp\\%s", szDir, pszFile);
   fp = fopen(szHelpLib, "rb");
   if(!fp) {
      pszFile = help_files[LANG_EN];
      sprintf(szHelpLib, "%shelp\\%s", szDir, pszFile);
      fp = fopen(szHelpLib, "rb");
   }
   if(fp) fclose(fp);
   else strcpy(szHelpLib, pszFile);        /* let the system search HELP / BOOKSHELF */

   memset(&hi, 0, sizeof(hi));
   hi.cb = sizeof(hi);
   hi.phtHelpTable = (PHELPTABLE)MAKELONG(MAIN_HELP_TABLE, 0xFFFF);
   hi.pszHelpWindowTitle = (PSZ)help_title[current_lang];
   hi.fShowPanelId = CMIC_HIDE_PANEL_ID;
   hi.pszHelpLibraryName = szHelpLib;

   hwndHelp = WinCreateHelpInstance(WinQueryAnchorBlock(hwndFrame), &hi);
   if(hwndHelp == NULLHANDLE || hi.ulReturnCode ||
      !WinAssociateHelpInstance(hwndHelp, hwndFrame)) {
      if(hwndHelp != NULLHANDLE) WinDestroyHelpInstance(hwndHelp);
      hwndHelp = NULLHANDLE;
      sprintf(szMsg, help_missing[current_lang], pszFile);
      WinMessageBox(HWND_DESKTOP, hwndFrame, szMsg, "HalfWorm", 0,
                    MB_OK | MB_APPLMODAL | MB_MOVEABLE | MB_ERROR);
   }
}

static void _Optlink HelpMessage(ULONG ulMsg)
{
   if(hwndHelp != NULLHANDLE)
      WinSendMsg(hwndHelp, ulMsg, 0L, 0L);
}


static void _Optlink set_menu_lang(HWND hwndMenu, int lang)
{
   static const USHORT ids[LS_COUNT] = {
      IDM_GAME, IDM_GAME_START, IDM_GAME_PAUSE, IDM_GAME_QUIT, IDM_GAME_EXIT,
      IDM_SETTINGS, IDM_SET_BOARDSIZE, IDM_SET_KEYS, IDM_SET_DOUBLE,
      IDM_SET_SOUND, IDM_SET_SOUNDFX, IDM_SET_SOUNDVOL,
      IDM_SUB_LANG, IDM_OPT_BACKGRND, IDM_OPT_FRAME, IDM_OPT_SAVEONEXIT,
      IDM_SUB_HELP, IDM_ABOUT, IDM_HELPINDEX, IDM_HELPEXTENDED, IDM_HELPHELPFORHELP
   };
   static const USHORT langIds[LANG_COUNT] = {
      IDM_LANG_EN, IDM_LANG_ES, IDM_LANG_NL,
      IDM_LANG_DE, IDM_LANG_FR, IDM_LANG_IT
   };
   int i;

   if(lang < 0 || lang >= LANG_COUNT) lang = LANG_EN;
   current_lang = lang;

   for(i = 0; i < LS_COUNT; i++) {
      WinSendMsg(hwndMenu, MM_SETITEMTEXT,
                 MPFROM2SHORT(ids[i], TRUE),
                 MPFROMP((char*)lang_strings[lang][i]));
   }

   for(i = 0; i < LANG_COUNT; i++) {
      WinCheckMenuItem(hwndMenu, langIds[i], (i == lang));
   }

   HelpInit(WinQueryWindow(hwndMenu, QW_PARENT));
}


static void _Optlink frame_show(PWINDOWDATA wd, BOOL show)
{
   ULONG ulFcf = FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_MENU;

   if(show) {
      WinSetParent(wd->hwndTitleBar, wd->hwndFrame, FALSE);
      WinSetParent(wd->hwndSysMenu,  wd->hwndFrame, FALSE);
      WinSetParent(wd->hwndMinMax,   wd->hwndFrame, FALSE);
      WinSetParent(wd->hwndMenuBar,  wd->hwndFrame, FALSE);
      wd->bFrameHidden = FALSE;
   }
   else {
      WinSetParent(wd->hwndTitleBar, wd->hwndObject, FALSE);
      WinSetParent(wd->hwndSysMenu,  wd->hwndObject, FALSE);
      WinSetParent(wd->hwndMinMax,   wd->hwndObject, FALSE);
      WinSetParent(wd->hwndMenuBar,  wd->hwndObject, FALSE);
      wd->bFrameHidden = TRUE;
   }

   WinSendMsg(wd->hwndFrame, WM_UPDATEFRAME, (MPARAM)ulFcf, NULL);
   WinInvalidateRect(wd->hwndFrame, NULL, TRUE);
   WinUpdateWindow(wd->hwndFrame);
}


static void _Optlink load_hw_settings(HWND hwnd, PWINDOWDATA wd)
{
   HAB hab = WinQueryAnchorBlock(hwnd);
   PAPPPRF prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);

   wd->bSaveOnexit  = TRUE;     /* project standard: on by default */
   wd->bBackgrndRun = FALSE;
   current_lang     = LANG_EN;

   if(prf) {
      int lang;
      wd->bSaveOnexit  = readProfileBoolean(prf, IDS_PRFKEY_SAVEONEXIT, TRUE);
      wd->bBackgrndRun = readProfileBoolean(prf, IDS_PRFKEY_BACKGRND,   FALSE);
      lang = (int)readProfileLong(prf, IDS_PRFKEY_LANG, (LONG)LANG_EN);
      if(lang < 0 || lang >= LANG_COUNT) lang = LANG_EN;
      current_lang = lang;
      closeAppProfile(prf);
   }

   WinCheckMenuItem(wd->hwndMenu, IDM_OPT_SAVEONEXIT, wd->bSaveOnexit);
   WinCheckMenuItem(wd->hwndMenu, IDM_OPT_BACKGRND,   wd->bBackgrndRun);
}


static void _Optlink save_hw_settings(HWND hwnd, PWINDOWDATA wd)
{
   HAB hab = WinQueryAnchorBlock(hwnd);
   PAPPPRF prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
   if(prf) {
      writeProfileBoolean(prf, IDS_PRFKEY_SAVEONEXIT, wd->bSaveOnexit);
      writeProfileBoolean(prf, IDS_PRFKEY_BACKGRND,   wd->bBackgrndRun);
      writeProfileLong(prf,    IDS_PRFKEY_LANG,       (LONG)current_lang);
      closeAppProfile(prf);
   }
}


static MRESULT EXPENTRY WindowProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   PWINDOWDATA wd = (PWINDOWDATA)WinQueryWindowPtr(hwnd, 0UL);
   HPS hps;
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

            case IDM_GAME_PAUSE:
               break;

            case IDM_GAME_QUIT:
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
                  KEYMAPDATA wndData;
                  HAB habLocal = WinQueryAnchorBlock(hwnd);
                  PAPPPRF prf = openAppProfile(habLocal, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
                  int pi = 0;

                  memset(&wndData, 0, sizeof(wndData));
                  wndData.cb = sizeof(wndData);
                  wndData.iCapturing = -1;

                  for(pi = 0; pi < 2; pi++)
                  {
                     setDefaultPlayerKeys(pi, &wndData.keys[pi]);
                     if(prf)
                        loadPlayerKeys(habLocal, prf, pi, &wndData.keys[pi]);
                  }
                  if(prf) closeAppProfile(prf);

                  if(WinDlgBox(HWND_DESKTOP, hwnd, KeyMappingDlgProc, (HMODULE)NULLHANDLE, IDD_KEY_MAPPING, &wndData) == DID_OK)
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

            case IDM_OPT_BACKGRND:
               wd->bBackgrndRun = !wd->bBackgrndRun;
               WinCheckMenuItem(wd->hwndMenu, IDM_OPT_BACKGRND, wd->bBackgrndRun);
               break;

            case IDM_OPT_FRAME:
               frame_show(wd, wd->bFrameHidden);
               WinCheckMenuItem(wd->hwndMenu, IDM_OPT_FRAME, wd->bFrameHidden);
               break;

            case IDM_OPT_SAVEONEXIT:
               wd->bSaveOnexit = !wd->bSaveOnexit;
               WinCheckMenuItem(wd->hwndMenu, IDM_OPT_SAVEONEXIT, wd->bSaveOnexit);
               break;

            case IDM_LANG_EN:
               set_menu_lang(wd->hwndMenu, LANG_EN);
               break;
            case IDM_LANG_ES:
               set_menu_lang(wd->hwndMenu, LANG_ES);
               break;
            case IDM_LANG_NL:
               set_menu_lang(wd->hwndMenu, LANG_NL);
               break;
            case IDM_LANG_DE:
               set_menu_lang(wd->hwndMenu, LANG_DE);
               break;
            case IDM_LANG_FR:
               set_menu_lang(wd->hwndMenu, LANG_FR);
               break;
            case IDM_LANG_IT:
               set_menu_lang(wd->hwndMenu, LANG_IT);
               break;

            case IDM_HELPINDEX:
               HelpMessage(HM_HELP_INDEX);
               break;
            case IDM_HELPEXTENDED:
               HelpMessage(HM_EXT_HELP);
               break;
            case IDM_HELPHELPFORHELP:
               HelpMessage(HM_DISPLAY_HELP);
               break;

            case IDM_ABOUT:
               WinDlgBox(HWND_DESKTOP, hwnd, AboutDlgProc,
                         (HMODULE)NULLHANDLE, IDD_ABOUT, NULL);
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

      case WM_SETFOCUS:
         if(wd && wd->hmqGameThread) {
            if(!SHORT1FROMMP(mp2)) {
               if(!wd->bBackgrndRun) {
                  wd->bFocusPaused = TRUE;
               }
            }
            else {
               wd->bFocusPaused = FALSE;
            }
         }
         fHandled = FALSE;
         break;

      case WM_PAINT:
         WinPostQueueMsg(wd->hmqGameThread, WM_PAINT, MPVOID, MPVOID);
         fHandled = FALSE;
         break;

      case WM_REALIZEPALETTE:
         dprintf(("WM_REALIZEPALETTE\n"));
         if((hps = WinGetPS(hwnd)) != NULLHANDLE)
         {
            GpiQueryRealColors(hps, 0, 0, 256, wd->alPal);
            DiveSetDestinationPalette(wd->hDive, 0, 256, (PBYTE)wd->alPal);
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
         /* VirtualBox never sends WM_VRNENABLED; drive blitter setup from WM_SIZE */
         if(wd && wd->hDive)
         {
            WinPostMsg(hwnd, WM_VRNENABLED, MPVOID, MPVOID);
         }
         break;

      case WM_VRNENABLED:
         dived_puts(("WM_VRNENABLED"));
         if(wd->sizlWindow.cx == 0 || wd->sizlWindow.cy == 0)
         {
            SWP swpClient = { 0 };
            WinQueryWindowPos(hwnd, &swpClient);
            wd->sizlWindow.cx = swpClient.cx;
            wd->sizlWindow.cy = swpClient.cy;
         }
         {
            ULONG cRects = 0;
            POINTL ptl = { 0, 0 };
            ULONG rc = 0;

            /* Map client origin to desktop coordinates (no HPS needed) */
            WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl, 1);

            /* Attempt GPI visible-region query */
            if((hps = WinGetPS(hwnd)) != NULLHANDLE)
            {
               HRGN hrgnVisible = GpiCreateRegion(hps, 0L, NULL);
               if(hrgnVisible)
               {
                  if(WinQueryVisibleRegion(hwnd, hrgnVisible) != RGN_ERROR)
                  {
                     RGNRECT rgnCtl = { 0 };
                     rgnCtl.ircStart = 0;
                     rgnCtl.crc = DIVE_MAX_RECT;
                     rgnCtl.ulDirection = RECTDIR_LFRT_TOPBOT;
                     GpiQueryRegionRects(hps, hrgnVisible, NULL, &rgnCtl, wd->SetupBlitter.pVisDstRects);
                     cRects = rgnCtl.crcReturned;
                  }
                  GpiDestroyRegion(hps, hrgnVisible);
               }
               WinReleasePS(hps);
            }

            /* VirtualBox fallback: GPI returned no rects; synthesise the full client area */
            if(cRects == 0)
            {
               WinQueryWindowRect(hwnd, &wd->SetupBlitter.pVisDstRects[0]);
               WinMapWindowPoints(hwnd, HWND_DESKTOP, (PPOINTL)&wd->SetupBlitter.pVisDstRects[0], 2);
               cRects = 1;
            }

            wd->SetupBlitter.ulStructLen      = sizeof(SETUP_BLITTER);
            wd->SetupBlitter.fInvert          = 0x00000000;
            wd->SetupBlitter.fccSrcColorFormat = FOURCC_LUT8;
            wd->SetupBlitter.ulDitherType     = 0UL;
            wd->SetupBlitter.fccDstColorFormat = FOURCC_SCRN;
            wd->SetupBlitter.ulSrcWidth       = wd->sizlGameBitmap.cx;
            wd->SetupBlitter.ulSrcHeight      = wd->sizlGameBitmap.cy;
            wd->SetupBlitter.ulSrcPosX        = 0;
            wd->SetupBlitter.ulSrcPosY        = 0;
            wd->SetupBlitter.ulDstWidth       = wd->sizlWindow.cx;
            wd->SetupBlitter.ulDstHeight      = wd->sizlWindow.cy;
            wd->SetupBlitter.lDstPosX         = 0;
            wd->SetupBlitter.lDstPosY         = 0;
            wd->SetupBlitter.lScreenPosX      = ptl.x;
            wd->SetupBlitter.lScreenPosY      = ptl.y;
            wd->SetupBlitter.ulNumDstRects    = cRects;


            if((rc = DiveSetupBlitter(wd->hDive, &wd->SetupBlitter)) != DIVE_SUCCESS)
            {
               dprintf(("e DiveSetupBlitter() returned %08x for DiveSetupBlitter(%08x, 0).\n", rc, wd->hDive));
            }
         }
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

      case WMU_RESET_WINDOW:
         resetWindow(hwnd, wd->sizlGameBitmap.cx, wd->sizlGameBitmap.cy);
         break;

      case WM_CLOSE:
         dputs(("ClientWindow got a WM_CLOSE message"));
         if(wd->bSaveOnexit) {
            save_hw_settings(hwnd, wd);
         }
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
         HelpDrop(WinQueryWindow(hwnd, QW_PARENT));
         if(wd) {
            if(wd->hwndObject != NULLHANDLE) {
               WinDestroyWindow(wd->hwndObject);
            }
            free(wd);
         }
         break;

      case WMU_FIRST_RUN:
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

      memset(wd, 0, sizeof(WINDOWDATA));

      wd->hwndFrame  = WinQueryWindow(hwnd, QW_PARENT);
      wd->hwndMenu   = WinWindowFromID(wd->hwndFrame, FID_MENU);
      wd->hwndObject = NULLHANDLE;

      memcpy(&wd->sizlGameBitmap, &ctldata->sizlBoard, sizeof(SIZEL));

      WinSetWindowPtr(hwnd, 0, wd);

      /* Capture FID_ child handles for Frame Controls parking */
      wd->hwndTitleBar = WinWindowFromID(wd->hwndFrame, FID_TITLEBAR);
      wd->hwndSysMenu  = WinWindowFromID(wd->hwndFrame, FID_SYSMENU);
      wd->hwndMinMax   = WinWindowFromID(wd->hwndFrame, FID_MINMAX);
      wd->hwndMenuBar  = WinWindowFromID(wd->hwndFrame, FID_MENU);

      /* Create parking window for frame controls hiding */
      wd->hwndObject = WinCreateWindow(HWND_OBJECT, WC_FRAME, "",
                          0L, 0,0,0,0, NULLHANDLE, HWND_TOP, 0, NULL, NULL);

      /* Load settings from profile and apply language */
      load_hw_settings(hwnd, wd);
      set_menu_lang(wd->hwndMenu, current_lang);

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

   if(wd && wd->hDive)
   {
      HPS hps = NULLHANDLE;
      if((hps = WinGetPS(hwnd)) != NULLHANDLE)
      {
         GpiQueryRealColors(hps, 0, 0, 256, wd->alPal);
         DiveSetDestinationPalette(wd->hDive, 0, 256, (PBYTE)wd->alPal);
         WinReleasePS(hps);
      }
   }


   if(wd && wd->hDive)
   {
      GAMETHREADPARAMS threadParams = { 0 };
      APIRET rc = NO_ERROR;
      PAPPPRF prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
      if(prf)
      {
         RECTL rclDesktop = { 0 };
         SIZEL sizlClient = { 320, 240 };
         BOOL fDouble = TRUE;

         rclDesktop.xRight = swpDesktop.cx;
         rclDesktop.yTop   = swpDesktop.cy;

         fDouble = readProfileBoolean(prf, IDS_PRFKEY_DOUBLE_SIZE, TRUE);
         WinCheckMenuItem(wd->hwndMenu, IDM_SET_DOUBLE, fDouble);

         WinCalcFrameRect(wd->hwndFrame, &rclDesktop, TRUE);
         sizlClient.cx = rclDesktop.xRight-rclDesktop.xLeft;
         sizlClient.cy = rclDesktop.yTop-rclDesktop.yBottom;
         if(fDouble)
         {
            sizlClient.cx /= ZOOM_FACTOR;
            sizlClient.cy /= ZOOM_FACTOR;
         }

         /* Just in case ... */
         sizlClient.cx = max(sizlClient.cx, MIN_BOARD_CX);
         sizlClient.cy = max(sizlClient.cy, MIN_BOARD_CY);

         /*
          * Query default size from profile, default to maximum size
          */
         /* default board: two thirds of the screen */
         wd->sizlGameBitmap.cx = readProfileLong(prf, IDS_PRFKEY_BOARD_WIDTH, max(sizlClient.cx*2/3, MIN_BOARD_CX));
         wd->sizlGameBitmap.cy = readProfileLong(prf, IDS_PRFKEY_BOARD_HEIGHT, max(sizlClient.cy*2/3, MIN_BOARD_CY));

         /* a board saved at the old (smaller) element size would not fit the screen: use the default */
         if(wd->sizlGameBitmap.cx > sizlClient.cx || wd->sizlGameBitmap.cy > sizlClient.cy)
         {
            wd->sizlGameBitmap.cx = max(sizlClient.cx*2/3, MIN_BOARD_CX);
            wd->sizlGameBitmap.cy = max(sizlClient.cy*2/3, MIN_BOARD_CY);
         }

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
   RECTL rclClient = { 0 };
   SWP swpDesk = { 0 };
   SWP swp = { 0 };

   rclClient.xRight = cxGameBitmap;
   rclClient.yTop   = cyGameBitmap;

   if(WinIsMenuItemChecked(hwndMenu, IDM_SET_DOUBLE))
   {
      rclClient.xRight *= ZOOM_FACTOR;
      rclClient.yTop *= ZOOM_FACTOR;
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


/* ?? Key Mapping Dialog helpers ??????????????????????????????????????????? */

static void _Optlink scancodeToName(BYTE sc, char *buf, int cBuf)
{
   static const struct { BYTE sc; const char *name; } aMap[] = {
      {0x01,"Esc"}, {0x0E,"BkSp"}, {0x0F,"Tab"}, {0x1C,"Enter"},
      {0x2A,"LShift"}, {0x36,"RShift"}, {0x38,"Alt"}, {0x39,"Space"},
      {0x3A,"CapsLk"},
      {0x3B,"F1"},{0x3C,"F2"},{0x3D,"F3"},{0x3E,"F4"},{0x3F,"F5"},
      {0x40,"F6"},{0x41,"F7"},{0x42,"F8"},{0x43,"F9"},{0x44,"F10"},
      {0x47,"Home"},{0x48,"Up"},{0x49,"PgUp"},
      {0x4B,"Left"},{0x4C,"Num5"},{0x4D,"Right"},
      {0x4F,"End"},{0x50,"Down"},{0x51,"PgDn"},
      {0x52,"Ins"},{0x53,"Del"},
      {0x57,"F11"},{0x58,"F12"},
      {0x10,"Q"},{0x11,"W"},{0x12,"E"},{0x13,"R"},{0x14,"T"},
      {0x15,"Y"},{0x16,"U"},{0x17,"I"},{0x18,"O"},{0x19,"P"},
      {0x1E,"A"},{0x1F,"S"},{0x20,"D"},{0x21,"F"},{0x22,"G"},
      {0x23,"H"},{0x24,"J"},{0x25,"K"},{0x26,"L"},
      {0x2C,"Z"},{0x2D,"X"},{0x2E,"C"},{0x2F,"V"},
      {0x30,"B"},{0x31,"N"},{0x32,"M"},
      {0x02,"1"},{0x03,"2"},{0x04,"3"},{0x05,"4"},{0x06,"5"},
      {0x07,"6"},{0x08,"7"},{0x09,"8"},{0x0A,"9"},{0x0B,"0"},
      {0x0C,"-"},{0x0D,"="},{0x1A,"["},{0x1B,"]"},
      {0x27,";"},{0x28,"'"},{0x2B,"\\"},
      {0x33,","},{0x34,"."},{0x35,"/"},
      {0,NULL}
   };
   int i = 0;
   while(aMap[i].name)
   {
      if(aMap[i].sc == sc)
      {
         int j = 0;
         while(j < cBuf-1 && aMap[i].name[j]) { buf[j] = aMap[i].name[j]; j++; }
         buf[j] = 0;
         return;
      }
      i++;
   }
   {
      static const char hex[] = "0123456789ABCDEF";
      if(cBuf >= 5)
      {
         buf[0]='0'; buf[1]='x';
         buf[2]=hex[(sc>>4)&0xF]; buf[3]=hex[sc&0xF];
         buf[4]=0;
      }
   }
}

static BYTE kmGetKey(PKEYMAPDATA wd, int iSlot)
{
   switch(iSlot)
   {
      case 0: return wd->keys[0].bCounterClockwise;
      case 1: return wd->keys[0].bClockwise;
      case 2: return wd->keys[0].bFire;
      case 3: return wd->keys[1].bCounterClockwise;
      case 4: return wd->keys[1].bClockwise;
      default: return wd->keys[1].bFire;
   }
}

static void kmSetKey(PKEYMAPDATA wd, int iSlot, BYTE v)
{
   switch(iSlot)
   {
      case 0: wd->keys[0].bCounterClockwise = v; break;
      case 1: wd->keys[0].bClockwise = v; break;
      case 2: wd->keys[0].bFire = v; break;
      case 3: wd->keys[1].bCounterClockwise = v; break;
      case 4: wd->keys[1].bClockwise = v; break;
      default: wd->keys[1].bFire = v; break;
   }
}

static void _Optlink kmUpdateButton(HWND hwnd, USHORT btnId, BYTE sc)
{
   char buf[16] = "";
   scancodeToName(sc, buf, sizeof(buf));
   WinSetDlgItemText(hwnd, btnId, buf);
}

static void kmUpdateAllButtons(HWND hwnd, PKEYMAPDATA wd)
{
   static const USHORT aIds[6] = {
      BN_KM_P1_CCW, BN_KM_P1_CW, BN_KM_P1_FIRE,
      BN_KM_P2_CCW, BN_KM_P2_CW, BN_KM_P2_FIRE
   };
   int i = 0;
   for(; i < 6; i++)
      kmUpdateButton(hwnd, aIds[i], kmGetKey(wd, i));
}

static MRESULT EXPENTRY KeyMappingDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   PKEYMAPDATA wd = (PKEYMAPDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   static const USHORT aIds[6] = {
      BN_KM_P1_CCW, BN_KM_P1_CW, BN_KM_P1_FIRE,
      BN_KM_P2_CCW, BN_KM_P2_CW, BN_KM_P2_FIRE
   };

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PKEYMAPDATA)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);
         centerDialogWindow(WinQueryWindow(hwnd, QW_PARENT), hwnd);
         kmUpdateAllButtons(hwnd, wd);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case BN_KM_P1_CCW:  wd->iCapturing = 0; break;
            case BN_KM_P1_CW:   wd->iCapturing = 1; break;
            case BN_KM_P1_FIRE: wd->iCapturing = 2; break;
            case BN_KM_P2_CCW:  wd->iCapturing = 3; break;
            case BN_KM_P2_CW:   wd->iCapturing = 4; break;
            case BN_KM_P2_FIRE: wd->iCapturing = 5; break;

            case BN_KM_DEFAULTS:
            {
               int pi = 0;
               for(; pi < 2; pi++)
                  setDefaultPlayerKeys(pi, &wd->keys[pi]);
               wd->iCapturing = -1;
               kmUpdateAllButtons(hwnd, wd);
               break;
            }

            case DID_OK:
               wd->iCapturing = -1;
               WinDismissDlg(hwnd, DID_OK);
               break;

            case DID_CANCEL:
               WinDismissDlg(hwnd, DID_CANCEL);
               break;

            default:
               fHandled = FALSE;
               break;
         }
         if(wd->iCapturing >= 0 && wd->iCapturing <= 5)
         {
            WinSetDlgItemText(hwnd, aIds[wd->iCapturing], "Press key...");
            wd->prev_key = 0;
            WinSetFocus(HWND_DESKTOP, hwnd);
         }
         break;

      case WM_CHAR:
         if(wd && wd->iCapturing >= 0)
         {
            USHORT fsflags = SHORT1FROMMP(mp1);
            BYTE ucscancode = HIBYTE(SHORT2FROMMP(mp1));

            if((fsflags & (KC_SCANCODE|KC_LONEKEY)) == (KC_SCANCODE|KC_LONEKEY)
               && ucscancode == wd->prev_key
               && ucscancode != 0)
            {
               int slot = wd->iCapturing;
               kmSetKey(wd, slot, ucscancode);
               kmUpdateButton(hwnd, aIds[slot], ucscancode);
               wd->iCapturing = -1;
            }
            if((fsflags & KC_PREVDOWN) == 0)
               wd->prev_key = ucscancode;
         }
         else
         {
            fHandled = FALSE;
         }
         break;

      default:
         fHandled = FALSE;
         break;
   }

   if(!fHandled)
      mReturn = WinDefDlgProc(hwnd, msg, mp1, mp2);
   return mReturn;
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

         WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETLIMITS, MPFROMLONG(2048), MPFROMLONG(MIN_BOARD_CX));
         WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETLIMITS, MPFROMLONG(2048), MPFROMLONG(MIN_BOARD_CY));

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
               WinSendDlgItemMsg(hwnd, SPBN_WIDTH, SPBM_SETCURRENTVALUE, MPFROMLONG((rect.xRight-rect.xLeft)/ZOOM_FACTOR), MPVOID);
               WinSendDlgItemMsg(hwnd, SPBN_HEIGHT, SPBM_SETCURRENTVALUE, MPFROMLONG((rect.yTop-rect.yBottom)/ZOOM_FACTOR), MPVOID);
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


static MRESULT EXPENTRY AboutDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   switch(msg) {
      case WM_COMMAND:
         switch(COMMANDMSG(&msg)->cmd) {
            case DID_OK:
            case DID_CANCEL:
               WinDismissDlg(hwnd, TRUE);
               return 0L;
         }
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


static void _Optlink centerDialogWindow(HWND hwndReference, HWND hwndDialog)
{
   SWP swpParent = { 0 };
   SWP swpDialog = { 0 };

   WinQueryWindowPos(hwndDialog, &swpDialog);
   WinQueryWindowPos(hwndReference, &swpParent);
   WinSetWindowPos(hwndDialog, HWND_TOP, swpParent.cx/2-swpDialog.cx/2, swpParent.cy/2-swpDialog.cy/2, 0, 0, SWP_MOVE);
}
