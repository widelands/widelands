#!/usr/bin/env python
# encoding: utf-8


"""Uses the Translate Toolkit to extract glossaries from our po files. These
extracted glossaries are then used to fill gaps in the given Transifex
glossary.

You will need to have the Translate Toolkit installed in order for the extraction to work:
http://toolkit.translatehouse.org/

For Debian-based Linux: sudo apt-get install translate-toolkit

You will need to provide an export of the Transifex glossary and specify it at
the command line. Make sure to select "Include glossary notes in file" when
exporting the csv from Transifex.

The resulting file then needs to be uploaded manually to Transifex as well.

"""

from collections import defaultdict
from subprocess import call, check_output, CalledProcessError
import csv
import os.path
import re
import subprocess
import sys
import time
import traceback

#############################################################################
# Data Containers                                                           #
#############################################################################


class GlossaryEntry:
    """An entry in our parsed glossaries."""

    def __init__(self):
        # The English term
        self.term = ''
        # The term's translation
        self.translation = ''
        # Comment for the source term
        self.term_comment = ''
        # Comment for the term's translation
        self.translation_comment = ''
        # Wordclass of the source term
        self.wordclass = ''


#############################################################################
# File System Functions                                                     #
#############################################################################


def read_csv_file(filepath):
    """Parses a CSV file into a 2-dimensional array."""
    result = []
    with open(filepath) as csvfile:
        csvreader = csv.reader(csvfile, delimiter=',', quotechar='"')
        for row in csvreader:
            result.append(row)
    return result


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

#############################################################################
# Glossary Loading                                                          #
#############################################################################


def load_extracted_glossary(glossary_file, locale):
    """Build a defaultdict(GlossaryEntry) glossary from the given extracted
    glossary csv file for the given locale, raising an error for entries that
    have no translation."""
    result = defaultdict(GlossaryEntry)
    counter = 0
    term_index = 0
    comment_index = 0
    for row in read_csv_file(glossary_file):
        # Detect the column indices
        if counter == 0:
            colum_counter = 0
            for header in row:
                if header == 'source':
                    term_index = colum_counter
                elif header == 'target':
                    translation_index = colum_counter
                colum_counter = colum_counter + 1
        # If there is a translation, parse the entry
        elif row[translation_index].strip() != '':
            if translation_index == 0:
                raise Exception(
                    'Glossary extracted for %s contains no translations.' % locale)
            entry = GlossaryEntry()
            entry.term = row[term_index].strip()
            entry.translation = row[translation_index].strip()
            # Remove source information with fuzzy matches
            regex = re.compile('(.+)( \{.*\})(.*)')
            match = regex.match(entry.translation)
            while match:
                entry.translation = match.group(1) + match.group(3)
                match = regex.match(entry.translation)
            result[entry.term] = entry
        counter = counter + 1
    return result


def load_transifex_glossary(glossary_file, locale):
    """Build a defaultdict(GlossaryEntry) glossary from the given Transifex
    glossary csv file for the given locale.

    Include empty translations in the result

    """
    result = defaultdict(GlossaryEntry)
    counter = 0
    term_index = 0
    term_comment_index = 0
    translation_index = 0
    comment_index = 0
    for row in read_csv_file(glossary_file):
        # Detect the column indices
        if counter == 0:
            colum_counter = 0
            for header in row:
                if header == 'term':
                    term_index = colum_counter
                elif header == 'comment':
                    term_comment_index = colum_counter
                elif header == 'translation_' + locale or header == locale:
                    translation_index = colum_counter
                elif header == 'comment_' + locale:
                    comment_index = colum_counter
                colum_counter = colum_counter + 1
        # Parse the entry
        else:
            if translation_index == 0:
                raise Exception(
                    'Locale %s is missing from glossary file.' % locale)
            if comment_index == 0:
                raise Exception(
                    'Comment field for locale %s is missing from glossary file.' % locale)
            entry = GlossaryEntry()
            entry.term = row[term_index].strip()
            entry.term_comment = row[term_comment_index].strip()
            entry.translation = row[translation_index].strip()
            entry.translation_comment = row[comment_index].strip()
            result[entry.term] = entry
        counter = counter + 1
    return result


def load_transifex_source_terms(glossary_file):
    """Loads a list of source terms with their comments and word classes as a
    defaultdict(GlossaryEntry) from the given Transifex glossary csv file."""
    result = defaultdict(GlossaryEntry)
    counter = 0
    term_index = 0
    term_comment_index = 0
    wordclass_index = 0
    for row in read_csv_file(glossary_file):
        # Detect the column indices
        if counter == 0:
            colum_counter = 0
            for header in row:
                if header == 'term':
                    term_index = colum_counter
                elif header == 'comment':
                    term_comment_index = colum_counter
                elif header == 'pos':
                    wordclass_index = colum_counter
                colum_counter = colum_counter + 1
        # Parse the entry
        else:
            entry = GlossaryEntry()
            entry.term = row[term_index].strip()
            entry.term_comment = row[term_comment_index].strip()
            entry.wordclass = row[wordclass_index].strip()
            result[entry.term] = entry
        counter = counter + 1
    return result


#############################################################################
# Main Loop                                                                 #
#############################################################################

def generate_glossary(po_dir, output_path, input_glossary, output_glossary, only_locale):
    """Main loop.

    Uses poterminology from the Translate Toolkit to collect glossary entries for all files in 'po_dir' for the given 'only_locale'. If 'only_locale' = "all", processes all locales. Then reads the <input_glossary>, adds new entries that were obtained by the glossary generation if there are any gaps, and then writes the results to <output_glossary>.

    """

    # Find the locale files to process
    print('Locale: ' + only_locale)
    locales = []
    glossaries = defaultdict(list)

    if only_locale != 'all':
        locales.append(only_locale)
    else:
        # Get locales from the Transifex glossary file
        header_row = read_csv_file(input_glossary)[0]
        regex = re.compile('^(translation_)(.+)$')
        for header in header_row:
            match = regex.match(header)
            if match:
                locales.append(match.group(2))

    temp_path = make_path(output_path, 'temp_glossary')

    for locale in locales:
        print('Processing locale: ' + locale)
        # Generate the pot glossary
        input_path = po_dir + '/*/' + locale + '.po'
        pot_path = os.path.join(temp_path, 'glossary_' + locale + '.po')

        try:
            # We need shell=True for the wildcards.
            poterminology_result = check_output(
                ['poterminology ' + input_path + ' -o ' + pot_path], stderr=subprocess.STDOUT, shell=True)
            if 'Error' in poterminology_result:
                print('Error running poterminology:\n  FILE: ' + input_path + '\n  OUTPUT PATH: ' +
                      output_path + '\n  ' + poterminology_result.split('\n', 1)[1])
                return False

        except CalledProcessError:
            print('Failed to run poterminology:\n  FILE: ' + input_path + '\n  OUTPUT PATH: ' +
                  output_path + '\n  ' + poterminology_result.split('\n', 1)[1])
            return False

        # Convert to csv for easy parsing
        csv_file = os.path.join(temp_path, 'glossary_' + locale + '.csv')
        call(['po2csv', '--progress=none', pot_path, csv_file])
        # The po file is no longer needed, delete it.
        os.remove(pot_path)

        transifex_glossary = load_transifex_glossary(input_glossary, locale)
        extracted_glossary = load_extracted_glossary(csv_file, locale)

        # Add generated translation if necessary
        for key in transifex_glossary.keys():
            if transifex_glossary[key].translation == '' and extracted_glossary.has_key(key):
                extracted_entry = extracted_glossary[key]
                if extracted_entry.translation != '':
                    transifex_entry = transifex_glossary[key]
                    transifex_entry.translation = extracted_entry.translation
                    transifex_entry.translation_comment = 'AUTOGENERATED - PLEASE PROOFREAD!'
                    transifex_glossary[key] = transifex_entry
        glossaries[locale] = transifex_glossary

    # Now collect the date for the global csv file
    # Write header
    print('Writing results to ' + output_glossary)
    result = 'term,pos,comment,'
    for locale in locales:
        result = result + 'translation_' + locale + ','
        result = result + 'comment_' + locale + ','
    result = result[0:-1] + '\n'

    source_terms = load_transifex_source_terms(input_glossary)
    # Collect all translations for each source term
    for key in source_terms:
        result = result + '"%s","%s","%s",' % (source_terms[key].term.replace('"', '""'), source_terms[
                                               key].wordclass.replace('"', '""'), source_terms[key].term_comment.replace('"', '""'))
        for locale in locales:
            glossary = glossaries[locale]
            translation = ''
            translation_comment = ''
            if glossary.has_key(key):
                translation = glossary[key].translation.replace('"', '""')
                translation_comment = glossary[
                    key].translation_comment.replace('"', '""')
            result = result + \
                '"%s","%s",' % (translation, translation_comment)
        result = result[0:-1] + '\n'

    # Now write the file.
    with open(output_glossary, 'wt') as dest_file:
        dest_file.write(result)

    # Cleanup.
    delete_path(temp_path)
    if not os.listdir(output_path):
        os.rmdir(output_path)
    print('Done.')
    return 0


def main():
    """Checks whether we are in the correct directory and everything's there,
    then collects glossary entries from all PO files and writes a new glossary
    file.

    Output is restricted to source terms that are already in the
    glossary.

    """
    if len(sys.argv) == 3 or len(sys.argv) == 4:
        print('Generating glossary:')
    else:
        print(
            'Usage: generate_glossary.py <input-glossary> <output-glossary> [locale]')
        return 1

    try:
        print('Current time: %s' % time.ctime())
        # Prepare the paths
        input_glossary = os.path.abspath(os.path.join(
            os.path.dirname(__file__), sys.argv[1]))
        output_glossary = os.path.abspath(os.path.join(
            os.path.dirname(__file__), sys.argv[2]))
        locale = 'all'
        if len(sys.argv) == 4:
            locale = sys.argv[3]

        if (not (os.path.exists(input_glossary) and os.path.isfile(input_glossary))):
            print('There is no glossary file at ' + input_glossary)
            return 1

        po_dir = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../po'))
        output_path = make_path(os.path.dirname(__file__), '../po_validation')
        result = generate_glossary(
            po_dir, output_path, input_glossary, output_glossary, locale)
        print('Current time: %s' % time.ctime())
        return result

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        delete_path(make_path(output_path, 'temp_glossary'))
        return 1

if __name__ == '__main__':
    sys.exit(main())
