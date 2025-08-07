#!/usr/bin/env python3
# encoding: utf-8
#

# Utility to enforce optional parentheses around i18n functions in lua scripts.

import os
import re
import sys
from file_utils import find_files


def check_line(filename, lidx, line, print_error=True):
    e = 0
    # check if i18n functions have parentheses
    check_i18n = '_' in line or 'gettext' in line  # s1 in s2 is much faster than re.search()
    for method in ['(^|[^A-Za-z0-9_])_', 'gettext'] if check_i18n else []:
        for stringtype in ['"',  r'\[=*\[']:
            if re.compile(method + ' *' + stringtype).search(line):
                e = e + 1
                if print_error:
                    print('Optional parentheses missing at {}:{}: {}'.format(
                        filename, lidx, line), end='')
    # possibly more checks
    return e


# Selftests against false-positives and false-negatives
known_issues = [
    '_"Hello World"',
    'gettext "Hello World"',
    '_[===[Hi]===]',
    '_[[Hi %1%]]:bformat("Foo")',
    '_([[Hi %1%]]):bformat(_"Foo")',
]
known_clean_lines = [
    '_("Hello World")',
    'gettext("Hello World")',
    '_([==[Hi]==])',
    '_([[Hi %1%]]):bformat("Foo")',
    '_([[Hi %1%]]):bformat(_("Foo"))',
    '_("_foo_")',
]

errors = 0

for line in known_clean_lines:
    e = check_line('selftest false-positives', 1, line)
    errors += e
    if e > 0:
        print('SELFTEST ERROR: false-positive in "{}"'.format(line))

for line in known_issues:
    e = check_line('selftest false-negatives', 1, line, False)
    if e == 0:
        errors += 1
        print('SELFTEST ERROR: false-negative in "{}"'.format(line))

if errors > 0:
    print('\nThere were selftest errors, please fix!')
    sys.exit(1)

# Actual check
for filename in find_files(os.path.relpath(os.path.join(os.path.dirname(__file__), '..')), ['.lua', '.rst', '.rst.org']):
    if filename.endswith('.lua'):
        for lidx, line in enumerate(open(filename, 'r')):
            errors += check_line(filename, lidx, line)
    else:  # rst file
        checking = True
        for lidx, line in enumerate(open(filename, 'r')):
            if checking:
                checking = line != '.. code-block:: ini\n'  # disable checking in an ini block
            else:
                checking = line[:1] not in (' ', '\n')  # enable checking at end of block
            if checking:
                errors += check_line(filename, lidx, line)

if errors > 0:
    print('\nThere were {} error(s), please fix!'.format(errors))
    sys.exit(1)
