#!/usr/bin/env python
# encoding: utf-8

import re
import sys

###################
# inputs, outputs #
###################
pairs = (
    ("../lua_map.cc", "source/wl_map.rst"),
)

def extract_rst(inname, outname=None):
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


if __name__ == '__main__':
    def main():
        for inf, outf in pairs:
            extract_rst(inf, outf)

    main()
