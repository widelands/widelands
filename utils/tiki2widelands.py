#!/usr/bin/python -tt

"""This script takes as arguments a list of wideland pages and file names
fetches the pages from the widelands tiki and converts them into widelands
texts. This is used to keep readme, authors and help files in the source
up2date so that only the homepage needs to be maintained (and this can be done
by more people then CVS

The other way round (code->Web would be really nice to have for example for the
Changelog for coders who do not have web access at their work station). But for
the moment this way round is more import"""

import os
import sys

# Where does the wiki live?
TIKI_ROOT="http://wiki.widelands.org/"

# Private functions to help parsing,
# also splits the string into an array
# of lines
def __remove_heading( str ):
    lines = str.split('\r\n')
    for i in range(0,len(lines)):
        line = lines[i]
        if( line == ""):
            break;

    while( lines[i] == ""):
        i += 1

    return lines[i:] 


#
# Simple class which is derivated from string
# and used as text. It knows how to parse the tikiwiki syntax
# 
# The following tags are currently implemented:
#  - all sorts of headings (! !! !!! and so on)
#  - lists up to three (*,**,***)
# 
class Widelands_Text( str ):
    def __init__( self, title ):
        self.__mystr = ""
        self.__title = title
        self.__in_para = False

    def __add__( self, str ):
        self.begin_paragraph()

        self.__mystr += str
        return self

    def __str__( self ):
        retval = '# Done by tiki2widelands.py\n\ntitle=_ %s\ntext=_ "%s"\n' % ( self.__title, self.__mystr )
        return retval

    def __parse_inline_tags( self, str ):
        print "TODO!!"
        self.__mystr += str
        
    # Make sure we're in a paragraph or end one if needed
    def begin_paragraph( self ):
        if( self.__in_para ):
            return

        self.__mystr += '"<p>"\n'
        self.__in_para = True

    def end_paragraph( self ):
        if not self.__in_para:
            return
        self.__mystr += '"</p>"\n'
        self.__in_para = False

#
# Parse and make headlines
#
    def heading( self, line ):
        self.end_paragraph()
        
        if( line[:6] == '!!!!!!' ):
            line = line[6:]
            align = "left"
            color = "f43131"
            decoration = "underline"
            fontsize = 14
        if( line[:5] == '!!!!!' ):
            line = line[5:]
            fontsize = 16
            align = "left"
            color = "f4a131"
            decoration = "underline"
        if( line[:4] == '!!!!' ):
            line = line[4:]
            fontsize = 18
            align = "left"
            color = "2F9131"
            decoration = "underline"
        if( line[:3] == '!!!' ):
            line = line[3:]
            fontsize = 20
            align = "center"
            color = "f43131"
            decoration = "none"
        if( line[:2] == '!!' ):
            line = line[2:]
            fontsize = 24
            align = "center"
            color = "f4a131"
            decoration = "none"
        if( line[:1] == '!' ):
            line = line[1:]
            fontsize = 28
            align = "center"
            color = "2F9131"
            decoration = "none"
        
        # remove +/-
        if( len( line ) and ( line[0] == '+' or line[0] == '-') ):
            line = line[1:]

        self.__parse_inline_tags( """"<p font-size=%i font-decoration=%s text-align=%s font-face=Domestic_Manners font-color=%s>%s</p>"\n""" 
                % ( fontsize, decoration, align, color, line ) )
            
    def list( self, line ):
        self.end_paragraph()
        
        pic = ""
        if( line[:3] == '***' ):
            line = line[3:]
            pic = "third"
        if( line[:2] == '**' ):
            line = line[2:]
            pic = "second"
        if( line[:1] == '*' ):
            line = line[1:]
            pic = "first"
        
        self.__parse_inline_tags( """"<p image=pics/list_%s_entry.png image-align=left text-align=left font-size=12>%s</p>"\n""" % ( pic, line ))
            
  
# 
# This function takes a tiki page as a string and
# converts it to a widelands parsable page
# 
def tiki_to_widelands( tikipage ):
    
    # Remove all unneeded nonsense
    lines = __remove_heading( tikipage )

    retval = Widelands_Text( "Development" )
    # Parse for known tags
    for line in lines:
        # parse for paragraphs of their own
        if( len(line) and line[0] == '!'):
            retval.heading( line )
        elif( len(line) and line[0] == '*'):
            retval.list( line )
        else:
            # TODO: trim this line down if it is too long
            retval += '"' + line + '<br>"\n'
    retval.end_paragraph()

    print retval 


if __name__ == "__main__":
    tiki_to_widelands( open("Developers").read() )
