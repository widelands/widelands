#!/usr/bin/python


"""
For standard containers and similar, do not use "something.size() > 0" to check
whether it contains something. Just use "!something.empty()".
"""

error_msg = "Do not use a.size() to check for emptiness, use a.empty() instead."

regexp=r""" *[a-zA-Z_][a-zA-Z_0-9]* *(\.|->) *size *\(\) *(> *0 *|== *0 *)"""

forbidden = [
    '	something.size() > 0',
    '	something. size() > 0',
    '	something.size()  >  0  ',
    '	!(something.size() > 0)',
    '	something.size() > 0 && something_else',
    '	something.size() == 0',
]

allowed = [
    '	!something.empty();',
    '	something.size() > 2;',
]
