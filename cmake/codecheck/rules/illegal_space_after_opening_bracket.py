#!/usr/bin/python


"""This catches an opening [ followed by a space or end of line."""

error_msg = 'No space or newline allowed after opening bracket.'

strip_comments_and_strings = True
strip_macros = True
regexp = r"""\[( |$|\n)"""

forbidden = [
    '\tfloat a[ 2];',
]

allowed = [
    '[h',
    '//[',
    '// [',
    '// Do [ what you want ( in comments',
]
