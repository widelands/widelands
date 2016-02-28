#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# Tests if .json files in data/map_object_info are valid JSON files

def validate_files_in_path(source_path):

	if (not os.path.isdir(source_path)):
		print("Error: Path " + source_path + " not found.")
		sys.exit(1)

	source_files = sorted(os.listdir(source_path), key=str.lower)

	print("Reading JSON files in:\n   " + source_path)
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
	else:
		if failed == 1:
			print("\n" + str(failed) + " file is not valid JSON!");
		else:
			print("\n" + str(failed) + " files are not valid JSON!");
	return failed < 1

base_path = os.path.abspath(os.path.join(os.path.dirname(__file__),os.path.pardir))

success = validate_files_in_path(os.path.normpath(base_path + "/data/map_object_info"))
success = success and validate_files_in_path(os.path.normpath(base_path + "/data/txts"))

if success:
	sys.exit(0)
else:
	sys.exit(1)
