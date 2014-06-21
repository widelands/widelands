#!/usr/bin/env python
# encoding: utf-8

"""
Simple script to check that the build dependencies in the CMakeLists.txt files
are correct. It relies on the established build rule formatting conventions in
the code base.
"""

from collections import defaultdict
from os import path
import argparse
import io
import os
import re
import sys

__INCLUDE = re.compile(r'#include "([^"]+)"')
def extract_includes(srcdir, source):
    """Returns all locally included files."""
    includes = set()
    for line in io.open(source, encoding="utf-8"):
        m = __INCLUDE.match(line)
        if m:
            includes.add(path.join(srcdir, m.group(1)))
    return includes

class Target(object):
    def __init__(self, type, name, defined_at, srcs, depends):
        self.type = type
        self.name = name
        self.defined_at = defined_at
        self.srcs = srcs
        self.depends = depends

    def __repr__(self):
        return "%s(%s, nsrcs:%s, %s)" % (self.type, self.name, len(self.srcs), self.depends)

def _parse_content(content):
    rv = defaultdict(list)
    opts = []
    for line in content[::-1]:
        if not line.strip():
            continue
        if re.match(r"^[A-Z_-]+$", line):
            rv[line] =  opts
            opts = []
        else:
            opts.append(line)
    return rv

__START_TARGET = re.compile(r'(wl_\w+)\s*\(\s*(\w+)')
def extract_targets(cmake_file):
    dirname = path.dirname(cmake_file)
    lines = list(enumerate(io.open(cmake_file, encoding="utf-8"), 1))
    # Remove comments.
    for i in range(len(lines)):
        idx = lines[i][1].find('#')
        if idx != -1:
            lines[i] = (lines[i][0], lines[i][1][:idx])
        lines[i] = (lines[i][0], lines[i][1].strip())

    # Remove empty lines
    lines = [(idx,line) for idx, line in lines if len(line)]

    targets = []
    while len(lines):
        line_idx, line = lines.pop(0)
        m = __START_TARGET.match(line)
        if m:
            # target.append(Target(m.group(1)))
            content = []
            while len(lines):
                _, line = lines.pop(0)
                content.append(line.strip(")").strip())
                if ")" in line:
                    break
            d = _parse_content(content)
            srcs = set(path.realpath(path.join(dirname, src)) for src in d["SRCS"])
            targets.append(Target(m.group(1), m.group(2), (line_idx, cmake_file),
                srcs, d["DEPENDS"]))
    return targets

def base_dir():
    """Returns the absolute path of the Widelands source dir."""
    dirname = __file__
    while not path.exists(path.join(dirname, "src")):
        dirname = path.abspath(path.join(dirname, os.pardir))
    return dirname

def find_source_and_cmake_files(srcdir):
    cmake_files = set()
    sources = set()
    for (dirpath, _, filenames) in os.walk(srcdir):
        for filename in filenames:
            if "CMakeLists.txt" in filename:
                cmake_files.add(path.join(dirpath, filename))
            ext = path.splitext(filename)[-1]
            if ext in ['.c', '.h', '.cc']:
                sources.add(path.join(dirpath, filename))
    return cmake_files, sources

def report_unused_sources(srcdir, target, sources):
    users = defaultdict(list)
    for lib in target.values():
        for src in lib.srcs:
            users[src].append(lib)

    unused_sources = sources - set(users.keys())
    for src in sorted(unused_sources):
        rel = os.path.relpath(src, srcdir)
        print "%s:1: File is not mentioned in any build rule." % (rel)

    return len(unused_sources)

def parse_args():
    p = argparse.ArgumentParser(description=
        "Simple build dependencies checker."
    )

    p.add_argument("-i", "--includes", action="store_true", default=False,
            help = "Print includes of the files listed on the commandline.")

    return p.parse_args()

def main():
    srcdir = path.join(base_dir(), "src")
    cmake_files, sources = find_source_and_cmake_files(srcdir)

    includes = {
        source_file: extract_includes(
            srcdir, source_file) for source_file in sources
    }

    errors = False

    target_list = []
    for cmake_file in cmake_files:
        target_list.extend(extract_targets(cmake_file))
    targets = {l.name: l for l in target_list}

    errors |= report_unused_sources(srcdir, targets, sources)

    return errors

if __name__ == '__main__':
    sys.exit(main())
