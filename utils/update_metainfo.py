#!/usr/bin/env python3
# encoding: utf-8

import subprocess
import codecs
import json
import os.path
import shutil
import sys

# This script collects translations for the metainfo.xml and .desktop files
#
# All non-translatable content for ../xdg/org.widelands.Widelands.metainfo.xml is taken from
# ../xdg/org.widelands.Widelands.metainfo.xml.stub
# That file contains a SUMMARY_DESCRIPTION_HOOK where the translatable information
# is inserted.
# A language list and textdomain info is inserted into LANGUAGES_HOOK
#
# The output is written to ../xdg/org.widelands.Widelands.metainfo.xml
#
# All non-translatable content for ../xdg/org.widelands.Widelands.desktop is taken from
# ../xdg/org.widelands.Widelands.desktop.stub
# That file contains a GENERIC_NAME_COMMENT_HOOK where the translatable information
# is inserted.
# The output is written to ../xdg/org.widelands.Widelands.desktop
#
# All translations are sourced from ../xdg/translations/
#
# If used with the --nonet command line option, the validator for the generated
# metainfo.xml file will be executed with the same, or the equivalent --no-net option.
# This skips the downloading of the screenshot images in restricted environments
# without network access, such as e.g. flatpak-builder.


print('Updating metainfo.xml and .desktop files')
print('- Checking files')

base_path = os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.path.pardir))

metainfo_input_filename = os.path.normpath(
    base_path + '/xdg/org.widelands.Widelands.metainfo.xml.stub')
if (not os.path.isfile(metainfo_input_filename)):
    print('Error: File ' + metainfo_input_filename + ' not found.')
    sys.exit(1)

desktop_input_filename = os.path.normpath(
    base_path + '/xdg/org.widelands.Widelands.desktop.stub')
if (not os.path.isfile(desktop_input_filename)):
    print('Error: File ' + desktop_input_filename + ' not found.')
    sys.exit(1)

translations_path = os.path.normpath(base_path + '/xdg/translations')
if (not os.path.isdir(translations_path)):
    print('Error: Path ' + translations_path + ' not found.')
    sys.exit(1)

english_source_filename = os.path.normpath(
    # metainfo used to be called appdata, that's how it was added on transifex
    base_path + '/xdg/translations/appdata.json')
if (not os.path.isfile(english_source_filename)):
    print('Error: File ' + english_source_filename + ' not found.')
    sys.exit(1)

print('- Reading textdomains:')

textdomain_path = os.path.normpath(
    base_path + '/data/i18n/translations')
textdomains = []
textdomain_path_contents = sorted(os.listdir(textdomain_path), key=str.lower)
for textdomain in textdomain_path_contents:
    if os.path.isdir(os.path.normpath(textdomain_path + '/' + textdomain)):
        textdomains.append(textdomain)

print('- Reading source from JSON:')

english_source_file = open(english_source_filename, 'r')
english_source = json.load(english_source_file)

name_en = english_source['name']
tagline_en = english_source['tagline']
descriptions_en = english_source['description']
generic_name_en = english_source['category']
desktop_name_en = english_source['name']

english_source_file.close()

# For metainfo.xml
names = '  <name>' + name_en + '</name>\n'
summaries = '  <summary>' + tagline_en + '</summary>\n'
descriptions = '  <description>\n'
for description in descriptions_en:
    descriptions += '    <p>\n'
    descriptions += '      ' + description + '\n'
    descriptions += '    </p>\n'

# For .desktop
desktop_names = 'Name=' + desktop_name_en + '\n'
generic_names = 'GenericName=' + generic_name_en + '\n'
comments = 'Comment=' + tagline_en + '\n'

print('- Reading translations from JSON:')

# Each language's translations live in a separate file, so we list the dir
translation_files = sorted(os.listdir(translations_path), key=str.lower)
langcodes = []

for translation_filename in translation_files:
    # Only json files, and not the template file please
    if translation_filename.endswith('.json') and translation_filename != 'appdata.json':
        translation_file = open(
            translations_path + '/' + translation_filename, 'r')
        translation = json.load(translation_file)
        lang_code = translation_filename[:-5]
        langcodes.append(lang_code)
        tagline = translation['tagline']
        if tagline != tagline_en:
            summaries += "  <summary xml:lang=\"" + lang_code + \
                "\">" + tagline + '</summary>\n'  # metainfo.xml
            comments += 'Comment[' + lang_code + ']=' + \
                tagline + '\n'  # .desktop
        generic_name = translation['category']
        if generic_name != generic_name_en:
            # .desktop
            generic_names += 'GenericName[' + \
                lang_code + ']=' + generic_name + '\n'
        if 'name' in translation:
            desktop_name = translation['name']
            if desktop_name != desktop_name_en:
                # .desktop
                desktop_names += 'Name[' + \
                    lang_code + ']=' + desktop_name + '\n'
        # metainfo.xml
        if 'name' in translation and translation['name'] != name_en:
            names += "  <name xml:lang=\"" + lang_code + \
                "\">" + translation['name'] + '</name>\n'
        if translation['description'] != descriptions_en:  # metainfo.xml
            for description in translation['description']:
                descriptions += "    <p xml:lang=\"" + lang_code + "\">\n"
                descriptions += '      ' + description + '\n'
                descriptions += '    </p>\n'
        translation_file.close()
descriptions += '  </description>\n'

print('- Writing org.widelands.Widelands.metainfo.xml')
input_file = open(metainfo_input_filename, 'r')
metainfo = ''

for line in input_file:
    if line.strip() == 'SUMMARY_DESCRIPTION_HOOK':
        metainfo += names + summaries + descriptions
    elif line.strip() == 'LANGUAGES_HOOK':
        metainfo += '  <languages>\n'
        for langcode in langcodes:
            metainfo += '    <lang>' + langcode + '</lang>\n'
        metainfo += '  </languages>\n'
        for textdomain in textdomains:
            metainfo += '  <translation type="gettext">' + textdomain + '</translation>\n'
    else:
        metainfo += line

input_file.close()

metainfo_filepath = base_path + '/xdg/org.widelands.Widelands.metainfo.xml'
dest_file = codecs.open(metainfo_filepath, encoding='utf-8', mode='w')
dest_file.write(metainfo)
dest_file.close()

print('- Writing .desktop')
desktop = ''

with open(desktop_input_filename, 'r', encoding='utf-8') as input_file:
    for line in input_file:
        if line.strip() != 'GENERIC_NAME_COMMENT_HOOK':
            desktop += line
        else:
            desktop += desktop_names + generic_names + comments


desktop_filepath = base_path + '/xdg/org.widelands.Widelands.desktop'
dest_file = codecs.open(desktop_filepath, encoding='utf-8', mode='w')
dest_file.write(desktop)
dest_file.close()

print('Done!')


# Validate the MetaInfo file
skip_screenshot_check = len(sys.argv) > 1 and sys.argv[1] == '--nonet'

if shutil.which('appstreamcli'):
    validate_cmd = ['appstreamcli', 'validate', metainfo_filepath]
    if skip_screenshot_check:
        validate_cmd.insert(2, '--no-net')
    metainfo_result = subprocess.run(validate_cmd)
elif shutil.which('appstream-util'):
    validate_cmd = ['appstream-util', 'validate-relax', metainfo_filepath]
    if skip_screenshot_check:
        validate_cmd.insert(2, '--nonet')
    metainfo_result = subprocess.run(validate_cmd)
else:
    print('Cannot validate generated metainfo file.')
    metainfo_result = FileNotFoundError() # dummy object for returncode
    metainfo_result.returncode = 0

desktop_result = subprocess.run(['desktop-file-validate', desktop_filepath])

sys.exit(max(desktop_result.returncode, metainfo_result.returncode))
