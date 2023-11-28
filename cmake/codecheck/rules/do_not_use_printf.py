#!/usr/bin/python3 -tt


import re

__PRINTF = re.compile(r'\bf?printf\s*\(')


def check_for_printf(lines, fn):
    errors = []
    for lineno, line in enumerate(lines, 1):
        if __PRINTF.search(line) and not 'NOLINT' in line:
            errors.append((fn, lineno,
                           'Use the log_...() macros from base/log.h instead of printf.'))
    return errors


strip_comments_and_strings = False
evaluate_matches = check_for_printf

#################
# ALLOWED TESTS #
#################
allowed = [
    """log_err("blubi")""",
    """log_info_time(100, "blubi")""",
    """trianetn sprintf("rnae")""",
	"""fprintf(file, "Foo"); // NOLINT"""
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
