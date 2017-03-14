

//net type: 

typedef  short              sint16;
typedef  unsigned short     uint16;
typedef  unsigned short     ushort;
typedef  unsigned char      uchar;


#ifdef _WIN32
typedef int		            sint32;
typedef unsigned int	    uint32;
typedef __int64		        sint64;
typedef unsigned __int64    uint64;
#else
typedef  int                sint32;
typedef  unsigned int       uint32;
typedef  long long          sint64;
typedef  unsigned long long uint64;
#endif

typedef uint16 cmd_t;

#define SINT64_INFINITE (sint64)-1
#define UINT64_INFINITE (uint64)-1
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) typedef char __static_assert_t[ (expr) ]
#endif

STATIC_ASSERT(4==sizeof(int));
STATIC_ASSERT(1==sizeof(uchar));
STATIC_ASSERT(2==sizeof(ushort));
STATIC_ASSERT(4==sizeof(sint32));
STATIC_ASSERT(8==sizeof(sint64));

