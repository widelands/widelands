#!/usr/bin/python -tt

"""This proggy parses given files for occurences of translatable strings
if some are found they are output in a xgettext style file"""

import sys
import os
from time import strftime,gmtime

time_now = gmtime()
### Set the header to something sensible, a much as is possible here.
head =  "# Widelands PATH/TO/FILE.PO\n"
head += "# Copyright (C) 2005-" + strftime("%Y", time_now) + " Widelands Development Team\n"
head += "# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n"
head += "#\n"
head += "msgid \"\"\n"
head += "msgstr \"\"\n"
head += "\"Project-Id-Version: Widelands svnVERSION\\n\"\n"
head += "\"Report-Msgid-Bugs-To: widelands-public@lists.sourceforge.net\\n\"\n"
head += "\"POT-Creation-Date: " + strftime("%Y-%m-%d %H:%M+0000", time_now) + "\\n\"\n"
head += "\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
head += "\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
head += "\"Language-Team: LANGUAGE <widelands-public@lists.sourceforge.net>\\n\"\n"
head += "\"MIME-Version: 1.0\\n\"\n"
head += "\"Content-Type: text/plain; charset=UTF-8\\n\"\n"
head += "\"Content-Transfer-Encoding: 8bit\\n\"\n"
head += "\n"

class occurences: 
    def __init__( self, file, line ):
        self.line = line
        self.file = file
        
class trans_string:
    def __init__( self ):
        self.occurences = []
        self.str = ""

def is_multiline( str ):
    l = str.find('""')
    if( l == -1 ):
        return False
    return True

def firstl( str, what ):
    for c in what:
        index = str.find( c )
        if( index != -1 ):
            return index
    return -1

def firstr( str, what ):
    for c in what:
        index = str.rfind( c )
        if( index != -1 ):
            return index
    return -1

def append_string( known_strings, array, string ):
    if known_strings.has_key( string.str ):
        i = known_strings[string.str]
        array[i].occurences += string.occurences 
    else:
        array.append( string )
        known_strings[string.str] = len( array ) - 1
    
def parse_conf( files ):
    translatable_strings = []
    known_strings = {}
    curstr = 0
    multiline = 0
    for file in files:
        lines = open( file, 'r' ).readlines();
        for i in range(0,len(lines)):
            line = lines[i].rstrip('\n')
            linenr = i+1
            curstr = 0
            
            if multiline and  len(line) and line[0]=='_':
                line = line[1:]
                rindex = line.rfind('""') 
                if rindex == -1 or line[:2] == '""':
                    line = line.strip()
                    line = line.strip('"')
                    curstr = trans_string()
                    curstr.str = line
                    curstr.occurences.append( occurences( file, linenr )) 
                    append_string( known_strings, translatable_strings, curstr )
                    continue
                else:
                    line = line[:(rindex+1)]
                    line = line.strip()
                    line = line.strip('"')
                    curstr = trans_string()
                    curstr.str = line
                    curstr.occurences.append( occurences( file, linenr )) 
                    append_string( known_strings, translatable_strings, curstr )
                    multiline = False
                    continue

            index = line.find( "=_" )
            if( index > 0 ):
                curstr = trans_string()
                curstr.occurences.append( occurences( file, linenr )) 
                restline = line[index+2:]
                multiline = is_multiline( restline );
                if multiline: # means exactly one "
                    index = firstl( restline, '"\'')
                    restline = restline[index+1:]
                    restline = restline.strip()
                    restline = restline.strip('"')
                    curstr.str += '"' + restline + '"\n'
                    continue
                # Is not multiline
                # If there are ' or " its easy
                l = firstl( restline, '"\'')
                r = firstr( restline[l+1:], '"\'')
                if( l != -1 and r != -1 ):
                    restline = restline[l+1:]
                    r = firstr( restline, '"\'')
                    restline = restline[:r] 
                else:
                    # Remove comments
                    rindex = max( restline.rfind('#'), restline.rfind('//'))
                    if rindex != -1:
                        restline = restline[:rindex]
                    # And strip
                    restline = restline.strip()
                curstr.str = restline
                append_string( known_strings, translatable_strings, curstr )

    translatable_strings.sort( lambda str1,str2: cmp(str1.occurences[0].file,str2.occurences[0].file) ) 

    retval = head
    for i in translatable_strings:
        for occ in i.occurences:
            retval += "#: %s:%i\n" % ( occ.file, occ.line )

        # escape the escapable characters 
        i.str = i.str.replace('\\', '\\\\')
        i.str = i.str.replace('"', '\\"')
        if( i.str.find('\n') == -1 ):
            retval += 'msgid "%s"\n' % i.str
        else:
            retval += 'msgid ""\n%s' % i.str
        retval += 'msgstr ""\n\n'
    return retval 


#: ../src/ui/ui_fs_menus/fullscreen_menu_main.cc:41
#msgid "Single Player"
#msgstr ""

if __name__ == "__main__":
    if( sys.argv < 2 ):
        print "Usage %s <conf file> [ <conf file> ... ]" % sys.argv[0]

    print parse_conf( sys.argv[1:] )
