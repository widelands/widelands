/*
** $Id: lzio.h,v 1.21.1.1 2007/12/27 13:02:25 roberto Exp $
** Buffered streams
** See Copyright Notice in lua.h
*/


#ifndef LZIO_H
#define LZIO_H

#include <cstddef>

struct Mbuffer {
	char * buffer;
	size_t n;
	size_t buffsize;
};

#endif
