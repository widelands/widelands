#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# This script collects translations for the appdata.xml
# All non-translatable content is taken from ../debian/widelands.appdata.xml.stub
# That file contains a SUMMARY_DESCRIPTION_HOOK where the translatable information
# is inserted.
#
# The translations are sourced from ../debian/translations/
#
# The output is written to ../debian/widelands.appdata.xml

print("Checking files")

base_path = os.path.abspath(os.path.join(os.path.dirname(__file__),os.path.pardir))

input_filename = os.path.normpath(base_path + "/debian/widelands.appdata.xml.stub")
if (not os.path.isfile(input_filename)):
	print("Error: File " + input_filename + " not found.")
	sys.exit(1)

translations_path = os.path.normpath(base_path + "/debian/translations")
if (not os.path.isdir(translations_path)):
	print("Error: Path " + translations_path + " not found.")
	sys.exit(1)

english_source_filename =  os.path.normpath(base_path + "/debian/translations/appdata.json")
if (not os.path.isfile(english_source_filename)):
	print("Error: File " + english_source_filename + " not found.")
	sys.exit(1)

print("Reading translations from JSON:")

english_source_file = open(english_source_filename, "r")
english_source = json.load(english_source_file)

summary_en = english_source["tagline"]
descriptions_en = english_source["description_long"]

summaries = "  <summary>" + summary_en + "</summary>\n"
descriptions = "  <description>\n"
for description in descriptions_en:
	descriptions += "    <p>\n"
	descriptions += "      " + description + "\n"
	descriptions += "    </p>\n"
descriptions += "  </description>\n"

# Each language's translations live in a separate file, so we list the dir
translation_files = sorted(os.listdir(translations_path), key=str.lower)

for translation_filename in translation_files:
	# Only json files, and not the template file please
	if translation_filename.endswith(".json") and translation_filename != "appdata.json":
		translation_file = open(translations_path + "/" + translation_filename, "r")
		translation = json.load(translation_file)
		lang_code = translation_filename[:-5]
		summary = translation["tagline"]
		if summary != summary_en:
			summaries += "  <summary xml:lang=\"" + lang_code + "\">"+ summary +"</summary>\n"
		if translation["description_long"] != descriptions_en:
			descriptions += "  <description xml:lang=\"" + lang_code + "\">\n"
			for description in translation["description_long"]:
				descriptions += "    <p>\n"
				descriptions += "      " + description + "\n"
				descriptions += "    </p>\n"
			descriptions += "  </description>\n"

print("Writing appdata")
input_file = open(input_filename, "r")
appdata = ""

for line in input_file:
	if line.strip() != "SUMMARY_DESCRIPTION_HOOK":
		appdata += line
	else:
		appdata += summaries + descriptions

# print(appdata)

print("Writing widelands.appdata.xml")
dest_filepath = base_path + "/debian/widelands.appdata.xml"
dest_file = codecs.open(dest_filepath, encoding='utf-8', mode='w')
dest_file.write(appdata)
dest_file.close()

print("Done!")
