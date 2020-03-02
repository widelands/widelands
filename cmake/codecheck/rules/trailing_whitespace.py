#!/usr/bin/python

"""A tab or space followed by the end of the line."""


regexp = r'[ \t]+$'
error_msg = 'Trailing whitespace at end of line'

forbidden = [
    """e; """,
    """e;	""",

    # Eriks tests
    # trailing_whitespace_and_missing_newline_at_eof.cc
    'void f() { ',

    '\t\t',
    '    ',
]

allowed = [
    'e;',
    'e;',
    '(o)	hi',
]
