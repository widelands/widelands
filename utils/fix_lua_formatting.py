#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""Enforces consistent formatting of Lua files.

The code base had inconsistent usage of tabs/spaces for indenting in Lua
files. Spaces were more prominent - and I prefer them over tabs. So I wrote
this small script to fix leading tabs in Lua files to spaces.

It also saves files in unix file endings ("\r\n") and strips empty lines at the
end of files and whitespace characters at the end of lines.

"""

import os
import re
import sys
from subprocess import check_call, CalledProcessError
from file_utils import read_text_file, write_text_file, find_files

LEADING_TABS = re.compile(r'^\s*\t+\s*')
SPACES_PER_TAB = 3


def format_lua_files(directory):
    sys.stdout.write(
        '\nFixing Lua tabs in directory: ' + directory + ' ')
    for filename in find_files(directory, ['.lua']):
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
        check_call(['git', 'add', '--renormalize', filename])
    print(' done.')


def main():
    if not os.path.isdir('src') or not os.path.isdir('utils'):
        print('CWD is not the root of the repository.')
        return 1

    directories = set()
    if len(sys.argv) > 1:
        for dir in sys.argv[1:] :
            directories.add(dir)
    else:
        directories = {'./data', './test'}
    for directory in directories:
        format_lua_files(directory)
    return 0


if __name__ == '__main__':
    sys.exit(main())
