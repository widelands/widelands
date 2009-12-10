#!/usr/bin/python


"""
For standard containers and similar, do not use "not something.size()" to test
if it is empty. Just use "something.empty()".
"""

error_msg = "Do not use !a.size(), use a.empty() instead."

regexp=r"""(not |!) *[a-zA-Z_][a-zA-Z_0-9]* *(\.|->) *size *\(\)"""

forbidden = [
    '	!something.size ();',
    '	not something.size();',
    '	not something .size();',
    '	not something  .size();',
    '	not something. size();',
    '	not something.  size();',
    '	not _.size();',
    '	not __.size();',
    '	not _9.size();',
    '	not __2.size();',
    '	not x_9.size();',
]

allowed = [
    '	something.empty();',
]


