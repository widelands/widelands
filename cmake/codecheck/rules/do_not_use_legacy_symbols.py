#!/usr/bin/python3

error_msg = 'Checks for outdated symbols, e.g. we should use std::function rather than boost::function'


def evaluate_matches(lines, fn):
    """Detects a list of unwanted string literals and suggests replacements for
    them."""
    result = []

    REPLACEMENTS = {
        ('#include <boost/function.hpp>', '#include <functional>'),
        ('boost::function', 'std::function'),
        ('#include <boost/shared_ptr.hpp>', '#include <memory>'),
        ('boost::shared_ptr', 'std::shared_ptr'),
        ('#include <boost/unordered_map.hpp>', '#include <unordered_map>'),
        ('boost::unordered_map', 'std::unordered_map')}

    for lineno, line in enumerate(lines, 1):
        for original, replacement in REPLACEMENTS:
            if original in line:
                result.append((fn, lineno, 'Do not use ' +
                               original + '. Use ' + replacement + ' instead.'))

    return result


forbidden = [
    '#include <boost/function.hpp>',
    'boost::function',
    '#include <boost/shared_ptr.hpp>',
    'boost::shared_ptr',
    '#include <boost/unordered_map.hpp>',
    'boost::unordered_map',
]

allowed = [
    '#include <functional>',
    'std::function',
    '#include <memory>',
    'std::shared_ptr',
    '#include <unordered_map>',
    'std::unordered_map',
]
