#!/usr/bin/env python
# encoding: utf-8

import codecs
import json
import os.path
import sys

# This script collects translations for the appdata.xml and .desktop files
#
# All non-translatable content for ../debian/widelands.appdata.xml is taken from
# ../debian/widelands.appdata.xml.stub
# That file contains a SUMMARY_DESCRIPTION_HOOK where the translatable information
# is inserted.
# The output is written to ../debian/widelands.appdata.xml
#
# All non-translatable content for ../debian/org.widelands.widelands.desktop is taken from
# ../debian/org.widelands.widelands.desktop.stub
# That file contains a GENERIC_NAME_COMMENT_HOOK where the translatable information
# is inserted.
# The output is written to ../debian/org.widelands.widelands.desktop
#
# All translations are sourced from ../debian/translations/


print('Updating appdata.xml and .desktop files')
print('- Checking files')

base_path = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.path.pardir))

appdata_input_filename = os.path.normpath(
    base_path + '/debian/widelands.appdata.xml.stub')
if (not os.path.isfile(appdata_input_filename)):
    print('Error: File ' + appdata_input_filename + ' not found.')
    sys.exit(1)

desktop_input_filename = os.path.normpath(
    base_path + '/debian/org.widelands.widelands.desktop.stub')
if (not os.path.isfile(desktop_input_filename)):
    print('Error: File ' + desktop_input_filename + ' not found.')
    sys.exit(1)

translations_path = os.path.normpath(base_path + '/debian/translations')
if (not os.path.isdir(translations_path)):
    print('Error: Path ' + translations_path + ' not found.')
    sys.exit(1)

english_source_filename = os.path.normpath(
    base_path + '/debian/translations/appdata.json')
if (not os.path.isfile(english_source_filename)):
    print('Error: File ' + english_source_filename + ' not found.')
    sys.exit(1)

print('- Reading source from JSON:')

english_source_file = open(english_source_filename, 'r')
english_source = json.load(english_source_file)

tagline_en = english_source['tagline']
descriptions_en = english_source['description']
generic_name_en = english_source['category']

english_source_file.close()

# For appdata.xml
summaries = '  <summary>' + tagline_en + '</summary>\n'
descriptions = '  <description>\n'
for description in descriptions_en:
    descriptions += '    <p>\n'
    descriptions += '      ' + description + '\n'
    descriptions += '    </p>\n'

# For .desktop
generic_names = 'GenericName=' + generic_name_en + '\n'
comments = 'Comment=' + tagline_en + '\n'

print('- Reading translations from JSON:')

# Each language's translations live in a separate file, so we list the dir
translation_files = sorted(os.listdir(translations_path), key=str.lower)

for translation_filename in translation_files:
    # Only json files, and not the template file please
    if translation_filename.endswith('.json') and translation_filename != 'appdata.json':
        translation_file = open(
            translations_path + '/' + translation_filename, 'r')
        translation = json.load(translation_file)
        lang_code = translation_filename[:-5]
        tagline = translation['tagline']
        if tagline != tagline_en:
            summaries += "  <summary xml:lang=\"" + lang_code + \
                "\">" + tagline + '</summary>\n'  # appdata.xml
            comments += 'Comment[' + lang_code + ']=' + \
                tagline + '\n'  # .desktop
        generic_name = translation['category']
        if generic_name != generic_name_en:
            # .desktop
            generic_names += 'GenericName[' + \
                lang_code + ']=' + generic_name + '\n'
        if translation['description'] != descriptions_en:  # appdata.xml
            for description in translation['description']:
                descriptions += "    <p xml:lang=\"" + lang_code + "\">\n"
                descriptions += '      ' + description + '\n'
                descriptions += '    </p>\n'
        translation_file.close()
descriptions += '  </description>\n'

print('- Writing widelands.appdata.xml')
input_file = open(appdata_input_filename, 'r')
appdata = ''

for line in input_file:
    if line.strip() != 'SUMMARY_DESCRIPTION_HOOK':
        appdata += line
    else:
        appdata += summaries + descriptions

input_file.close()

dest_filepath = base_path + '/debian/widelands.appdata.xml'
dest_file = codecs.open(dest_filepath, encoding='utf-8', mode='w')
dest_file.write(appdata)
dest_file.close()

print('- Writing .desktop')
input_file = open(desktop_input_filename, 'r')
desktop = ''

for line in input_file:
    if line.strip() != 'GENERIC_NAME_COMMENT_HOOK':
        desktop += line
    else:
        desktop += generic_names + comments

input_file.close()

dest_filepath = base_path + '/debian/org.widelands.widelands.desktop'
dest_file = codecs.open(dest_filepath, encoding='utf-8', mode='w')
dest_file.write(desktop)
dest_file.close()

print('Done!')

from subprocess import call
call(['appstreamcli', 'validate', base_path + '/debian/widelands.appdata.xml'])
