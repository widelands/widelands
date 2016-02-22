#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# Tests if mapobjects.json is a valid JSON file

base_path = os.path.abspath(os.path.dirname(__file__))

print("Reading mapinfo from JSON:")

source_path = os.path.normpath(base_path + "/mapobjects.json")
source_file = open(source_path, "r")
dataset = json.load(source_file)
print("Done.")
