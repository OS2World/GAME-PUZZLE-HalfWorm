#ifndef __PMX_H__
#define __PMX_H__

typedef struct _APPPRF *PAPPPRF;

BOOL _Optlink EnableMenuItems(HWND hwndMenu, ULONG idFirst, ULONG idLast, BOOL fEnable);

HAB _Optlink prfAnchorBlock(PAPPPRF prfApp);
HINI _Optlink prfHandle(PAPPPRF prfApp);
PSZ _Optlink prfAppName(PAPPPRF prfApp);


PAPPPRF _Optlink openAppProfile(HAB hab, PSZ pszProfileName, ULONG idsProfile, ULONG idsApp);
BOOL _Optlink closeAppProfile(PAPPPRF appPrf);

BOOL _Optlink writeProfileLong(PAPPPRF appPrf, ULONG idsKey, LONG val);
LONG _Optlink readProfileLong(PAPPPRF appPrf, ULONG idsKey, LONG defaultVal);

BOOL _Optlink writeProfileData(PAPPPRF appPrf, ULONG idsKey, PVOID pBuf, ULONG cbBuf);
BOOL _Optlink readProfileData(PAPPPRF appPrf, ULONG idsKey, PVOID pBuf, PULONG pcbBuf);

BOOL _Optlink readProfileBoolean(PAPPPRF appPrf, ULONG idsKey, BOOL fDefault);
BOOL _Optlink writeProfileBoolean(PAPPPRF appPrf, ULONG idsKey, BOOL fValue);


#define WinQueryParent(hwnd) WinQueryWindow((hwnd), QW_PARENT)

#define WinQueryOwner(hwnd) WinQueryWindow((hwnd), QW_OWNER)

#define WinWindowFromParentID(hwnd, id) WinWindowFromID(WinQueryParent( (hwnd) ), (id) )

#endif /* __PMX_H__ */
