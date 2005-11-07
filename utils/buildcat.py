#!/usr/bin/python -tt

"""This programm generates all the gettext files from available translations
This could surely be done with a shell script, but my python abilities are better then
my shell scripting

Usage: Edit the available languages to your need, run this script in the 'locale' directory"""

# TODO: Scenarios and Campaigns
# TODO: General Help

LANGUAGES = [ "de_DE" ]  # Add your new language here

#################################################################
# NO MORE CHANGES BELOW
#################################################################

TRIBES = [ "barbarians" ]
WORLDS = [ "greenland" ]  

import os
import sys
import confgettext 
from glob import glob

###############################
# here we go: Widelands binarys
###############################
os.system("xgettext -k_ -o widelands.pot ../src/*.cc ../src/*/*.cc ../src/*/*/*.cc")

for lang in LANGUAGES:
    # merge new strings with existing translations
    if not os.system( "msgmerge widelands_%s.po widelands.pot > tmp" % lang ):
        os.system( "mv tmp widelands_%s.po" % lang ) 

    # compile message catalogs
    os.system( "mkdir -p %s/LC_MESSAGES" % lang )
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

    for lang in LANGUAGES:
        # merge new strings with existing translations
        if not os.system( "msgmerge tribe_%s_%s.po tribe_%s.pot > tmp" % ( tribe, lang, tribe )):
            os.system( "mv tmp tribe_%s_%s.po" % ( tribe, lang) ) 
        
        # compile message catalogs
        os.system( "mkdir -p %s/LC_MESSAGES" % lang )
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

    for lang in LANGUAGES:
        # merge new strings with existing translations
        if not os.system( "msgmerge world_%s_%s.po world_%s.pot > tmp" % ( world, lang, world )):
            os.system( "mv tmp world_%s_%s.po" % ( world, lang) ) 

        # compile message catalogs
        os.system( "mkdir -p %s/LC_MESSAGES" % lang )
        os.system( "msgfmt -o %s/LC_MESSAGES/world_%s.mo world_%s_%s.po" % ( lang, world, world, lang ))

    

     

    
