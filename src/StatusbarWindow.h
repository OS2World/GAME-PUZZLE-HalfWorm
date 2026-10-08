#define WC_STATUSBAR                     "StatusbarWindowClass"

typedef struct _STATUSBARWNDCDATA
{
   USHORT cb;
   ULONG flStyle;                        /* See SBSF_* */
}STATUSBARWNDCDATA, *PSTATUSBARWNDCDATA;

/*
 * Statusbar messages
 */
#define SBMSG_QUEUE_RES_STRING           WM_USER+200 /* Enque resource string. mp1 - HMODULE, mp2 - resource string ID */
#define SBMSG_QUEUE_PSZ                  WM_USER+201 /* Enque C string. mp1 - PSZ */
#define SBMSG_NEXT                       WM_USER+202 /* Display next string in queue */
#define SBMSG_SETSTYLE                   WM_USER+203 /* See SBSF_* flags */
#define SBMSG_CLEARSTYLE                 WM_USER+204 /* See SBSF_* flags */
#define SBMSG_RESET_QUEUE                WM_USER+205 /* Clear all entries in list */
#define SBMSG_SETAUTODIM                 WM_USER+206 /* mp1 - TRUE/FALSE */

#define SBMSG_SETFADESTEPDELAY           WM_USER+207
#define SBMSG_SETTEXTTIMEOUT             WM_USER+208


/*
 * Statusbar style flags
 */
#define SBSF_CIRCULAR                    0x00000001  /* Don't remove text entries after displaying them -- loop through all entries in queue */

