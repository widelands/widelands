#!/usr/bin/env python3
# encoding: utf-8


"""Uses pocount from the Translate Toolkit to write translation statistics to
data/i18n/translation_stats.conf.

You will need to have the Translate Toolkit installed:
http://toolkit.translatehouse.org/

For Debian-based Linux: sudo apt-get install translate-toolkit
"""

from collections import defaultdict
from subprocess import call, check_output, CalledProcessError
import os.path
import re
import subprocess
import sys
import traceback

#############################################################################
# Data Containers                                                           #
#############################################################################


class TranslationStats:
    """Total source words and translated source words."""

    def __init__(self):
        # We need the total only once, but since the entries come in per
        # directory rather than per locale, we just store it here to keep the
        # algorithm simpler
        self.total = 0
        self.translated = 0


#############################################################################
# Main Loop                                                                 #
#############################################################################

def generate_translation_stats(po_dir, output_file):
    locale_stats = defaultdict(TranslationStats)

    sys.stdout.write('Fetching translation stats ')

    # Regex to extract the locale from the po filenames.
    regex_po = re.compile(r'/\S+/(\w+)\.po')

    # We get errors for non-po files in the base po dir, so we have to walk
    # the subdirs.
    for subdir in sorted(os.listdir(po_dir), key=str.lower):
        subdir = os.path.join(po_dir, subdir)
        if not os.path.isdir(subdir):
            continue

        sys.stdout.write('.')
        sys.stdout.flush()

        try:
            # We need shell=True, otherwise we get "No such file or directory".
            stats_output = check_output(
                ['pocount ' + subdir + ' --csv'],
                encoding='utf-8',
                stderr=subprocess.STDOUT,
                shell=True)
            if 'ERROR' in stats_output:
                print('\nError running pocount:\n' + stats_output.split('\n', 0)
                      [0]) + '\nAborted creating translation statistics.'
                return 1

        except CalledProcessError:
            print('Failed to run pocount:\n  FILE: ' + po_dir +
                  '\n  ' + stats_output.split('\n', 1)[1])
            return 1

        result = stats_output.split('\n')

        # pocount distributes its header over multiple lines, so we have to do some
        # collecting here before we parse it
        header = ''
        for line in result:
            # Non-header rows will have a file path in them
            if '/' in line or '\\' in line:
                break
            header = header + ' ' + line
        header_entries = header.split(',')
        column_counter = 0
        total_column = 0
        translated_column = 0
        while column_counter < len(header_entries):
            if header_entries[column_counter].strip() == 'Total Source Words':
                total_column = column_counter
            elif header_entries[column_counter].strip() == 'Translated Source Words':
                translated_column = column_counter
            column_counter = column_counter + 1

        # Now do the actual counting for the current textdomain
        for line in result:
            cells = line.split(',')
            po_filename = cells[0]
            if po_filename.endswith('.po'):
                entry = TranslationStats()
                locale = regex_po.match(po_filename).group(1)
                if locale in locale_stats:
                    entry = locale_stats[locale]
                entry.total = entry.total + int(cells[total_column])
                entry.translated = entry.translated + \
                    int(cells[translated_column])
                if entry.translated > entry.total:
                    print('Error! Translated ' + str(entry.translated) + ' (+' + (cells[
                          translated_column]) + ') is bigger than the total of ' + str(entry.total) + '(' + (cells[total_column]) + ')\n' + line)
                    sys.exit(1)
                locale_stats[locale] = entry

    print('\n\nLocale\tTotal\tTranslated')
    print('------\t-----\t----------')

    # The total goes in a [global] section and is identical for all locales
    result = '[global]\n'
    result = '{}total={}\n\n'.format(
        result, list(locale_stats.values())[0].total)
    # Write translation stats for all locales
    for locale in (sorted(list(locale_stats.keys()), key=str.lower)):
        entry = locale_stats[locale]
        print(locale + '\t' + str(entry.total) + '\t' + str(entry.translated))
        result = '{}[{}]\n'.format(result, locale)
        result = '{}translated={}\n\n'.format(result, str(entry.translated))

    with open(output_file, 'w+') as destination:
        destination.write(result[:-1])  # Strip the final \n
    print('\nResult written to ' + output_file)
    return 0


def main():
    try:
        po_dir = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../po'))
        output_file = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../data/i18n/translation_stats.conf'))
        result = generate_translation_stats(po_dir, output_file)
        sys.stdout.flush()
        return result

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    if sys.version_info[0] < 3:
        sys.exit('At least python version 3 is needed.')

    sys.exit(main())
