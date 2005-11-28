#!/usr/bin/python -tt

"""This programm generates all the gettext files from available translations
This could surely be done with a shell script, but my python abilities are better then
my shell scripting

Usage: Edit the available languages to your need, run this script in the 'locale' directory"""


import os
import sys
import confgettext 
from glob import glob
import string

TRIBES = [ "barbarians" ]
WORLDS = [ "greenland" ]  
CAMPAING_MISSIONS = glob("../campaigns/*.wmf")

def do_rename( src, dst ):
    try:
        os.remove( dst )
    except: 
        "" # do nothing
    os.rename( src, dst )

def do_makedirs( dirs ):
    try: 
        os.makedirs( dirs )
    except:
        "" # nothing
        
def main( ):
    LANGUAGES = extract_languages()
###############################
# here we go: Widelands binarys
###############################
    files = glob("../src/*.cc")
    files += glob("../src/*/*.cc")
    files += glob("../src/*/*/*.cc")
    files += glob("../src/*.h")
    files += glob("../src/*/*.h")
    files += glob("../src/*/*/*.h")
    os.system("xgettext -k_ -o widelands.pot %s" % string.join( files )) 

    for lang in LANGUAGES:
        # merge new strings with existing translations
        if not os.system( "msgmerge widelands_%s.po widelands.pot > tmp" % lang ):
            do_rename("tmp", "widelands_%s.po" % lang )
        
        # compile message catalogs
        do_makedirs( "%s/LC_MESSAGES" % lang )
        os.system( "msgfmt -o %s/LC_MESSAGES/widelands.mo widelands_%s.po" % ( lang, lang ))

##############################
# Tribes
##############################
    for tribe in TRIBES:
        # Get all strings
        files = glob("../tribes/%s/conf" % tribe )
        files += glob("../tribes/%s/*/*/conf" % tribe )
        catalog = confgettext.parse_conf( files )
        file = open( "tribe_%s.pot" % tribe, "w")
        file.write(catalog)
        file.close()

        for lang in LANGUAGES:
            # merge new strings with existing translations
            if not os.system( "msgmerge tribe_%s_%s.po tribe_%s.pot > tmp" % ( tribe, lang, tribe )):
                do_rename("tmp", "tribe_%s_%s.po" % (tribe, lang ) )
            
            # compile message catalogs
            do_makedirs( "%s/LC_MESSAGES" % lang )
            os.system( "msgfmt -o %s/LC_MESSAGES/tribe_%s.mo tribe_%s_%s.po" % ( lang, tribe, tribe, lang ))

##############################
# Worlds
##############################
    for world in WORLDS:
        # Get all strings
        files = glob("../worlds/%s/*conf" % world )
        files += glob("../worlds/%s/*/*/conf" % world )
        catalog = confgettext.parse_conf( files )
        file = open( "world_%s.pot" % world, "w")
        file.write(catalog)
        file.close()

        for lang in LANGUAGES:
            # merge new strings with existing translations
            if not os.system( "msgmerge world_%s_%s.po world_%s.pot > tmp" % ( world, lang, world )):
                do_rename("tmp", "world_%s_%s.po" % ( world, lang ) )

            # compile message catalogs
            do_makedirs( "%s/LC_MESSAGES" % lang )
            os.system( "msgfmt -o %s/LC_MESSAGES/world_%s.mo world_%s_%s.po" % ( lang, world, world, lang ))

##############################
# tutorial campaign 
##############################
    files = []
    for mission in CAMPAING_MISSIONS:
        for file in glob("%s/*" % mission):
            if( file[-3:] == "CVS" or file[-1] == '~' or file[-6:]=="binary" or file[-4:]=="pics"): 
                continue
            files.append( file )

    catalog = confgettext.parse_conf( files )
    file = open( "campaign_ank.pot", "w")
    file.write(catalog)
    file.close()

    for lang in LANGUAGES:
        # merge new strings with existing translations
        if not os.system( "msgmerge campaign_ank_%s.po campaign_ank.pot > tmp" % lang ):
                do_rename("tmp", "campaign_ank_%s.po" % ( lang ) )

        # compile message catalogs
        do_makedirs( "%s/LC_MESSAGES" % lang )
        os.system( "msgfmt -o %s/LC_MESSAGES/campaign_ank.mo campaign_ank_%s.po" % ( lang, lang ))

##############################
# Texts (General help, Readme and so on) 
##############################
    files = [] 
    for file in glob("../txts/*"):
        if( file[-3:] == "CVS" or file[-1] == '~'): 
            continue
        files.append( file )

    catalog = confgettext.parse_conf( files )
    file = open( "texts.pot", "w")
    file.write(catalog)
    file.close()

    for lang in LANGUAGES:
        # merge new strings with existing translations
        if not os.system( "msgmerge texts_%s.po texts.pot > tmp" % lang ):
                do_rename("tmp", "texts_%s.po" % ( lang ) )

        # compile message catalogs
        do_makedirs( "%s/LC_MESSAGES" % lang )
        os.system( "msgfmt -o %s/LC_MESSAGES/texts.mo texts_%s.po" % ( lang, lang ))



# 
# This function extracts the available languages from the source files languages.h
#
def extract_languages(  ):
    lines = []
    extract = False
    for line in (open("../src/languages.h").readlines()):
        if( line.find("EXTRACT BEGIN") != -1 ):
            extract = True
            continue;
        if( line.find("EXTRACT END") != -1 ):
            break;
        if extract:
            lines.append( line.strip(" \n\t\r(,}{"))
      
    retval = []
    for line in lines:
        ( long, abr ) = line.split(',');
        long = long.strip()
        abr = abr.strip();
        abr = abr.strip('"')
        retval.append( abr )
    return retval


if __name__ == "__main__":
    main()
