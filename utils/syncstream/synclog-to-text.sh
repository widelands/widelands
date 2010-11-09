#!/bin/bash
# Extract syncstream log data from a Widelands log file
# and format it like binary-to-text.sh
# to make comparing via diff easier.
#
# WARNING: You must first split the sync.log based on the "[sync] Reset" markers

grep -e \\[sync\:.*t=.*\\] |\
sed "s@.*\\[sync.*\\] @@" | sed "s@\([a-z0-9]*\)@'\\\\x\1'@g" |\
tr " \t" "\n\n" |\
python -c "import sys
for l in sys.stdin:
	sys.stdout.write(eval(l.strip()))
"|\
hexdump -v -e '"%07_ad " 4/1 " %02x" "\n"'
