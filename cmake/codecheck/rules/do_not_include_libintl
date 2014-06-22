#!/usr/bin/python -tt


def does_include_libintl(lines, fn):
    if "i18n.h" in fn:
        return []
    for lineno, line in enumerate(lines, 1):
        if "include <libintl.h>" in line:
            return [ (fn, lineno,
                "Do not include libintl.h, instead include base/i18n.h which includes it.") ]
    return []

evaluate_matches = does_include_libintl


#################
# ALLOWED TESTS #
#################
allowed = [
"""#include <memory>

std::unique_ptr foo;
""",
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
"""
#include <libintl.h>
""",
]
