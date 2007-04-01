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
	line2=line.expandtabs(2).rstrip()
	line8=line.expandtabs(8).rstrip()

	lines+=1
	if len(line8)>80:
		if len(line2)>80:
			longlines+=1
		else:
			oversizelines+=1
	else:
		shortlines+=1

print
print "Total lines:                         %7i   %6.2f%%" % (lines, 100.0)
print "Short lines        <80 @ tabwidth=8: %7i   %6.2f%%" % (shortlines, 100.0*shortlines/lines)
print "Long lines         >80 @ tabwidth=8: %7i   %6.2f%%" % (longlines, 100.0*longlines/lines)
print
print "OVERSIZE LINES !!  >80 @ tabwidth=2: %7i   %6.2f%%" % (oversizelines, 100.0*oversizelines/lines)
print

