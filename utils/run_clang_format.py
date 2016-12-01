#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""This script runs clang-format over src and all its subdirectories."""

import argparse
import os
import sys
from subprocess import call


def parse_args():
    p = argparse.ArgumentParser(description='Run clang-format over the code base.'
                                )
    return p.parse_args()


def find_cplusplus_files():
    for (dirpath, _, filenames) in os.walk('./src'):
        for filename in filenames:
            if os.path.splitext(filename)[-1].lower() == '.cc' or os.path.splitext(filename)[-1].lower() == '.h':
                yield os.path.join(dirpath, filename)


def main():
    parse_args()

    if not os.path.isdir('src') or not os.path.isdir('utils'):
        print('CWD is not the root of the repository.')
        return 1

    sys.stdout.write('Running clang-format ')
    for filename in find_cplusplus_files():
        # print "Formatting %r" % filename
        sys.stdout.write('.')
        sys.stdout.flush()
        call(['clang-format-3.8', '-i', filename])
    print '\nFormatting finished.'
    return 0

if __name__ == '__main__':
    sys.exit(main())
