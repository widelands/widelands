#!/usr/bin/python -tt

"""
This checks that the include guard is complete and the same style.
"""

import os

def find_relpath_to_base_dir(abspath):
    fn = os.path.abspath(abspath)
    while fn != "/" and os.path.basename(fn) != "src":
        fn = os.path.abspath(os.path.join(fn, os.path.pardir))
    return os.path.relpath(abspath, fn)

def evaluate_matches(lines, fn):
    if not fn.endswith('.h'):
        return []

    rel = find_relpath_to_base_dir(os.path.join(os.path.dirname(fn), fn))
    parts = ("WL",) + tuple(filter(len, rel.split(os.path.sep)))
    once = "_".join(a.replace(".", "_") for a in parts).upper()

    for lineno,line in enumerate(lines, 1):
        if not line.startswith("#ifndef"):
            continue
        if line.strip() != "#ifndef %s" % once:
            return [(fn, lineno, "Use %s as include guard." % once)]
        return []
    return [(fn, 1, "No include guard found. Use %s." % once)]

# File this is called on is always called testdir/test.h
forbidden = [
]

allowed = [
]
