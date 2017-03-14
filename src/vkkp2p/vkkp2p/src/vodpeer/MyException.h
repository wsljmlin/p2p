#pragma once

#ifdef _WIN32
namespace MyException
{
	typedef struct _MyExceptionInfo
	{
		DWORD code;
		PVOID addr;
		char szerr_msg[1024];
		char szcall_stack[10240];
	}MyExceptionInfo;
	typedef int (WINAPI *MY_EXCEPTION_FUNC)(__in MyExceptionInfo *p);

	int init(MY_EXCEPTION_FUNC f=NULL);
	int fini();
};



/*
typedef struct _EXCEPTION_POINTERS { // exp 
    PEXCEPTION_RECORD ExceptionRecord; 
    PCONTEXT ContextRecord; 
} EXCEPTION_POINTERS; 
其中PCONTEXT是一个指向进程上下文结构的指针，保存了各个寄存器在异常发生的时候的值，详细信息参考《Windows核心编程》。 
ExceptionRecord则指向另一个结构体EXCEPTION_RECORD： 
typedef struct _EXCEPTION_RECORD { // exr 
    DWORD ExceptionCode; 
    DWORD ExceptionFlags; 
    struct _EXCEPTION_RECORD *ExceptionRecord; 
    PVOID ExceptionAddress; 
    DWORD NumberParameters; 
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]; 
} EXCEPTION_RECORD; 
DWORD ExceptionCode;
异常代码,指出异常原因。常见异常代码有：
EXCEPTION_ACCESS_VIOLATION = C0000005h
读写内存冲突
EXCEPTION_INT_DIVIDE_BY_ZERO = C0000094h
除0错误
EXCEPTION_STACK_OVERFLOW = C00000FDh
堆栈溢出或者越界
EXCEPTION_GUARD_PAGE = 80000001h
由Virtual Alloc建立起来的属性页冲突
EXCEPTION_NONCONTINUABLE_EXCEPTION = C0000025h
不可持续异常,程序无法恢复执行,异常处理例程不应处理这个异常
EXCEPTION_INVALID_DISPOSITION = C0000026h
在异常处理过程中系统使用的代码
EXCEPTION_BREAKPOINT = 80000003h
调试时中断（INT 3）
EXCEPTION_SINGLE_STEP = 80000004h
单步调试状态(INT 1) 

*/
#else

namespace MyException
{
	typedef int (MY_EXCEPTION_FUNC)(void *p);
	inline int init(MY_EXCEPTION_FUNC f=NULL) { return 0;}
	inline int fini() { return 0; }
}

#endif
