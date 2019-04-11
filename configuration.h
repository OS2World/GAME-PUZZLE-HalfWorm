#include "pmx.h"

/*
 * Player controls
 */
typedef struct _PLAYERKEYS
{
   BYTE bCounterClockwise;
   BYTE bClockwise;
   BYTE bFire;
}PLAYERKEYS, *PPLAYERKEYS;

BOOL _Optlink loadPlayerKeys(HAB hab, PAPPPRF prf, int iPlayer, PPLAYERKEYS keys);
BOOL _Optlink savePlayerKeys(HAB hab, PAPPPRF prf, int iPlayer, PPLAYERKEYS keys);

