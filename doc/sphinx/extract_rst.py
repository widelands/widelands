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
# These files ar known to have rst comments; cpp files
cpp_pairs = (
    ('src/scripting/lua_root.cc', 'autogen_wl.rst'),
    ('src/scripting/lua_bases.cc', 'autogen_wl_bases.rst'),
    ('src/scripting/lua_editor.cc', 'autogen_wl_editor.rst'),
    ('src/scripting/lua_map.cc', 'autogen_wl_map.rst'),
    ('src/scripting/lua_game.cc', 'autogen_wl_game.rst'),
    ('src/scripting/lua_ui.cc', 'autogen_wl_ui.rst'),
    ('src/scripting/lua_globals.cc', 'autogen_globals.rst'),
)

# This directories are scanned without knowing which file
# has rst comments; scan for *.lua files
lua_dirs = (
    ('data/scripting', 'aux'),
    ('data/scripting/editor', 'editor'),
    ('data/scripting/win_conditions', 'editor'),
    ('data/tribes', 'tribes'),
    ('data/tribes/scripting/help', 'tribes'),
    ('data/tribes/buildings/militarysites/altanteans/castle', 'tribes'),
)


def _find_basedir():
    """Walk upwards in the directory tree till we are in the base directory of
    Widelands.

    Return the base directory and the source/ directory.

    """
    curdir = p.abspath(p.dirname(__file__))
    source_dir = p.join(curdir, 'source')
    while not (p.exists(p.join(curdir, 'data/tribes')) and p.exists(p.join(curdir, 'data/world'))):
        curdir = p.abspath(p.join(curdir, p.pardir))
    return source_dir, curdir
source_dir, base_dir = _find_basedir()


def extract_rst_from_cpp(inname, outname=None):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    data = open(p.join(base_dir, inname), 'r').read()
    res = re.findall(r"\s*/\* RST\s(.*?)\*/", data, re.M | re.DOTALL)

    output = ''
    for r in res:
        r = r.expandtabs(4)
        output += r + '\n'

    if output.strip():
        out = sys.stdout
        if outname is not None:
            out = open(p.join(source_dir, outname), 'w')
        out.write(output)


def extract_rst_from_lua(directory):
    """
    Searches for /* RST comments in the given directory, strips the lines
    and prints them out on stdout or writes them to outname. Returns a list
    with produced files (outname) for each directory.
    """
    # Used for replace_auxiliary_toc:
    rstnames = []
    for filename in glob(p.join(base_dir, directory, '*.lua')):
        data = open(p.join(base_dir, filename), 'r').read()
        res = re.findall(r"-- RST\s(.*?)(?:^(?!--))", data, re.M | re.DOTALL)
        if res:
            outname = 'autogen_aux_%s.rst' % os.path.basename(
                os.path.splitext(filename)[0])
            rstnames.append(outname)
            outname = p.join(source_dir, outname)

            output = ''
            for r in res:
                r = re.subn(re.compile(r'^-- ?', re.M | re.DOTALL), '', r)[0]
                output += r + '\n'

            if output.strip():
                out = sys.stdout if not outname else open(outname, 'w')
                out.write(output)
    return rstnames


def replace_auxilary_toc(aux_fns):
    aux_in = open(p.join(source_dir, 'auxiliary.rst.in'), 'r').read()
    aux_in = aux_in.replace('REPLACE_ME',
                            '\n'.join('   ' + fn for fn in aux_fns))
    open(p.join(source_dir, 'auxiliary.rst'), 'w').write(aux_in)

if __name__ == '__main__':
    def main():
        for inf, outf in cpp_pairs:
            extract_rst_from_cpp(inf, outf)

        # Search the given dirs for rst-comments in every lua files
        all_names = []
        for dirs, toc in lua_dirs:
            rstnames = extract_rst_from_lua(dirs)
            all_names.extend(rstnames)
        # Replace the auxiliary toc with all found rst comments in lua files.
        replace_auxilary_toc(all_names)

    main()
