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
sys.path.append('build/scons-tools')
from detect_revision import detect_revision
from time import strftime,gmtime

# The current version of source
SRCVERSION=detect_revision()
NO_HEADER_REWRITE = 0
HEADER_YEAR = strftime("%Y",gmtime())

# Holds the names of non-iterative catalogs to build and the
# corresponding source paths list. Note that paths MUST be relative to po/pot,
# to let .po[t] comments point to somewhere useful
MAINPOTS = [( "maps", ["../../maps/*/elemental", "../../campaigns/cconfig"] ),
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
XGETTEXTOPTS ="-k_ --from-code=UTF-8"
# escaped double quotes are necessary for windows, as it ignores single quotes
XGETTEXTOPTS+=" --copyright-holder=\"Widelands Development Team\""
XGETTEXTOPTS+=" --msgid-bugs-address=\"widelands-public@lists.sourceforge.net\""

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
		# call xgettext and supply source filenames via stdin
		gettext_input = os.popen("xgettext %s --files-from=- --output=%s" % \
				(XGETTEXTOPTS, potfile), "w")
		try:
			for one_pattern in srcfiles:
				# 'normpath' is necessary for windows ('/' vs. '\')
				# 'glob' handles filename wildcards
				for one_file in glob(os.path.normpath(one_pattern)):
					gettext_input.write(one_file + "\n")
			return gettext_input.close()
		except IOError, err_msg:
			sys.stderr.write("Failed to call xgettext: %s\n" % err_msg)
			return -1


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
				if (os.path.isdir(os.path.normpath("%s/%s" %
						          (preffix, file))) and
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
				path = os.path.normpath("po/pot/" + os.path.dirname(pot))
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
# Check each headerline for correctness
#
# Take input:
#  filename:    Name of the file worked upon
#  filehandle:  Handle to write clean headers to
#  lines[]:     The headerlines to be checked
#
# Always returns true
#
##############################################################################
def do_header_check(filename, filehandle, lines):
	
	# Array of regex to match lines that will be checked. Place all matches
	# that have a template before those that do not. Templates are used by
	# re_compiled_array index numbers.
	re_compiled_array=[
		re.compile(r"^# Widelands " + filename + r"$"),
		re.compile(r"^# Copyright \(C\) 200[0-9](-200[56789])* Widelands Development Team$"),
		re.compile(r"^# [^\s<>]+( [^\s<>]+)* <[^\s]+@[^\s]+>, 20[0-9]{2}\.$"),
		re.compile(r"^#$"),
		re.compile(r"^msgid \"\"$"),
		re.compile(r"^msgstr \"\"$"),
		re.compile(r"^\"Project-Id-Version: Widelands " + SRCVERSION + r"\\n\"$"),
		re.compile(r"^\"Report-Msgid-Bugs-To: widelands-public@lists\.sourceforge\.net\\n\"$"),
		re.compile(r"^\"POT-Creation-Date: [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}\+[0-9]{4}\\n\"$"),
		re.compile(r"^\"PO-Revision-Date: [0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}\+[0-9]{4}\\n\"$"),
		re.compile(r"^\"Last-Translator: [^\s<>]+( [^\s<>]+)* <[^\s]+@[^\s]+>\\n\"$"),
		re.compile(r"^\"Language-Team: [^\s<>]+( [^\s<>]+)* <[^\s]+@[^\s]+>\\n\"$"),
		re.compile(r"^\"MIME-Version: 1\.0\\n\"$"),
		re.compile(r"^\"Content-Type: text/plain; charset=UTF-8\\n\"$"),
		re.compile(r"^\"Content-Transfer-Encoding: 8bit\\n\"$"),
		re.compile(r"^\"Plural-Forms: nplurals=[0-9]+; plural=.*;\\n\"$")
	]

	# Create an array of template strings to output on header line
	# mismatch. Final \n is appended on each use, so don't append one here.
	HEADERTEMPLATE=[
		"# Widelands " + filename,
		"# Copyright (C) " + HEADER_YEAR + " Widelands Development Team.",
		"# FIRST AUTHOR <EMAIL@ADDRESS.TLD>, YEAR.",
		"#",
		"msgid \"\"",
		"msgstr \"\"",
		"\"Project-Id-Version: Widelands " + SRCVERSION + "\\n\"",
		"\"Report-Msgid-Bugs-To: widelands-public@lists.sourceforge.net\\n\"",
		"\"POT-Creation-Date: YYYY-MM-DD hh:mm+ZZZZ\\n\"",
		"\"PO-Revision-Date: YYYY-MM-DD hh:mm+ZZZZ\\n\"",
		"\"Last-Translator: REAL NAME <EMAIL@ADDRESS.TLD>\\n\"",
		"\"Language-Team: Language <widelands-public@lists.sourceforge.net>\\n\"",
		"\"MIME-Version: 1.0\\n\"",
		"\"Content-Type: text/plain; charset=UTF-8\\n\"",
		"\"Content-Transfer-Encoding: 8bit\\n\""
	]

	# Extended Headers are checked, as well.
	re_extended_header = re.compile(r"^\"X-.+: .+\\n\"$")

	# The function
	## Obey option and just write the header!
	if NO_HEADER_REWRITE:
		for line in lines:
			filehandle.write(line)
		return 1

	# No header found, write new and return
	if len(lines) == 0:
		# Write a fresh header and return true
		for line in HEADERTEMPLATE:
			filehandle.write(line + "\n")
		return 1

	# This array will be populated with the checked headerlines
	results = []

	# Check headers, sort, insert missing
	# Append correct extended headers
	# Write file and show discarded lines
	for regexnr in range(len(re_compiled_array)):
		for linenr in range(len(lines)):
			if re_compiled_array[regexnr].match(lines[linenr]):
				results.append(lines[linenr])
				lines.pop(linenr)
				break
		else:
			if regexnr < len(HEADERTEMPLATE):
				results.append(HEADERTEMPLATE[regexnr] + "\n")

	for line in lines[:]:
		if re_extended_header.match(line):
			results.append(line)
			lines.remove(line)
	
	for line in results:
		filehandle.write(line)
	
	if len(lines) != 0:
		print "\nDiscarded the following lines from " + filename + "!"
		for line in lines:
			print ">>>" + line.rstrip("\n")

	return 1


##############################################################################
#
# Modify source .po file to suit project specific needs. Dump result to a
# different destination file
#
##############################################################################
def do_tunepo(src, dst):
		input = open(src)
		output = open(dst, 'w')

		# Check file consistency

		header = 1
		headerlines = []

		for l in input:

				# Check headers in .po files.
				# Eat up all headerlines and check them
				if header:
					if not re.match(r"^$", l):
						headerlines.append(l)
						continue
					else:
						do_header_check(dst, output, headerlines)
						header = 0

				# Some comments in .po[t] files show filenames and line numbers for
				# reference in platform-dependent form (slash/backslash). We
				# standarize them to slashes, since this results in smaller SVN diffs
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
				po = os.path.normpath(("po/%s/%s" % 
						(lang, f.rstrip("t").lstrip("/"))))
				pot = os.path.normpath(("po/pot/%s" % f))
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

		if (os.getenv('NO_HEADER_REWRITE')):
			NO_HEADER_REWRITE = 1

		if (sys.argv[1] == "-a"):
				lang = do_find_dirs("po/", RE_ISO639)
				print "all available."
		else:
				lang = sys.argv[1:]
				print lang

		# Assemble a list of .pot files available
		srcfiles = do_find_files("po/pot", ".*\.pot$")

		for l in lang:
				do_update_po(l, srcfiles)
		print ""
