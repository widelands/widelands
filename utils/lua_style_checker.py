#!/usr/bin/env python3
# encoding: utf-8
#

# Utility to enforce optional parentheses around i18n functions in lua scripts.

import os
import re
import sys
from file_utils import read_text_file, find_files

def check_line(filename, lidx, line, print_error = True):
	e = 0
	for method in [ '_', 'gettext' ]:
		for stringtype in [ '"', '\[=*\[' ]:
			if re.compile(method + ' *' + stringtype).search(line):
				e = e + 1
				if print_error:
					print('Optional parentheses missing at {}:{}: {}'.format(filename, lidx, line))
	return e

errors = 0

# Selftests against false-positives
for line in [
	'_("Hello World")',
	'gettext("Hello World")',
	'_([[Hi]])',
	'_([[Hi %1%]]):bformat("Foo")',
]:
	e = check_line("selftest false-positives", 1, line)
	errors += e
	if e > 0:
		print('SELFTEST ERROR: false-positive in "{}"'.format(line))

# Selftests against false-negatives
for line in [
	'_"Hello World"',
	'gettext "Hello World"',
	'_[[Hi]]',
	'_[[Hi %1%]]:bformat("Foo")',
]:
	e = check_line("selftest false-negatives", 1, line, False)
	if e == 0:
		errors += 1
		print('SELFTEST ERROR: false-negatives in "{}"'.format(line))

if errors > 0:
	print('\nThere were selftest errors, please fix!')
	sys.exit(1)

# Actual check
for filename in find_files(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')), ['.lua', '.rst', '.rst.org']):
	for lidx, line in enumerate(read_text_file(filename).strip().split('\n')):
		errors += check_line(re.compile(r""".*widelands.(.*)""").search(filename).group(1), lidx, line)

if errors > 0:
	print('\nThere were {} error(s), please fix!'.format(errors))
	sys.exit(1)
