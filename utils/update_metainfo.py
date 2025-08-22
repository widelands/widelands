#!/usr/bin/env python3
# encoding: utf-8

import subprocess
import codecs
import json
import os.path
import shutil
import sys
import re
import datetime

# This script collects translations for the metainfo.xml and .desktop files
#
# All non-translatable content for ../xdg/org.widelands.Widelands.metainfo.xml is taken from
# ../xdg/org.widelands.Widelands.metainfo.xml.stub
# That file contains a SUMMARY_DESCRIPTION_HOOK where the translatable information
# is inserted.
# A language list and textdomain info is inserted into LANGUAGES_HOOK
#
# Release notes and their translations are inserted for the last three releases.
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

# Not critical, we only need its last modification time here.
release_notes_filename = os.path.normpath(base_path + '/Release_Notes.md')

english_source_filename = os.path.normpath(
    # metainfo used to be called appdata, that's how it was added on transifex
    base_path + '/xdg/translations/appdata.json')
if (not os.path.isfile(english_source_filename)):
    print('Error: File ' + english_source_filename + ' not found.')
    sys.exit(1)

release_urls_filename = os.path.normpath(
    base_path + '/xdg/release_urls.json')
if (not os.path.isfile(release_urls_filename)):
    print('Error: File ' + release_urls_filename + ' not found.')
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

release_urls_file = open(release_urls_filename, 'r', encoding = 'utf-8')
release_urls = json.load(release_urls_file)
release_urls_file.close()

english_source_file = open(english_source_filename, 'r', encoding = 'utf-8')
english_source = json.load(english_source_file)
english_source_file.close()

name_en = english_source['name']
tagline_en = english_source['tagline']
descriptions_en = english_source['description']
generic_name_en = english_source['category']
desktop_name_en = english_source['name']
developer_en = english_source['developer']
release_notes_en = {}
if 'release_notes' in english_source:
    release_notes_en = english_source['release_notes']

# For metainfo.xml
names = '  <name>' + name_en + '</name>\n'
summaries = '  <summary>' + tagline_en + '</summary>\n'
developer = '    <name>' + developer_en + '</name>\n'
descriptions = '  <description>\n'
for description in descriptions_en:
    descriptions += '    <p>\n'
    descriptions += '      ' + description + '\n'
    descriptions += '    </p>\n'
release_notes = {}
for release in release_notes_en.keys():
    release_notes[release] = '      <description><ul>\n'
    for highlight in release_notes_en[release]:
        release_notes[release] += '        <li>' + highlight +'</li>\n'

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
            translations_path + '/' + translation_filename, 'r', encoding = 'utf-8')
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
        if 'developer' in translation and translation['developer'] != developer_en:
            developer_lang = translation['developer']
            developer += f'    <name xml:lang="{ lang_code }">{ developer_lang }</name>\n'
        if translation['description'] != descriptions_en:  # metainfo.xml
            for description in translation['description']:
                descriptions += "    <p xml:lang=\"" + lang_code + "\">\n"
                descriptions += '      ' + description + '\n'
                descriptions += '    </p>\n'
        if 'release_notes' in translation:
            trans_relnotes = translation['release_notes']
            for release in release_notes_en.keys():
                if release in trans_relnotes and \
                   trans_relnotes[release] != release_notes_en[release]:
                    for highlight in trans_relnotes[release]:
                        release_notes[release] += f'        <li xml:lang="{ lang_code }">{ highlight }</li>\n'
        translation_file.close()

descriptions += '  </description>\n'
for release in release_notes_en.keys():
    release_notes[release] += '      </ul></description>\n'

print('- Writing org.widelands.Widelands.metainfo.xml')
input_file = open(metainfo_input_filename, 'r', encoding = 'utf-8')
metainfo = ''

dev_version = open(os.path.join(base_path, 'NEXT_STABLE_VERSION')).read().strip()

release_regex = re.compile(r'^ *<release[^>]* version="([0-9.A-Za-z~@ _-]*)"/>$')
releases_processed = 0
if len(release_notes) == 0:
    releases_processed = -1

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
    elif line.strip() == 'DEVELOPER_HOOK':
        metainfo += developer
    elif releases_processed < len(release_notes) and release_regex.match(line):
        version = release_regex.match(line).group(1)
        if version.startswith(dev_version + '~'):
            if releases_processed > 0:
                print('Development version is not first one in metainfo stub!')
                sys.exit(1)
            version = dev_version
            if not ' date=' in line:
                # Date is not in stub to not have to update it all the time, but we need something
                # stable to avoid churn in git, so we use the date of the last modification of
                # Release_Notes.md. We will use the current date as fallback value if the detection
                # fails.
                date = datetime.date.today().isoformat()
                result = subprocess.run( \
                    ('git log -1 --format=%ci -- ' + release_notes_filename).split(), \
                    text=True, encoding='utf-8', capture_output=True)
                if result.returncode == 0:
                    date = result.stdout[:10]
                idx = line.find('version=')
                line = line[:idx] + f'date="{ date }" ' + line[idx:]
        if len(release_notes) > 0:
            metainfo += line.rstrip()[:-2] + '>\n'  # replace '/>' with '>'
            if not version in release_notes:
                print('<releases> in metainfo stub are not in sync with Release_Notes.md!')
                sys.exit(1)
            metainfo += release_notes[version]
            if version in release_urls:
                metainfo += f'      <url type="details">{ release_urls[version] }</url>\n'
            else:
                print('WARNING: No URL for version ' + version)
            metainfo += '    </release>\n'
        else:
            metainfo += line
        releases_processed += 1
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
