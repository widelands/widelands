#!/usr/bin/python

"""
This disallows the use of SDL's integer types. C99 is now widely available and
is part of c++11, so we use this instead.
"""

error_msg ="Do not use SDL types. Use stdint.h types (e.g. uint32_t)"

regexp = r"""([SU]int(8|16|32|64)|#include.*SDL_types\.h|#include.*SDL_stdinc\.h)"""

forbidden = [
    "#include <SDL_types.h>",
    "#include <SDL_stdinc.h>",
    "blub Uint8",
    "Sint32"
    "int function(Uint16 x);"
]

allowed = [
    "#include <stdint.h>",
    "uint8_t",
    "int32_t"
]
