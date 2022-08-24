#!/usr/bin/env python
# encoding: utf-8

"""Simple script to check that the build dependencies in the CMakeLists.txt
files are correct.

It relies on the established build rule formatting conventions in the
code base.
"""

from collections import defaultdict
from os import path
import argparse
import io
import os
import re
import sys

ANSI_COLORS = {
    'default': '\033[0m',

    'black': '\033[30m',
    'red': '\033[31m',
    'green': '\033[32m',
    'yellow': '\033[33m',
    'blue': '\033[34m',
    'magenta': '\033[35m',
    'purple': '\033[35m',
    'cyan': '\033[36m',
    'white': '\033[37m',

    'reset': '\033[0;0m',
    'bold': '\033[1m',

    'blackbg': '\033[40m',
    'redbg': '\033[41m',
    'greenbg': '\033[42m',
    'yellowbg': '\033[43m',
    'bluebg': '\033[44m',
    'magentabg': '\033[45m',
    'cyanbg': '\033[46m',
    'whitebg': '\033[47m',
}


def print_error(filename, line_index, message):
    """Print an error to stdout."""
    term = os.environ.get('TERM', 'dumb')
    if term != 'dumb':
        filename = '%s%s%s' % (
            ANSI_COLORS['green'], filename, ANSI_COLORS['reset'])
        line_index = '%s%s%s' % (
            ANSI_COLORS['cyan'], line_index, ANSI_COLORS['reset'])
        message = '%s%s%s%s' % (
            ANSI_COLORS['yellow'], ANSI_COLORS['bold'],
            message, ANSI_COLORS['reset'])
    print('%s:%s: %s' % (filename, line_index, message))


__INCLUDE = re.compile(r'#include "([^"]+)"')


def extract_includes(srcdir, source):
    """Returns all locally included files."""
    includes = set()
    for line in io.open(source, encoding='utf-8'):
        match = __INCLUDE.match(line)
        if match:
            includes.add(path.join(srcdir, match.group(1)))
    return includes


__USES_INCLUDES = defaultdict(str)
__USES_INCLUDES['USES_OPENGL'] = r'(<glbinding\/\w+\.h>)|(<GL\/glew\.h>)'
__USES_INCLUDES['USES_ICU'] = r'<unicode\/.+\.h>'
__USES_INCLUDES['USES_INTL'] = r'"third_party\/gettext\/gettext\.h"'
__USES_INCLUDES['USES_PNG'] = r'<png\.h>'
__USES_INCLUDES['USES_SDL2_IMAGE'] = r'<SDL_image\.h>'
__USES_INCLUDES['USES_SDL2_MIXER'] = r'<SDL_mixer\.h>'
__USES_INCLUDES['USES_SDL2_TTF'] = r'<SDL_ttf\.h>'
__USES_INCLUDES['USES_SDL2'] = r'(?!(<SDL_image\.h>|<SDL_mixer\.h>|<SDL_ttf\.h>))(<SDL.+\.h>)'
__USES_INCLUDES['USES_ZLIB'] = r'"zlib\.h"'
__USES_INCLUDES['USES_ATOMIC'] = r'<atomic>'
__USES_INCLUDES['USES_MINIZIP'] = r'(<zip\.h>|<unzip\.h>)'


def extract_uses_includes(srcdir, source):
    """Returns all included files for the USES_* libraries."""
    includes = set()
    for line in io.open(source, encoding='utf-8'):
        for key in __USES_INCLUDES.keys():
            match = re.compile(
                r'^#include (' + __USES_INCLUDES[key] + ')').match(line)
            if match:
                includes.add(match.group(1))
    return includes


class Target(object):
    """Container for data for a cmake target."""

    def __init__(self, type, name, defined_at, srcs, depends):
        self.type = type
        self.name = name
        self.defined_at = defined_at
        self.srcs = srcs
        self.depends = depends

    def __repr__(self):
        return '%s(%s, nsrcs:%s, %s)' % (self.type, self.name, len(self.srcs), self.depends)


__USES = re.compile(r'\s*(USES_\w+)$')


def _parse_content(content):
    rv = defaultdict(list)
    opts = []
    for line in content[::-1]:
        if not line.strip():
            continue
        if __USES.match(line):
            rv['USES'].append(line)
        elif re.match(r'^[A-Z0-9_-]+$', line):
            rv[line] = opts
            opts = []
        else:
            opts.append(line)
    return rv


__START_TARGET = re.compile(r'\s*(wl_\w+)\s*\(\s*(\w+)')


def extract_targets(cmake_file):
    dirname = path.dirname(cmake_file)
    lines = list(enumerate(io.open(cmake_file, encoding='utf-8'), 1))
    # Remove comments.
    for i in range(len(lines)):
        idx = lines[i][1].find('#')
        if idx != -1:
            lines[i] = (lines[i][0], lines[i][1][:idx])
        lines[i] = (lines[i][0], lines[i][1].strip())

    # Remove empty lines
    lines = [(idx, line) for idx, line in lines if len(line)]

    targets = []
    while len(lines):
        line_idx, line = lines.pop(0)
        match = __START_TARGET.match(line)
        if match:
            content = []
            while len(lines):
                _, line = lines.pop(0)
                content.append(line.strip(')').strip())
                if ')' in line:
                    break
            d = _parse_content(content)
            srcs = set(path.realpath(path.join(dirname, src))
                       for src in d['SRCS'])
            targets.append(Target(match.group(1), match.group(2), (line_idx, cmake_file),
                                  srcs, d['DEPENDS'] + d['USES']))
    return targets


def base_dir():
    """Returns the absolute path of the Widelands source dir."""
    dirname = __file__
    while not path.exists(path.join(dirname, 'src')):
        dirname = path.abspath(path.join(dirname, os.pardir))
    return dirname


def find_source_and_cmake_files(srcdir):
    cmake_files = set()
    sources = set()
    for (dirpath, _, filenames) in os.walk(srcdir):
        for filename in filenames:
            if 'CMakeLists.txt' in filename:
                cmake_files.add(path.join(dirpath, filename))
            ext = path.splitext(filename)[-1]
            if ext in ['.c', '.h', '.cc', '.hpp']:
                sources.add(path.join(dirpath, filename))
    return cmake_files, sources


def report_unused_sources(srcdir, sources, owners_of_src):
    unused_sources = sources - set(owners_of_src.keys())
    for src in sorted(unused_sources):
        print_error(src, 1, '(CRITICAL) File not mentioned in any build rule.')
    return len(unused_sources) != 0


def report_unmentioned_or_unnecessary_dependencies(srcdir, target, includes_by_src, uses_includes_by_src, owners_of_src):
    # Collect for DEPENDS
    target_includes = set()
    for src in target.srcs:
        target_includes.update(includes_by_src[src])

    necessary = defaultdict(set)
    for include in target_includes:
        rel = os.path.relpath(include, srcdir)
        if include in owners_of_src:  # Might be an autogenerated file.
            necessary[owners_of_src[include].name].add(rel)

    # Collect for USES_*
    target_uses_includes = set()
    for src in target.srcs:
        target_uses_includes.update(uses_includes_by_src[src])

    for include in target_uses_includes:
        for key in __USES_INCLUDES.keys():
            match = re.compile(
                r'^(' + __USES_INCLUDES[key] + ')').match(include)
            if match:
                necessary[key].add(match.group(1))

    # Remove matches
    if target.name in necessary:
        del necessary[target.name]

    # Report errors
    unmentioned = set(necessary.keys()) - set(target.depends)
    for name in unmentioned:
        dependson_string = 'DEPENDS on '
        if name.startswith('USES'):
            dependson_string = ''

        print_error(
            target.defined_at[1], target.defined_at[0],
            ('%s misses ' + dependson_string + '%s, because it includes %s') % (
                target.name, name, ','.join(sorted(necessary[name]))))

    unnecessary = set(target.depends) - set(necessary.keys())
    for name in unnecessary:
        dependson_string = 'DEPENDS on '
        if name.startswith('USES'):
            dependson_string = ''

        print_error(
            target.defined_at[1], target.defined_at[0],
            ('%s ' + dependson_string + '%s, but includes none of its headers.') % (target.name, name))


def parse_args():
    p = argparse.ArgumentParser(description='Simple build dependencies checker.'
                                )

    p.add_argument('-i', '--includes', action='store_true', default=False,
                   help='Print includes of the files listed on the commandline.')

    return p.parse_args()


def main():
    srcdir = path.join(base_dir(), 'src')
    cmake_files, sources = find_source_and_cmake_files(srcdir)

    includes_by_src = defaultdict(set)
    for src in sources:
        includes_by_src[src] = extract_includes(srcdir, src)

    uses_includes_by_src = defaultdict(set)
    for src in sources:
        uses_includes_by_src[src] = extract_uses_includes(srcdir, src)

    target_list = []
    for cmake_file in cmake_files:
        target_list.extend(extract_targets(cmake_file))
    targets = {l.name: l for l in target_list}

    owners_of_src = {}
    for lib in targets.values():
        for src in lib.srcs:
            if src in owners_of_src:
                print('%s:1 is owned by more than one target.' % src)
            owners_of_src[src] = lib

    if (report_unused_sources(srcdir, sources, owners_of_src)):
        return 1

    for t in targets.values():
        report_unmentioned_or_unnecessary_dependencies(
            srcdir, t, includes_by_src, uses_includes_by_src, owners_of_src)


if __name__ == '__main__':
    sys.exit(main())
