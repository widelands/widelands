#!/usr/bin/env python
# encoding: utf-8

import os.path
import re
import sys
import traceback
from file_utils import read_text_file, write_text_file, find_files


def update_file(filename, regex, replace):
    sys.stdout.write('.')
    sys.stdout.flush()
    lines = read_text_file(filename).strip().split('\n')
    new_lines = []
    for line in lines:
        match = regex.match(line)
        if match:
            line = replace(match)
        new_lines.append(line.rstrip() + '\n')
    write_text_file(filename, ''.join(new_lines))


def main():
    """Updates the copyright year in all source files to the given year."""
    if len(sys.argv) != 2:
        print('Usage: update_copyright.py <year>')
        return 1

    try:
        year = sys.argv[1]
        sys.stdout.write('Updating copyright year to: ' + year + ' ')
        base_path = os.path.abspath(
            os.path.join(os.path.dirname(__file__), '..'))
        src_path = os.path.join(base_path, 'src')
        # Fix copyright headers in C++ files
        regex_header = re.compile(
            '(.*Copyright \(C\) \d\d\d\d)(.*)( by the Widelands Development Team.*)')

        def repl_header(match):
            if match.group(1).endswith(year):
                return match.group(1) + match.group(3)
            else:
                return match.group(1) + '-' + year + match.group(3)

        for filename in find_files(src_path, ['.h', '.cc']):
            update_file(filename, regex_header, repl_header)

        # Now update the Buildinfo
        filename = os.path.join(src_path, 'build_info.h')
        regex = re.compile(
            '(.*constexpr uint16_t kWidelandsCopyrightEnd = )(\d\d\d\d)(;)')

        def replace(match):
            return match.group(1) + year + match.group(3)

        update_file(filename, regex, replace)

        # Now update special files
        filename = os.path.join(base_path, 'data/txts/LICENSE.lua')
        regex = re.compile(
            '(.*\"Copyright 2002 - %1% by the Widelands Development Team\.\"\):bformat\()(\d\d\d\d)(\).*)')
        update_file(filename, regex, replace)

        filename = os.path.join(
            base_path, 'utils/win32/innosetup/Widelands.iss')
        regex = re.compile(
            '(#define Copyright \"Widelands Development Team 2001\-)(\d\d\d\d)(\")')
        update_file(filename, regex, replace)

        filename = os.path.join(base_path, 'utils/win32/widelands.rc.cmake')
        update_file(filename, regex_header, repl_header)

        filename = os.path.join(base_path, 'debian/copyright')
        regex = re.compile(
            '(Copyright: 2002\-)(\d\d\d\d)( by the Widelands Development Team)')
        update_file(filename, regex, replace)

        print(' done.')

        print('You might need to update some copyright years in %s manually.' % filename)

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
