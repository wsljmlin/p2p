#pragma once

//net type: 
#ifdef _WIN32
typedef int		            sint32;
typedef unsigned int	    uint32;
typedef __int64		        sint64;
typedef unsigned __int64    uint64;
#endif 

#ifdef  LINUX32
typedef  int                sint32;
typedef  unsigned int       uint32;
typedef  long long          sint64;
typedef  unsigned long long uint64;
#endif

#ifdef  LINUX64
typedef  int                sint32;
typedef  unsigned int       uint32;
typedef  long               sint64;
typedef  unsigned long      uint64;
#endif

typedef  short              sint16;
typedef  unsigned short     uint16;
typedef  unsigned short     ushort;
typedef  unsigned char      uchar;

typedef uint16 cmd_t;

#define SINT64_INFINITE (sint64)-1
#define UINT64_INFINITE (uint64)-1

#define HASHLEN   48
#define PUIDLEN 32
typedef char puid_t[PUIDLEN];   //user id¿‡–Õ
typedef char fhash_t[HASHLEN];

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) typedef char __static_assert_t[ (expr) ]
#endif


STATIC_ASSERT(4==sizeof(int));
STATIC_ASSERT(1==sizeof(uchar));
STATIC_ASSERT(2==sizeof(ushort));
STATIC_ASSERT(4==sizeof(sint32));
STATIC_ASSERT(8==sizeof(sint64));

