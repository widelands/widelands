#!/usr/bin/python -tt

##############################################################################
#
# This script holds the common functions for locale handling & generation.
# 
# Usage: add non-iterative catalogs to MAINPOT list, or iterative catalogs 
# (i.e., tribes) to ITERATIVEPOTS as explained below
#
# This file assumes to be called from base directory. Accepts a list of
# language codes to work on, or "-a" option to update all available. At least
# one of these is mandatory.
#
##############################################################################

import confgettext
from glob import glob
import os
import re
import string
import sys


# Holds the names of non-iterative catalogs to build and the
# corresponding source paths list. Note that paths MUST be relative to po/pot,
# to let .po[t] comments point to somewhere useful
MAINPOTS = [( "maps", ["../../maps/*/elemental"] ),
			( "texts", ["../../txts/*"] ),
			( "widelands", ["../../src/*.cc",
							"../../src/*/*.cc",
							"../../src/*/*/*.cc",
							"../../src/*.h",
							"../../src/*/*.h",
							"../../src/*/*/*.h"] ) ]


# This defines the rules for iterative generation of catalogs. This allows
# to automatically add new .pot files for newly created tribes, worlds, ...
#
# This is a list with structure:
#	- target .pot file mask
#	- base directory to scan for catalogs (referred to Widelands' base dir)
#	- List of source paths for catalog creation: tells the program which files
#			to use for building .pot files (referred to "po/pot/<path_to_pot/"
#			dir, so the file pointers inside .pot files actually point
#			somewhere useful)
#
# For every instance found of a given type, '%s' in this values is replaced
# with the name of the instance.
ITERATIVEPOTS = [ ("campaigns/%s", "campaigns/", 
											["../../../campaigns/%s/e*",
											"../../campaigns/%s/objective"] ),
				  ("tribes/%s", "tribes/", ["../../../tribes/%s/conf",
											 "../../../tribes/%s/*/*/conf"] ),
				  ("worlds/%s", "worlds/", ["../../../worlds/%s/*conf",
											 "../../../worlds/%s/*/*/conf"] )
				]


# Some useful regular expressions

RE_NO_DOTFILE="^[^\.]"		# Matches everything but dot-leaded filenames.
RE_ISO639="^[a-z]{2,2}(_[A-Z]{2,2})?$"	# Matches ISO-639 language codes
										# structure. Note that this doesn't
										# garantees correctness of code.

# Options passed to common external programs
XGETTEXTOPTS="-k_ --from-code=UTF-8 "
MSGMERGEOPTS="-q --no-wrap"

##############################################################################
#
# Check if we are called from the right place
#
##############################################################################
def do_check_root():
		if (not os.path.isdir("po")):

				print "Error: no 'po/' subdir found.\n"
				print ("This script needs to access translations placed " +
					"under 'po/' subdir, but these seem unavailable. Check " +
					"that you called this script from Widelands' main dir.\n")
				sys.exit(1)


##############################################################################
#
# Check correctness of passed arguments
#
##############################################################################
def do_check_parameters():
		if len(sys.argv) < 2:
				print "Usage: buildcat.py [-a | ll1 ll2 ... ]"
				print "\twhere ll1 ll2 ... are language codes to update/generate."
				sys.exit(1)


##############################################################################
#
# Create subdirs as needed
#
##############################################################################
def do_makedirs( dirs ):
		try:
				os.makedirs( dirs )
		except:
				"" # do nothing


##############################################################################
#
# Compile a .pot file using python scripts, as xgettext is unable to handle
# our conffile format.
#
##############################################################################
def do_compile( potfile, srcfiles ):
		files = []
		for i in srcfiles:
				files += glob(i)

		catalog = confgettext.parse_conf(files)
		file = open(potfile, "w")
		file.write(catalog)
		file.close()


##############################################################################
#
# Compile known source code files with xgettext.
#
##############################################################################
def do_compile_src( potfile, srcfiles ):
		return os.system("xgettext %s -o %s %s" %  (XGETTEXTOPTS, potfile, 
					string.join(srcfiles)))


##############################################################################
#
# Return a list of directories under a given preffix, matching regular
# expression provided.
#
##############################################################################
def do_find_dirs(preffix, pattern):
		res = []
		p = re.compile(pattern)

		for file in os.listdir(preffix):
				if (os.path.isdir("%s/%s" % (preffix, file)) and
								p.match(file)):
						res.append(file)

		res.sort()
		return res


##############################################################################
#
# Build a list of catalogs from iterative rules above. Returns a list of 
# type ("catalog_name", ["source_paths_list"])
#
##############################################################################
def do_find_iterative(preffix, basedir, srcmasks):
		res = []

		for file in do_find_dirs(basedir, RE_NO_DOTFILE):
				srcfiles = []
				for p in srcmasks:
						srcfiles.append(string.replace(p, "%s", file))
				name = string.replace(preffix, "%s", file)
				res.append((name, srcfiles))

		return res


##############################################################################
#
# Find files under "root" matching given pattern
#
##############################################################################
def do_find_files(root, pattern):
		res = []
		p = re.compile(pattern)

		for base, dirs, files in os.walk(root):
				for f in (files):
						file = ("%s/%s" % (base[len(root):], f))
						if p.match(file):
								res.append(file)

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
		for preffix, basedir, srcfiles in ITERATIVEPOTS:
				potfiles += do_find_iterative(preffix, basedir, srcfiles)

		# Generate .pot catalogs
		for pot, srcfiles in potfiles:
				path = "po/pot/" + os.path.dirname(pot)
				do_makedirs(path)
				oldcwd = os.getcwd()
				os.chdir(path)
				potfile = os.path.basename(pot) + '.pot'

				print("\tpo/pot/%s.pot" % pot)
				if potfile == 'widelands.pot':
						# This catalogs can be built with xgettext
						do_compile_src( potfile, srcfiles )
				else:
						do_compile( potfile, srcfiles )
		
				os.chdir(oldcwd)

		print("")


##############################################################################
#
# Compile a target .po file from source "po" and "pot" catalogs. Dump result
# to "dst" file.
#
##############################################################################
def do_buildpo(po, pot, dst):
		return os.system("msgmerge %s %s %s -o %s" % 
						(MSGMERGEOPTS, po, pot, dst))


##############################################################################
#
# Modify source .po file to suit project specific needs. Dump result to a
# different destination file
#
##############################################################################
def do_tunepo(src, dst):
		input = open(src)
		output = open(dst, 'w')

		# Here we should update file headers. Maybe for a future release...
		# do_update_headers(.......)

		# Some comments in .po[t] files show filenames and line numbers for
		# reference in platform-dependent form (slash/backslash). We
		# standarize them to slashes, since this results in smaller SVN diffs
		for l in input:
				if l[0:2] == "#:":
						output.write(l.replace('\\', '/'))
				else:
						output.write(l)

		input.close()
		output.close()
		return


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
				po = ("po/%s/%s" % (lang, f.rstrip("t")))
				pot = ("po/pot/%s" % f)
				tmp = "tmp.po"
			
				if not (os.path.exists(po)):
						# No need to call mesgmerge if there's no translation
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


##############################################################################
#
# Update .po catalogs for specified languages, or all available if "-a" is
# specified.
#
##############################################################################
if __name__ == "__main__":
		# Sanity checks
		do_check_parameters()
		do_check_root()

		# Make sure .pot files are up to date.
		do_update_potfiles()

		sys.stdout.write("Updating translations: ")
		if (sys.argv[1] == "-a"):
				lang = do_find_dirs("po/", RE_ISO639)
				print "all available."
		else:
				lang = sys.argv[1:]
				print lang

		# Assemble a list of .pot files available
		srcfiles = do_find_files("po/pot", ".*\.pot$")

		for l in lang:
				do_update_po(l.lstrip("/"), srcfiles)
		print ""
