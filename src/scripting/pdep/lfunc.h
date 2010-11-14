/*
** $Id: lfunc.h,v 2.4.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef LFUNC_H
#define LFUNC_H

#define sizeLclosure(n)                                                       \
   (cast(int, sizeof(LClosure)) + cast(int, sizeof(TValue *) * ((n) - 1)))    \

#endif
