#!/bin/sh

# ccache clang++ -Qunused-arguments -Wno-unknown-warning-option \
ccache g++-4.7 \
   -isystem /usr/local/include/ \
   $@
