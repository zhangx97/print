#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* typedefs for portability */
typedef unsigned       char uint08;
typedef   signed       char  int08;
typedef unsigned        int uint16;
typedef   signed        int  int16;
typedef unsigned       long uint32;
typedef   signed       long  int32;
typedef unsigned 	   char   byte;
typedef unsigned       char   BOOL;

#define TRUE    1
#define FALSE   0

/** Extract 8-bit value from 16-bit field */
#define LS_BYTE(x)  ((uint08)x)
#define MS_BYTE(x)  ((uint08)(x>>8))

/** Extract 16-bit value from 32-bit field */
#define LS_WORD(x)  ((uint16)x)
#define MS_WORD(x)  ((uint16)(x>>16))


#ifdef __cplusplus		/* matches __cplusplus construct above */
}
#endif

#endif /* __COMMON_H */
