/*
** $Id: lgc.h,v 2.15.1.1 2007/12/27 13:02:25 roberto Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef LGC_H
#define LGC_H


/*
** some userful bit tricks
*/
#define bitmask(b) (1 << (b))
#define bit2mask(b1, b2) (bitmask(b1) | bitmask(b2))



/*
** Layout for bit use in `marked' field:
** bit 0 - object is white (type 0)
** bit 1 - object is white (type 1)
** bit 2 - object is black
** bit 3 - for userdata: has been finalized
** bit 3 - for tables: has weak keys
** bit 4 - for tables: has weak values
** bit 5 - object is fixed (should not be collected)
** bit 6 - object is "super" fixed (only the main thread)
*/


#define WHITE0BIT 0
#define WHITE1BIT 1
#define WHITEBITS bit2mask(WHITE0BIT, WHITE1BIT)

#define otherwhite(g) (g->currentwhite ^ WHITEBITS)
#define isdead(g,v) ((v)->gch.marked & otherwhite(g) & WHITEBITS)

#define luaC_white(g) cast(lu_byte, (g)->currentwhite & WHITEBITS)

#endif
