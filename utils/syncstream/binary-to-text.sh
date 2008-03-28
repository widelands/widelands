#!/bin/sh
# Script to format a .wss binary sync stream dump into hex format.
# Requires hexdump
#
# See also synclog-to-text.sh

hexdump -v -e '"%07_ad " 4/1 " %02x" "\n"'
