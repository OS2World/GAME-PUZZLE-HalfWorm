#pragma strings(readonly)

#define INCL_WINWINDOWMGR
#define INCL_WINSHELLDATA

#include <os2.h>

#include <stdio.h>

#include "pmx.h"
#include "configuration.h"

#include "resources.h"



BOOL _Optlink loadPlayerKeys(HAB hab, PAPPPRF prf, int iPlayer, PPLAYERKEYS keys)
{
   BOOL fSuccess = FALSE;
   LONG lLength = 0;
   char pszFormat[256] = "";

   if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_PLAYER_FORMAT, sizeof(pszFormat), pszFormat)) != 0)
   {
      ULONG cbLen = 1;
      char prfKey[256] = "";
      char *p = prfKey;

      sprintf(prfKey, pszFormat, iPlayer);

      while(*p)
         p++;

      /* Counter clockwise rotation */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_COUNTERCLOCKROT, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfQueryProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bCounterClockwise, &cbLen))
         return FALSE;

      /* Clockwise rotation */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_CLOCKROT, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfQueryProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bClockwise, &cbLen))
         return FALSE;

      /* Fire */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_FIRE, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfQueryProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bFire, &cbLen))
         return FALSE;

      return TRUE;
   }
   return FALSE;
}

BOOL _Optlink savePlayerKeys(HAB hab, PAPPPRF prf, int iPlayer, PPLAYERKEYS keys)
{
   BOOL fSuccess = FALSE;
   LONG lLength = 0;
   char pszFormat[256] = "";

   if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_PLAYER_FORMAT, sizeof(pszFormat), pszFormat)) != 0)
   {
      char prfKey[256] = "";
      char *p = prfKey;

      sprintf(prfKey, pszFormat, iPlayer);

      while(*p)
         p++;

      /* Counter clockwise rotation */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_COUNTERCLOCKROT, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfWriteProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bCounterClockwise, sizeof(BYTE)))
         return FALSE;

      /* Clockwise rotation */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_CLOCKROT, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfWriteProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bClockwise, sizeof(BYTE)))
         return FALSE;

      /* Fire */
      if((lLength = WinLoadString(hab, (HMODULE)NULLHANDLE, IDS_PRFKEY_CONTROL_FIRE, sizeof(prfKey)-(p-prfKey), p)) == 0L)
      {
         return FALSE;
      }
      if(!PrfWriteProfileData(prfHandle(prf), prfAppName(prf), prfKey, &keys->bFire, sizeof(BYTE)))
         return FALSE;

      return TRUE;
   }
   return FALSE;
}
