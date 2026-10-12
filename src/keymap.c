/*
 * keymap.c -- keyboard and joystick mapping of HalfWorm and the "Key Mapping..."
 *             dialog (notebook: Keyboard, Joystick).
 *
 * Keyboard: the three keys of each worm are scancodes (the game thread polls the
 * physical key state, so the dialog captures scancodes).
 * Joystick: through the GAME$ driver.  Each worm can use stick A or B; every action
 * (rotate left, rotate right, fire) is a joystick button or a stick direction.
 * The joystick mapping is saved in the application profile (key "KeyMapJoy").
 */
#pragma strings(readonly)

#define INCL_WIN
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_WINSTDBOOK
#define INCL_WINENTRYFIELDS
#define INCL_WININPUT
#define INCL_WINSHELLDATA

#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pmx.h"
#include "keymap.h"
#include "resources.h"
#include "joyos2.h"

/* ------------------------------------------------------------------ texts */

enum {
   KMS_TITLE = 0, KMS_TAB_KBD, KMS_TAB_JOY, KMS_JOY_EN, KMS_JOY_A, KMS_JOY_B,
   KMS_CAL_A, KMS_CAL_B, KMS_OK, KMS_CANCEL, KMS_CAL_MSG, KMS_CAL_TITLE,
   KMS_NONE, KMS_AXL, KMS_AXR, KMS_AXU, KMS_AXD, KMS_BTN,
   KMS_P1, KMS_P2, KMS_CCW, KMS_CW, KMS_FIRE, KMS_PRESS,
   KMS_KBD_H, KMS_JOY_H1, KMS_JOY_H2, KMS_DEFAULT,
   KMS_COUNT
};

static const char *km_txt[6][KMS_COUNT] = {
   /* EN */ {
      "Key Mapping", "Keyboard", "Joystick", "~Enable Joystick",
      "Joystick ~A", "Joystick ~B", "Calibrate A", "Calibrate B",
      "Ok", "Cancel", "Center Joystick %c, then click OK.", "Calibrate Joystick %c",
      "None", "Axis L", "Axis R", "Axis Up",
      "Axis Down", "Btn %d", "Player 1", "Player 2",
      "Rotate counterclockwise", "Rotate clockwise", "Fire", "Press key...",
      "Click a key button, then press the key.", "Click a box, then press a joystick button", "or move the stick (Delete clears).", "Default"
   },
   /* ES */ {
      "Asignacion de teclas", "Teclado", "Joystick", "~Activar joystick",
      "Joystick ~A", "Joystick ~B", "Calibrar A", "Calibrar B",
      "Aceptar", "Cancelar", "Centre el joystick %c y pulse Aceptar.", "Calibrar joystick %c",
      "Ninguno", "Eje izq.", "Eje der.", "Eje arriba",
      "Eje abajo", "Boton %d", "Jugador 1", "Jugador 2",
      "Girar a la izquierda", "Girar a la derecha", "Disparar", "Pulse una tecla...",
      "Haga clic en un boton y pulse la tecla.", "Haga clic en un cuadro y pulse un boton", "del joystick o mueva la palanca (Supr borra).", "Por defecto"
   },
   /* NL */ {
      "Toetsen toewijzen", "Toetsenbord", "Joystick", "Joystick ~gebruiken",
      "Joystick ~A", "Joystick ~B", "Kalibreer A", "Kalibreer B",
      "OK", "Annuleren", "Centreer joystick %c en klik op OK.", "Joystick %c kalibreren",
      "Geen", "As links", "As rechts", "As omhoog",
      "As omlaag", "Knop %d", "Speler 1", "Speler 2",
      "Linksom draaien", "Rechtsom draaien", "Vuren", "Druk op een toets...",
      "Klik op een knop en druk op de toets.", "Klik op een vak en druk op een joystickknop", "of beweeg de stick (Delete wist).", "Standaard"
   },
   /* DE */ {
      "Tastenzuordnung", "Tastatur", "Joystick", "Joystick ~aktivieren",
      "Joystick ~A", "Joystick ~B", "A kalibrieren", "B kalibrieren",
      "OK", "Abbrechen", "Joystick %c zentrieren, dann OK klicken.", "Joystick %c kalibrieren",
      "Keine", "Achse links", "Achse rechts", "Achse oben",
      "Achse unten", "Taste %d", "Spieler 1", "Spieler 2",
      "Links drehen", "Rechts drehen", "Feuern", "Taste druecken...",
      "Klicke auf eine Schaltflaeche und druecke die Taste.", "Klicke auf ein Feld und druecke eine Joystick-Taste", "oder bewege den Stick (Entf loescht).", "Standard"
   },
   /* FR */ {
      "Affectation des touches", "Clavier", "Joystick", "~Activer le joystick",
      "Joystick ~A", "Joystick ~B", "Calibrer A", "Calibrer B",
      "OK", "Annuler", "Centrez le joystick %c puis cliquez sur OK.", "Calibrer le joystick %c",
      "Aucun", "Axe gauche", "Axe droite", "Axe haut",
      "Axe bas", "Bouton %d", "Joueur 1", "Joueur 2",
      "Tourner a gauche", "Tourner a droite", "Tirer", "Appuyez sur une touche...",
      "Cliquez sur un bouton puis appuyez sur la touche.", "Cliquez sur une case puis appuyez sur un bouton", "du joystick ou bougez le manche (Suppr efface).", "Defaut"
   },
   /* IT */ {
      "Assegnazione tasti", "Tastiera", "Joystick", "~Attiva joystick",
      "Joystick ~A", "Joystick ~B", "Calibra A", "Calibra B",
      "OK", "Annulla", "Centra il joystick %c e fai clic su OK.", "Calibra joystick %c",
      "Nessuno", "Asse sinistra", "Asse destra", "Asse su",
      "Asse giu", "Pulsante %d", "Giocatore 1", "Giocatore 2",
      "Ruota a sinistra", "Ruota a destra", "Spara", "Premi un tasto...",
      "Fai clic su un pulsante e premi il tasto.", "Fai clic su una casella e premi un pulsante", "del joystick o muovi la leva (Canc cancella).", "Predefinito"
   },
};

/* text of the active language; a mnemonic tilde is removed when plain is TRUE */
static const char *kmText(int id)
{
   static char abuf[8][100];
   static int n;
   const char *s = km_txt[currentLanguage()][id];
   char *d;

   n = (n + 1) & 7;
   d = abuf[n];
   while(*s && d < abuf[n] + sizeof(abuf[n]) - 1)
   {
      if(*s != '~')
         *d++ = *s;
      s++;
   }
   *d = 0;
   return abuf[n];
}

/* ------------------------------------------------------------------- data */

/* joystick: 0 = none, 1-4 = button, 5-8 = axis left / right / up / down */
#define JOY_AXIS_L  5
#define JOY_AXIS_R  6
#define JOY_AXIS_U  7
#define JOY_AXIS_D  8

#define KM_ROWS     6        /* 2 players x 3 actions */

typedef struct _JOYCFG
{
   int ver;
   int enabled;              /* joystick control on / off              */
   int sel[2];               /* stick of each worm: 1 = A, 2 = B       */
   int act[KM_ROWS];         /* 0 none, 1-4 button, 5-8 axis direction */
}JOYCFG;

static JOYCFG joy = { 1, 0, { 1, 2 },
   { JOY_AXIS_L, JOY_AXIS_R, 1, JOY_AXIS_L, JOY_AXIS_R, 1 } };

static HFILE             hGame = 0;
static GAME_2DPOS_STRUCT joyCal[3];                 /* centre of stick A (1) and B (2) */
static BOOL              bJoyAvail[2];
static GAME_STATUS_STRUCT joyNow;                   /* last status read by KmJoyPoll() */
static volatile BOOL     bJoyNow = FALSE;

static const USHORT usButBit[4] = { JOY_BUT1_BIT, JOY_BUT2_BIT, JOY_BUT3_BIT, JOY_BUT4_BIT };

static const char *kmJoyLabel(int n)
{
   static char b[100];

   if(n == 0) return kmText(KMS_NONE);
   if(n >= 1 && n <= 4) { sprintf(b, kmText(KMS_BTN), n); return b; }
   if(n == JOY_AXIS_L) return kmText(KMS_AXL);
   if(n == JOY_AXIS_R) return kmText(KMS_AXR);
   if(n == JOY_AXIS_U) return kmText(KMS_AXU);
   if(n == JOY_AXIS_D) return kmText(KMS_AXD);
   return "?";
}

/* ----------------------------------------------------------------- joystick */

void _Optlink KmInit(HAB hab)
{
   APIRET            rc;
   ULONG             action, dataLen;
   GAME_PARM_STRUCT  parms;
   GAME_CALIB_STRUCT calib;
   PAPPPRF           prf;

   rc = DosOpen(GAMEPDDNAME, &hGame, &action, 0, FILE_READONLY, FILE_OPEN,
                OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, NULL);
   if(rc != NO_ERROR)
      hGame = 0;
   else
   {
      dataLen = sizeof(parms);
      rc = DosDevIOCtl(hGame, IOCTL_CAT_USER, GAME_GET_PARMS, NULL, 0, NULL, &parms, dataLen, &dataLen);
      if(rc != NO_ERROR)
      {
         DosClose(hGame);
         hGame = 0;
      }
   }
   if(hGame != 0)
   {
      bJoyAvail[0] = (parms.useA & GAME_USE_BOTH_NEWMASK) != 0;
      bJoyAvail[1] = (parms.useB & GAME_USE_BOTH_NEWMASK) != 0;
      dataLen = sizeof(calib);
      if(DosDevIOCtl(hGame, IOCTL_CAT_USER, GAME_GET_CALIB, NULL, 0, NULL, &calib, dataLen, &dataLen) == NO_ERROR)
      {
         joyCal[1].x = calib.Ax.centre;  joyCal[1].y = calib.Ay.centre;
         joyCal[2].x = calib.Bx.centre;  joyCal[2].y = calib.By.centre;
      }
   }

   /* saved mapping */
   prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);
   if(prf)
   {
      JOYCFG c;
      ULONG cb = sizeof(c);
      int i, ok = 1;

      if(PrfQueryProfileData(prfHandle(prf), prfAppName(prf), "KeyMapJoy", &c, &cb) && cb == sizeof(c) && c.ver == 1)
      {
         for(i = 0; i < KM_ROWS; i++)
            if(c.act[i] < 0 || c.act[i] > 8) ok = 0;
         for(i = 0; i < 2; i++)
            if(c.sel[i] < 1 || c.sel[i] > 2) ok = 0;
         if(c.enabled < 0 || c.enabled > 1) ok = 0;
         if(ok)
            joy = c;
      }
      closeAppProfile(prf);
   }
   if(!hGame)
      joy.enabled = 0;
}

void _Optlink KmDone(void)
{
   if(hGame)
   {
      HFILE h = hGame;
      hGame = 0;
      DosClose(h);
   }
}

static void kmSave(HAB hab)
{
   PAPPPRF prf = openAppProfile(hab, NULL, IDS_PROFILE_NAME, IDS_PRFAPP);

   if(prf)
   {
      PrfWriteProfileData(prfHandle(prf), prfAppName(prf), "KeyMapJoy", &joy, sizeof(joy));
      closeAppProfile(prf);
   }
}

/* Game thread, once per key scan */
void _Optlink KmJoyPoll(void)
{
   ULONG len = sizeof(joyNow);

   if(!joy.enabled || !hGame)
   {
      bJoyNow = FALSE;
      return;
   }
   bJoyNow = DosDevIOCtl(hGame, IOCTL_CAT_USER, GAME_GET_STATUS, NULL, 0, NULL, &joyNow, len, &len) == NO_ERROR;
}

BOOL _Optlink KmJoyDown(int iPlayer, int iAct)
{
   int row, c, stick, dX, dY, tolX, tolY;
   GAME_2DPOS_STRUCT *jd, *cal;

   if(!bJoyNow || iPlayer < 0 || iPlayer > 1 || iAct < 0 || iAct >= KM_NACT)
      return FALSE;
   row = iPlayer * KM_NACT + iAct;
   c = joy.act[row];
   if(c == 0)
      return FALSE;
   if(c >= 1 && c <= 4)
      return (joyNow.curdata.butMask & usButBit[c - 1]) == 0;

   stick = joy.sel[iPlayer];
   jd  = (stick == 1) ? &joyNow.curdata.A : &joyNow.curdata.B;
   cal = &joyCal[stick];
   tolX = abs((int)cal->x) / 10 + 1;
   tolY = abs((int)cal->y) / 10 + 1;
   if(tolX < 16) tolX = 16;
   if(tolY < 16) tolY = 16;
   dX = (int)jd->x - (int)cal->x;
   dY = (int)jd->y - (int)cal->y;
   switch(c)
   {
      case JOY_AXIS_L: return dX < -tolX;
      case JOY_AXIS_R: return dX >  tolX;
      case JOY_AXIS_U: return dY < -tolY;
      case JOY_AXIS_D: return dY >  tolY;
   }
   return FALSE;
}

/* ============================================================== dialogs */

#define WM_KM_INPUT   (WM_USER + 1)

typedef struct _KMDATA
{
   PLAYERKEYS keys[2];       /* working copy */
   JOYCFG     joy;           /* working copy */
   int        iCapturing;    /* keyboard page: slot waiting for a key, -1 none */
   BYTE       prev_key;
   HWND       hwndKbd, hwndJoy;
   HWND       hwndNB;
   ULONG      ulKbdPage, ulJoyPage;
}KMDATA, *PKMDATA;

static const USHORT aKbdIds[6] = {
   BN_KM_P1_CCW, BN_KM_P1_CW, BN_KM_P1_FIRE, BN_KM_P2_CCW, BN_KM_P2_CW, BN_KM_P2_FIRE
};

static void scancodeToName(BYTE sc, char *buf, int cBuf)
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
   if(cBuf >= 5)
      sprintf(buf, "0x%02X", sc);
}

static BYTE *kmKeySlot(PKMDATA wd, int iSlot)
{
   PLAYERKEYS *k = &wd->keys[iSlot / 3];

   switch(iSlot % 3)
   {
      case 0: return &k->bCounterClockwise;
      case 1: return &k->bClockwise;
      default: return &k->bFire;
   }
}

/* Enable or disable every child of hwnd except the given ids */
static void kmEnableChildren(HWND hwnd, USHORT id1, BOOL en)
{
   HENUM  he = WinBeginEnumWindows(hwnd);
   HWND   hc;

   while((hc = WinGetNextWindow(he)) != NULLHANDLE)
   {
      if(WinQueryWindowUShort(hc, QWS_ID) != id1)
         WinEnableWindow(hc, en);
   }
   WinEndEnumWindows(he);
}

/* ------------------------------------------------------------- keyboard page */

static void kbdShowAll(HWND hwnd, PKMDATA wd)
{
   char buf[16];
   int i;

   for(i = 0; i < 6; i++)
   {
      buf[0] = 0;
      scancodeToName(*kmKeySlot(wd, i), buf, sizeof(buf));
      WinSetDlgItemText(hwnd, aKbdIds[i], buf);
   }
}

static MRESULT EXPENTRY KbdPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PKMDATA wd = (PKMDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   int i;

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PKMDATA)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);
         WinSetDlgItemText(hwnd, IDC_KM_KH, kmText(KMS_KBD_H));
         WinSetDlgItemText(hwnd, IDC_KM_KP1, kmText(KMS_P1));
         WinSetDlgItemText(hwnd, IDC_KM_KP2, kmText(KMS_P2));
         WinSetDlgItemText(hwnd, IDC_KM_KCCW, kmText(KMS_CCW));
         WinSetDlgItemText(hwnd, IDC_KM_KCW, kmText(KMS_CW));
         WinSetDlgItemText(hwnd, IDC_KM_KFIRE, kmText(KMS_FIRE));
         kbdShowAll(hwnd, wd);
         return (MRESULT)FALSE;

      case WM_COMMAND:
         for(i = 0; i < 6; i++)
            if(SHORT1FROMMP(mp1) == aKbdIds[i])
            {
               wd->iCapturing = i;
               WinSetDlgItemText(hwnd, aKbdIds[i], kmText(KMS_PRESS));
               wd->prev_key = 0;
               WinSetFocus(HWND_DESKTOP, hwnd);
               return 0;
            }
         if(SHORT1FROMMP(mp1) == BN_KM_DEFAULTS)
         {
            setDefaultPlayerKeys(0, &wd->keys[0]);
            setDefaultPlayerKeys(1, &wd->keys[1]);
            wd->iCapturing = -1;
            kbdShowAll(hwnd, wd);
         }
         return 0;

      case WM_CHAR:
         if(wd && wd->iCapturing >= 0)
         {
            USHORT fsflags = SHORT1FROMMP(mp1);
            BYTE ucscancode = HIBYTE(SHORT2FROMMP(mp1));

            if((fsflags & (KC_SCANCODE|KC_LONEKEY)) == (KC_SCANCODE|KC_LONEKEY)
               && ucscancode == wd->prev_key && ucscancode != 0)
            {
               *kmKeySlot(wd, wd->iCapturing) = ucscancode;
               wd->iCapturing = -1;
               kbdShowAll(hwnd, wd);
            }
            if((fsflags & KC_PREVDOWN) == 0)
               wd->prev_key = ucscancode;
            return (MRESULT)TRUE;
         }
         break;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* ------------------------------------------------------------- joystick page */

static PFNWP pfnEntryOrig;

/* Keys pressed in a mapping box go to the owning page */
static MRESULT EXPENTRY KmEntryProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   HWND   hwndOwner = WinQueryWindow(hwnd, QW_OWNER);
   USHORT id        = WinQueryWindowUShort(hwnd, QWS_ID);

   if(msg == WM_CHAR)
   {
      USHORT fs = SHORT1FROMMP(mp1);
      USHORT vk = SHORT2FROMMP(mp2);

      if(fs & KC_ALT)
         return pfnEntryOrig(hwnd, msg, mp1, mp2);
      if((fs & KC_VIRTUALKEY) && (vk == VK_TAB || vk == VK_BACKTAB || vk == VK_ESC || vk == VK_F1))
         return pfnEntryOrig(hwnd, msg, mp1, mp2);
      if(!(fs & KC_KEYUP))
         WinSendMsg(hwndOwner, WM_KM_INPUT, MPFROMSHORT(id), MPFROM2SHORT(fs, vk));
      return (MRESULT)TRUE;
   }
   return pfnEntryOrig(hwnd, msg, mp1, mp2);
}

static void joyShowAll(HWND hwnd, PKMDATA wd)
{
   int i;

   for(i = 0; i < KM_ROWS; i++)
      WinSetDlgItemText(hwnd, IDC_KM_JBOX + i, (PSZ)kmJoyLabel(wd->joy.act[i]));
}

/* Calibrate and stick choices follow what the driver reports */
static void joyEnableState(HWND hwnd, PKMDATA wd)
{
   BOOL en = wd->joy.enabled != 0;

   kmEnableChildren(hwnd, IDC_KM_JEN, en);
   WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JCALA), en && hGame && bJoyAvail[0]);
   WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JCALB), en && hGame && bJoyAvail[1]);
   if(!hGame)
      WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JEN), FALSE);
   if(!bJoyAvail[0])
   {
      WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JP1A), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JP2A), FALSE);
   }
   if(!bJoyAvail[1])
   {
      WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JP1B), FALSE);
      WinEnableWindow(WinWindowFromID(hwnd, IDC_KM_JP2B), FALSE);
   }
}

static void joyCalibrate(HWND hwnd, int stick)
{
   char szMsg[160], szTitle[160];
   char ch = (stick == 1) ? 'A' : 'B';

   if(!hGame)
      return;
   sprintf(szMsg,   kmText(KMS_CAL_MSG),   ch);
   sprintf(szTitle, kmText(KMS_CAL_TITLE), ch);
   if(WinMessageBox(HWND_DESKTOP, hwnd, szMsg, szTitle, 0, MB_OKCANCEL | MB_INFORMATION) == MBID_OK)
   {
      GAME_STATUS_STRUCT s;
      ULONG len = sizeof(s);

      if(DosDevIOCtl(hGame, IOCTL_CAT_USER, GAME_GET_STATUS, NULL, 0, NULL, &s, len, &len) == NO_ERROR)
      {
         if(stick == 1) { joyCal[1].x = s.curdata.A.x; joyCal[1].y = s.curdata.A.y; }
         else           { joyCal[2].x = s.curdata.B.x; joyCal[2].y = s.curdata.B.y; }
      }
   }
}

static MRESULT EXPENTRY JoyPageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PKMDATA wd = (PKMDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   static int    iSelRow;
   static USHORT usPrevButs;
   static BOOL   bPAxL, bPAxR, bPAxU, bPAxD;
   int i;

   switch(msg)
   {
      case WM_INITDLG:
         wd = (PKMDATA)mp2;
         WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);
         iSelRow = -1;
         usPrevButs = 0xFFFF;
         bPAxL = bPAxR = bPAxU = bPAxD = FALSE;

         WinSetDlgItemText(hwnd, IDC_KM_JEN,  kmText(KMS_JOY_EN));
         WinSetDlgItemText(hwnd, IDC_KM_JH1,  kmText(KMS_JOY_H1));
         WinSetDlgItemText(hwnd, IDC_KM_JH2,  kmText(KMS_JOY_H2));
         WinSetDlgItemText(hwnd, IDC_KM_JP1,  kmText(KMS_P1));
         WinSetDlgItemText(hwnd, IDC_KM_JP2,  kmText(KMS_P2));
         WinSetDlgItemText(hwnd, IDC_KM_JP1A, kmText(KMS_JOY_A));
         WinSetDlgItemText(hwnd, IDC_KM_JP1B, kmText(KMS_JOY_B));
         WinSetDlgItemText(hwnd, IDC_KM_JP2A, kmText(KMS_JOY_A));
         WinSetDlgItemText(hwnd, IDC_KM_JP2B, kmText(KMS_JOY_B));
         WinSetDlgItemText(hwnd, IDC_KM_JCALA, kmText(KMS_CAL_A));
         WinSetDlgItemText(hwnd, IDC_KM_JCALB, kmText(KMS_CAL_B));
         for(i = 0; i < KM_ROWS; i++)
         {
            WinSetDlgItemText(hwnd, IDC_KM_JLBL + i,
                              i % 3 == 0 ? kmText(KMS_CCW) : i % 3 == 1 ? kmText(KMS_CW) : kmText(KMS_FIRE));
            if(pfnEntryOrig == NULL)
               pfnEntryOrig = WinSubclassWindow(WinWindowFromID(hwnd, IDC_KM_JBOX + i), KmEntryProc);
            else
               WinSubclassWindow(WinWindowFromID(hwnd, IDC_KM_JBOX + i), KmEntryProc);
         }
         WinSendDlgItemMsg(hwnd, IDC_KM_JEN, BM_SETCHECK, MPFROMSHORT(wd->joy.enabled != 0), MPVOID);
         WinSendDlgItemMsg(hwnd, wd->joy.sel[0] == 2 ? IDC_KM_JP1B : IDC_KM_JP1A, BM_SETCHECK, MPFROMSHORT(1), MPVOID);
         WinSendDlgItemMsg(hwnd, wd->joy.sel[1] == 1 ? IDC_KM_JP2A : IDC_KM_JP2B, BM_SETCHECK, MPFROMSHORT(1), MPVOID);
         joyShowAll(hwnd, wd);
         joyEnableState(hwnd, wd);
         WinStartTimer(WinQueryAnchorBlock(hwnd), hwnd, 1, 50);
         return (MRESULT)FALSE;

      case WM_TIMER:
         /* a new button press or stick direction goes to the focused box */
         if(hGame && wd->joy.enabled && iSelRow >= 0)
         {
            GAME_STATUS_STRUCT s;
            ULONG len = sizeof(s);

            if(DosDevIOCtl(hGame, IOCTL_CAT_USER, GAME_GET_STATUS, NULL, 0, NULL, &s, len, &len) == NO_ERROR)
            {
               int stick = wd->joy.sel[iSelRow / 3];
               GAME_2DPOS_STRUCT *jd  = (stick == 1) ? &s.curdata.A : &s.curdata.B;
               GAME_2DPOS_STRUCT *cal = &joyCal[stick];
               int tolX = abs((int)cal->x) / 10 + 1, tolY = abs((int)cal->y) / 10 + 1;
               int dX = (int)jd->x - (int)cal->x, dY = (int)jd->y - (int)cal->y;
               USHORT curButs = s.curdata.butMask;
               USHORT pressed = usPrevButs & ~curButs;
               BOOL axL = dX < -tolX, axR = dX > tolX, axU = dY < -tolY, axD = dY > tolY;
               int newAct = 0;

               if(pressed)
                  for(i = 0; i < 4; i++)
                     if(pressed & usButBit[i]) { newAct = i + 1; break; }
               if(!newAct)
               {
                  if(axL && !bPAxL) newAct = JOY_AXIS_L;
                  else if(axR && !bPAxR) newAct = JOY_AXIS_R;
                  else if(axU && !bPAxU) newAct = JOY_AXIS_U;
                  else if(axD && !bPAxD) newAct = JOY_AXIS_D;
               }
               if(newAct)
               {
                  wd->joy.act[iSelRow] = newAct;
                  WinSetDlgItemText(hwnd, (USHORT)(IDC_KM_JBOX + iSelRow), (PSZ)kmJoyLabel(newAct));
               }
               usPrevButs = curButs;
               bPAxL = axL; bPAxR = axR; bPAxU = axU; bPAxD = axD;
            }
         }
         return 0;

      case WM_KM_INPUT:
         {
            int r = (int)SHORT1FROMMP(mp1) - IDC_KM_JBOX;
            USHORT fs = SHORT1FROMMP(mp2), vk = SHORT2FROMMP(mp2);

            if(r >= 0 && r < KM_ROWS && (fs & KC_VIRTUALKEY) && (vk == VK_BACKSPACE || vk == VK_DELETE))
            {
               wd->joy.act[r] = 0;
               WinSetDlgItemText(hwnd, (USHORT)(IDC_KM_JBOX + r), (PSZ)kmJoyLabel(0));
            }
         }
         return (MRESULT)TRUE;

      case WM_CONTROL:
         if(SHORT1FROMMP(mp1) >= IDC_KM_JBOX && SHORT1FROMMP(mp1) < IDC_KM_JBOX + KM_ROWS)
         {
            if(SHORT2FROMMP(mp1) == EN_SETFOCUS)
               iSelRow = SHORT1FROMMP(mp1) - IDC_KM_JBOX;
            else if(SHORT2FROMMP(mp1) == EN_KILLFOCUS)
               iSelRow = -1;
            return 0;
         }
         if(SHORT2FROMMP(mp1) == BN_CLICKED)
         {
            switch(SHORT1FROMMP(mp1))
            {
               case IDC_KM_JEN:
                  wd->joy.enabled = (int)(SHORT)(ULONG)WinSendDlgItemMsg(hwnd, IDC_KM_JEN, BM_QUERYCHECK, MPVOID, MPVOID) != 0;
                  joyEnableState(hwnd, wd);
                  break;
               case IDC_KM_JP1A: wd->joy.sel[0] = 1; break;
               case IDC_KM_JP1B: wd->joy.sel[0] = 2; break;
               case IDC_KM_JP2A: wd->joy.sel[1] = 1; break;
               case IDC_KM_JP2B: wd->joy.sel[1] = 2; break;
            }
         }
         return 0;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case IDC_KM_JCALA: joyCalibrate(hwnd, 1); break;
            case IDC_KM_JCALB: joyCalibrate(hwnd, 2); break;
            case BN_KM_DEFAULTS:
               {
                  static const int defAct[KM_ROWS] = { JOY_AXIS_L, JOY_AXIS_R, 1, JOY_AXIS_L, JOY_AXIS_R, 1 };
                  for(i = 0; i < KM_ROWS; i++)
                     wd->joy.act[i] = defAct[i];
                  wd->joy.sel[0] = 1;  wd->joy.sel[1] = 2;
                  WinSendDlgItemMsg(hwnd, IDC_KM_JP1A, BM_SETCHECK, MPFROMSHORT(1), MPVOID);
                  WinSendDlgItemMsg(hwnd, IDC_KM_JP1B, BM_SETCHECK, MPFROMSHORT(0), MPVOID);
                  WinSendDlgItemMsg(hwnd, IDC_KM_JP2A, BM_SETCHECK, MPFROMSHORT(0), MPVOID);
                  WinSendDlgItemMsg(hwnd, IDC_KM_JP2B, BM_SETCHECK, MPFROMSHORT(1), MPVOID);
                  joyShowAll(hwnd, wd);
               }
               break;
         }
         return 0;

      case WM_DESTROY:
         WinStopTimer(WinQueryAnchorBlock(hwnd), hwnd, 1);
         return 0;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

/* --------------------------------------------------------------- main dialog */

static void kmSizeNotebook(HWND hwnd, PKMDATA wd)
{
   SWP swpBtn, swpDlg;

   WinQueryWindowPos(hwnd, &swpDlg);
   WinQueryWindowPos(WinWindowFromID(hwnd, DID_OK), &swpBtn);
   WinSetWindowPos(wd->hwndNB, HWND_TOP,
      4, swpBtn.y + swpBtn.cy + 4,
      swpDlg.cx - 8,
      swpDlg.cy - (swpBtn.y + swpBtn.cy + 4) - 4 - WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR),
      SWP_MOVE | SWP_SIZE);
}

static MRESULT EXPENTRY KeyMapDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   PKMDATA wd = (PKMDATA)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);

   switch(msg)
   {
      case WM_INITDLG:
         {
            SWP swpFr;

            wd = (PKMDATA)mp2;
            WinSetWindowULong(hwnd, QWL_USER, (ULONG)wd);

            WinSetWindowText(hwnd, kmText(KMS_TITLE));
            WinSetDlgItemText(hwnd, DID_OK,        kmText(KMS_OK));
            WinSetDlgItemText(hwnd, DID_CANCEL,    kmText(KMS_CANCEL));
            WinSetDlgItemText(hwnd, BN_KM_DEFAULTS, kmText(KMS_DEFAULT));
            WinQueryWindowPos(hwnd, &swpFr);
            WinSetWindowPos(hwnd, HWND_TOP,
               (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN) - swpFr.cx) / 2,
               (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN) - swpFr.cy) / 2,
               0, 0, SWP_MOVE | SWP_ACTIVATE);

            wd->hwndNB = WinCreateWindow(hwnd, WC_NOTEBOOK, "",
                           BKS_TABBEDDIALOG | BKS_BACKPAGESBR | WS_VISIBLE | WS_TABSTOP,
                           0, 0, 1, 1, hwnd, HWND_TOP, IDC_KM_NB, NULL, NULL);
            if(wd->hwndNB == NULLHANDLE)
            {
               WinDismissDlg(hwnd, DID_CANCEL);
               return (MRESULT)FALSE;
            }
            kmSizeNotebook(hwnd, wd);
            WinSendMsg(wd->hwndNB, BKM_SETNOTEBOOKCOLORS,
                       MPFROMLONG(WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0)),
                       MPFROMSHORT(BKA_BACKGROUNDPAGECOLOR));

            wd->ulKbdPage = (ULONG)WinSendMsg(wd->hwndNB, BKM_INSERTPAGE, NULL, MPFROM2SHORT(BKA_AUTOPAGESIZE | BKA_MAJOR, BKA_LAST));
            WinSendMsg(wd->hwndNB, BKM_SETTABTEXT, MPFROMLONG(wd->ulKbdPage), MPFROMP(kmText(KMS_TAB_KBD)));
            wd->ulJoyPage = (ULONG)WinSendMsg(wd->hwndNB, BKM_INSERTPAGE, NULL, MPFROM2SHORT(BKA_AUTOPAGESIZE | BKA_MAJOR, BKA_LAST));
            WinSendMsg(wd->hwndNB, BKM_SETTABTEXT, MPFROMLONG(wd->ulJoyPage), MPFROMP(kmText(KMS_TAB_JOY)));

            wd->hwndKbd = WinLoadDlg(wd->hwndNB, wd->hwndNB, KbdPageProc, NULLHANDLE, DLG_KM_KBD, wd);
            WinSendMsg(wd->hwndNB, BKM_SETPAGEWINDOWHWND, MPFROMLONG(wd->ulKbdPage), MPFROMHWND(wd->hwndKbd));
            wd->hwndJoy = WinLoadDlg(wd->hwndNB, wd->hwndNB, JoyPageProc, NULLHANDLE, DLG_KM_JOY, wd);
            WinSendMsg(wd->hwndNB, BKM_SETPAGEWINDOWHWND, MPFROMLONG(wd->ulJoyPage), MPFROMHWND(wd->hwndJoy));

            WinSendMsg(wd->hwndNB, BKM_TURNTOPAGE, MPFROMLONG(wd->ulKbdPage), MPVOID);
         }
         return (MRESULT)FALSE;

      case WM_SIZE:
         if(wd && wd->hwndNB)
            kmSizeNotebook(hwnd, wd);
         break;

      case WM_COMMAND:
         switch(SHORT1FROMMP(mp1))
         {
            case DID_OK:
               wd->iCapturing = -1;
               WinDismissDlg(hwnd, DID_OK);
               return 0;
            case DID_CANCEL:
               WinDismissDlg(hwnd, DID_CANCEL);
               return 0;
            case BN_KM_DEFAULTS:
               WinSendMsg(wd->hwndKbd, WM_COMMAND, MPFROMSHORT(BN_KM_DEFAULTS), MPVOID);
               WinSendMsg(wd->hwndJoy, WM_COMMAND, MPFROMSHORT(BN_KM_DEFAULTS), MPVOID);
               return 0;
         }
         return 0;
   }
   return WinDefDlgProc(hwnd, msg, mp1, mp2);
}

BOOL _Optlink KmDialog(HWND hwndOwner, PLAYERKEYS keys[2])
{
   KMDATA wd;
   BOOL   fOk;

   memset(&wd, 0, sizeof(wd));
   wd.keys[0] = keys[0];
   wd.keys[1] = keys[1];
   wd.joy = joy;
   wd.iCapturing = -1;

   fOk = WinDlgBox(HWND_DESKTOP, hwndOwner, KeyMapDlgProc, NULLHANDLE, IDD_KEY_MAPPING, &wd) == DID_OK;
   if(fOk)
   {
      keys[0] = wd.keys[0];
      keys[1] = wd.keys[1];
      joy = wd.joy;
      if(!hGame)
         joy.enabled = 0;
      kmSave(WinQueryAnchorBlock(hwndOwner));
   }
   return fOk;
}
