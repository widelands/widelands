#!/usr/bin/env python
# encoding: utf-8

"""Tool to rename all files for a widelands animation in bulk."""


from subprocess import call
import os.path
import subprocess
import sys


def main():
    """Finds all files with the given relative source path with file prefix and
    git moves all files that match this prefix to the given destination path
    with file prefix."""

    print('Tool to rename all files for a widelands animation in bulk')

    if len(sys.argv) != 3:
        print('Usage:   rename_animation.py <old_name> <new_name>')
        print('Example: rename_animation.py <mybuilding/foo> <mybuilding/bar>')
        return 1

    source_prefix = sys.argv[1]
    destination_prefix = sys.argv[2]
    print('Renaming "%s"\nto       "%s"' % (source_prefix, destination_prefix))

    source_absdir = os.path.dirname(os.path.join(os.getcwd(), source_prefix))
    source_path = os.path.dirname(source_prefix)

    for source_file in os.listdir(source_absdir):
        source_file = os.path.join(source_path, source_file)
        if source_file.startswith(source_prefix):
            suffix = source_file[len(source_prefix):]
            destination_file = destination_prefix + suffix
            call(['git', 'mv', source_file, destination_file])
    return 0


if __name__ == '__main__':
    sys.exit(main())
