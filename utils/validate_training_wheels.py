#!/usr/bin/env python3
# encoding: utf-8

"""This script checks whether all training wheels have defined their locks
properly."""

import os
import sys


def main():
    """Check whether all training wheels have defined their locks properly."""

    # Sanity check
    if not os.path.isdir('utils') or not os.path.isdir('data'):
        print('CWD is not the root of the repository.')
        return 1

    return_code = 0
    training_wheels_dir = os.path.join(
        os.getcwd(), 'data/scripting/training_wheels')
    for filename in os.listdir(training_wheels_dir):

        if filename == 'init.lua':
            continue
        file_path = os.path.join(training_wheels_dir, filename)
        if not os.path.isfile(file_path):
            continue
        print('Checking {}'.format(filename))
        lock_acquired = False
        marked_solved = False
        with open(file_path, 'r', encoding='utf-8') as filecontents:
            for line in filecontents.readlines():
                if 'wait_for_lock(' in line:
                    lock_acquired = True
                elif lock_acquired and 'mark_training_wheel_as_solved(' in line:
                    marked_solved = True
        if not lock_acquired:
            print("- ERROR: File did not call 'wait_for_lock'")
            return_code = 1
        if not marked_solved:
            print(
                "- ERROR: File did not call 'mark_training_wheel_as_solved' after 'wait_for_lock'")
            return_code = 1

    print('Done.')
    return return_code


if __name__ == '__main__':
    sys.exit(main())
