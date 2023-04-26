#!/usr/bin/env python
# encoding: utf-8

"""Runs a glossary check on all po files and writes the check results to
po_validation/glossary.

You will need to have the Translate Toolkit installed in order for the checks to work:
http://toolkit.translatehouse.org/

This script also uses hunspell to reduce the number of false positive hits, so
install as many of the needed hunspell dictionaries as you can find. This script
will inform you about missing hunspell locales.

For Debian-based Linux: sudo apt-get install translate-toolkit hunspell hunspell-ar hunspell-bg hunspell-br hunspell-ca hunspell-cs hunspell-da hunspell-de-de hunspell-el hunspell-en-ca hunspell-en-gb hunspell-en-us hunspell-eu hunspell-fr hunspell-gd hunspell-gl hunspell-he hunspell-hr hunspell-hu hunspell-it hunspell-ko hunspell-lt hunspell-nl hunspell-no hunspell-pl hunspell-pt-br hunspell-pt-pt hunspell-ro hunspell-ru hunspell-si hunspell-sk hunspell-sl hunspell-sr hunspell-sv hunspell-uk hunspell-vi

You will need to provide an export of the Transifex glossary and specify it at
the command line. Make sure to select "Include glossary notes in file" when
exporting the csv from Transifex.

Translators can 'misuse' their languages' comment field on Transifex to add
inflected forms of their glossary translations. We use the delimiter '|' to
signal that the field has inflected forms in it. Examples:

Source    Translation    Comment             Translation will be matched against
------    -----------    ----------------    -----------------------------------
sheep     sheep          Nice, fluffy!       'sheep'
ax        axe            axes|               'axe', 'axes'
click     click          clicking|clicked    'click', 'clicking', 'clicked'
click     click          clicking | clicked  'click', 'clicking', 'clicked'
"""

import os.path
import re
import subprocess
import sys
import time
import traceback
from file_utils import read_csv_file, make_path, delete_path

#############################################################################
# Data Containers                                                           #
#############################################################################


class GlossaryEntry:
    """An entry in our parsed glossaries."""

    def __init__(self):
        # Base form of the term, followed by any inflected forms
        self.terms = []
        # Base form of the translation, followed by any inflected forms
        self.translations = []


class FailedTranslation:
    """Information about a translation that failed a check."""

    def __init__(self):
        # The locale where the check failed
        self.locale = ''
        # The po file containing the failed translation
        self.po_file = ''
        # Source text
        self.source = ''
        # Target text
        self.target = ''
        # Location in the source code
        self.location = ''
        # The glossary term that failed the check
        self.term = ''
        # The base form of the translated glossary term
        self.translation = ''


PREFERRED_HUNSPELL_LOCALES = {
    'ar': 'ar_EG',
    'ca': 'ca_ES',
    'de': 'de_DE',
    'en': 'en_US',
    'es': 'es_ES',
    'eu': 'eu_ES',
    'fr': 'fr_FR',
    'it': 'it_IT',
    'kk': 'kk_KZ',
    'ko': 'ko_KR',
    'nl': 'nl_NL',
    'pt': 'pt_PT',
    'sr': 'sr_Latn_RS',
    'sv': 'sv_SE',
    'sw': 'sw_KE',
}

HUNSPELL_LOCALES = {}

""" Hunspell needs country-specific locales"""


#############################################################################
# Glossary Loading                                                          #
#############################################################################

def get_hunspell_locale(locale):
    """Returns the corresponding Hunspell locale for this locale, or empty
    string if not available."""
    if locale in HUNSPELL_LOCALES.keys():
        return HUNSPELL_LOCALES[locale]
    return ''


def load_hunspell_locales():
    """Registers locales for Hunspell.

    Maps a list of generic locales to specific locales from available
    Hunspell dictionaries.
    """

    try:
        hunspell_result = subprocess.run(
            ['hunspell', '-D'], capture_output=True, text=True, check=True)
        hunspell_result.check_returncode()

        # Track added locales so that we can print out a hint to add to preferred list if we have many
        new_hunspell_locales = {}

        # Dictionary information is printed to stderr
        for line in sorted(hunspell_result.stderr.split('\n')):
            # Discard text output that is not a file prefix
            if os.path.isfile(line + '.dic') or os.path.isfile(line + '.aff'):
                full_locale = os.path.basename(line)
                language = full_locale.split('_')[0]

                if language in PREFERRED_HUNSPELL_LOCALES.keys():
                    if PREFERRED_HUNSPELL_LOCALES[language] == full_locale:
                        print('Found Hunspell locale:',
                              language, ':', full_locale)
                        new_hunspell_locales[language] = full_locale
                        HUNSPELL_LOCALES[language] = full_locale
                else:
                    if language in new_hunspell_locales.keys():
                        print('WARNING: Multiple countries found for', language,
                              ':', full_locale, ' - ', new_hunspell_locales[language])
                    if language not in HUNSPELL_LOCALES:
                        print('Found Hunspell locale:',
                              language, ':', full_locale)
                        new_hunspell_locales[language] = full_locale
                        HUNSPELL_LOCALES[language] = full_locale
                        HUNSPELL_LOCALES[full_locale] = full_locale

    except subprocess.CalledProcessError:
        print('Failed to run hunspell. Error code:',
              hunspell_result.returncode, '\n', hunspell_result.stderr)


def is_vowel(character):
    """Helper function for creating inflections of English words."""
    return character in ('a', 'e', 'i', 'o', 'u', 'y')


def make_english_plural(word):
    """Create plural forms for nouns.

    This will create a few nonsense entries for irregular plurals, but
    it's good enough for our purpose. Glossary contains pluralized
    terms, so we don't add any plural forms for strings ending in 's'.
    """
    result = ''
    if not word.endswith('s'):
        if word.endswith('y') and not is_vowel(word[-2:-1]):
            result = word[0:-1] + 'ies'
        elif word.endswith('z') or word.endswith('x') or word.endswith('ch') or word.endswith('sh') or word.endswith('o'):
            result = word + 'es'
        else:
            result = word + 's'
    return result


def make_english_verb_forms(word):
    """Create inflected forms of an English verb: -ed and -ing forms.

    Will create nonsense for irregular verbs.
    """
    result = []
    if word.endswith('e'):
        result.append(word[0:-1] + 'ing')
        result.append(word + 'd')
    elif is_vowel(word[-2:-1]) and not is_vowel(word[-1]):
        # The consonant is duplicated here if the last syllable is stressed.
        # We can't detect stress, so we add both variants.
        result.append(word + word[-1] + 'ing')
        result.append(word + 'ing')
        result.append(word + word[-1] + 'ed')
        result.append(word + 'ed')
    elif word.endswith('y') and not is_vowel(word[-2:-1]):
        result.append(word + 'ing')
        result.append(word[0:-1] + 'ed')
    else:
        result.append(word + 'ing')
        result.append(word + 'ed')
    # 3rd person s has the same pattern as noun plurals.
    # We ommitted words ending on s i the plural, so we add them here.
    if word.endswith('s'):
        result.append(word + 'es')
    else:
        result.append(make_english_plural(word))
    return result


def load_glossary(glossary_file, locale):
    """Build a glossary from the given Transifex glossary csv file for the
    given locale."""
    result = []
    counter = 0
    term_index = 0
    term_comment_index = 0
    wordclass_index = 0
    translation_index = 0
    comment_index = 0
    for row in read_csv_file(glossary_file):
        # Detect the column indices
        if counter == 0:
            colum_counter = 0
            for header in row:
                if header == 'term':
                    term_index = colum_counter
                elif header == 'comment' or header == 'notes':
                    term_comment_index = colum_counter
                elif header == 'pos':
                    wordclass_index = colum_counter
                elif header in ('translation_' + locale, locale):
                    translation_index = colum_counter
                elif header == 'comment_' + locale or header == 'notes_' + locale:
                    comment_index = colum_counter
                colum_counter = colum_counter + 1
        # If there is a translation, parse the entry
        # We also have some obsolete terms in the glossary that we want to
        # filter out.
        elif len(row[translation_index].strip()) > 0 and not row[term_comment_index].startswith('OBSOLETE'):
            if translation_index == 0:
                raise Exception(
                    'Locale %s is missing from glossary file.' % locale)
            if comment_index == 0:
                raise Exception(
                    'Comment field for locale %s is missing from glossary file.' % locale)
            entry = GlossaryEntry()
            entry.terms.append(row[term_index].strip())
            if row[wordclass_index] == 'Noun':
                plural = make_english_plural(entry.terms[0])
                if len(plural) > 0:
                    entry.terms.append(plural)
            elif row[wordclass_index] == 'Verb':
                verb_forms = make_english_verb_forms(entry.terms[0])
                for verb_form in verb_forms:
                    entry.terms.append(verb_form)

            entry.translations.append(row[translation_index].strip())

            # Misuse the comment field to provide a list of inflected forms.
            # Otherwise, we would get tons of false positive hits in the checks
            # later on and the translators would have our heads on a platter.
            delimiter = '|'
            if len(row[comment_index].strip()) > 1 and delimiter in row[comment_index]:
                inflections = row[comment_index].split(delimiter)
                for inflection in inflections:
                    entry.translations.append(inflection.strip())

            result.append(entry)
        counter = counter + 1
    return result


#############################################################################
# Term Checking                                                             #
#############################################################################


def contains_term(string, term):
    """Checks whether 'string' contains 'term' as a whole word.

    This check is case-insensitive.
    """
    result = False
    # Regex is slow, so we do this preliminary check
    if term.lower() in string.lower():
        # Now make sure that it's whole words!
        # We won't want to match "AI" against "again" etc.
        regex = re.compile(r'^|(.+\W)' + term + r'(\W.+)|$', re.IGNORECASE)
        result = regex.match(string)
    return result


def source_contains_term(source_to_check, entry, glossary):
    """Checks if the source string contains the glossary entry while filtering
    out superstrings from the glossary, e.g. we don't want to check 'arena'
    against 'battle arena'."""
    source_to_check = source_to_check.lower()
    for term in entry.terms:
        term = term.lower()
        if term in source_to_check:
            source_regex = re.compile(r'.+[\s,.]' + term + r'[\s,.].+')
            if source_regex.match(source_to_check):
                for entry2 in glossary:
                    if entry.terms[0] != entry2.terms[0]:
                        for term2 in entry2.terms:
                            term2 = term2.lower()
                            if term2 != term and term in term2 and term2 in source_to_check:
                                source_to_check = source_to_check.replace(
                                    term2, '')
                # Check if the source still contains the term to check
                return contains_term(source_to_check, term)
    return False


def append_hunspell_stems(hunspell_locale, translation):
    """ Use hunspell to append the stems for terms found = less work for glossary editors.
    The effectiveness of this check depends on how good the hunspell data is."""
    try:
        hunspell_result = subprocess.run(['hunspell', '-d', hunspell_locale,
                                          '-s'], input=translation, capture_output=True, text=True, check=True)
        hunspell_result.check_returncode()

        for line in hunspell_result.stdout.split('\n'):
            linedata = line.split(' ')
            if len(linedata) == 2:
                translation = ' '.join([translation, linedata[1]])

    except subprocess.CalledProcessError:
        print('Failed to run hunspell for locale: ' + hunspell_locale)
    return translation


def translation_has_term(entry, target):
    """Verify the target translation against all translation variations from
    the glossary."""
    result = False
    for translation in entry.translations:
        if contains_term(target, translation):
            result = True
            break
    return result


def check_file(csv_file, glossaries, locale, po_file):
    """Run the actual check."""
    translations = read_csv_file(csv_file)
    source_index = 0
    target_index = 0
    location_index = 0
    hits = []
    counter = 0
    hunspell_locale = get_hunspell_locale(locale)
    for row in translations:
        # Detect the column indices
        if counter == 0:
            colum_counter = 0
            for header in row:
                if header == 'source':
                    source_index = colum_counter
                elif header == 'target':
                    target_index = colum_counter
                elif header == 'location':
                    location_index = colum_counter
                colum_counter = colum_counter + 1
        else:
            for entry in glossaries[locale]:
                # Check if the source text contains the glossary term.
                # Filter out superstrings, e.g. we don't want to check
                # "arena" against "battle arena"
                if source_contains_term(row[source_index], entry, glossaries[locale]):
                    # Skip empty translations
                    if row[target_index] == '':
                        continue
                    # Now verify the translation against all translation
                    # variations from the glossary
                    term_found = translation_has_term(entry, row[target_index])
                    # Add Hunspell stems for better matches and try again
                    # We do it here because the Hunspell manipulation is slow.
                    if not term_found and hunspell_locale != '':
                        target_to_check = append_hunspell_stems(
                            hunspell_locale, row[target_index])
                        term_found = translation_has_term(
                            entry, target_to_check)
                    if not term_found:
                        hit = FailedTranslation()
                        hit.source = row[source_index]
                        hit.target = row[target_index]
                        hit.location = row[location_index]
                        hit.term = entry.terms[0]
                        hit.translation = entry.translations[0]
                        hit.locale = locale
                        hit.po_file = po_file
                        hits.append(hit)
        counter = counter + 1
    return hits


#############################################################################
# Main Loop                                                                 #
#############################################################################


def check_translations_with_glossary(input_path, output_path, glossary_file, only_locale):
    """Main loop.

    Loads the Transifex and Hunspell glossaries, converts all po files
    for languages that have glossary entries to temporary csv files,
    runs the check and then reports any hits to csv files.
    """
    print('Locale: ' + only_locale)
    temp_path = make_path(output_path, 'temp_glossary')
    hits = []
    locale_list = []

    glossaries = {}
    load_hunspell_locales()

    source_directories = sorted(os.listdir(input_path), key=str.lower)
    for dirname in source_directories:
        dirpath = os.path.join(input_path, dirname)
        if os.path.isdir(dirpath):
            source_files = sorted(os.listdir(dirpath), key=str.lower)
            sys.stdout.write("\nChecking text domain '" + dirname + "': ")
            sys.stdout.flush()
            for source_filename in source_files:
                po_file = dirpath + '/' + source_filename
                if source_filename.endswith('.po'):
                    locale = source_filename[0:-3]
                    if only_locale in ('all', locale):
                        # Load the glossary if we haven't seen this locale
                        # before
                        if locale not in glossaries:
                            sys.stdout.write(
                                '\nLoading glossary for ' + locale)
                            glossaries[locale] = load_glossary(
                                glossary_file, locale.lower())
                            sys.stdout.write(' - %d entries ' %
                                             len(glossaries[locale]))
                            sys.stdout.flush()
                        # Only bother with locales that have glossary entries
                        if len(glossaries[locale]) > 0:
                            sys.stdout.write(locale + ' ')
                            sys.stdout.flush()
                            if locale not in locale_list:
                                locale_list.append(locale)
                            csv_file = os.path.abspath(os.path.join(
                                temp_path, dirname + '_' + locale + '.csv'))
                            # Convert to csv for easy parsing
                            try:
                                subprocess.run(
                                    ['po2csv', '--progress=none', po_file, csv_file], check=True)

                                # Now run the actual check
                                current_hits = check_file(
                                    csv_file, glossaries, locale, dirname)
                                for hit in current_hits:
                                    hits.append(hit)
                            except subprocess.CalledProcessError:
                                print('Error with po2csv for:',
                                      po_file, csv_file)

                            # The csv file is no longer needed, delete it.
                            os.remove(csv_file)

    hits = sorted(hits, key=lambda FailedTranslation: [
                  FailedTranslation.locale, FailedTranslation.translation])
    for locale in locale_list:
        locale_result = '"glossary_term","glossary_translation","source","target","file","location"\n'
        counter = 0
        for hit in hits:
            if hit.locale == locale:
                row = '"%s","%s","%s","%s","%s","%s"\n' % (
                    hit.term, hit.translation, hit.source, hit.target, hit.po_file, hit.location)
                locale_result = locale_result + row
                counter = counter + 1
        dest_filepath = make_path(output_path, locale)
        with open(dest_filepath + '/glossary_check.csv', 'wt') as dest_file:
            dest_file.write(locale_result)
        # Uncomment this line to print a statistic of the number of hits for each locale
        # print("%s\t%d"%(locale, counter))

    delete_path(temp_path)
    return 0


def main():
    """Checks whether we are in the correct directory and everything's there,
    then runs a glossary check over all PO files."""
    if len(sys.argv) == 2 or len(sys.argv) == 3:
        print('Running glossary checks:')
    else:
        print(
            'Usage: glossary_checks.py <relative-path-to-glossary> [locale]')
        return 1

    try:
        print('Current time: %s' % time.ctime())
        # Prepare the paths
        glossary_file = os.path.abspath(os.path.join(
            os.path.dirname(__file__), sys.argv[1]))
        locale = 'all'
        if len(sys.argv) == 3:
            locale = sys.argv[2]

        if not (os.path.exists(glossary_file) and os.path.isfile(glossary_file)):
            print('There is no glossary file at ' + glossary_file)
            return 1

        input_path = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../po'))
        output_path = make_path(os.path.dirname(
            __file__), '../po_validation/translators')
        result = check_translations_with_glossary(
            input_path, output_path, glossary_file, locale)
        print('Current time: %s' % time.ctime())
        return result

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        delete_path(make_path(output_path, 'temp_glossary'))
        return 1


if __name__ == '__main__':
    sys.exit(main())
