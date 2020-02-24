#!/usr/bin/env python3
# encoding: utf-8

"""Script to find unused includes.

Call from src directory.
"""

import os.path
import re
import sys

# NOCOM find forward declarations too
HEADER_REGEX = re.compile(r'^#include\s+\"(\S+)\"$')

USING_REGEX = re.compile(r'.*using.*\s+(\S+)\s+=')
CLASS_REGEX = re.compile(r'.*(class|enum|struct)\s+(\S+)\s+.*{')
DEFINE_REGEX = re.compile(r'\#define\s+(\w+)')
CONSTEXPR_REGEX = re.compile(r'.*constexpr.*\s+(\w+)\s+=')
STRING_CONSTANT_REGEX = re.compile(
    r'const\sstd::string\s+(k\w+)\s+=\s+\"\S+\";')
STRING_CONSTANT_OLD_REGEX = re.compile(
    r'.*const\sstd::string\s+(\w+_\w+)\s+=\s+\"\S+\";')
EXTERN_REGEX = re.compile(r'extern\s+\S+\s+(\S+);')
# Extern macros used in third_party/minizip
EXTERN_ZIP_REGEX = re.compile(r'extern\s+\S+\s+\S+\s+(\S+)\s+')
TYPEDEF_REGEX = re.compile(r'typedef\s+\S+\s+(\S+);')
# Special regex for #include "graphic/text/rt_errors.h"
RT_ERRORS_REGEX = re.compile(r'DEF_ERR\((\S+)\)')
INLINE_FUNCTION_REGEX = re.compile(r'inline\s+\S+\s+(\S+\()')
FORWARD_DECLARATION_REGEX = re.compile(r'(class|struct)\s+(\S+);')

# Files that are hard to capture by regex
FILE_EXCLUDES = {'graphic/gl/system_headers.h', 'scripting/lua.h',
                 'third_party/eris/lua.hpp', 'scripting/report_error.h', 'editor/tools/set_resources_tool.h'}


def find_classes(file_to_check):
    """Returns a set of classes defined by this file."""
    classes = set()
    with open(file_to_check, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            line = line.strip()
            match = CLASS_REGEX.match(line)
            if match and len(match.groups()) == 2:
                classes.add(match.groups()[1])
            match = USING_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = DEFINE_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = CONSTEXPR_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = STRING_CONSTANT_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = STRING_CONSTANT_OLD_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = EXTERN_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = EXTERN_ZIP_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = TYPEDEF_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = RT_ERRORS_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = INLINE_FUNCTION_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = FORWARD_DECLARATION_REGEX.match(line)
            if match and len(match.groups()) == 2:
                classes.add(match.groups()[1])

    return classes


def find_includes(file_to_check):
    """Returns a set of includes."""
    files = set()
    with open(file_to_check, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            line = line.strip()
            match = HEADER_REGEX.match(line)
            if match and len(match.groups()) == 1:
                include_file = match.groups()[0]
                if os.path.isfile(include_file):
                    files.add(include_file)

    return files


def check_file(file_to_check):
    """Checks if the includes in this file are needed, and returns the ones
    that aren't."""

    hits = []

    with open(file_to_check, 'r', encoding='utf-8') as f:
        file_contents = f.read()

        header_files = find_includes(file_to_check)
        for header_file in header_files:
            # NOCOM implement support for these special files
            if header_file.startswith('base/'):
                continue
            elif header_file in FILE_EXCLUDES:
                continue
            header_classes = find_classes(header_file)
            is_useful = False
            for header_class in header_classes:
                if header_class in file_contents:
                    is_useful = True
                    break
            if not is_useful:
                hits.append(header_file)

    return hits


# For .cc files
FUNCTION_REGEX1 = re.compile(r'.*\S+\s+([a-z_0-9]+)\(.*\).*')
FUNCTION_REGEX2 = re.compile(r'.*\S+\s+([a-z_0-9]+)\(.*,')


def find_functions(file_to_check):
    """Returns a set of functions defined by this file."""
    functions = set()

    with open(file_to_check, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            line = line.strip()

            match = FUNCTION_REGEX1.match(line)
            if match and len(match.groups()) > 0:
                functions.add(match.groups()[0])
            match = FUNCTION_REGEX2.match(line)
            if match and len(match.groups()) > 0:
                functions.add(match.groups()[0])

    return functions


def check_cc_file(file_to_check, header_files):
    """Checks if all header files have a function in common with this file, and
    returns the ones that don't."""

    hits = []

    with open(file_to_check, 'r', encoding='utf-8') as f:
        file_contents = f.read()

        for header_file in header_files:
            # NOCOM implement support for these special files
            if header_file.startswith('base/'):
                continue
            elif header_file in FILE_EXCLUDES:
                continue
            header_functions = find_functions(header_file)
            is_useful = False
            for header_function in header_functions:
                if header_function in file_contents:
                    is_useful = True
                    break
            if not is_useful:
                hits.append(header_file)

    return hits


def main():
    """Script to find unused includes and print them to console.

    Call from src directory.
    """

    error_count = 0

    print('Tool to check for superfluous includes in header files. Call from src diectory.')
    print('Checking...')

    for (dirpath, _, filenames) in os.walk('.'):
        for filename in filenames:
            full_path = os.path.join(dirpath, filename)
            hits = []

            if filename.endswith('.h'):
                hits = check_file(full_path)

            elif filename.endswith('.cc'):
                hits = check_cc_file(full_path, check_file(full_path))

            if hits:
                print('\nSuperfluous includes in ' + full_path)
                for hit in hits:
                    print('\t' + hit)
                error_count = error_count + len(hits)

    if error_count > 0:
        print('\nFound %d errors.' % error_count)
        return 1
    else:
        print('\nDone.')

    return 0


if __name__ == '__main__':
    sys.exit(main())
