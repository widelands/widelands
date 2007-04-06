#!/usr/bin/python -tt

##############################################################################
#
# This script compiles all .po files available for the languages specified
# in the command line, or all available if none is specified.
#
# Usage: assumes to be called from base directory, and that every argument 
# passed is a language code to compile. If no argument is passed, will compile
# every .po file found in po/ directories looking like ISO-639 language codes
#
##############################################################################

import buildcat
import os
import sys
import string


##############################################################################
#
# Merge & compile every .po file found in 'po/lang' directory
#
##############################################################################
def do_compile(lang):
		sys.stdout.write("\t%s:\t" % lang)

		for f in buildcat.do_find_files(("po/%s" % lang), ".*\.po$"):
				# File names and paths to use
				po = ("po/%s%s" % (lang, f))
				pot = ("po/pot%st" % f)
				mo = ("locale/%s/LC_MESSAGES%s" % (lang,
						string.replace(f, ".po", ".mo")))
				
				if not buildcat.do_buildpo(po, pot, "tmp.po"):
						buildcat.do_makedirs(os.path.dirname(mo))
						if not (os.system("msgfmt -o %s tmp.po" % mo)):
								os.remove("tmp.po")
								sys.stdout.write(".")
								sys.stdout.flush()

		sys.stdout.write("\n")


##############################################################################
#
# Compile new translations from existing translations and freshly created .pot
# catalogs.
#
##############################################################################
if __name__ == "__main__":
		# Sanity checks
		buildcat.do_check_root()

		# Make sure .pot files are up to date.
		buildcat.do_update_potfiles()

		sys.stdout.write("Compiling translations: ")

		if len(sys.argv) > 1:
				# Assume all parameters are language codes to compile
				lang = sys.argv[1:]
				print lang
		else:
				# Find every directory that looks like ISO-639 
				lang = buildcat.do_find_dirs("po", buildcat.RE_ISO639)
				print "all available."

		for l in lang:
				do_compile(l)

		print("")
