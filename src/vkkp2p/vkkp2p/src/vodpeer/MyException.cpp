
#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>
#include "MyException.h"

#pragma warning(disable:4996)

#pragma comment (lib, "imagehlp")
//#define ASSERT(f)          do{if(0==(f)) throw(0);}while(0)
//#define VERIFY(f)          ASSERT(f)

namespace MyException
{
	MY_EXCEPTION_FUNC g_fun=NULL;
	LPTOP_LEVEL_EXCEPTION_FILTER g_pref=NULL;

	int my_default_exception(MyExceptionInfo *p);
	int on_exception(EXCEPTION_POINTERS *pEP);

	LONG WINAPI _excp(EXCEPTION_POINTERS *pEP)
	{
		return on_exception(pEP);
	}

	int init(MY_EXCEPTION_FUNC f/*=NULL*/)
	{
		if(NULL==g_pref)
		{
			SetErrorMode (SEM_NOGPFAULTERRORBOX);
			g_pref = SetUnhandledExceptionFilter(_excp);
			g_fun = f;
			return 0;
		}
		return -1;
	}
	int fini()
	{
		if(g_pref)
		{
			SetErrorMode (0);
			SetUnhandledExceptionFilter(g_pref);
			g_pref = NULL;
			g_fun = NULL;
		}
		return 0;
	}

	int my_default_exception(MyExceptionInfo *p)
	{
		char buf[102400];
		sprintf(buf,"code=0x%08x,addr=0x%08x \r\n\r\n%s \r\n\r\n%s",p->code,(int)p->addr,p->szerr_msg,p->szcall_stack);
		MessageBoxA(0,buf,"exception",MB_OK);
		return 0;
	}
	int on_exception(EXCEPTION_POINTERS *pEP)
	{
		MyExceptionInfo inf;

		//***********************************************************************
		char sztmp[1024];
		memset (sztmp, 0x00, sizeof (sztmp));
		PEXCEPTION_RECORD pExcp = pEP->ExceptionRecord;
		PCONTEXT pCon = pEP->ContextRecord;
		inf.code = pExcp->ExceptionCode;
		inf.addr = pExcp->ExceptionAddress;
		//////////////////////////
		//
		//section inf
		//
		{
			switch (pExcp->ExceptionCode)
			{
			case STATUS_ACCESS_VIOLATION:
				sprintf (sztmp, "(0x%08x)非法内存操作", pExcp->ExceptionCode);
				break;
			case STATUS_STACK_OVERFLOW:
				sprintf (sztmp, "(0x%08x)堆栈溢出", pExcp->ExceptionCode);
				break;
			case STATUS_INTEGER_DIVIDE_BY_ZERO:
				sprintf (sztmp, "(0x%08x)除数为0", pExcp->ExceptionCode);
				break;
			case EXCEPTION_GUARD_PAGE:
				sprintf (sztmp, "(0x%08x)由Virtual Alloc建立起来的属性页冲突", pExcp->ExceptionCode);
				break;
			case EXCEPTION_NONCONTINUABLE_EXCEPTION:
				sprintf (sztmp, "(0x%08x)不可持续异常", pExcp->ExceptionCode);
				break;
			case EXCEPTION_INVALID_DISPOSITION:
				sprintf (sztmp, "(0x%08x)在异常处理过程中系统使用的代码", pExcp->ExceptionCode);
				break;
			case EXCEPTION_BREAKPOINT:
				sprintf (sztmp, "(0x%08x)调试时中断(INT 3)", pExcp->ExceptionCode);
				break;
			case EXCEPTION_SINGLE_STEP:
				sprintf (sztmp, "(0x%08x)单步调试状态(INT 1)", pExcp->ExceptionCode);
				break;
			default:
				sprintf (sztmp, "(0x%08x)其它异常", pExcp->ExceptionCode);
				break;
			}
			strcpy (inf.szerr_msg, sztmp);
			//sprintf (sztmp, "exception address = 0x%08x \r\nModule: ", pExcp->ExceptionAddress);
			//strcat (inf.szsec_msg, sztmp);
			// 得到异常所在的module
			//MEMORY_BASIC_INFORMATION mem;
			//VirtualQuery (pExcp->ExceptionAddress, &mem, sizeof (MEMORY_BASIC_INFORMATION));
			//GetModuleFileNameA ((HMODULE)mem.AllocationBase, sztmp, sizeof (sztmp));
			//strcat (inf.szsec_msg, sztmp);
			//// 定位异常的偏移位置(相对地址)
			//PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)(mem.AllocationBase);
			//PIMAGE_NT_HEADERS pNts = (PIMAGE_NT_HEADERS)((PBYTE)pDos + pDos->e_lfanew);
			//PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION (pNts);
			//// get relative virtual address
			//DWORD dRva = (DWORD)pExcp->ExceptionAddress - (DWORD)mem.AllocationBase;
			//// trips every section.
			//for (WORD wCnt = 0; wCnt < pNts->FileHeader.NumberOfSections; ++wCnt) {
			//	DWORD dStart = pSection->VirtualAddress;
			//	DWORD dEnd = dStart + max (pSection->SizeOfRawData, pSection->Misc.VirtualSize);
			//	if (dRva >= dStart && dRva <= dEnd) {
			//		sprintf (sztmp, "\r\nSection name: %s ; offset(rva) : 0x%08x", pSection->Name, dRva - dStart);
			//		strcat (inf.szsec_msg, sztmp);
			//		break;
			//	}
			//	++pSection;
			//}
		}

		//////////////////////////


		//////////////////////////
		//
		//fun call stack
		//
		{
			// 先搞到模块名字
			MEMORY_BASIC_INFORMATION mem;
			VirtualQuery ((PVOID)pCon->Eip, &mem, sizeof (MEMORY_BASIC_INFORMATION));
			GetModuleFileNameA ((HMODULE)mem.AllocationBase, sztmp, sizeof (sztmp));
			strcpy (inf.szcall_stack, sztmp);
			if (SymInitialize (GetCurrentProcess (), NULL, TRUE)) {
				STACKFRAME sf;
				memset (&sf, 0x00, sizeof (STACKFRAME));
				// Initialize the STACKFRAME structure for the first call.  This is only
				// necessary for Intel CPUs, and isn't mentioned in the documentation.
				sf.AddrPC.Offset       = pCon->Eip;
				sf.AddrPC.Mode         = AddrModeFlat;
				sf.AddrStack.Offset    = pCon->Esp;
				sf.AddrStack.Mode      = AddrModeFlat;
				sf.AddrFrame.Offset    = pCon->Ebp;
				sf.AddrFrame.Mode      = AddrModeFlat;
				int i=0;
				while (true) {
					if (!StackWalk (IMAGE_FILE_MACHINE_I386,
						GetCurrentProcess (),
						GetCurrentThread (),
						&sf,
						pCon,
						NULL,
						SymFunctionTableAccess,
						SymGetModuleBase,
						NULL))
					{
						break;
					}
					if ( 0 == sf.AddrFrame.Offset ) {// Basic sanity check to make sure
						break;                      // the frame is OK.  Bail if not.
					}
					// make image buffer
					BYTE imgBuf[sizeof (IMAGEHLP_SYMBOL) + 512];
					PIMAGEHLP_SYMBOL pSymbol = reinterpret_cast <PIMAGEHLP_SYMBOL> (imgBuf);
					pSymbol->SizeOfStruct = sizeof (IMAGEHLP_SYMBOL);
					pSymbol->MaxNameLength = 512;
					DWORD dLen = 0;
					if (SymGetSymFromAddr (GetCurrentProcess (), sf.AddrPC.Offset,
						&dLen, pSymbol))
					{
						sprintf (sztmp, "\r\n~%3d : %s()",++i, pSymbol->Name);
						strcat (inf.szcall_stack, sztmp);
						if(strlen(inf.szcall_stack)>9*1024) break;
					}
					IMAGEHLP_LINE line;
					if(SymGetLineFromAddr(GetCurrentProcess (),sf.AddrPC.Offset,&dLen,&line))
					{
						sprintf (sztmp, " ---> %s (%d)",line.FileName,line.LineNumber);
						strcat (inf.szcall_stack, sztmp);
						if(strlen(inf.szcall_stack)>9*1024) break;
					}
				}
				SymCleanup (GetCurrentProcess ());
			}
		}
		//////////////////////////
		
		//***********************************************************************

		if(g_fun)
			g_fun(&inf);
		else
			my_default_exception(&inf);
		return 0;
	}
}

#endif

