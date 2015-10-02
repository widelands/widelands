#!/usr/bin/python -tt


def does_include_using_namespace(lines, fn):
    if "system_headers.h" in fn:
        return []
    if not fn.endswith(".h"):
        return []
    for lineno, line in enumerate(lines, 1):
        if "using namespace" in line:
            return [ (fn, lineno,
                "Do not use \"using namespace\" in header files.") ]
    return []

evaluate_matches = does_include_using_namespace

#################
# ALLOWED TESTS #
#################
allowed = [
"""
""",
]

###################
# FORBIDDEN TESTS #
###################
forbidden = [
"""
using namespace
""",
]
