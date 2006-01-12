#!/usr/bin/python -tt
"""./count-longlines.py TABWIDTH 
(called from main widelands directory)

Count lines that are too long. One TAB (ASCII 0x09) character
is assumed to expand to TABWIDTH spaces"""

import fnmatch
import os
import string
import fileinput
import sys

def find(root, glob):
	files=[]
	for file in os.listdir(root):
		file=os.path.join(root, file)
		if fnmatch.fnmatch(file, glob):
			files.append(file)
		if os.path.isdir(file):
			files+=find(file, glob)
	return files

files =find(".", "*.h")
files+=find(".", "*.cc")

lines=0
shortlines=0
longlines=0
oversizelines=0
tabwidth=int(sys.argv[1])

for line in fileinput.input(files):
	line=line.expandtabs(tabwidth).rstrip()

	lines+=1
	if len(line)<=80:
		shortlines+=1
	else:
		if len(line)=84:
			longlines+=1
		else:
			oversizelines+=1

print "Total lines: %i" % (lines,)

print "At %i characters tabwidth:\n" % (tabwidth,)
print "Short lines    <=80 characters: %6i" % (shortlines,)
print "Long lines     <=84 characters: %6i" % (longlines,)
print "Oversize lines > 84 characters: %6i" % (oversizelines,)

