#!/usr/bin/env python
# encoding: utf-8

import json
import os
import os.path
import sys
from pprint import pprint # NOCOM for testing

"""
This script collects translator credits from the JSON files in
../txts/translators and writes them into a lua table.
"""

translators_path = os.path.normpath("../txts/translators")

if (not os.path.isdir(translators_path)):
	print("Error: Path " + translators_path + " not found.")
	sys.exit(1)

source_files = os.listdir(translators_path)
lua_string = "translators = {"

for source_filename in source_files:
	if source_filename.endswith(".json") and source_filename != "translators.json":
		source_file = open(translators_path + "/" + source_filename, "r")
		translators = json.load(source_file)
		#pprint(translators)
		print("==========")
		lua_string += "{"
		lua_string += "\"" + translators["locale-translators"]["your-language-name"] + "\","
		lua_string += "{"
		for transl_name in translators["locale-translators"]["translator-list"].split("\n"):
			print(transl_name);
			lua_string += "\"" + transl_name + "\","
		#print(translators["locale-translators"]["your-language-name"])
		#print(translators["locale-translators"]["translator-list"])
		lua_string += "}"
		lua_string += "},"

lua_string += "}"
#print(lua_string)

dest_filename = "translators.lua"
dest_filepath = translators_path + "/" + dest_filename
dest_file = open(dest_filepath, 'w')
dest_file.write(lua_string)
