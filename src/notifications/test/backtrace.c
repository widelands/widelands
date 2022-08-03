/**
 * This source file is used to print out a stack-trace when your program
 * segfaults. It is relatively reliable and spot-on accurate.
 *
 * This code is in the public domain. Use it as you see fit, some credit
 * would be appreciated, but is not a prerequisite for usage. Feedback
 * on it's use would encourage further development and maintenance.
 *
 * Due to a bug in gcc-4.x.x you currently have to compile as C++ if you want
 * demangling to work.
 *
 * Please note that it's been ported into my ULS library, thus the check for
 * HAS_ULSLIB and the use of the sigsegv_outp macro based on that define.
 *
 * Author: Jaco Kroon <jaco@kroon.co.za>
 *
 * Copyright (C) 2005 - 2010 Jaco Kroon
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* Bug in gcc prevents from using CPP_DEMANGLE in pure "C" */
#if !defined(__cplusplus) && !defined(NO_CPP_DEMANGLE)
#define NO_CPP_DEMANGLE
#endif

#include <execinfo.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#ifndef NO_CPP_DEMANGLE
#include <cxxabi.h>
#ifdef __cplusplus
using __cxxabiv1::__cxa_demangle;
#endif
#endif

#ifdef HAS_ULSLIB
#include "uls/logger.h"
#define sigsegv_outp(x)         sigsegv_outp(,gx)
#else
#define sigsegv_outp(x, ...)    fprintf(stderr, x "\n", ##__VA_ARGS__)
#endif

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

static void signal_segv(int signum, siginfo_t* info, void*ptr) {
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};

    int i, f = 0;
    ucontext_t *ucontext = (ucontext_t*)ptr;

    sigsegv_outp("Segmentation Fault!");
    sigsegv_outp("info.si_signo = %d", signum);
    sigsegv_outp("info.si_errno = %d", info->si_errno);
    sigsegv_outp("info.si_code  = %d (%s)", info->si_code, si_codes[info->si_code]);
    sigsegv_outp("info.si_addr  = %p", info->si_addr);
    for(i = 0; i < NGREG; i++)
        sigsegv_outp("reg[%02d]       = 0x" REGFORMAT, i, ucontext->uc_mcontext.gregs[i]);

#ifndef SIGSEGV_NOSTACK
	int sz;
	void **bt = (void**) malloc(20 * sizeof(void *));
    sigsegv_outp("Stack trace (non-dedicated):");
    sz = backtrace(bt, 20);
    backtrace_symbols_fd(bt, sz, 2);
    sigsegv_outp("End of stack trace.");
#else
    sigsegv_outp("Not printing stack strace.");
#endif
    _exit (-1);
}

static void __attribute__((constructor)) setup_sigsegv() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGSEGV, &action, NULL) < 0)
        perror("sigaction");
}
