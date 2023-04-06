#!/usr/bin/python -tt


import re

__PRINTF = re.compile(r'\bf?printf\s*\(')


def check_for_printf(lines, fn):
    errors = []
    for lineno, line in enumerate(lines, 1):
        if __PRINTF.search(line):
            errors.append((fn, lineno,
                           'Use the log_...() macros from base/log.h instead of printf.'))
    return errors


strip_comments_and_strings = True
evaluate_matches = check_for_printf

#################
# ALLOWED TESTS #
#################
allowed = [
    """log_err("blubi")""",
    """log_info_time(100, "blubi")""",
    """trianetn sprintf("rnae")""",
]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
    """printf ("blubi")""",
    """trianetn printf("rnae")""",
    """fprintf ("blubi")""",
    """trianetn fprintf("rnae")""",
]
