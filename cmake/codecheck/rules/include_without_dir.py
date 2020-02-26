#!/usr/bin/python -tt

"""
This checks for

#include "file.h"

and reports an error. The include should always be from the base dir upwards:

#include "somedir/file.h"

"""

import os

def find_relpath_to_base_dir(abspath):
    fn = os.path.abspath(abspath)
    while fn != "/" and os.path.basename(fn) != "src":
        fn = os.path.abspath(os.path.join(fn, os.path.pardir))
    return os.path.relpath(abspath, fn)

def evaluate_matches(lines, fn):
    errors = []

    splitted_fn = fn.rsplit('/')
    if len(splitted_fn) == 1:
        return []

    for lineno,line in enumerate(lines):
        if not line.startswith("#include"):
            continue

        inc = line.split()[-1]
        if inc[0] != '"':
            continue
        inc = inc.strip('"')

        inc_to_base = find_relpath_to_base_dir(os.path.join(os.path.dirname(fn), inc))

        if (os.path.exists(os.path.join(os.path.dirname(fn), inc)) and not
            (inc_to_base == inc)):
            errors.append( (fn, lineno+1, "Local include without subdir. Change to \"dir/file.h\""))

    return errors

# File this is called on is always called testdir/test.h
forbidden = [
]

allowed = [
]
