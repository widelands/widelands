#!/bin/sh

# ccache clang++ -Qunused-arguments -Wno-unknown-warning-option \
ccache g++-4.8 \
   -isystem /usr/local/include/ \
   $@
