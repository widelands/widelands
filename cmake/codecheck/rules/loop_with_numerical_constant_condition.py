#!/usr/bin/python


"""This catches a numerical constant used as a loop condition. Instead of while
(true) or while (1), one should write for (;;). But it avoids catching some
cases that are used in macro definitions (see.

[http://vivekkutal.blogspot.com/2006/03/do-while0.html]): #define
macro(...) do {...} while (false)
"""

error_msg = 'Do not use numerical constants in loop condition! ' \
    'Use for(;;) for endless loops.'

regexp = r"""(?x)
(^.?|[^}].|.[^ ])
while\s*
\(
(true|false|(0x[\da-fA-f]+|\d+)[Uu]?(L|l|LL|ll)?|\d*\.\d+[Ll]?)\)"""

forbidden = [
    'while (false)',
    'while (true)',
    'while (0)',
    'while (1)',
    'while (00)',
    'while (0U)',
    'while (0u)',
    'while (0UL)',
    'while (0uL)',
    'while (0Ul)',
    'while (0ul)',
    'while (0ULL)',
    'while (0uLL)',
    'while (0Ull)',
    'while (0ull)',
    'while (01)',
    'while (1U)',
    'while (1u)',
    'while (1UL)',
    'while (1uL)',
    'while (1Ul)',
    'while (1ul)',
    'while (1ULL)',
    'while (1uLL)',
    'while (1Ull)',
    'while (1ull)',
    'while (01.1)',
    'while (.1)',
    'while (.1L)',
    'while (.1l)',
    'while (.0)',
    'while (.0L)',
    'while (.0l)',
    'while (1.01)',
    'while (1.01L)',
    'while (1.01l)',
    'while (0xf)',
    'while (0x9d)',
    ' while (0x9d)',
    '  while (0x9d)',
]

allowed = [
    'for (;;)',
    'while (a)',
    'while (1 + b)',
    '} while (false)',
]
