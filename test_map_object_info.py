#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# Tests if .json files in data/map_object_info are valid JSON files

base_path = os.path.abspath(os.path.dirname(__file__))

source_path = os.path.normpath(base_path + "/data/map_object_info")

if (not os.path.isdir(source_path)):
	print("Error: Path " + source_path + " not found.")
	sys.exit(1)

source_files = sorted(os.listdir(source_path), key=str.lower)

print("Reading map object info from JSON files in:\n   " + source_path)
failed = 0
for source_filename in source_files:
	file_path = source_path + "/" + source_filename
	if source_filename.endswith(".json"):
		source_file = open(file_path, "r")
		try:
			dataset = json.load(source_file)
		except ValueError as err:
			failed = failed + 1
			print("\n   Error reading " + source_filename + ":");
			print("      " + str(err))

if failed == 0:
	print("\nAll JSON files are OK.")
	sys.exit(0)
else:
	if failed == 1:
		print("\n" + str(failed) + " file is not valid JSON!");
	else:
		print("\n" + str(failed) + " files are not valid JSON!");
	sys.exit(1)
