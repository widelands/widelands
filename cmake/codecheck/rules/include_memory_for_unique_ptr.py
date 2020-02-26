#!/usr/bin/python -tt


def does_include_memory(lines, fn):
    includes_memory = False
    for lineno, line in enumerate(lines, 1):
        if "include <memory>" in line:
            includes_memory = True

        if "unique_ptr" in line and not includes_memory:
            return [ (fn, lineno,
                "This file uses unique_ptr but does not include <memory>.") ]
    return []

evaluate_matches = does_include_memory


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
    std::unique_ptr<Blub> foo;
""",
"""
std::unique_ptr<Blub> foo;

#include <memory>
""",
]
