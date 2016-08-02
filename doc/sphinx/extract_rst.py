#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import os
import os.path as p
import re
import sys

###################
# inputs, outputs #
###################
cpp_pairs = (
    ("src/scripting/lua_root.cc", "autogen_wl.rst"),
    ("src/scripting/lua_bases.cc", "autogen_wl_bases.rst"),
    ("src/scripting/lua_editor.cc", "autogen_wl_editor.rst"),
    ("src/scripting/lua_map.cc", "autogen_wl_map.rst"),
    ("src/scripting/lua_game.cc", "autogen_wl_game.rst"),
    ("src/scripting/lua_ui.cc", "autogen_wl_ui.rst"),
    ("src/scripting/lua_globals.cc", "autogen_globals.rst"),
)

# Where to search for RST comments in lua files
lua_dirs = (
    "data/scripting",
    "data/tribes/scripting/help"    
)

def _find_basedir():
    """Walk upwards in the directory tree till we are in the base directory of
    Widelands. Return the base directory and the source/ directory."""
    curdir = p.abspath(p.dirname(__file__))
    source_dir = p.join(curdir, "source")
    while not (p.exists(p.join(curdir, "data/tribes")) and p.exists(p.join(curdir, "data/world"))):
        curdir = p.abspath(p.join(curdir, p.pardir))
    return source_dir, curdir
source_dir, base_dir = _find_basedir()


def extract_rst_from_cpp(inname, outname=None):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    data = open(p.join(base_dir, inname), "r").read()
    res = re.findall(r"\s*/\* RST\s(.*?)\*/", data, re.M | re.DOTALL)

    output = ""
    for r in res:
        r = r.expandtabs(4)
        output += r + '\n'

    if output.strip():
        out = sys.stdout
        if outname is not None:
            out = open(p.join(source_dir, outname), "w")
        out.write(output)

def extract_rst_from_lua(inname):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    print 'franku inname: ', inname
    data = open(p.join(base_dir, inname), "r").read()

    res = re.findall(r"-- RST\s(.*?)(?:^(?!--))", data, re.M | re.DOTALL)

    outname = "autogen_aux_%s.rst" % os.path.basename(os.path.splitext(inname)[0])
    outname = p.join(source_dir, outname)

    output = ""
    for r in res:
        r = re.subn(re.compile(r'^-- ?', re.M | re.DOTALL), "", r)[0]
        output += r + '\n'

    if output.strip():
        out = sys.stdout if not outname else open(outname, "w")
        out.write(output)
        print 'franku return: ', os.path.basename(outname)
        return os.path.basename(outname)

def replace_auxilary_toc(aux_fns):
    print 'frank aux_fns: ', aux_fns
    aux_in = open(p.join(source_dir, "auxiliary.rst.in"), "r").read()
    aux_in = aux_in.replace("REPLACE_ME",
                '\n'.join('   ' + fn for fn in aux_fns))
    open(p.join(source_dir, "auxiliary.rst"), "w").write(aux_in)

if __name__ == '__main__':
    def main():
        for inf, outf in cpp_pairs:
            extract_rst_from_cpp(inf, outf)
        
        # NOCOMM franku: I replaced the commented part with the follwoing lines
        names = []
        for dirs in lua_dirs:
            for i in glob(p.join(base_dir, dirs, "*.lua")):
                name = extract_rst_from_lua(i)
                if name != None:
                    names.append(name)
        replace_auxilary_toc(names)
                                    
        # replace_auxilary_toc(
        #     filter(lambda a: a, [
        #         extract_rst_from_lua(i) for i in glob(p.join(base_dir, "data/scripting", "*.lua"))
        #     ])
        # )

    main()
