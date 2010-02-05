#!/usr/bin/python


"""
Do not create a temporary std::string with std::string::substr just to compare
it with another std::string or a char[] constant. Use std::string::compare
instead.
"""

error_msg = "Do not create a substring just to compare. Use compare instead."

regexp=r"""substr *\(.*, *.*\) *[!=]="""

forbidden = [
    'substr(0, 3) == str',
    'substr(0, 3) != str',
    'substr(pos, 3) == "abc"',
    'substr(pos, 3) != "abc"',
]

allowed = [
    'compare(0, 3, str)',
    'compare(pos, 3, "abc")',
]
