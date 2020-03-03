#!/usr/bin/python


"""This catches a tab that comes after the first non-whitespace character on a
line.

Tabs should only be used for indentation.
"""

error_msg = 'Tabs are only allowed before the first non-whitespace character in line.'

regexp = r"""[^\t ].*[\t]"""

forbidden = [
    'a	{',

    # Eriks tests
    # misused_tab.cc
    'void\tf();',
]

allowed = [
    '	 	h;',
]
