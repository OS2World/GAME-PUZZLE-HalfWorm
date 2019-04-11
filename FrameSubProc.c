#pragma strings(readonly)

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINFRAMEMGR
#define INCL_WINSYS
#define INCL_WINTRACKRECT
#define INCL_WINSCROLLBARS

#include <os2.h>

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#include "FrameSubProc.h"

#include "debug.h"


typedef struct _FRAMESUBPROCPARAMS
{
   PFNWP pfnOldProc;
   LONG cyStatusBar;
   HWND hwndLeftStatusbar;
   HWND hwndRightStatusbar;
}FRAMESUBPROCPARAMS, *PFRAMESUBPROCPARAMS;


static MRESULT EXPENTRY FrameSubProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);




BOOL _Optlink subclassFrameWindow(HWND hwndFrame)
{
   BOOL fSuccess = FALSE;
   PFRAMESUBPROCPARAMS fspp = NULL;

   /*
    * Sub allocate frame window
    */
   if((fspp = (PFRAMESUBPROCPARAMS)malloc(sizeof(FRAMESUBPROCPARAMS))) != NULL)
   {
      memset(fspp, 0, sizeof(FRAMESUBPROCPARAMS));

      WinSetWindowULong(hwndFrame, QWL_USER, (ULONG)fspp);

      /*
       * Store some constants in application data area
       * Note: Normally, system values should be queried when used in case they have changed (you never know what has happened).
       */
      fspp->cyStatusBar = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
      fspp->hwndLeftStatusbar = WinWindowFromID(hwndFrame, FID_LEFTSTATUSBAR);
      fspp->hwndRightStatusbar = WinWindowFromID(hwndFrame, FID_RIGHTSTATUSBAR);

      if((fspp->pfnOldProc = WinSubclassWindow(hwndFrame, FrameSubProc)) == 0L)
      {
         free(fspp);
         _heapmin();
      }
      else
      {
         fSuccess = TRUE;
      }
   }
   return fSuccess;
}





static MRESULT EXPENTRY FrameSubProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   PFRAMESUBPROCPARAMS wd = (PFRAMESUBPROCPARAMS)(ULONG)WinQueryWindowULong(hwnd, QWL_USER);
   SHORT cControls;
   SWP swp;
   PSWP aswp;
   RECTL rect;
   int iClient;
   int iLeftStatusBar;
   int iRightStatusBar;
   BOOL fLeftStatusbar;
   BOOL fRightStatusbar;

   switch(msg)
   {
      case WM_CALCFRAMERECT:
         mReturn = (*wd->pfnOldProc)(hwnd, msg, mp1, mp2);
         if(mReturn)
         {
            PRECTL pRect;

            if(SHORT1FROMMP(mp2))
            {
               pRect = (PRECTL)mp1;
               pRect->yBottom += wd->cyStatusBar;
               mReturn = (MRESULT)TRUE;
            }
            else
            {
               pRect = (PRECTL)mp1;
               pRect->yBottom -= wd->cyStatusBar;
               mReturn = (MRESULT)TRUE;
            }
         }
         break;

      case WM_QUERYFRAMECTLCOUNT:
         cControls = SHORT1FROMMR((*wd->pfnOldProc)(hwnd, msg, mp1, mp2));
         if(WinIsChild(wd->hwndLeftStatusbar, hwnd))
         {
            cControls++;
         }
         if(WinIsChild(wd->hwndRightStatusbar, hwnd))
         {
            cControls++;
         }
         return ((MRESULT)(cControls));

      case WM_FORMATFRAME:
         iClient = 0;
         aswp = (PSWP)mp1;

         cControls = SHORT1FROMMR((*wd->pfnOldProc)(hwnd, WM_FORMATFRAME, mp1, mp2));

         /* Find out which aswp index is client window */
         while(aswp[iClient].hwnd != WinWindowFromID(hwnd, FID_CLIENT))
         {
            iClient++;
         }

         if(WinIsChild(wd->hwndLeftStatusbar, hwnd))
         {
            iLeftStatusBar = cControls++;
            aswp[iLeftStatusBar].hwnd = wd->hwndLeftStatusbar;
            fLeftStatusbar = TRUE;
         }
         else
         {
            fLeftStatusbar = FALSE;
         }
         if(WinIsChild(wd->hwndRightStatusbar, hwnd))
         {
            iRightStatusBar = cControls++;
            aswp[iRightStatusBar].hwnd = wd->hwndRightStatusbar;
            fRightStatusbar = TRUE;
         }
         else
         {
            fRightStatusbar = FALSE;
         }

         /*
          * NOTA BENE: No coordinates, sizes, etc have been modified before this point.
          */

         if(fLeftStatusbar)
         {
            aswp[iLeftStatusBar].x = aswp[iClient].x;
            aswp[iLeftStatusBar].y = aswp[iClient].y;
            aswp[iLeftStatusBar].cx = aswp[iClient].cx/2;
            aswp[iLeftStatusBar].cy = wd->cyStatusBar;
            aswp[iLeftStatusBar].fl = SWP_MOVE | SWP_SIZE;
         }

         if(fRightStatusbar)
         {
            aswp[iRightStatusBar].x = aswp[iClient].x+aswp[iClient].cx/2;
            aswp[iRightStatusBar].y = aswp[iClient].y;
            aswp[iRightStatusBar].cx = aswp[iClient].cx/2;
            aswp[iRightStatusBar].cy = wd->cyStatusBar;
            aswp[iRightStatusBar].fl = SWP_MOVE | SWP_SIZE;
         }

         if(fLeftStatusbar || fRightStatusbar)
         {
            aswp[iClient].y += wd->cyStatusBar;
            aswp[iClient].cy -= wd->cyStatusBar;
         }

         mReturn = MRFROMSHORT(cControls);
         break;

      case WM_DESTROY:
         free(wd);
         break;

      default:
         fHandled = FALSE;
         break;
   }
   if(!fHandled)
   {
      mReturn = (*wd->pfnOldProc)(hwnd, msg, mp1, mp2);
   }

   return mReturn;
}
