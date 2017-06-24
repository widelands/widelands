#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Some common file util functions."""

import csv
import os
import sys

PYTHON3 = sys.version_info >= (3, 0)


def read_text_file(filename):
    """Reads the contens of a text file."""
    if PYTHON3:
        with open(filename, 'r', encoding='utf-8') as f:
            return f.read()
    else:
        with open(filename, 'r') as f:
            return f.read().decode('utf-8')


def write_text_file(filename, content):
    """Writes 'content' into a text file."""
    if PYTHON3:
        with open(filename, 'w', encoding='utf-8', newline='\n') as f:
            f.write(content)
    else:
        with open(filename, 'w') as f:
            f.write(content.encode('utf-8'))


def find_files(startpath, extensions):
    for (dirpath, _, filenames) in os.walk(startpath):
        for filename in filenames:
            if os.path.splitext(filename)[-1].lower() in extensions:
                yield os.path.join(dirpath, filename)


def read_csv_file(filepath):
    """Parses a CSV file into a 2-dimensional array."""
    result = []
    with open(filepath) as csvfile:
        csvreader = csv.reader(csvfile, delimiter=',', quotechar='"')
        for row in csvreader:
            result.append(row)
    return result

def make_path(base_path, subdir):
    """Creates the correct form of the path and makes sure that it exists."""
    result = os.path.abspath(os.path.join(base_path, subdir))
    if not os.path.exists(result):
        os.makedirs(result)
    return result


def delete_path(path):
    """Deletes the directory specified by 'path' and all its subdirectories and
    file contents."""
    if os.path.exists(path) and not os.path.isfile(path):
        files = sorted(os.listdir(path), key=str.lower)
        for deletefile in files:
            deleteme = os.path.abspath(os.path.join(path, deletefile))
            if os.path.isfile(deleteme):
                try:
                    os.remove(deleteme)
                except Exception:
                    print('Failed to delete file ' + deleteme)
            else:
                delete_path(deleteme)
        try:
            os.rmdir(path)
        except Exception:
            print('Failed to delete path ' + deleteme)
