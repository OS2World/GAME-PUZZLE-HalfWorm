#ifndef OW_COMPAT_H
#define OW_COMPAT_H

/* OpenWatcom compatibility shims for IBM C/Set++ source */
#ifdef __WATCOMC__
# define _Optlink
# define _Inline __inline
#endif

#endif /* OW_COMPAT_H */
