#!/usr/bin/python

strip_comments_and_strings = False


def evaluate_matches(lines, fn):
    errors = []
    allow_boost_format = 0

    for lineno, line in enumerate(lines):
        if line.count('@CodeCheck allow boost::format'):
            allow_boost_format += 1
        elif line.count('boost::format'):
            if allow_boost_format > 0:
                allow_boost_format -= 1
            else:
                errors.append(
                    (fn, lineno+1, "Do not call 'boost::format', use bformat from base/log.h instead."))

    return errors
# /end evaluate_matches


forbidden = [
    '(boost::format("Foo %s") % "bar").str();'
]

allowed = [
    'bformat("Foo %s", "bar");',
    """
    // @CodeCheck allow boost::format
    (boost::format("Foo %s") % "bar").str();
    """
]
