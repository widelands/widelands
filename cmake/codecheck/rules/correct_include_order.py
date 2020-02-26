#!/usr/bin/python -tt

import re
import os


class EvalMatches(object):
    _include_regexp = re.compile(r'^#include *([<"])([^">]+)[>"]')
    _ifdef_regexp = re.compile(r'^#ifn?def.*')
    _endif_regexp = re.compile(r'^#endif.*')

    def __call__(self, lines, fn):
        blocks = []

        cur_block = []
        errors = []
        seen_includes = set()
        inside_ifdefs = 0
        for lineno, line in enumerate(lines, 1):
            if self._ifdef_regexp.match(line):
                inside_ifdefs += 1
            elif self._endif_regexp.match(line):
                inside_ifdefs -= 1
            if inside_ifdefs:
                continue
            m = self._include_regexp.match(line)
            if m:
                cur_block.append((lineno, m.group(1), m.group(2)))
                if m.group(2) in seen_includes:
                    errors.append(
                        (fn, lineno, 'This header has been included more than once.'))
                    return errors
                seen_includes.add(m.group(2))
            elif line.startswith('#'):
                continue
            elif cur_block:
                blocks.append(cur_block)
                cur_block = []
        if cur_block:
            blocks.append(cur_block)

        if not blocks:
            return errors

        for block in blocks:
            includes = [entry[2] for entry in block]
            delims = set(entry[1] for entry in block)
            if len(delims) != 1:
                errors.append(
                    (fn, block[0][0], """Use either '"' or '<' for all includes in a block."""))
            if sorted(includes) != includes:
                errors.append(
                    (fn, block[0][0], 'Include block is not sorted alphabetically.'))
                return errors

        if '.cc' in fn:
            base_file = os.path.basename(fn)[:-3]
            if len(blocks[0]) != 1 or not blocks[0][0][2][:-2].endswith(base_file):
                if os.path.exists(os.path.abspath(fn)[:-3] + '.h'):
                    errors.append(
                        (fn, blocks[0][0][0], 'In a .cc file, include the corresponding header first in a line of its own it exists.'))
                return errors
            else:
                blocks.pop(0)

        if blocks:
            # cxx includes.
            if blocks[0][0][1] == '<' and '.h' not in blocks[0][0][2]:
                for lineno, delimiter, header in blocks[0]:
                    if '.h' in header or delimiter != '<':
                        errors.append(
                            (fn, lineno, 'This include block must only contain C++ headers and must come first.'))
                        return errors
                blocks.pop(0)

        if blocks and blocks[0][0][1] == '<':  # library includes.
            for lineno, delimiter, header in blocks[0]:
                if '.h' not in header or delimiter != '<':
                    errors.append(
                        (fn, lineno, 'This include block must only contain library headers (e.g. SDL2).'))
                    return errors
            blocks.pop(0)

        if blocks:  # Widelands includes.
            for lineno, delimiter, header in blocks[0]:
                if not header.endswith('.h') or delimiter != '"':
                    errors.append(
                        (fn, lineno, 'This include block must contain all Widelands includes and must come last.'))
                    return errors
            blocks.pop(0)

        if blocks:
            errors.append(
                (fn, blocks[0][0][0], """Unexpected include block after Widelands includes. Ordering of blocks should be <file.h>, c++, libraries then Widelands includes."""))
        return errors


evaluate_matches = EvalMatches()


#################
# ALLOWED TESTS #
#################
allowed = [
    """#include <memory>

#include <SDL2.h>
""",

    """
#include <config.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
"""

]


###################
# FORBIDDEN TESTS #
###################
forbidden = [
    """
#include <SDL2.h>

#include <memory>
""",

    """
#include <hallo>
#include <memory>
#include <memory>
""",

    """
#include <memory>
#include <SDL2.h>
"""

]
