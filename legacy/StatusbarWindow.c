#pragma strings(readonly)

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINTIMER

#include <os2.h>

#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#ifdef DEBUG_TERM
#include <stdio.h>
#endif

#include "StatusbarWindow.h"
#include "AppleTypes.h"
#include "resources.h"


#define TEXTLIMIT                        512
#define FADE_STEP_TIME                   50
#define MESSAGE_LIFETIME                 2500
#define FAST_MESSAGE_LIFETIME            1000     /* In case queue becomes half full  (unused) */

/* Timer ID's */
#define TID_FADE                         TID_USERMAX-10
#define TID_MESSAGE_TIMEOUT              TID_USERMAX-11


#define DB_RAISED    0x0400
#define DB_DEPRESSED 0x0800


#define MAX_QUEUED                       10


typedef struct _MSGNODE
{
   HMODULE hModule;                      /* Module handle to load string from (not used if pszString is not NULL) */
   ULONG idString;                       /* Resource string ID (not used if pszString is not NULL) */
   PSZ pszString;                        /* Pointer to string */
}MSGNODE, *PMSGNODE;

typedef struct _MESSAGEQUEUE
{
   int iFirst;
   int iLast;
   int iCurrent;                         /* Used for circular queues */
   int cNodes;
   MSGNODE node[MAX_QUEUED];
}MESSAGEQUEUE, *PMESSAGEQUEUE;

enum StatusTextState
{
   Intermediate,                         /* Between two strings -- nothing is displayed */
   FadeIn,                               /* Fading in a string */
   Display,                              /* Displaying a string at full intensity */
   FadeOut                               /* Fading out a string */
};

typedef struct _WINDOWDATA
{
   HAB hab;
   PSZ pszQueuedText;
   ULONG cchQueuedText;
   PSZ pszText;
   ULONG cchText;
   RECTL rclText;
   ULONG flDrawText;
   enum StatusTextState textState;
   ULONG idFadeTimer;                    /* Fade in/out timer */
   ULONG idMessageTimer;                 /* How long to keep messages */
   ULONG cFadeColors;
   PLONG alFadeColors;
   int iTextColor;
   ULONG ulTextTimeout;                  /* How many milliseconds to keep message before fading it out */
   ULONG ulFadeStepDelay;
   ULONG ulFastFadeStepDelay;
   MESSAGEQUEUE msgq;
   ULONG flags;                          /* see FLG_* */
}WINDOWDATA, *PWINDOWDATA;
#define FLG_QUEUE_IN_USE                 0x00000001  /* WinSetWindowText() has been issued while anotther text was displayed */
#define FLG_AUTODIM                      0x00000002  /* Automatically dim text after ulTextTimeout milliseconds */


#define FADE_COLORS                      6
const LONG fade_colors[FADE_COLORS] = {
   CLR_BLACK,
   CLR_DARKCYAN,
   /* CLR_BLUE, */
   CLR_DARKGRAY,
   CLR_PALEGRAY,
   CLR_CYAN,
   CLR_WHITE
};



#define WMU_PROCESS_NEXT                 WM_USER+1


/*
 * Function prototypes
 */
static MRESULT EXPENTRY WindowProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

BOOL _Inline push_resource_string(PMESSAGEQUEUE q, HMODULE hMod, ULONG idString);
BOOL _Inline push_string(PMESSAGEQUEUE q, PSZ pszString);

BOOL _Inline pop_node(PMESSAGEQUEUE q, PMSGNODE n);
BOOL _Inline next_node(PMESSAGEQUEUE q, PMSGNODE n);

BOOL _Inline setWindowResString(HAB hab, HWND hwnd, HMODULE hMod, ULONG id);



BOOL _Optlink registerStatusbarClass(HAB hab)
{
   return WinRegisterClass(hab, WC_STATUSBAR, WindowProcedure, CS_SIZEREDRAW | CS_CLIPSIBLINGS, sizeof(PWINDOWDATA));
}



static MRESULT EXPENTRY WindowProcedure(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   MRESULT mReturn = 0;
   BOOL fHandled = TRUE;
   HPS hps;
   RECTL rect;
   PWINDOWDATA wd = (PWINDOWDATA)WinQueryWindowPtr(hwnd, 0);
   PWNDPARAMS pwndparams;
   int i;

   switch(msg)
   {
      case WM_CREATE:
         wd = (PWINDOWDATA)malloc(sizeof(WINDOWDATA));
         if(wd)
         {
            PCREATESTRUCT pCREATE = (PCREATESTRUCT)mp2;

            memset(wd, 0, sizeof(WINDOWDATA));

            wd->hab = WinQueryAnchorBlock(hwnd);

            /*
             * Set up fading palette
             */
            wd->alFadeColors = calloc(FADE_COLORS, sizeof(LONG));
            wd->cFadeColors = FADE_COLORS;
            memcpy(wd->alFadeColors, fade_colors, FADE_COLORS*sizeof(LONG));

            /*
             * Set some defaults
             */
            wd->ulTextTimeout = 2500;
            wd->ulFadeStepDelay = 50;
            wd->ulFastFadeStepDelay = 25;

            wd->flags |= FLG_AUTODIM;

            /*
             * Allocate string buffer, and initialize it if pszText field of the WinCreateWindow() parameter structure is used.
             */
            wd->pszText = malloc(TEXTLIMIT);
            if(pCREATE->pszText)
            {
               wd->cchText = min(strlen(pCREATE->pszText), TEXTLIMIT);
               memcpy(wd->pszText, pCREATE->pszText, wd->cchText);
               wd->textState = FadeIn;
               wd->idFadeTimer = WinStartTimer(wd->hab, hwnd, TID_FADE, FADE_STEP_TIME);
            }
            wd->pszQueuedText = malloc(TEXTLIMIT);
            wd->cchQueuedText = 0UL;

            wd->flDrawText |= (pCREATE->flStyle & (DT_LEFT | DT_RIGHT | DT_CENTER | DT_BOTTOM | DT_VCENTER | DT_TOP));

            WinSetWindowPtr(hwnd, 0, wd);
         }
         break;

      case WM_SIZE:
         if(WinQueryWindowRect(hwnd, &rect))
         {
            RECTL r;

            rect.xLeft += 2;
            rect.yBottom += 2;
            rect.xRight -= 2;
            rect.yTop -= 2;

            memcpy(&wd->rclText, &rect, sizeof(RECTL));
         }
         break;

      case WM_PAINT:
         if((hps = WinBeginPaint(hwnd, NULLHANDLE, &rect)) != NULLHANDLE)
         {
            RECTL r;

            WinQueryWindowRect(hwnd, &r);

            WinDrawBorder(hps, &r, 1, 1, CLR_PALEGRAY, CLR_PALEGRAY, 0UL);

            r.xLeft+=1;
            r.yBottom+=1;
            r.xRight-=1;
            r.yTop-=1;

            WinDrawBorder(hps, &r, 1, 1, CLR_WHITE, CLR_BLACK, DB_DEPRESSED);
            r.xLeft++;
            r.yBottom++;
            r.xRight--;
            r.yTop--;

            WinDrawText(hps, wd->cchText, wd->pszText, &r, wd->alFadeColors[wd->iTextColor], CLR_BLACK, wd->flDrawText | DT_ERASERECT);

            WinEndPaint(hps);
         }
         break;

      case WM_SETWINDOWPARAMS:
         pwndparams = (PWNDPARAMS)mp1;
         switch(pwndparams->fsStatus)
         {
            case WPM_TEXT:
               if(wd->flags & FLG_QUEUE_IN_USE)
               {
                  if(pwndparams->pszText)
                  {
                     /* Simply ignore (overwrite) whatever was queued */
                     wd->cchQueuedText = min(pwndparams->cchText, TEXTLIMIT);
                     memcpy(wd->pszQueuedText, pwndparams->pszText, wd->cchQueuedText);
                     mReturn = (MRESULT)TRUE;
                  }
               }
               else
               {
                  /* Queued buffer is not in use */
                  if(pwndparams->pszText)
                  {
                     switch(wd->textState)
                     {
                        case Display:
                           if(wd->idMessageTimer)
                           {
                              WinStopTimer(wd->hab, hwnd, wd->idMessageTimer);
                              wd->idMessageTimer = 0UL;
                           }
                           wd->idFadeTimer = WinStartTimer(wd->hab, hwnd, TID_FADE, wd->ulFadeStepDelay);
                        case FadeIn:
                           wd->textState = FadeOut;
                        case FadeOut:
                           wd->cchQueuedText = min(pwndparams->cchText, TEXTLIMIT);
                           memcpy(wd->pszQueuedText, pwndparams->pszText, wd->cchQueuedText);
                           wd->flags |= FLG_QUEUE_IN_USE;     /* Mark that queue is in use */
                           WinInvalidateRect(hwnd, &wd->rclText, FALSE);
                           mReturn = (MRESULT)TRUE;
                           break;

                        case Intermediate:
                           wd->cchText = min(pwndparams->cchText, TEXTLIMIT);
                           memcpy(wd->pszText, pwndparams->pszText, wd->cchText);
                           wd->textState = FadeIn;
                           wd->idFadeTimer = WinStartTimer(wd->hab, hwnd, TID_FADE, wd->ulFadeStepDelay);
                           WinInvalidateRect(hwnd, &wd->rclText, FALSE);
                           mReturn = (MRESULT)TRUE;
                           break;
                     }
                  }
               }
               break;

            default:
               fHandled = FALSE;
               break;
         }
         break;
/*
      case WMU_QUEUE_MESSAGE:
         push_message(&wd->msgq, (int)SHORT1FROMMP(mp1), (ULONG)SHORT2FROMMP(mp1));
         WinPostMsg(hwnd, WMU_PROCESS_NEXT, MPVOID, MPVOID);
         break;
*/
      case WM_TIMER:
         switch(SHORT1FROMMP(mp1))
         {
            case TID_FADE:
               switch(wd->textState)
               {
                  case FadeIn:
                     if(wd->iTextColor < (wd->cFadeColors-1))
                     {
                        wd->iTextColor++;
                     }
                     else
                     {
                        wd->textState = Display;

                        if(!(wd->flags & FLG_QUEUE_IN_USE))
                        {
                           if(WinStopTimer(wd->hab, hwnd, wd->idFadeTimer))
                           {
                              wd->idFadeTimer = 0UL;
                           }
                           if(wd->flags & FLG_AUTODIM)
                           {
                              wd->idMessageTimer = WinStartTimer(wd->hab, hwnd, TID_MESSAGE_TIMEOUT, wd->ulTextTimeout);
                           }
                        }
                     }
                     break;

                  case FadeOut:
                     if(wd->iTextColor > 0)
                     {
                        wd->iTextColor--;
                     }
                     else
                     {
                        wd->textState = Intermediate;

                        if(!(wd->flags & FLG_QUEUE_IN_USE))
                        {
                           if(WinStopTimer(wd->hab, hwnd, wd->idFadeTimer))
                           {
                              wd->idFadeTimer = 0UL;
                           }

                           if(wd->msgq.cNodes)
                           {
                              WinPostMsg(hwnd, SBMSG_NEXT, MPVOID, MPVOID);
                           }
                        }
                     }
                     break;
               }
               if((hps = WinGetPS(hwnd)) != NULLHANDLE)
               {
                  WinDrawText(hps, wd->cchText, wd->pszText, &wd->rclText, wd->alFadeColors[wd->iTextColor], CLR_BLACK, wd->flDrawText);
                  WinReleasePS(hps);
               }

               /* Check if current state is between string and if queue is in use */
               if((wd->textState == Intermediate) && (wd->flags & FLG_QUEUE_IN_USE))
               {
                  /* Copy queued buffer to real text buffer */
                  memcpy(wd->pszText, wd->pszQueuedText, wd->cchQueuedText);
                  wd->cchText = wd->cchQueuedText;
                  wd->flags &= ~FLG_QUEUE_IN_USE;
                  wd->textState = FadeIn;
                  WinInvalidateRect(hwnd, &wd->rclText, FALSE);
               }
               break;

            case TID_MESSAGE_TIMEOUT:
               if(WinStopTimer(wd->hab, hwnd, wd->idMessageTimer))
               {
                  wd->idMessageTimer = 0UL;
               }
               wd->textState = FadeOut;
               wd->idFadeTimer = WinStartTimer(wd->hab, hwnd, TID_FADE, wd->ulFadeStepDelay);
               break;

            default:
               fHandled = FALSE;
               break;
         }
         break;

      case WM_DESTROY:
         if(wd->idFadeTimer)
         {
            WinStopTimer(wd->hab, hwnd, wd->idFadeTimer);
         }
         if(wd->idMessageTimer)
         {
            WinStopTimer(wd->hab, hwnd, wd->idMessageTimer);
         }
         WinSendMsg(hwnd, SBMSG_RESET_QUEUE, MPVOID, MPVOID);
         free(wd->pszText);
         free(wd);
         break;

      case SBMSG_QUEUE_RES_STRING:
         push_resource_string(&wd->msgq, LONGFROMMP(mp1), LONGFROMMP(mp2));
         WinPostMsg(hwnd, SBMSG_NEXT, MPVOID, MPVOID);
         break;

      case SBMSG_QUEUE_PSZ:
         push_string(&wd->msgq, (PSZ)mp1);
         WinPostMsg(hwnd, SBMSG_NEXT, MPVOID, MPVOID);
         break;

      case SBMSG_NEXT:
         if(wd->textState != Intermediate)
            break;
         if(wd->msgq.cNodes)
         {
            MSGNODE n;
            if(wd->flags & SBSF_CIRCULAR)
            {
               next_node(&wd->msgq, &n);
            }
            else
            {
               pop_node(&wd->msgq, &n);
            }
            if(n.pszString)
            {
               WinSetWindowText(hwnd, n.pszString);
               if(!(wd->flags & SBSF_CIRCULAR))
               {
                  /*
                   * This pointer has been released from the circular
                   * buffer -- it *must* be released from the heap
                   */
                  free(n.pszString);
               }
            }
            else
            {
               setWindowResString(wd->hab, hwnd, n.hModule, n.idString);
            }
         }
         break;

      case SBMSG_SETSTYLE:
         wd->flags |= (LONGFROMMP(mp1) & 0x0000ffff);
         mReturn = (MRESULT)TRUE;
         break;

      case SBMSG_CLEARSTYLE:
         wd->flags &= (0xffff0000 | ~(LONGFROMMP(mp1) & 0x0000ffff));
         mReturn = (MRESULT)TRUE;
         break;

      case SBMSG_SETAUTODIM:
         if(LONGFROMMP(mp1))
         {
            /* Enable autodim */
            wd->flags |= FLG_AUTODIM;
            switch(wd->textState)
            {
               case FadeIn:
               case FadeOut:
               case Display:
                  wd->idMessageTimer = WinStartTimer(wd->hab, hwnd, TID_MESSAGE_TIMEOUT, wd->ulTextTimeout);
                  break;
            }
         }
         else
         {
            /* Disable autodim */
            wd->flags &= ~FLG_AUTODIM;
            if(wd->idMessageTimer)
            {
               if(WinStopTimer(wd->hab, hwnd, wd->idMessageTimer))
               {
                  wd->idMessageTimer = 0UL;
               }
            }
         }
         break;

      case SBMSG_SETFADESTEPDELAY:
         wd->ulFadeStepDelay = LONGFROMMP(mp1);
         mReturn = (MRESULT)TRUE;
         break;

      case SBMSG_SETTEXTTIMEOUT:
         wd->ulTextTimeout = LONGFROMMP(mp1);
         mReturn = (MRESULT)TRUE;
         break;

      case SBMSG_RESET_QUEUE:
         for(i = wd->msgq.iFirst; wd->msgq.cNodes; i = (i + 1) % MAX_QUEUED, wd->msgq.cNodes--)
         {
            PMSGNODE n = &wd->msgq.node[i];
            if(n->pszString)
            {
               free(n->pszString);
               n->pszString = NULL;
            }
         }
         wd->msgq.iFirst = wd->msgq.iLast = wd->msgq.iCurrent = wd->msgq.cNodes = 0;
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

BOOL _Inline push_resource_string(PMESSAGEQUEUE q, HMODULE hMod, ULONG idString)
{
   if(q->cNodes < MAX_QUEUED)
   {
      PMSGNODE n = &q->node[q->iLast];
      n->hModule = hMod;
      n->idString = idString;
      n->pszString = NULL;               /* Not used for this stringtype */
      q->iLast = (q->iLast+1) % MAX_QUEUED;
      q->cNodes++;
      return TRUE;
   }
   return FALSE;
}
BOOL _Inline push_string(PMESSAGEQUEUE q, PSZ pszString)
{
   if(q->cNodes < MAX_QUEUED)
   {
      PMSGNODE n = &q->node[q->iLast];
      LONG len = strlen(pszString)+1;
      n->hModule = NULLHANDLE;           /* Not used for this stringtype */
      n->idString = 0UL;                 /* Not used for this stringtype */
      n->pszString = malloc(len);
      if(n->pszString)
      {
         memcpy(n->pszString, pszString, len);
      }
      q->iLast = (q->iLast+1) % MAX_QUEUED;
      q->cNodes++;
      return TRUE;
   }
   return FALSE;
}

/*
 * Nota bene: The string pointer is handed over to the n node; the function
 *            calling this function must release that pointer!
 */
BOOL _Inline pop_node(PMESSAGEQUEUE q, PMSGNODE n)
{
   if(q->cNodes == 0)
      return FALSE;
   memcpy(n, &q->node[q->iFirst], sizeof(MSGNODE));
   memset(&q->node[q->iFirst], 0, sizeof(MSGNODE));
   q->iFirst = (q->iFirst+1) % MAX_QUEUED;
   q->iCurrent = q->iFirst;
   q->cNodes--;
   return TRUE;
}
BOOL _Inline next_node(PMESSAGEQUEUE q, PMSGNODE n)
{
   if(q->cNodes == 0)
      return FALSE;
   memcpy(n, &q->node[q->iCurrent], sizeof(MSGNODE));
   q->iCurrent = (q->iCurrent+1) % MAX_QUEUED;
   if(q->iCurrent == q->iLast)
   {
      q->iCurrent = q->iFirst;
   }
   return TRUE;
}




BOOL _Inline setWindowResString(HAB hab, HWND hwnd, HMODULE hMod, ULONG id)
{
   LONG lLength = 0L;
   char achText[256] = "";

   if((lLength = WinLoadString(hab, hMod, id, sizeof(achText), achText)) != 0L)
   {
      WinSetWindowText(hwnd, achText);
      return TRUE;
   }
   return FALSE;
}





/*
   Anna, Bosse och Lars springer utefter en bana. F”rst springer Lars, 300 meter efter honom springer Bosse och 100 meter efter Bosse
   springer Anna. Efter sex minuter har Anna sprungit ikapp Bosse, och efter ytterliga sex minuter har hon springit ikapp Lars.
   Hur m†nga minuter tar det f”r Bosse att springa ikapp Lars, om var och en springer med konstant fart?

   Rita upp en tallinje i bas sju och en linje i bas fem och ge med hj„lp av dem tv† olika l”sningar p† f”ljande:
   24base7 + 16base7 = ?base5 - ?base5

      112 - 1
      113 - 2

   base5
                    |
   +----+----+----+----+----+----+----+---+----+-----+---+----+----+----+-----+
   000000000000000000000000011111111111111111111111112222222222222222222222222
   000001111122222333334444400000111112222233333444440000011111222223333344444
   012340123401234012340123401234012340123401234012340123401234012340123401234

   base7
                     |
   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
   00000000000000000000000000000000000000000000000000000000111111111111111111111111111111111111111111
   00000001111111222222233333334444444555555566666660000000111111122222223333333444444455555556666666
   01234560123456012345601234560123456012345601234560123456012345601234560123456012345601234560123456

   24base7 + 16base7 = 43base7  (31base10)


   1, 2, 3, 4, 5, 6, 10, 11, 12, 13, 14, 15, 16
*/



