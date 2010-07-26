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

import os
import sys
import string
from glob import glob
import os.path as p

import buildcat

#Mac OS X hack to support XCode and macports default path					
MACPORTSPREFIX = "/opt/local/bin/"
MSGFMT = "msgfmt"
if os.path.isfile(MACPORTSPREFIX + MSGFMT):
    MSGFMT = MACPORTSPREFIX + MSGFMT
	
##############################################################################
#
# Merge & compile every .po file found in 'po/lang' directory
#
##############################################################################
def do_compile(lang):
        sys.stdout.write("\t%s:\t" % lang)

        for po in glob("po/*/%s.po" % lang):
            # Hopefully only one pot
            pot, = glob(p.join(p.dirname(po),"*.pot"))
            mo = p.join("locale", lang, "LC_MESSAGES",
                        p.splitext(p.basename(pot))[0] + '.mo'
            )

            if not buildcat.do_buildpo(po, pot, "tmp.po"):
                buildcat.do_makedirs(os.path.dirname(mo))
                err_code = os.system(MSGFMT + " -o %s tmp.po" % mo)
                if not err_code: # Success
                    os.remove("tmp.po")
                    sys.stdout.write(".")
                    sys.stdout.flush()
                else:
                    raise RuntimeError(
                        "msgfmt exited with errorcode %i!" % err_code
                    )

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

        print("Compiling translations: ")

        if len(sys.argv) > 1:
                # Assume all parameters are language codes to compile
                lang = sys.argv[1:]
        else:
                # Find every directory that looks like ISO-639
                lang = set(p.splitext(p.basename(l))[0] for
                         l in glob("po/*/*.po"))
                print "all available."

        for l in lang:
                do_compile(l)

        print("")
