#!/usr/bin/env python
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
        self.total = 0
        self.translated = 0


#############################################################################
# Main Loop                                                                 #
#############################################################################

def generate_translation_stats(po_dir, output_file):
    locale_stats = defaultdict(TranslationStats)

    sys.stdout.write('Fetching translation stats ')

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
                ['pocount ' + subdir + ' --short-words'], stderr=subprocess.STDOUT, shell=True)
            if 'ERROR' in stats_output:
                print('\nError running pocount:\n' + stats_output.split('\n', 1)
                      [0]) + '\nAborted creating translation statistics.'
                return False

        except CalledProcessError:
            print('Failed to run pocount:\n  FILE: ' + po_dir +
                  '\n  ' + stats_output.split('\n', 1)[1])
            return False

        result = stats_output.split('\n')

        # Format provided by pocount:
        # /home/<snip>/po/<textdomain>/<locale>.po  source words: total: 1701	| 500t	0f	1201u	| 29%t	0%f	70%u
        regex_translated = re.compile(
            '/\S+/(\w+)\.po\s+source words: total: (\d+)\t\| (\d+)t\t\d+f\t\d+u\t\| (\d+)%t\t\d+%f\t\d+%u')

        for line in result:
            match = regex_translated.match(line)
            if match:
                entry = TranslationStats()
                locale = match.group(1)

                if locale in locale_stats:
                    entry = locale_stats[locale]

                entry.total = entry.total + int(match.group(2))
                entry.translated = entry.translated + int(match.group(3))
                locale_stats[locale] = entry

    print('\n\nLocale\tTotal\tTranslated')
    print('------\t-----\t----------')
    result = ''
    for locale in sorted(locale_stats.keys(), key=str.lower):
        entry = locale_stats[locale]
        print(locale + '\t' + str(entry.total) + '\t' + str(entry.translated))
        result = result + '[' + locale + ']\n'
        result = result + 'translated=' + str(entry.translated) + '\n'
        result = result + 'total=' + str(entry.total) + '\n\n'

    with open(output_file, 'w+') as destination:
        destination.write(result[:-1])  # Strip the final \n
    print('\nResult written to ' + output_file)
    return True


def main():
    try:
        po_dir = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../po'))
        output_file = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '../data/i18n/translation_stats.conf'))
        result = generate_translation_stats(po_dir, output_file)
        return result

    except Exception:
        print('Something went wrong:')
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())
