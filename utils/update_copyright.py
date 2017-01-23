#!/usr/bin/env python
# encoding: utf-8

import os.path
import re
import sys

file_utils_script = os.path.abspath(os.path.join(os.path.dirname(__file__), 'file_utils.py'))
exec(compile(source=open(file_utils_script).read(), filename=file_utils_script, mode='exec'))

def main():
    """Updates the copyright year in all source files to the given year."""
    if len(sys.argv) != 2:
        print('Usage: update_copyright.py <year>')
        return 1

    try:
        year = sys.argv[1]
        sys.stdout.write('Updating copyright year to: ' + year + ' ')
        src_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../src"))
        # Fix copyright headers in C++ files
        for filename in find_files(src_path, ['.h', '.cc']):
            sys.stdout.write('.')
            sys.stdout.flush()
            lines = read_text_file(filename).strip().split('\n')
            new_lines = []
            regex = re.compile('(.*Copyright \(C\) \d\d\d\d)(.*)( by the Widelands Development Team.*)')
            for line in lines:
                match = regex.match(line)
                if match:
                    line = match.group(1) + "-" + year + match.group(3)
                new_lines.append(line.rstrip() + '\n')
            write_text_file(filename, ''.join(new_lines))

        # Now update the Buildinfo
        filename = os.path.join(src_path, "build_info.h")
        #print(filename)
        lines = read_text_file(filename).strip().split('\n')
        new_lines = []
        regex = re.compile('(.*constexpr uint16_t kWidelandsCopyrightEnd = )(\d\d\d\d)(;)')
        for line in lines:
            match = regex.match(line)
            if match:
                line = match.group(1) + year + match.group(3)
            new_lines.append(line.rstrip() + '\n')
        write_text_file(filename, ''.join(new_lines))
        print(' done.')

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())
