#!/usr/bin/env python
# encoding: utf-8

## NOCOM(GunChleoc): This is to merge the tribes templates. Remove when this branch has been merged.
## Call utils/merge_and_push_translations.sh directly before merging the branch!!!!
## Don't forget to create the stub for "tribes_encyclopedia" in Transifex.
## We will merge one tribe and one destination template at a time.

from glob import glob
from itertools import takewhile
import os
import string
import subprocess
import sys

from lua_xgettext import Lua_GetText
import confgettext

base_path = os.path.abspath(os.path.join(os.path.dirname(__file__),os.path.pardir))

## Encyclopedia
dst_path = os.path.normpath(base_path + "/po/tribes_encyclopedia/")
pot = os.path.normpath(base_path + "/po/tribes_encyclopedia/tribes_encyclopedia.pot")

#source_path = os.path.normpath(base_path + "/po/tribes")
source_path = os.path.normpath(base_path + "/po/tribe_atlanteans")
#source_path = os.path.normpath(base_path + "/po/tribe_barbarians")
#source_path = os.path.normpath(base_path + "/po/tribe_empire")

## Tribes
#dst_path = os.path.normpath(base_path + "/po/tribes/")
#pot = os.path.normpath(base_path + "/po/tribes/tribes.pot")

#source_path = os.path.normpath(base_path + "/po/tribe_atlanteans")
#source_path = os.path.normpath(base_path + tribes.pottribes.pot"/po/tribe_barbarians")
#source_path = os.path.normpath(base_path + "/po/tribe_empire")

source_files = sorted(os.listdir(source_path), key=str.lower)
for po in source_files:
	if po.endswith(".po"):
		os.system("msgmerge -q --no-wrap %s %s -o %s" % ((source_path + "/" + po), pot, (dst_path + "/" + po)))
