#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""The code base had inconsistent usage of tabs/spaces for indenting in Lua.

files. Spaces were more prominent - and I prefer them over tabs. So I wrote
this small script to fix leading tabs in Lua files to spaces.

It also saves files in unix file endings ("\r\n") and strips empty lines at the
end of files and whitespace characters at the end of lines.

After fixing the Lua tabs, this script also executes clang-format over the src
directory and pyformat over the utils directory.

"""

import argparse
import os
import re
import sys
from subprocess import call
from file_utils import read_text_file, write_text_file, find_files

LEADING_TABS = re.compile(r'^\s*\t+\s*')
SPACES_PER_TAB = 3


def parse_args():
    p = argparse.ArgumentParser(
        description='Fix common whitespace errors in Lua files, run clang-format'
        ' over the code base and pyformat over the utils directory.'
        ' Recurses over all relevant files.')
    return p.parse_args()

def main():
    parse_args()

    if not os.path.isdir('src') or not os.path.isdir('utils'):
        print('CWD is not the root of the repository.')
        return 1

    sys.stdout.write('Fixing Lua tabs ')
    for filename in find_files('.', ['.lua']):
        sys.stdout.write('.')
        sys.stdout.flush()
        lines = read_text_file(filename).strip().split('\n')
        new_lines = []
        for line in lines:
            m = LEADING_TABS.match(line)
            if m is not None:
                line = line[m.start():m.end()].expandtabs(
                    SPACES_PER_TAB) + line[m.end():]
            new_lines.append(line.rstrip() + '\n')
        write_text_file(filename, ''.join(new_lines))
    print(' done.')

    sys.stdout.write('\nFormatting C++ ')
    for filename in find_files('./src', ['.cc', '.h']):
        if 'third_party' in filename:
            continue
        sys.stdout.write('.')
        sys.stdout.flush()
        call(['clang-format', '-i', filename])
    print(' done.')

    sys.stdout.write('\nFormatting Python utils ')
    for filename in find_files('./utils', ['.py']):
        sys.stdout.write('.')
        sys.stdout.flush()
        call(['pyformat', '-i', filename])
    print(' done.')

    print 'Formatting finished.'
    return 0

if __name__ == '__main__':
    sys.exit(main())
