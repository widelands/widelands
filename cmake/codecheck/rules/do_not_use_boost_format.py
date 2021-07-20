#!/usr/bin/python

strip_comments_and_strings = True


def evaluate_matches(lines, fn):
    errors = []

    if not fn.endswith("src/base/log.h"):
        for lineno, line in enumerate(lines):
            if line.count('boost::format'):
                errors.append(
                    (fn, lineno+1, "Do not call 'boost::format', use bformat from base/log.h instead."))

    return errors
# /end evaluate_matches


forbidden = [
    '(boost::format("Foo %s") % "bar").str();'
]

allowed = [
    'bformat("Foo %s", "bar");'
]
