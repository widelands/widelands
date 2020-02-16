#!/usr/bin/env python
# encoding: utf-8

"""Tool to rename files in all subdirectories in bulk."""

from subprocess import call
import os.path
import subprocess
import sys


def main():
    """Finds all files with the given old_name in basedir and git moves all
    files that match the old_name to new_name."""

    print('Tool to rename all files for a widelands animation in bulk')

    if len(sys.argv) != 4:
        print('Usage:   bulk_rename.py <basedir> <old_name> <new_name>')
        print('Example: bulk_rename.py data/tribes names.lua register.lua')
        return 1

    basedir = sys.argv[1]
    source_name = sys.argv[2]
    destination_name = sys.argv[3]
    print('Renaming "%s"\nto       "%s\nin       "%s"' %
          (source_name, destination_name, basedir))

    for (dirpath, _, filenames) in os.walk(basedir):
        for filename in filenames:
            if filename == source_name:
                source_path = os.path.join(dirpath, source_name)
                destination_path = os.path.join(dirpath, destination_name)
                call(['git', 'mv', source_path, destination_path])
    return 0


if __name__ == '__main__':
    sys.exit(main())
