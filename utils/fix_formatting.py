#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""The code base had inconsistent usage of tabs/spaces for indenting in Lua.

files. Spaces were more prominent - and I prefer them over tabs. So I wrote
this small script to fix leading tabs in Lua files to spaces.

It also saves files in unix file endings ("\r\n") and strips empty lines at the
end of files and whitespace characters at the end of lines.

After fixing the Lua tabs, this script also executes clang-format over the src directory and pyformat over the utils directory.

"""

import argparse
import os
import re
import sys
from subprocess import call

LEADING_TABS = re.compile(r'^\s*\t+\s*')
PYTHON3 = sys.version_info >= (3, 0)
SPACES_PER_TAB = 3


def parse_args():
    p = argparse.ArgumentParser(
        description='Fix common whitespace errors in Lua files and run clang-format over the code base. Recurses over all relevant files.')
    return p.parse_args()


def read_text_file(filename):
    """Reads the contens of a text file."""
    if PYTHON3:
        return open(filename, 'r', encoding='utf-8').read()
    else:
        return open(filename, 'r').read().decode('utf-8')


def write_text_file(filename, content):
    """Writes 'content' into a text file."""
    if PYTHON3:
        open(filename, 'w', encoding='utf-8').write(content)
    else:
        open(filename, 'w').write(content.encode('utf-8'))


def find_files(startpath, extensions):
    for (dirpath, _, filenames) in os.walk(startpath):
        for filename in filenames:
            if os.path.splitext(filename)[-1].lower() in extensions:
                yield os.path.join(dirpath, filename)


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
