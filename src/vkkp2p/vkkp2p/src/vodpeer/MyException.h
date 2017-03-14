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
����PCONTEXT��һ��ָ����������Ľṹ��ָ�룬�����˸����Ĵ������쳣������ʱ���ֵ����ϸ��Ϣ�ο���Windows���ı�̡��� 
ExceptionRecord��ָ����һ���ṹ��EXCEPTION_RECORD�� 
typedef struct _EXCEPTION_RECORD { // exr 
    DWORD ExceptionCode; 
    DWORD ExceptionFlags; 
    struct _EXCEPTION_RECORD *ExceptionRecord; 
    PVOID ExceptionAddress; 
    DWORD NumberParameters; 
    DWORD ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS]; 
} EXCEPTION_RECORD; 
DWORD ExceptionCode;
�쳣����,ָ���쳣ԭ�򡣳����쳣�����У�
EXCEPTION_ACCESS_VIOLATION = C0000005h
��д�ڴ��ͻ
EXCEPTION_INT_DIVIDE_BY_ZERO = C0000094h
��0����
EXCEPTION_STACK_OVERFLOW = C00000FDh
��ջ�������Խ��
EXCEPTION_GUARD_PAGE = 80000001h
��Virtual Alloc��������������ҳ��ͻ
EXCEPTION_NONCONTINUABLE_EXCEPTION = C0000025h
���ɳ����쳣,�����޷��ָ�ִ��,�쳣�������̲�Ӧ��������쳣
EXCEPTION_INVALID_DISPOSITION = C0000026h
���쳣���������ϵͳʹ�õĴ���
EXCEPTION_BREAKPOINT = 80000003h
����ʱ�жϣ�INT 3��
EXCEPTION_SINGLE_STEP = 80000004h
��������״̬(INT 1) 

*/
#else

namespace MyException
{
	typedef int (MY_EXCEPTION_FUNC)(void *p);
	inline int init(MY_EXCEPTION_FUNC f=NULL) { return 0;}
	inline int fini() { return 0; }
}

#endif
