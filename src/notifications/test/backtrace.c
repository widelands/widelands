#ifdef _WIN32
// From https://spin.atomicobject.com/2013/01/13/exceptions-stack-traces-c/
#include <windows.h>
#include <stdio.h>
#include <dbghelp.h>
#include <stdlib.h>

#ifdef _WIN64
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_AMD64
#else
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_I386
#endif
 
/* Resolve symbol name and source location given the path to the executable 
   and an address */
int addr2line(void const * const addr)
{
  char addr2line_cmd[512] = {0};
 
  /* have addr2line map the address to the relent line in the code */
  sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", icky_global_program_name, addr); 
 
  /* This will print a nicely formatted string specifying the
     function and source line of the address */
  return system(addr2line_cmd);
}

void windows_print_stacktrace(CONTEXT* context)
{
  SymInitialize(GetCurrentProcess(), 0, true);
 
  STACKFRAME frame = { 0 };
 
  /* setup initial stack frame */
  frame.AddrPC.Offset         = context->Eip;
  frame.AddrPC.Mode           = AddrModeFlat;
  frame.AddrStack.Offset      = context->Esp;
  frame.AddrStack.Mode        = AddrModeFlat;
  frame.AddrFrame.Offset      = context->Ebp;
  frame.AddrFrame.Mode        = AddrModeFlat;
 
  while (StackWalk64(IMAGE_FILE_MACHINE ,
                   GetCurrentProcess(),
                   GetCurrentThread(),
                   &frame,
                   context,
                   0,
                   SymFunctionTableAccess,
                   SymGetModuleBase,
                   0 ) )
  {
    addr2line((void*)frame.AddrPC.Offset);
  }
 
  SymCleanup( GetCurrentProcess() );
}

LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
  switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION:
      fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stderr);
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
      break;
    case EXCEPTION_BREAKPOINT:
      fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      fputs("Error: EXCEPTION_DATATYPE_MISALIGNMENT\n", stderr);
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      fputs("Error: EXCEPTION_FLT_DENORMAL_OPERAND\n", stderr);
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      fputs("Error: EXCEPTION_FLT_DIVIDE_BY_ZERO\n", stderr);
      break;
    case EXCEPTION_FLT_INEXACT_RESULT:
      fputs("Error: EXCEPTION_FLT_INEXACT_RESULT\n", stderr);
      break;
    case EXCEPTION_FLT_INVALID_OPERATION:
      fputs("Error: EXCEPTION_FLT_INVALID_OPERATION\n", stderr);
      break;
    case EXCEPTION_FLT_OVERFLOW:
      fputs("Error: EXCEPTION_FLT_OVERFLOW\n", stderr);
      break;
    case EXCEPTION_FLT_STACK_CHECK:
      fputs("Error: EXCEPTION_FLT_STACK_CHECK\n", stderr);
      break;
    case EXCEPTION_FLT_UNDERFLOW:
      fputs("Error: EXCEPTION_FLT_UNDERFLOW\n", stderr);
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      fputs("Error: EXCEPTION_ILLEGAL_INSTRUCTION\n", stderr);
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      fputs("Error: EXCEPTION_IN_PAGE_ERROR\n", stderr);
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      fputs("Error: EXCEPTION_INT_DIVIDE_BY_ZERO\n", stderr);
      break;
    case EXCEPTION_INT_OVERFLOW:
      fputs("Error: EXCEPTION_INT_OVERFLOW\n", stderr);
      break;
    case EXCEPTION_INVALID_DISPOSITION:
      fputs("Error: EXCEPTION_INVALID_DISPOSITION\n", stderr);
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      fputs("Error: EXCEPTION_NONCONTINUABLE_EXCEPTION\n", stderr);
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      fputs("Error: EXCEPTION_PRIV_INSTRUCTION\n", stderr);
      break;
    case EXCEPTION_SINGLE_STEP:
      fputs("Error: EXCEPTION_SINGLE_STEP\n", stderr);
      break;
    case EXCEPTION_STACK_OVERFLOW:
      fputs("Error: EXCEPTION_STACK_OVERFLOW\n", stderr);
      break;
    default:
      fputs("Error: Unrecognized Exception\n", stderr);
      break;
  }
  fflush(stderr);
  /* If this is a stack overflow then we can't walk the stack, so just show
    where the error happened */
  if (EXCEPTION_STACK_OVERFLOW != ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
      windows_print_stacktrace(ExceptionInfo->ContextRecord);
  }
  else
  {
      addr2line((void*)ExceptionInfo->ContextRecord->Eip);
  }
 
  return EXCEPTION_EXECUTE_HANDLER;
}

static void __attribute__((constructor)) setup_sigsegv() {
  SetUnhandledExceptionFilter(windows_exception_handler);
}
#endif
