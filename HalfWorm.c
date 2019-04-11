#pragma strings(readonly)

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS

#ifdef DEBUG_TERM
#define INCL_DOSQUEUES
#define INCL_DOSSESMGR
#define INCL_DOSERRORS
#endif

#include <os2.h>

#include <stdlib.h>
#include <memory.h>

#include "ClientWindow.h"
#include "FrameSubProc.h"
#include "StatusbarWindow.h"

#include "resources.h"

#ifdef DEBUG_TERM
static void launchDebugTerminal(void);
#endif

#include "debug.h"

/*
 * Function prototypes - external functions
 */
extern BOOL _Optlink registerClientClass(HAB hab);
extern BOOL _Optlink registerStatusbarClass(HAB hab);

extern BOOL _Optlink subclassFrameWindow(HWND hwndFrame);


/*
 * Function prototypes - local functions
 */
static HWND _Optlink createFrameWindow(HAB hab);
static HWND _Optlink createClientWindow(HAB hab, HWND hwndFrame, int argc, char *argv[]);


int main(int argc, char *argv[])
{
   int iRet = 0;
   HAB hab = NULLHANDLE;
   HMQ hmq = NULLHANDLE;
   HWND hwndFrame = NULLHANDLE;
   HWND hwndClient = NULLHANDLE;

   #ifdef DEBUG_TERM
   launchDebugTerminal();
   dputs(("Initial message. If you don't see this, then something is very fishy."));
   #endif

   if((hab = WinInitialize(0UL)) != NULLHANDLE)
   {
      if(registerClientClass(hab))
      {
         if(registerStatusbarClass(hab))
         {
            hmq = WinCreateMsgQueue(hab, 0L);
         }
      }
   }

   if(hmq)
   {
      hwndFrame = createFrameWindow(hab);
   }

   if(hwndFrame)
   {
      if(subclassFrameWindow(hwndFrame))
      {
         hwndClient = createClientWindow(hab, hwndFrame, argc, argv);
      }
   }

   if(hwndClient)
   {
      QMSG qmsg = { 0 };

      while(WinGetMsg(hab, &qmsg, NULLHANDLE, 0UL, 0UL))
      {
         WinDispatchMsg(hab, &qmsg);
      }
   }

   if(hab)
   {
      if(hmq)
      {
         if(hwndFrame)
         {
            if(hwndClient)
            {
               WinDestroyWindow(hwndClient);
            }
            WinDestroyWindow(hwndFrame);
         }
         WinDestroyMsgQueue(hmq);
      }
      WinTerminate(hab);
   }
   return iRet;
}


static HWND _Optlink createFrameWindow(HAB hab)
{
   HWND hwndFrame = NULLHANDLE;
   HWND hwndTemp = NULLHANDLE;
   char achName[256] = "";
   LONG lLength = 0;

   /* Load application title from module resources */
   if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_APPTITLE, sizeof(achName), achName)) != 0L)
   {
      FRAMECDATA fcd = { sizeof(fcd) };
      fcd.flCreateFlags = FCF_TITLEBAR | FCF_TASKLIST | FCF_MINBUTTON | FCF_SYSMENU | FCF_DLGBORDER | FCF_ICON | FCF_MENU | FCF_ACCELTABLE | FCF_NOBYTEALIGN;
      fcd.hmodResources = (HMODULE)NULLHANDLE;
      fcd.idResources = WIN_HALFWORM;

      /* Create application frame window */
      hwndTemp = WinCreateWindow(HWND_DESKTOP, WC_FRAME, achName, 0UL, 0L, 0L, 0L, 0L, (HWND)NULLHANDLE, HWND_TOP, fcd.idResources, &fcd, NULL);
      if(hwndTemp)
      {
         HWND hwndLeftStatusbar = NULLHANDLE;
         HWND hwndRightStatusbar = NULLHANDLE;
         char pp_tmp[256] = "";
         PPRESPARAMS pp = (PPRESPARAMS)pp_tmp;
         PARAM *p = NULL;
         LONG lColor = CLR_PALEGRAY;
         char font[] = "9.WarpSans Bold";
         /* char font[] = "9.WarpSans"; */

         memset(pp_tmp, 0, sizeof(pp_tmp));

         pp->cb = 0;

         p = pp->aparam;

         p->id = PP_BACKGROUNDCOLORINDEX;
         p->cb = sizeof(LONG);
         memcpy(p->ab, &lColor, sizeof(LONG));
         pp->cb += (8+p->cb);

         p = (PPARAM)(((ULONG)p) + (8+p->cb));

         p->id = PP_FONTNAMESIZE;
         p->cb = sizeof(font);
         memcpy(p->ab, font, sizeof(font));
         pp->cb += (8+p->cb);

         lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_LEFTWELCOME, sizeof(achName), achName);
         hwndLeftStatusbar = WinCreateWindow(hwndTemp, WC_STATUSBAR, achName, WS_VISIBLE | DT_LEFT | DT_VCENTER, 0L, 0L, 0L, 0L, hwndTemp, HWND_TOP, FID_LEFTSTATUSBAR, NULL, pp);
         if(hwndLeftStatusbar)
         {
            lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_RIGHTWELCOME, sizeof(achName), achName);
            hwndRightStatusbar = WinCreateWindow(hwndTemp, WC_STATUSBAR, achName, WS_VISIBLE | DT_RIGHT | DT_VCENTER, 0L, 0L, 0L, 0L, hwndTemp, HWND_TOP, FID_RIGHTSTATUSBAR, NULL, pp);
         }
         if(hwndRightStatusbar)
         {
            hwndFrame = hwndTemp;
         }
         else
         {
            WinDestroyWindow(hwndFrame);
         }
      }
   }
   return hwndFrame;
}

static HWND _Optlink createClientWindow(HAB hab, HWND hwndFrame, int argc, char *argv[])
{
   HWND hwndClient = NULLHANDLE;
   WORMWNDCDATA wwcd = { sizeof(wwcd) };
   int i = 1;

   /*
    * set defaults
    */
   wwcd.sizlBoard.cx = 640;
   wwcd.sizlBoard.cy = 480;

   for(; i < argc; i++)
   {
      switch(argv[i][0])
      {
         case '/':
         case '-':
            switch(argv[i][1])
            {
               case 'w':
               case 'W':
                  wwcd.sizlBoard.cx = atol(&argv[i][2]);
                  break;

               case 'h':
               case 'H':
                  wwcd.sizlBoard.cy = atol(&argv[i][2]);
                  break;
            }
            break;

         default:
            break;
      }
   }

   wwcd.sizlBoard.cx = max(wwcd.sizlBoard.cx, 320);
   wwcd.sizlBoard.cy = max(wwcd.sizlBoard.cy, 240);

   hwndClient = WinCreateWindow(hwndFrame, WC_WORMCLIENT, (PSZ)NULLHANDLE, 0UL, 0L, 0L, 0L, 0L, hwndFrame, HWND_TOP, FID_CLIENT, &wwcd, NULL);

   return hwndClient;
}





#ifdef DEBUG_TERM
static void launchDebugTerminal(void)
{
   APIRET rc = NO_ERROR;
   HFILE pipeDebugRead = 0;
   HFILE pipeDebugWrite = 0;
   STARTDATA sd = { 0 };
   ULONG SessID = 0;
   PID pid = 0;
   HFILE hfNew = 1;                      /* stdout */
   char PgmTitle[30] = "";
   char PgmName[100] = "";
   char szCommandLine[60] = "";
   char ObjBuf[200] = "";

   rc = DosCreatePipe(&pipeDebugRead, &pipeDebugWrite, 4096);
   if(rc != NO_ERROR)
   {
      DosBeep(1000, 100);
      exit(42);
   }

   _ultoa(pipeDebugRead, szCommandLine, 10);

   memset(&sd, 0, sizeof(sd));

   sd.Length = sizeof(STARTDATA);
   sd.Related = SSF_RELATED_CHILD;
   sd.FgBg = SSF_FGBG_BACK;
   sd.TraceOpt = SSF_TRACEOPT_NONE;
   memcpy((char*) PgmTitle, "Debug terminal\0", 15);
   sd.PgmTitle = PgmTitle;
   memcpy((char*) PgmName, "PMDebugTerminal.exe\0", 20);
   sd.PgmName = PgmName;
   sd.PgmInputs = szCommandLine;
   sd.InheritOpt = SSF_INHERTOPT_PARENT;
   sd.SessionType = SSF_TYPE_PM;
   sd.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_SETPOS;
   sd.ObjectBuffer = ObjBuf;
   sd.ObjectBuffLen = sizeof(ObjBuf);

   rc = DosStartSession(&sd, &SessID, &pid);
   if(rc != NO_ERROR)
   {
      DosBeep(1000, 100);
      exit(43);
   }
   DosDupHandle(pipeDebugWrite, &hfNew);
}
#endif
