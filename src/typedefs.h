#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

//---- Typdefinitionen ------------

#ifndef BYTE
#define BYTE	unsigned char
#endif
#ifndef WORD
#define WORD	unsigned short
#endif
#ifndef DWORD
#define DWORD	unsigned long
#endif

#ifndef BOOL
#define BOOL	int
#endif
#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

typedef signed char byte;
typedef unsigned char ubyte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef ubyte bool;

#ifndef NULL
#define NULL 0
#endif


#define ison(fl,f)		(((fl)&(f))!=0)
#define isoff(fl,f)		(!ison(fl,f))
#define isallon(fl,f)	(((fl)&(f))==(f))
#define on(fl,f)		(fl)|=(f)
#define off(fl,f)		(fl)&=(~(f))
#define toggle(fl,f)	(fl)^=(f)
#define setbits(fl,f,b)	if(b)on(fl,f);else off(fl,f)

#define lobyte(w)		(((BYTE*)&(w))[0])
#define hibyte(w)		(((BYTE*)&(w))[1])


#endif //_TYPEDEFS_H
