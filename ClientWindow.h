#define WC_WORMCLIENT                    "WormClientClass"

typedef struct _WORMWNDCDATA
{
   USHORT cb;
   SIZEL sizlBoard;                      /* Board size - default 640x480 */
   LONG lWormCoreLength;                 /* default: 4 */
}WORMWNDCDATA, *PWORMWNDCDATA;


/* User messages */
#define WMU_RESET_WINDOW                 WM_USER+1
#define WMU_INFORM                       WM_USER+2
#define WMU_FIRST_RUN                    WM_USER+3
/* #define WMU_ENABLE_DIVE                  WM_USER+4 */
#define WMU_VOLUME_DIALOG                WM_USER+5

/* WMU_INFORM types - mp1  */
#define GAMETHREAD_HMQ                   1
#define GAMETHREAD_TERMINATED            2

