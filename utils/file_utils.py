#!/usr/bin/env python
# -*- coding: utf-8 -*-


"""Some common file util functions."""

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
