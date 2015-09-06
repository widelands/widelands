#!/usr/bin/env python
# encoding: utf-8

## NOCOM(GunChleoc): This is to merge the tribes templates. Remove when this branch has been merged.
## Call utils/merge_and_push_translations.sh directly before merging the branch!!!!
## Don't forget to create the stub for "help" in Transifex.

from glob import glob
from itertools import takewhile
import os
import string
import subprocess
import sys

from lua_xgettext import Lua_GetText
import confgettext


base_path = os.path.abspath(os.path.join(os.path.dirname(__file__),os.path.pardir))

dst_path = os.path.normpath(base_path + "/po/tribes/")
pot = os.path.normpath(base_path + "/po/tribes/tribes.pot")

source_path = os.path.normpath(base_path + "/po/tribe_atlanteans")
source_files = sorted(os.listdir(source_path), key=str.lower)

for po in source_files:
	if po.endswith(".po"):
		os.system("msgmerge -q --no-wrap %s %s -o %s" % ((source_path + "/" + po), pot, (dst_path + "/" + po)))

source_path = os.path.normpath(base_path + "/po/tribe_barbarians")
source_files = sorted(os.listdir(source_path), key=str.lower)

for po in source_files:
	if po.endswith(".po"):
		os.system("msgmerge -q --no-wrap %s %s -o %s" % ((source_path + "/" + po), pot, (dst_path + "/" + po)))

source_path = os.path.normpath(base_path + "/po/tribe_empire")
source_files = sorted(os.listdir(source_path), key=str.lower)

for po in source_files:
	if po.endswith(".po"):
		os.system("msgmerge -q --no-wrap %s %s -o %s" % ((source_path + "/" + po), pot, (dst_path + "/" + po)))
