#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import os
import re
import sys

###################
# inputs, outputs #
###################
cpp_pairs = (
    ("../lua_map.cc", "source/wl_map.rst"),
    ("../lua_game.cc", "source/wl_game.rst"),
    ("../lua_debug.cc", "source/wl_debug.rst"),
    ("../lua_globals.cc", "source/globals.rst"),
)

def extract_rst_from_cpp(inname, outname=None):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    data = open(inname, "r").read()

    res = re.findall(r"\s*/\* RST\s(.*?)\*/", data, re.M | re.DOTALL)

    out = sys.stdout if not outname else open(outname, "w")

    for r in res:
        r = r.expandtabs(4)
        out.write(r + '\n')

def extract_rst_from_lua(inname):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    data = open(inname, "r").read()

    res = re.findall(r"-- RST\s(.*?)(?:^(?!--))", data, re.M | re.DOTALL)

    outname = "source/aux_%s.rst" %os.path.basename(os.path.splitext(inname)[0])

    out = sys.stdout if not outname else open(outname, "w")

    for r in res:
        r = re.subn(re.compile(r'^-- ?', re.M | re.DOTALL), "", r)[0]
        out.write(r + '\n')


if __name__ == '__main__':
    def main():
        for inf, outf in cpp_pairs:
            extract_rst_from_cpp(inf, outf)
        for inf in glob("../../../scripting/*.lua"):
            extract_rst_from_lua(inf)

    main()
