#!/usr/bin/env python
# encoding: utf-8

##############################################################################
#
# This script holds the common functions for locale handling & generation.
#
# Usage: add non-iterative catalogs to MAINPOT list, or iterative catalogs
# (i.e., tribes) to ITERATIVEPOTS as explained below
#
##############################################################################

from glob import glob
from itertools import takewhile
import os
import string
import subprocess
import sys

from lua_xgettext import Lua_GetText
import confgettext

# Holds the names of non-iterative catalogs to build and the
# corresponding source paths list. Note that paths MUST be relative to po/pot,
# to let .po[t] comments point to somewhere useful
MAINPOTS = [
    ( "maps/maps", [
        "../../maps/*/elemental",
        "../../maps/*/*/elemental",
        "../../campaigns/*.conf",
        "../../campaigns/*/elemental"
    ] ),
    ( "texts/texts", ["../../txts/*.lua",
                  "../../txts/tips/*.tip"] ),
    ( "widelands/widelands", [
                    "../../src/wlapplication.cc",
                    "../../src/*/*.cc",
                    "../../src/*/*/*.cc",
                    "../../src/wlapplication.h",
                    "../../src/*/*.h",
                    "../../src/*/*/*.h",
                    "../../scripting/widelands/*.lua",
    ] ),
    ( "widelands_console/widelands_console", [
                    "../../src/wlapplication_messages.cc",
                    "../../src/wlapplication_messages.h",
    ] ),
    ( "win_conditions/win_conditions", [
        "../../scripting/win_conditions/*.lua",
        "../../scripting/win_condition_texts.lua",
    ]),
    ("world/world", [
        "../../world/*.lua",
        "../../world/*/*.lua",
        "../../world/*/*/*.lua",
        "../../world/*/*/*/*.lua",
        "../../world/*/*/*/*/*.lua",
        "../../world/*/*/*/*/*/*.lua",
    ]),
    ("tribes/tribes", [
        "../../tribes/scripting/starting_conditions/*/*.lua" # NOCOM the starting conditions aren't being picked up.
        "../../tribes/*.lua",
        "../../tribes/preload.lua", # NOCOM Why do I have to load both *.lua and preload.lua for this to work?
        "../../tribes/*/init.lua",
        "../../tribes/*/*/init.lua",
        "../../tribes/*/*/*/init.lua",
        "../../tribes/*/*/*/*/init.lua",
        "../../tribes/*/*/*/*/*/init.lua",
    ]),

    ("tribes_encyclopedia/tribes_encyclopedia", [
        "../../tribes/scripting/help/*.lua",
        "../../tribes/*/helptexts.lua",
        "../../tribes/*/*/helptexts.lua",
        "../../tribes/*/*/*/helptexts.lua",
        "../../tribes/*/*/*/*/helptexts.lua",
        "../../tribes/*/*/*/*/*/helptexts.lua",
    ]),
]


# This defines the rules for iterative generation of catalogs. This allows
# to automatically add new .pot files for newly created directories.
#
# This is a list with structure:
#       - target .pot file mask
#       - base directory to scan for catalogs (referred to Widelands' base dir)
#       - List of source paths for catalog creation: tells the program which
#         files to use for building .pot files (referred to
#         "po/pot/<path_to_pot/" dir, so the file pointers inside .pot files
#         actually point somewhere useful)
#
# For every instance found of a given type, '%s' in this values is replaced
# with the name of the instance.
ITERATIVEPOTS = [
    ("scenario_%(name)s/scenario_%(name)s", "campaigns/",
         ["../../campaigns/%(name)s/extra_data",
          "../../campaigns/%(name)s/objective",
          "../../campaigns/%(name)s/scripting/*.lua",
          "../../scripting/format_scenario.lua"
         ]
    ),
    ("map_%(name)s/map_%(name)s", "maps/",
         [ "../../maps/%(name)s/scripting/*.lua", ]
    ),
    ("mp_scenario_%(name)s/mp_scenario_%(name)s", "maps/MP Scenarios/",
         [ "../../maps/MP Scenarios/%(name)s/scripting/*.lua", ]
    ),
]


# Options passed to common external programs
XGETTEXTOPTS ="-k_ --from-code=UTF-8"
XGETTEXTOPTS+=" -F -c\"* TRANSLATORS\""
# escaped double quotes are necessary for windows, as it ignores single quotes
XGETTEXTOPTS+=" --copyright-holder=\"Widelands Development Team\""
XGETTEXTOPTS+=" --msgid-bugs-address=\"https://bugs.launchpad.net/widelands\""


def are_we_in_root_directory():
    """Make sure we are called in the root directory"""
    if (not os.path.isdir("po")):
        print("Error: no 'po/' subdir found.\n")
        print("This script needs to access translations placed " +
            "under 'po/' subdir, but these seem unavailable. Check " +
            "that you called this script from Widelands' main dir.\n")
        sys.exit(1)


def do_makedirs( dirs ):
    """Create subdirectories. Ignore errors"""
    try:
        os.makedirs( dirs )
    except:
        pass


def do_compile( potfile, srcfiles ):
    """
    Search Lua and conf files given in srcfiles for translatable strings.
    Merge the results and write out the corresponding pot file.
    """
    files = []
    for i in srcfiles:
        files += glob(i)
    files = set(files)

    lua_files = set([ f for f in files if
        os.path.splitext(f)[-1].lower() == '.lua' ])
    conf_files = files - lua_files

    l = Lua_GetText()
    for fname in lua_files:
        l.parse(open(fname, "r").read(), fname)

    l.merge(confgettext.parse_conf(conf_files))

    if not l.found_something_to_translate:
        return False

    with open(potfile, "w") as potfileobject:
        potfileobject.write(str(l))
        potfileobject.close()
    return True


def do_compile_src( potfile, srcfiles ):
    """
    Use xgettext for parse the given C++ files in srcfiles. Merge the results
    and write out the given potfile
    """
    # call xgettext and supply source filenames via stdin
    gettext_input = subprocess.Popen("xgettext %s --files-from=- --output=%s" % \
            (XGETTEXTOPTS, potfile), shell=True, stdin=subprocess.PIPE).stdin
    try:
        for one_pattern in srcfiles:
            # 'normpath' is necessary for windows ('/' vs. '\')
            # 'glob' handles filename wildcards
            for one_file in glob(os.path.normpath(one_pattern)):
                gettext_input.write(one_file + "\n")
        return gettext_input.close()
    except IOError as err_msg:
        sys.stderr.write("Failed to call xgettext: %s\n" % err_msg)
        return -1


##############################################################################
#
# Build a list of catalogs from iterative rules above. Returns a list of
# type ("catalog_name", ["source_paths_list"])
#
##############################################################################
def do_find_iterative(prefix, basedir, srcmasks):
    res = []

    directories = sorted(
        d for d in os.listdir(basedir) if
            os.path.isdir(os.path.normpath("%s/%s" % (basedir, d))) and
            not os.path.basename(d).startswith('.')
    )
    for filename in directories:
        srcfiles = []
        for p in srcmasks:
            srcfiles.append(p % { "name": filename })
        name = prefix % { "name": filename }
        res.append((name, srcfiles))

    return res


##############################################################################
#
# Regenerate all .pot files specified above and place them under pot/ tree
#
##############################################################################
def do_update_potfiles():
        print("Generating reference catalogs:")

        # Build the list of catalogs to generate
        potfiles = MAINPOTS
        for prefix, basedir, srcfiles in ITERATIVEPOTS:
            potfiles += do_find_iterative(prefix, basedir, srcfiles)

        # Generate .pot catalogs
        dangerous_chars = "'\" " # Those chars are replaced via '_'
        for pot, srcfiles in potfiles:
            pot = pot.lower().translate(string.maketrans(dangerous_chars, len(dangerous_chars)*"_"))
            path = os.path.normpath("po/" + os.path.dirname(pot))
            do_makedirs(path)
            oldcwd = os.getcwd()
            os.chdir(path)
            potfile = os.path.basename(pot) + '.pot'
            if pot.startswith('widelands'):
                # This catalogs can be built with xgettext
                do_compile_src(potfile , srcfiles )
                succ = True
            else:
                succ = do_compile(potfile, srcfiles)

            os.chdir(oldcwd)

            if succ:
                print("\tpo/%s.pot" % pot)
            else:
                os.rmdir(path)

        print("")


##############################################################################
#
# Compile a target .po file from source "po" and "pot" catalogs. Dump result
# to "dst" file.
#
##############################################################################
def do_buildpo(po, pot, dst):
    rv = os.system("msgmerge -q --no-wrap %s %s -o %s" % (po, pot, dst))
    if rv:
        raise RuntimeError("msgmerge exited with errorcode %i!" % rv)
    return rv


##############################################################################
#
# Modify source .po file to suit project specific needs. Dump result to a
# different destination file
#
##############################################################################
def do_tunepo(src, dst):
    """
    Try to reduce the differences in generated po files: we keep the generated
    header, but do some modifications in comments to make diffs smaller
    """
    outlines = []

    # Copy header verbatim.
    input_iter = open(src)
    for line in takewhile(lambda l: l.strip(), input_iter):
        outlines.append(line)
    outlines.append("\n")

    for line in input_iter:
        # Some comments in .po[t] files show filenames and line numbers for
        # reference in platform-dependent form (slash/backslash). We
        # standarize them to slashes, since this results in smaller SVN diffs
        if line.startswith("#:"):
            line = line.replace('\\', '/')

        outlines.append(line)

    open(dst, 'w').writelines(outlines)


##############################################################################
#
# Update .po files for a given language dir, or create empty ones if there's
# no translation present.
#
##############################################################################
def do_update_po(lang, files):
    sys.stdout.write("\t%s:\t" % lang)

    for f in files:
        # File names to use
        pot = os.path.normpath("po/%s" % f)
        po = os.path.join(os.path.dirname(pot), lang + '.po')
        tmp = "tmp.po"

        if not (os.path.exists(po)):
            # No need to call msgmerge if there's no translation
            # to merge with. We can use .pot file as input file
            # below, but we need to make sure the target dir is
            # ready.
            do_makedirs(os.path.dirname(po))
            tmp = pot
            fail = 0
        else:
            fail = do_buildpo(po, pot, tmp)

        if not fail:
            # tmp file is ready, but we need to tune some aspects
            # of it
            do_tunepo(tmp, po)

            if tmp == "tmp.po":
                os.remove("tmp.po")

            sys.stdout.write(".")
            sys.stdout.flush()

    sys.stdout.write("\n")


if __name__ == "__main__":
    # Sanity checks
    are_we_in_root_directory()

    # Make sure .pot files are up to date.
    do_update_potfiles()

    print("")
