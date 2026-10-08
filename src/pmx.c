#pragma strings(readonly)

#define INCL_WINMENUS
#define INCL_WINSHELLDATA
#define INCL_WINERRORS

#include <os2.h>

#include <malloc.h>
#include <memory.h>

#include "debug.h"

#include "PMX.h"



/*
 * Enable/Disable a range of menuitems
 */
BOOL _Optlink EnableMenuItems(HWND hwndMenu, ULONG idFirst, ULONG idLast, BOOL fEnable)
{
   BOOL fSuccess = TRUE;
   ULONG i = idFirst;
   for(; i <= idLast; i++)
   {
      if(!WinEnableMenuItem(hwndMenu, i, fEnable))
      {
         fSuccess = FALSE;
      }
   }
   return fSuccess;
}


/*
 * Application profile data structure
 */
typedef struct _APPPRF
{
   HAB hab;
   HINI hIni;
   char app[256];
}APPPRF;

HAB _Optlink prfAnchorBlock(PAPPPRF prfApp)
{
   return prfApp->hab;
}

HINI _Optlink prfHandle(PAPPPRF prfApp)
{
   return prfApp->hIni;
}

PSZ _Optlink prfAppName(PAPPPRF prf)
{
   return prf->app;
}

PAPPPRF _Optlink openAppProfile(HAB hab, PSZ pszProfileName, ULONG idsProfile, ULONG idsApp)
{
   PAPPPRF appPrf = malloc(sizeof(APPPRF));

   if(appPrf)
   {
      LONG lLength = 0;
      char achName[256] = "";
      PSZ prfName = pszProfileName;

      memset(appPrf, 0, sizeof(APPPRF));

      appPrf->hab = hab;

      if(pszProfileName == NULL)
      {
         if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsProfile, sizeof(achName), achName)) != 0L)
         {
            prfName = achName;
         }
      }

      if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsApp, 256, appPrf->app)) != 0L)
      {
         if((appPrf->hIni = PrfOpenProfile(appPrf->hab, prfName)) == NULLHANDLE)
         {
            ERRORID erridErrorCode = WinGetLastError(appPrf->hab);

            #ifdef DEBUG_TERM
            printf("e PrfOpenProfile() failed in %s. WinGetLastError() reports %04x.\n", __FUNCTION__, LOUSHORT(erridErrorCode));
            puts(pszProfileName);
            #endif
         }
      }
      else
      {
         #ifdef DEBUG_TERM
         puts("e WinLoadString() failed in "__FUNCTION__".");
         #endif
      }
   }
   if(appPrf->hIni == NULLHANDLE)
   {
      free(appPrf);
      appPrf = NULL;
   }
   return appPrf;
}


BOOL _Optlink closeAppProfile(PAPPPRF appPrf)
{
   BOOL fSuccess = FALSE;
   if(appPrf)
   {
      if(appPrf->hIni)
      {
         if(PrfCloseProfile(appPrf->hIni))
         {
            fSuccess = TRUE;
         }
      }
      free(appPrf);
      _heapmin();
   }
   return fSuccess;
}

BOOL _Optlink writeProfileLong(PAPPPRF appPrf, ULONG idsKey, LONG val)
{
   LONG lLength = 0;
   char achKey[256] = "";
   BOOL fSuccess = FALSE;
   lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey);
   if(lLength != 0L)
   {
      fSuccess = PrfWriteProfileData(appPrf->hIni, appPrf->app, achKey, &val, sizeof(val));
   }
   return fSuccess;
}

LONG _Optlink readProfileLong(PAPPPRF appPrf, ULONG idsKey, LONG defaultVal)
{
   LONG lLength = 0;
   char achKey[256] = "";
   LONG retVal = defaultVal;
   lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey);
   if(lLength != 0L)
   {
      ULONG cbBufferMax = sizeof(LONG);
      if(!PrfQueryProfileData(appPrf->hIni, appPrf->app, achKey, &retVal, &cbBufferMax))
      {
         retVal = defaultVal;
      }
   }
   return retVal;
}

BOOL _Optlink writeProfileData(PAPPPRF appPrf, ULONG idsKey, PVOID pBuf, ULONG cbBuf)
{
   BOOL fSuccess = FALSE;
   LONG lLength = 0;
   char achKey[256] = "";
   if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey)) != 0L)
   {
      fSuccess = PrfWriteProfileData(appPrf->hIni, appPrf->app, achKey, pBuf, cbBuf);
   }
   return fSuccess;
}

BOOL _Optlink readProfileData(PAPPPRF appPrf, ULONG idsKey, PVOID pBuf, PULONG pcbBuf)
{
   BOOL fSuccess = FALSE;
   LONG lLength = 0;
   char achKey[256] = "";
   if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey)) != 0L)
   {
      fSuccess = PrfQueryProfileData(appPrf->hIni, appPrf->app, achKey, pBuf, pcbBuf);
   }
   return fSuccess;
}


BOOL _Optlink readProfileBoolean(PAPPPRF appPrf, ULONG idsKey, BOOL fDefault)
{
   BOOL fRet = fDefault;
   LONG lLength = 0;
   char achKey[256] = "";

   if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey)) != 0)
   {
      char pszDefault[16] = "";
      char tmp[16] = "";

      if(fDefault)
         memcpy(pszDefault, "true", 5);
      else
         memcpy(pszDefault, "false", 6);

      if(PrfQueryProfileString(appPrf->hIni, appPrf->app, achKey, pszDefault, tmp, sizeof(tmp)))
      {
         if(memcmp(tmp, "true", 5) == 0)
            fRet = TRUE;
         else if(memcmp(tmp, "false", 6) == 0)
            fRet = FALSE;
      }
   }
   return fRet;
}

BOOL _Optlink writeProfileBoolean(PAPPPRF appPrf, ULONG idsKey, BOOL fValue)
{
   BOOL fSuccess = FALSE;
   LONG lLength = 0;
   char achKey[256] = "";

   if((lLength = WinLoadString(appPrf->hab, (HMODULE)NULLHANDLE, idsKey, sizeof(achKey), achKey)) != 0)
   {
      char tmp[16] = "";
      if(fValue)
         memcpy(tmp, "true", 5);
      else
         memcpy(tmp, "false", 6);
      if(PrfWriteProfileString(appPrf->hIni, appPrf->app, achKey, tmp))
         fSuccess = TRUE;
   }
   return fSuccess;
}
