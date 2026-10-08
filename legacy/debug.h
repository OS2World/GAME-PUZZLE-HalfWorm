#ifdef DEBUG_TERM
#include <stdio.h>
#endif

#ifdef DEBUG_TERM
#define dprintf(a) printf a
   #ifdef DEBUG_DIVE
   #define dived_printf(a) printf a
   #endif
#else
#define dprintf(a)
#endif

#ifdef DEBUG_TERM
#define dputs(a) puts(a)
   #ifdef DEBUG_DIVE
   #define dived_puts(a) puts(a)
   #endif
#else
#define dputs(a)
#endif


#ifndef dived_printf
#define dived_printf(a)
#endif

#ifndef dived_puts
#define dived_puts(a)
#endif

