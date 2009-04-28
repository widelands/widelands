#!/usr/bin/python


"""
This catches use of the infamous TRUE and FALSE macros that are sometimes
sighted, while allowing identifiers like THE_TRUE_STORY.
"""

regexp = r'''[^_a-zA-Z0-9"](TRUE|FALSE)[^_a-zA-Z0-9"]'''
error_msg = "Don't use TRUE/FALSE. Use true/false."

forbidden = [
    'while (TRUE)',
    'a = FALSE;',
]

allowed = [
    'while (IS_TRUE)',
    'while (TRUESOME)',
    'a = FALSENTRY;',
]


