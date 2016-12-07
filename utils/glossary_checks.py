#!/usr/bin/env python
# encoding: utf-8

"""Runs a glossary check on all po files and writes the check results to
po_validation/glossary.

You will need to have the Translate Toolkit installed in order for the checks to work:
http://toolkit.translatehouse.org/

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

from collections import defaultdict
from subprocess import call, check_output, CalledProcessError
import csv
import os.path
import re
import subprocess
import sys
import traceback


def read_csv_file(filepath):
    """Parses a CSV file into a 2-dimensional array."""
    result = []
    with open(filepath) as csvfile:
        csvreader = csv.reader(csvfile, delimiter=',', quotechar='"')
        for row in csvreader:
            result.append(row)
    return result


def is_vowel(character):
    """Helper function for creating inflections of English words."""
    return character == 'a' or character == 'e' or character == 'i' \
        or character == 'o' or character == 'u' or character == 'y'


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
        elif word.endswith('z') or word.endswith('x'):
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
    return result


class GlossaryEntry:
    """An entry in our parsed glossaries."""

    def __init__(self):
         # Base form of the term, followed by any inflected forms
        self.terms = []
        # Base form of the translation, followed by any inflected forms
        self.translations = []


def load_glossary(glossary_file, locale):
    """Build a glossary from the given Transifex glossary csv file for the
    given locale."""
    result = []
    counter = 0
    term_index = 0
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
                elif header == 'pos':
                    wordclass_index = colum_counter
                elif header == 'translation_' + locale:
                    translation_index = colum_counter
                elif header == 'comment_' + locale:
                    comment_index = colum_counter
                colum_counter = colum_counter + 1
        # If there is a translation, parse the entry
        elif len(row[translation_index].strip()) > 0:
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


def contains_term(string, term):
    """Checks whether 'string' contains 'term' as a whole word.

    This check is case-ionsensitive.

    """
    result = False
    # Regex is slow, so we do this preliminary check
    if term.lower() in string.lower():
        # Now make sure that it's whole words!
        # We won't want to match "AI" against "again" etc.
        regex = re.compile('^|(.+\W)' + term + '(\W.+)|$', re.IGNORECASE)
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
            source_regex = re.compile('.+[\s,.]' + term + '[\s,.].+')
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

class HunspellLocale:
    """A specific locale for Hunspell, plus whether its dictionary is installed."""

    def __init__(self, locale):
         # Specific language/country code for Hunspell, e.g. el_GR
        self.locale = locale
        # Whether a dictionary has been found for the locale
        self.is_available = False

hunspell_locales = defaultdict(list)
""" Hunspell needs specific locales"""

def set_has_hunspell_locale(hunspell_locale, csv_dir):
    """Tries calling hunspell with the given locale and returns false if it has failed."""
    hunspell_temppath = os.path.abspath(os.path.join(csv_dir, "temp.txt"))
    with open(hunspell_temppath, 'wt') as hunspell_tempfile:
        hunspell_tempfile.write("foo")
        try:
            hunspell_result = check_output(['hunspell', '-d', hunspell_locale.locale, '-s', hunspell_temppath], stderr=subprocess.STDOUT)
            if "error" in hunspell_result:
                print("Error loading Hunspell dictionary for locale " + hunspell_locale.locale + ": " + hunspell_result.split('\n', 1)[0])
                return False
            hunspell_locale.is_available = True
            return True
        except CalledProcessError:
            print("Error loading Hunspell dictionary for locale " + hunspell_locale.locale)
            return False

def get_hunspell_locale(locale):
    """Returns the corresponding Hunspell locale for this locale, or empty string if not available."""
    if len(hunspell_locales[locale]) == 1 and hunspell_locales[locale][0].is_available:
        return hunspell_locales[locale][0].locale
    return ""

def load_hunspell_locales(csv_dir):
    """Registers locales for Hunspell. Maps a list of generic locales to specific locales and checks which dictionaries are available."""
    print("Looking for Hunspell dictionaries");
    hunspell_locales["bg"].append(HunspellLocale("bg_BG"))
    hunspell_locales["br"].append(HunspellLocale("br_FR"))
    hunspell_locales["ca"].append(HunspellLocale("ca_ES"))
    hunspell_locales["da"].append(HunspellLocale("da_DK"))
    hunspell_locales["cs"].append(HunspellLocale("cs_CZ"))
    hunspell_locales["de"].append(HunspellLocale("de_DE"))
    hunspell_locales["el"].append(HunspellLocale("el_GR"))
    hunspell_locales["en_CA"].append(HunspellLocale("en_CA"))
    hunspell_locales["en_GB"].append(HunspellLocale("en_GB"))
    hunspell_locales["en_US"].append(HunspellLocale("en_US"))
    hunspell_locales["eo"].append(HunspellLocale("eo"))
    hunspell_locales["es"].append(HunspellLocale("es_ES"))
    hunspell_locales["et"].append(HunspellLocale("et_EE"))
    hunspell_locales["eu"].append(HunspellLocale("eu_ES"))
    hunspell_locales["fa"].append(HunspellLocale("fa_IR"))
    hunspell_locales["fi"].append(HunspellLocale("fi_FI"))
    hunspell_locales["fr"].append(HunspellLocale("fr_FR"))
    hunspell_locales["gd"].append(HunspellLocale("gd_GB"))
    hunspell_locales["gl"].append(HunspellLocale("gl_ES"))
    hunspell_locales["he"].append(HunspellLocale("he_IL"))
    hunspell_locales["hr"].append(HunspellLocale("hr_HR"))
    hunspell_locales["hu"].append(HunspellLocale("hu_HU"))
    hunspell_locales["ia"].append(HunspellLocale("ia"))
    hunspell_locales["id"].append(HunspellLocale("id_ID"))
    hunspell_locales["it"].append(HunspellLocale("it_IT"))
    hunspell_locales["ja"].append(HunspellLocale("ja_JP"))
    hunspell_locales["jv"].append(HunspellLocale("jv_ID"))
    hunspell_locales["ka"].append(HunspellLocale("ka_GE"))
    hunspell_locales["ko"].append(HunspellLocale("ko_KR"))
    hunspell_locales["krl"].append(HunspellLocale("krl_RU"))
    hunspell_locales["la"].append(HunspellLocale("la"))
    hunspell_locales["lt"].append(HunspellLocale("lt_LT"))
    hunspell_locales["mr"].append(HunspellLocale("mr_IN"))
    hunspell_locales["ms"].append(HunspellLocale("ms_MY"))
    hunspell_locales["my"].append(HunspellLocale("my_MM"))
    hunspell_locales["nb"].append(HunspellLocale("nb_NO"))
    hunspell_locales["nds"].append(HunspellLocale("nds_DE"))
    hunspell_locales["nl"].append(HunspellLocale("nl_NL"))
    hunspell_locales["nn"].append(HunspellLocale("nn_NO"))
    hunspell_locales["oc"].append(HunspellLocale("oc_FR"))
    hunspell_locales["pl"].append(HunspellLocale("pl_PL"))
    hunspell_locales["pt"].append(HunspellLocale("pt_PT"))
    hunspell_locales["ro"].append(HunspellLocale("ro_RO"))
    hunspell_locales["ru"].append(HunspellLocale("ru_RU"))
    hunspell_locales["rw"].append(HunspellLocale("rw_RW"))
    hunspell_locales["si"].append(HunspellLocale("si_LK"))
    hunspell_locales["sk"].append(HunspellLocale("sk_SK"))
    hunspell_locales["sl"].append(HunspellLocale("sl_SI"))
    hunspell_locales["sr"].append(HunspellLocale("sr_RS"))
    hunspell_locales["sv"].append(HunspellLocale("sv_SE"))
    hunspell_locales["tr"].append(HunspellLocale("tr_TR"))
    hunspell_locales["uk"].append(HunspellLocale("uk_UA"))
    hunspell_locales["vi"].append(HunspellLocale("vi_VN"))
    hunspell_locales["zh_CN"].append(HunspellLocale("zh_CN"))
    hunspell_locales["zh_TW"].append(HunspellLocale("zh_TW"))
    for locale in hunspell_locales:
        set_has_hunspell_locale(hunspell_locales[locale][0], csv_dir)

def append_hunspell_stems(csv_dir, hunspell_locale, translation):
    """ Use hunspell to append the stems for terms found = less work for glossary editors.
    The effectiveness of this check depends on how good the hunspell data is."""
    hunspell_temppath = os.path.abspath(os.path.join(csv_dir, "temp.txt"))
    with open(hunspell_temppath, 'wt') as hunspell_tempfile:
        hunspell_tempfile.write(translation)
    try:
        hunspell_result = check_output(['hunspell', '-d', hunspell_locale, '-s', hunspell_temppath])
        if hunspell_result != "":
            translation = " ".join([translation, hunspell_result])
    except CalledProcessError:
        print("Failed to run hunspell for locale: " + hunspell_locale)
    return translation

def translation_has_term(entry, target):
    """ Verify the target translation against all translation variations from the glossary"""
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
    has_hunspell = True
    csv_dir = os.path.dirname(csv_file)
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
        elif hunspell_locale != "":
            for entry in glossaries[locale][0]:
                # Check if the source text contains the glossary term.
                # Filter out superstrings, e.g. we don't want to check
                # "arena" against "battle arena"
                if source_contains_term(row[source_index], entry, glossaries[locale][0]):
                    # Now verify the translation against all translation
                    # variations from the glossary
                    if not translation_has_term(entry, row[target_index]):
                        # Add Hunspell stems for better matches and try again
                        # We do it here because the Hunspell manipulation is slow.
                        target_to_check = append_hunspell_stems(csv_dir, hunspell_locale, row[target_index])
                        if not translation_has_term(entry, target_to_check):
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


def make_path(base_path, subdir):
    """Creates the correct form of the path and makes sure that it exists."""
    result = os.path.abspath(os.path.join(base_path, subdir))
    if not os.path.exists(result):
        os.makedirs(result)
    return result


def delete_path(path):
    """Deletes the directory specified by 'path' and all its subdirectories and
    file contents."""
    if os.path.exists(path) and not os.path.isfile(path):
        files = sorted(os.listdir(path), key=str.lower)
        for deletefile in files:
            deleteme = os.path.abspath(os.path.join(path, deletefile))
            if os.path.isfile(deleteme):
                try:
                    os.remove(deleteme)
                except Exception:
                    print('Failed to delete file ' + deleteme)
            else:
                delete_path(deleteme)
        try:
            os.rmdir(path)
        except Exception:
            print('Failed to delete path ' + deleteme)


def check_translations_with_glossary(input_path, output_path, glossary_file, only_locale):
    """Main loop.

    Loads the Transifex glossary, converts all po files for languages
    that have glossary entries to csv files, runs the check and then
    writes any hits into csv files.

    """
    print("Locale: " + only_locale)
    csv_path = make_path(output_path, 'csv')
    hits = []
    locale_list = defaultdict(list)

    glossaries = defaultdict(list)
    load_hunspell_locales(csv_path)

    source_directories = sorted(os.listdir(input_path), key=str.lower)
    for dirname in source_directories:
        dirpath = os.path.join(input_path, dirname)
        if os.path.isdir(dirpath):
            source_files = sorted(os.listdir(dirpath), key=str.lower)
            print('Checking ' + dirname)
            failed = 0
            for source_filename in source_files:
                po_file = dirpath + '/' + source_filename
                if source_filename.endswith('.po'):
                    locale = source_filename[0:-3]
                    if only_locale == "all" or locale == only_locale:
                        # Load the glossary if we haven't seen this locale before
                        if len(glossaries[locale]) < 1:
                            sys.stdout.write('Loading glossary for ' + locale)
                            glossaries[locale].append(
                                load_glossary(glossary_file, locale))
                            sys.stdout.write(' - %d entries\n' %
                                             len(glossaries[locale][0]))
                            sys.stdout.flush()
                        # Only bother with locales that have glossary entries
                        if len(glossaries[locale][0]) > 0:
                            if len(locale_list[locale]) < 1:
                                locale_list[locale].append(locale)
                            csv_file = os.path.abspath(os.path.join(
                                csv_path, dirname + '_' + locale + '.csv'))
                            # Convert to csv for easy parsing
                            call(['po2csv', '--progress=none', po_file, csv_file])

                            # Now run the actual check
                            current_hits = check_file(
                                csv_file, glossaries, locale, dirname)
                            for hit in current_hits:
                                hits.append(hit)

                            # The csv file is no longer needed, delete it.
                            os.remove(csv_file)

    hits = sorted(hits, key=lambda FailedTranslation: [FailedTranslation.locale, FailedTranslation.translation])
    for locale in locale_list:
        locale_result = '"glossary_term","glossary_translation","source","target","file","location"\n'
        counter = 0
        for hit in hits:
            if hit.locale == locale:
                row = '"%s","%s","%s","%s","%s","%s"\n' % (
                    hit.term, hit.translation, hit.source, hit.target, hit.po_file, hit.location)
                locale_result = locale_result + row
                counter = counter + 1
        dest_filepath = output_path + '/glossary_check_' + locale + '.csv'
        with open(dest_filepath, 'wt') as dest_file:
            dest_file.write(locale_result)
        # Uncomment this line to print a statistic of the number of hits for each locale
        # print("%s\t%d"%(locale, counter))

    delete_path(csv_path)
    return 0


def main():
    """Checks whether we are in the correct directory and everything's there,
    then runs a glossary check over all PO files."""
    if len(sys.argv) == 2 or len(sys.argv) == 3 :
        print('Running glossary checks:')
    else:
        print('Usage: utils/glossary_checks.py <relative-path-to-glossary> [locale]')
        return 1

    try:
        # Prepare the paths
        glossary_file = os.path.abspath(os.path.join(
            os.path.dirname(__file__), sys.argv[1]))
        locale = "all"
        if len(sys.argv) == 3:
            locale = sys.argv[2]

        if (not (os.path.exists(glossary_file) and os.path.isfile(glossary_file))):
            print('There is no glossary file at ' + glossary_file)
            return 1

        input_path = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../po'))
        output_path = make_path(os.path.dirname(__file__), '../po_validation')
        return check_translations_with_glossary(input_path, output_path, glossary_file, locale)

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        delete_path(make_path(output_path, 'csv'))
        return 1

if __name__ == '__main__':
    sys.exit(main())
