#!/usr/bin/env python3
# encoding: utf-8


"""Uses pocount from the Translate Toolkit to write translation statistics to
data/i18n/translation_stats.conf.

You will need to have the Translate Toolkit installed:
http://toolkit.translatehouse.org/

For Debian-based Linux: sudo apt-get install translate-toolkit

"""

from collections import defaultdict
import csv
import os.path
import re
import subprocess
import sys

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
    subdirs = []
    for subdir in sorted(os.listdir(po_dir), key=str.lower):
        subdir = os.path.join(po_dir, subdir)
        if os.path.isdir(subdir):
            subdirs.append(subdir)

    def joinrow(a_row):
        return ', '.join(v for v in a_row.values() if v is not None)

    if True:  # TODO(aDiscoverer) delete, just kept for code review
        proc = subprocess.Popen(
            ['pocount', '--csv'] + subdirs,
            encoding='utf-8',
            stderr=subprocess.STDOUT,
            stdout=subprocess.PIPE,
        )

        COLUMNS = {
            'filename': 'Filename',
            'total': 'Total Source Words',
            'translated': 'Translated Source Words'
        }
        result = csv.DictReader(proc.stdout, dialect='unix', skipinitialspace=True)
        missing_cols = set(COLUMNS.values()) - set(result.fieldnames)
        if missing_cols:
            sys.exit(
                'Column(s) "{}" not found in output of pocount'.format('", "'.join(missing_cols)))

        # Now do the actual counting for the current textdomain
        l1_row = l2_row = {}  # last rows, for error message
        one_locale = ''  # to report progress
        for row in result:
            po_filename = row[COLUMNS['filename']]

            if po_filename.endswith('.po'):
                locale = regex_po.match(po_filename).group(1)
                if one_locale == '' or one_locale == locale:
                    # once for each directory, always the same locale
                    sys.stdout.write('.')
                    sys.stdout.flush()
                    if not one_locale:
                        one_locale = locale
                entry = locale_stats[locale]
                entry.total += int(row[COLUMNS['total']])
                entry.translated += int(row[COLUMNS['translated']])
                if entry.translated > entry.total:
                    print(('Error! Translated {entry.translated} ({c_translated}) is bigger than ' +
                           'the total of {entry.total} ({c_total}) on line {line}\n').format(
                        entry=entry, c_translated=row[COLUMNS['translated']],
                        c_total=row[COLUMNS['total']], line=result.line_num))
                    sys.exit(1)
                locale_stats[locale] = entry
            elif 'ERROR' in next(iter(row.values())):
                print('\nError running pocount:\n  ' + joinrow(l2_row) +
                      '\n  ' + joinrow(l1_row) + '\n  ' + joinrow(row) +
                      '\nAborted creating translation statistics.')
                return 1
            l2_row = l1_row
            l1_row = row
        proc.wait(1)
        if proc.returncode != 0:
            print('Failed to run pocount:\n  FILES: ' + ' '.join(subdirs[0:2]) +
                  ' ...\n  ' + joinrow(l2_row) + '\n  ' + joinrow(l1_row))
            return 1

    print('\n\nLocale\tTotal\tTranslated')
    print('------\t-----\t----------')

    # The total goes in a [global] section and is identical for all locales
    result = ('# This file is managed by utils/update_translations_stats.py\n\n'
              '[global]\n')

    result += 'total={}\n'.format(list(locale_stats.values())[0].total)
    # Write translation stats for all locales
    for locale in sorted(locale_stats.keys(), key=str.lower):
        entry = locale_stats[locale]
        print(locale + '\t' + str(entry.total) + '\t' + str(entry.translated))
        result += '\n[{}]\n'.format(locale)
        result += 'translated={}\n'.format(entry.translated)

    with open(output_file, 'w') as destination:
        destination.write(result)
    print('\nResult written to ' + output_file)
    return 0


def main():
    po_dir = os.path.abspath(os.path.join(
        os.path.dirname(__file__), '../data/i18n/translations'))
    output_file = os.path.abspath(os.path.join(
        os.path.dirname(__file__), '../data/i18n/translation_stats.conf'))
    result = generate_translation_stats(po_dir, output_file)
    sys.stdout.flush()
    return result


if __name__ == '__main__':
    if sys.version_info[0] < 3:
        sys.exit('At least python version 3 is needed.')

    sys.exit(main())
