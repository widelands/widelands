#!/usr/bin/env python3
# -*- coding: utf-8 -*-


"""Enforce unified formatting of Lua, C++ and python files.

This script executes fix_lua_formatting.py over the data and test directories,
clang-format over the src directory, and pyformat over the utils directory.

"""

import argparse
import os
import re
import sys
from subprocess import check_call, CalledProcessError
from file_utils import read_text_file, write_text_file, find_files

import autoflake
import autopep8
import unify

from fix_lua_formatting import format_lua_files

LEADING_TABS = re.compile(r'^\s*\t+\s*')
SPACES_PER_TAB = 3


def parse_args():
    p = argparse.ArgumentParser(
        description='Fix common whitespace errors in Lua files, run clang-format'
        ' over the code base and pyformat over the utils directory.'
        ' Recurses over all relevant files.')

    p.add_argument('-c', '--c++', action='store_true',
                   help='Format C++ files only')
    p.add_argument('-l', '--lua', action='store_true',
                   help='Format Lua files only')
    p.add_argument('-p', '--python', action='store_true',
                   help='Format Python files only')
    p.add_argument('-d', '--dir', action='store',
                   help='Format the given directory and its subdirectories only')

    return vars(p.parse_args())

def pyformatters():
    """Return list of code formatters."""
    yield lambda code: autoflake.fix_code(
        code,
        remove_all_unused_imports=True,
        remove_unused_variables=True)

    yield autopep8.fix_code
    yield unify.format_code

def pyformat_file(filename):
    """Run format_code() on a file.

    Return True if the new formatting differs from the original.
    Based on pyformat https://github.com/myint/pyformat @ 7293e4d

    """
    encoding = autopep8.detect_encoding(filename)
    with autopep8.open_with_encoding(filename,
                                     encoding=encoding) as input_file:
        source = input_file.read()

    if not source:
        return False

    for fix in pyformatters():
        formatted_source = fix(source)

    if source != formatted_source:
        with autopep8.open_with_encoding(filename, mode='w',
                                         encoding=encoding) as output_file:
            output_file.write(formatted_source)
        return True

    return False


def main():
    args = parse_args()
    format_cplusplus = args['c++'] or not (args['lua'] or args['python'])
    format_lua = args['lua'] or not (args['c++'] or args['python'])
    format_python = args['python'] or not (args['c++'] or args['lua'])

    if not os.path.isdir('src') or not os.path.isdir('utils'):
        print('CWD is not the root of the repository.')
        return 1

    if format_cplusplus:
        directory = args['dir']
        if not directory:
            directory = './src'
        sys.stdout.write('\nFormatting C++ in directory: ' + directory + ' ')
        for filename in find_files(directory, ['.cc', '.h']):
            if 'third_party' in filename:
                continue
            sys.stdout.write('.')
            sys.stdout.flush()
            check_call(['clang-format', '-i', filename])
            check_call(['git', 'add', '--renormalize', filename])
        print(' done.')

    if format_lua:
        directories = set()
        if args['dir']:
            directories.add(args['dir'])
        else:
            directories = {'./data', './test'}
        for directory in directories:
            format_lua_files(directory)

    if format_python:
        directories = set()
        if args['dir']:
            directories.add(args['dir'])
        else:
            directories = {'./utils', './cmake/codecheck'}
        for directory in directories:
            sys.stdout.write(
                '\nFormatting Python scripts in directory: ' + directory + ' ')
            python_files = find_files(directory, ['.py'])
            try:
                check_call(['docformatter', '-i', *python_files])
            except CalledProcessError as e:
                # Return code 3 means there are changes. That's OK for us, next
                # step will pick them up
                if e.returncode != 3 :
                    raise
            for filename in find_files(directory, ['.py']):
                sys.stdout.write('.')
                sys.stdout.flush()
                pyformat_file(filename)
                check_call(['git', 'add', '--renormalize', filename])
            print(' done.')

    print('Formatting finished.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
