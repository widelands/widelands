#! /usr/bin/python -tt

#  THIS PROGRAM IS DEPRECATED. USE whitespace_checker INSTEAD!"
#
#  Detect lines with too deep indentation. A line is not allowed to be indented
#  more than 1 level deeper than the previous line.
#
#  There is another special case that allows a parameter list to be indented 2
#  levels deeper than the previous line (which contains the function
#  identifier). This special case was added because Nicolai writes code like
#  that.

import sys

file = open(sys.argv[1], "r")
line_number = 0
indentation_depth_previous_line = 0
for line in file:
	line_number += 1
	indentation_depth = 0
	for char in line:
		if '\n' == char: #  the line is empty, skip it
			break
		if '#'  == char: #  the line is a macro, skip it
			break
		if '\t' == char:
			indentation_depth += 1
		else:

			#  Allow special case that parameter lists can be indented 2 levels
			#  deeper than the previous line, because Nicolai does that.
			if '(' == char:
				allowed_incr = 2
			else:
				allowed_incr = 1

			if indentation_depth > indentation_depth_previous_line + allowed_incr:
				print "%s:%u: indentation is too deep" % (sys.argv[1], line_number)

			indentation_depth_previous_line = indentation_depth
			break
