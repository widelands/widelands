#!/usr/bin/python


"""
This catches use of the contrived syntax for dereference and member selection:
(*pointer).member

Use the more convenient syntax instead:
pointer->member
"""

error_msg = "Do not use (*a).member, use a->member."

regexp = r"""[^]_A-Za-z0-9 >] *\(\*[a-zA-Z_][_a-zA-Z_]*\)\."""

forbidden = [
    '+ (*abc).def',
    '* (*abc).def',
    '/ ((*abc).def)',
]

allowed = [
    'abc->def',
    '_cast<T>(*abc).def',
    '_cast<T> (*abc).def',
    'operator [] (*p).jkl',
    'QString("%1 %2").arg(*it).arg(8)',
    'some_string.arg(*it).arg(8)',
]
