#!/usr/bin/python -tt


def does_include_algorithm(lines, fn):
    includes_algorithm = False
    for lineno, line in enumerate(lines, 1):
        if "include <algorithm>" in line:
            includes_algorithm = True

        if "std::sort" in line and not includes_algorithm:
            return [ (fn, lineno,
                "This file uses std::sort but does not include <algorithm>.") ]
    return []

evaluate_matches = does_include_algorithm


#################
# ALLOWED TESTS #
#################
allowed = [
"""#include <algorithm>

std::sort(foo, bar, baz);
""",
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
"""
    std::sort(foo, bar, baz);
""",
"""
std::sort(foo, bar, baz);

#include <algorithm>
""",
]
