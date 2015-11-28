#!/usr/bin/python -tt

import re

__RE = re.compile(r"""virtual[^;]*override""", re.MULTILINE)
def evaluate_matches(lines, fn):
    errors = []

    for lineno, line in enumerate(lines, 1):
        if __RE.search(line) is not None:
            errors.append( (fn, lineno, "virtual is redundant when override is present. Remove virtual."))
    return errors

# File this is called on is always called testdir/test.h
forbidden = [
    "   virtual foo() override;",
    "   virtual foo() override {",
]

allowed = [
    "virtual void foo() = 0;",
    "void foo() override;",
]
