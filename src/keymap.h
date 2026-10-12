/*
 * keymap.h -- keyboard and joystick mapping of HalfWorm (Options - Key Mapping...)
 */
#ifndef __KEYMAP_H__
#define __KEYMAP_H__

#include "configuration.h"

/* actions of one worm */
#define KM_CCW    0     /* rotate counterclockwise */
#define KM_CW     1     /* rotate clockwise        */
#define KM_FIRE   2
#define KM_NACT   3

void _Optlink KmInit(HAB hab);          /* open GAME$, read the saved joystick mapping */
void _Optlink KmDone(void);
void _Optlink KmJoyPoll(void);          /* game thread: read the sticks once per scan   */
BOOL _Optlink KmJoyDown(int iPlayer, int iAct);   /* is that action held on the joystick? */

/* the dialog; keys[] are the keyboard scancodes of both players and are
   changed when the user confirms (returns TRUE); the joystick part is saved by the dialog */
BOOL _Optlink KmDialog(HWND hwndOwner, PLAYERKEYS keys[2]);

/* implemented in ClientWindow.c: index of the active language (0 EN, 1 ES, 2 NL, 3 DE, 4 FR, 5 IT) */
int _Optlink currentLanguage(void);

#endif
