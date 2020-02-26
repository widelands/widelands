#!/usr/bin/python -tt


def does_include_cstring(lines, fn):
    includes_cstring = False
    for lineno, line in enumerate(lines, 1):
        if "include <cstring>" in line:
            includes_cstring = True

        if "strlen" in line and not includes_cstring:
            return [ (fn, lineno,
                "This file uses strlen but does not include <cstring>.") ]
    return []

evaluate_matches = does_include_cstring


#################
# ALLOWED TESTS #
#################
allowed = [
"""#include <cstring>

if (strlen(char_array) != 0);
""",
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
"""
    if (strlen(char_array) != 0);
""",
"""
if (strlen(char_array) != 0);

#include <cstring>
""",
]
