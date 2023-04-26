#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# Tests if the .json files in the directories on the bottom are valid JSON
# files


def validate_files_in_path(source_path):
    if (not os.path.isdir(source_path)):
        print('Error: Path ' + source_path + ' not found.')
        sys.exit(1)

    source_files = sorted(os.listdir(source_path), key=str.lower)

    print('\nReading JSON files in: ' + source_path + '\n')
    failed = 0
    for source_filename in source_files:
        file_path = source_path + '/' + source_filename
        if source_filename.endswith('.json'):
            source_file = open(file_path, 'r')
            try:
                dataset = json.load(source_file)
                print(source_filename)
            except ValueError as err:
                failed = failed + 1
                print('\n   Error reading ' + source_filename + ':')
                print('      ' + str(err))

    if failed == 0:
        print('\nAll JSON files are OK.')
    else:
        if failed == 1:
            print('\n' + str(failed) + ' file is not valid JSON!')
        else:
            print('\n' + str(failed) + ' files are not valid JSON!')
    return failed < 1


success = False

if (len(sys.argv) == 2):
    base_path = os.path.normpath(sys.argv[1])
    if (not os.path.exists(base_path) or os.path.isfile(base_path)):
        base_path = os.path.abspath(os.path.join(
            os.path.dirname(__file__), os.path.pardir))

    if (os.path.exists(base_path) and not os.path.isfile(base_path)):
        success = validate_files_in_path(os.path.normpath(base_path))

if success:
    sys.exit(0)
else:
    sys.exit(1)
