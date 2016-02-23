#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# Tests if mapobjects.json is a valid JSON file

base_path = os.path.abspath(os.path.dirname(__file__))

source_path = os.path.normpath(base_path + "/data/mapobject_info")

if (not os.path.isdir(source_path)):
	print("Error: Path " + source_path + " not found.")
	sys.exit(1)

source_files = sorted(os.listdir(source_path), key=str.lower)

print("Reading mapinfo from JSON:")
for source_filename in source_files:
	file_path = source_path + "/" + source_filename
	if source_filename.endswith(".json"):
		print("- Reading " + file_path)
		source_file = open(file_path, "r")
		dataset = json.load(source_file)

print("Done.")
