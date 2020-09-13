#!/usr/bin/python -tt

"""Checks that cstdlib is included for some of its functions that we use
commonly.

Older Macs that build with libc++ instead of libstc++ need this.
"""

import re

FUNCTION_REGEX = re.compile(
    r"""((\s|std::|\(|\[)(abs|ato[fil]|atoll|strto[dfl]|strtol[ld]|strtoul[l]{0,1}|malloc|getenv)|(\bstd::[s]{0,1}rand))\(""")


def does_include_cstdlib(lines, fn):
    includes_cstdlib = False
    for lineno, line in enumerate(lines, 1):
        if line.startswith('#include <cstdlib>'):
            includes_cstdlib = True

        matches = []
        for match in FUNCTION_REGEX.findall(line):
            matches.append(match[0])

        if matches and not includes_cstdlib:
            return [(fn, lineno,
                     'This file uses ' + ', '.join(matches) + ' but does not include <cstdlib>.')]
    return []


evaluate_matches = does_include_cstdlib


#################
# ALLOWED TESTS #
#################
allowed = [
    """#include <cstdlib>

    std::abs(-5);
""",
    """#include <cstdlib>

    std::strtoull;
""",
    """
    uint32_t const hit = game.logic_rand() % 100;
""",
    """
    uint32_t rand();
"""
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
    """
    const long int x = strtol(endp, &endp, 0);
""",
    """
    std::abs(-5);
""",
    """
    std::abs(-5);

#include <cstdlib>
""",
    """
    std::strtoull(endp, &endp, 0);
""",
    """
    std::strtoull(endp, &endp, 0);

#include <cstdlib>
""",
]
