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

USING_OR_CONSTEXPR_REGEX = re.compile(r'.*(using|constexpr).*\s+(\w+)\s+=')
CLASS_REGEX = re.compile(r'.*(class|enum|struct)\s+(\S+)\s+.*{')
DEFINE_REGEX = re.compile(r'\#define\s+(\w+)')
STRING_CONSTANT_REGEX = re.compile(
    r'.*const\sstd::string\s+(k\w+|\w+_\w+)\s+=\s+\"\S+\";')
EXTERN_OR_TYPEDEF_REGEX = re.compile(r'(extern|typedef)\s+\S+\s+(\S+);')

# Extern macros used in third_party/minizip
EXTERN_ZIP_REGEX = re.compile(r'extern\s+\S+\s+\S+\s+(\S+)\s+')

# Special regex for #include "graphic/text/rt_errors.h" and Map_Object_Packet
MACRO_CLASS_DEFINITION_REGEX = re.compile(r'^[A-Z_0-9]+\((\w+)\)$')

INLINE_FUNCTION_REGEX = re.compile(r'inline\s+\S+\s+(\S+\()')
FORWARD_DECLARATION_REGEX = re.compile(r'(class|struct)\s+(\S+);')

# For .cc files
FUNCTION_REGEX = re.compile(r'(^|.*\s+)([a-zA-Z_0-9]+)\(.*(\)|,).*')

# Special regex
HEADER_LOG_REGEX = re.compile(r'(void|bool)\s+(\w+)\(.*\);')


# Files that are hard to capture by regex
FILE_EXCLUDES = {'graphic/gl/system_headers.h', 'scripting/lua.h',
                 'third_party/eris/lua.hpp', 'scripting/eris.h'}

# Headers that need to be detected by functions
DIFFICULT_FILES = {'graphic/build_texture_atlas.h',
                   'scripting/report_error.h', 'editor/tools/set_resources_tool.h'}


def find_classes(file_to_check, include_functions, special_regex, special_regex_group):
    """Returns a set of classes defined by this file."""
    classes = set()
    with open(file_to_check, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            line = line.strip()

            match = CLASS_REGEX.match(line)
            if match and len(match.groups()) == 2:
                classes.add(match.groups()[1])

            match = USING_OR_CONSTEXPR_REGEX.match(line)
            if match and len(match.groups()) > 1:
                classes.add(match.groups()[1])

            match = DEFINE_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])

            match = STRING_CONSTANT_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])

            match = EXTERN_OR_TYPEDEF_REGEX.match(line)
            if match and len(match.groups()) > 1:
                classes.add(match.groups()[1])

            match = EXTERN_ZIP_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])

            match = MACRO_CLASS_DEFINITION_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])

            match = INLINE_FUNCTION_REGEX.match(line)
            if match and len(match.groups()) == 1:
                classes.add(match.groups()[0])

            match = FORWARD_DECLARATION_REGEX.match(line)
            if match and len(match.groups()) == 2:
                classes.add(match.groups()[1])

            if include_functions:
                match = FUNCTION_REGEX.match(line)
                if match and len(match.groups()) > 1:
                    classes.add(match.groups()[1])

            if HEADER_LOG_REGEX:
                match = HEADER_LOG_REGEX.match(line)
                if match and len(match.groups()) > special_regex_group:
                    classes.add(match.groups()[special_regex_group])

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


def check_file(file_to_check, include_functions):
    """Checks if the includes in this file are needed, and returns the ones
    that aren't."""

    hits = []

    with open(file_to_check, 'r', encoding='utf-8') as f:
        file_contents = f.read()

        header_files = find_includes(file_to_check)
        for header_file in header_files:
            header_classes = set()
            is_useful = False

            if header_file in FILE_EXCLUDES:
                continue
            elif header_file in DIFFICULT_FILES:
                header_classes = find_classes(header_file, True, None, 0)
            elif header_file == 'base/log.h':
                header_classes = find_classes(header_file, False, HEADER_LOG_REGEX, 1)
            else:
                header_classes = find_classes(header_file, include_functions, None, 0)

            for header_class in header_classes:
                if header_class in file_contents:
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
                hits = check_file(full_path, False)

            elif filename.endswith('.cc'):
                hits = check_file(full_path, True)

            if hits:
                print('\nSuperfluous includes in ' + full_path)
                for hit in hits:
                    print('\t' + hit)
                error_count = error_count + len(hits)

    if error_count > 0:
        print('\nFound %d error(s).' % error_count)
        return 1
    else:
        print('\nDone, files are clean.')

    return 0


if __name__ == '__main__':
    sys.exit(main())
