#!/usr/bin/python -tt

import sys
import os

expr = sys.argv[1]
files = sys.argv[2:]

for file in files:
    os.system("cat '%s' | sed '%s' > /tmp/tmpfile" % ( file, expr ));
    os.system("mv /tmp/tmpfile '%s'" % file );

