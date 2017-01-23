#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import re
import sys

PYTHON3 = sys.version_info >= (3, 0)

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
    """Updates the copyright year in all source files to the given year."""
    if len(sys.argv) != 2:
        print('Usage: update_copyright.py <year>')
        return 1

    try:
        year = sys.argv[1]
        sys.stdout.write('Updating copyright year to: ' + year + ' ')
        src_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "../src"))
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
        print(' done.')

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())
