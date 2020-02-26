#!/usr/bin/python


"""
This catches an opening parenthesis followed by a space or end of line.
"""

error_msg="No space or newline allowed after opening parenthesis."

strip_comments_and_strings = True
strip_macros = True
regexp=r"""\( """

forbidden = [
    '( h',
    '( ',

    # Eriks tests
    ## illegal_character_after_opening_parenthesis.cc
    'void f( float) {',
]

allowed = [
    '/(',
    '(',
    '/ /(',
    '/ / (',
    '/ / /(',
    '/ / /(\nadjh',
    '(h',
    '//(',
    '// (',
    '// (',
    '/* Convert \n * string blah ( \n */',
]
