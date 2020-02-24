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
CONSTEXPR_REGEX = re.compile(r'constexpr.*\s+(\w+)\s+=')
EXTERN_REGEX = re.compile(r'extern\s+\S+\s+(\S+);')
EXTERN_ZIP_REGEX = re.compile(r'extern\s+\S+\s+\S+\s+(\S+)\s+')
TYPEDEF_REGEX = re.compile(r'typedef\s+\S+\s+(\S+);')

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
            match = EXTERN_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = EXTERN_ZIP_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
            match = TYPEDEF_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])
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
    """Checks if the includes in this file are needed and prints superfluous
    includes."""

    sys.stdout.write('.')
    sys.stdout.flush()

    hits = []

    with open(file_to_check, 'r', encoding='utf-8') as f:
        file_contents = f.read()

        header_files = find_includes(file_to_check)
        for header_file in header_files:
            # NOCOM implement support for these special files
            if header_file.startswith('base/'):
                continue
            header_classes = find_classes(header_file)
            is_useful = False
            for header_class in header_classes:
                if header_class in file_contents:
                    is_useful = True
                    break
            if not is_useful:
                hits.append('\t' + header_file)
    if hits:
        print('\nSuperfluous includes in ' + file_to_check)
        for hit in hits:
            print(hit)


def main():
    """Script to find unused includes.

    Call from src directory.
    """

    print('Tool to check for superfluous includes in header files. Cal from src diectory.')

    for (dirpath, _, filenames) in os.walk('.'):
        for filename in filenames:
            if filename.endswith('.h'):
                check_file(os.path.join(dirpath, filename))
    print('\nDone.');

    return 0


if __name__ == '__main__':
    sys.exit(main())
