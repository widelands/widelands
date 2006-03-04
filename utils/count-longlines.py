#!/usr/bin/python -tt
"""./count-longlines.py 
(called from main widelands directory)

Count lines that are too long to fit on 80 character screens
when using different tabwidths."""

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

for line in fileinput.input(files):
	line3=line.expandtabs(3).rstrip()
	line8=line.expandtabs(8).rstrip()

	lines+=1
	if len(line3)<=80:
		shortlines+=1
	else:
		if len(line8)<=80:
			longlines+=1
		else:
			oversizelines+=1

print "Total lines: %i\n" % (lines,)

print "Short lines    =<80 @ tabwidth=8: %6i   %6.2f%%" % (shortlines, 100.0*shortlines/lines)
print "Long lines     =<80 @ tabwidth=3: %6i   %6.2f%%" % (longlines, 100.0*longlines/lines)
print "Oversize lines  >80 @ tabwidth=3: %6i   %6.2f%%" % (oversizelines, 100.0*oversizelines/lines)

