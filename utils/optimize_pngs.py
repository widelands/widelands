#!/usr/bin/env python
# encoding: utf-8

"""Walk a directory and use command line tools to optimize all png files in
it."""

import argparse
import os
import os.path
import shutil
import subprocess
import sys
import tempfile

def log(text):
    """Write text without newline to stdout and flush."""
    sys.stdout.write(text)
    sys.stdout.flush()


class Tool:
    """Command line tool for optimizing PNG file.

    Searches for the tool and defines its call parameters.
    """

    def __init__(self, name, options, inplace=False):
        self._name = name
        self._options = options.split()
        self._inplace = inplace

        self._check_for_tool()

    def _check_for_tool(self):
        """Tries to execute a tool, returns True if it is found."""
        log("Checking for '%s' ... " % self._name)
        self.found = False

        try:
            subprocess.call(self._name, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT)
            log('found')
            self.found = True
        except OSError:
            log('not found. Not used.')
        log('\n')

    def __call__(self, inf):
        """Attempts to shrink the PNG, if successful, returns True and
        overwrites the input file.

        Otherwise, returns false and does nothing
        """
        log('%s:' % (self._name))
        _file_handle1, temp_in = tempfile.mkstemp(self._name, 'png')
        os.close(_file_handle1)
        _file_handle2, temp_out = tempfile.mkstemp(self._name, 'png')
        os.close(_file_handle2)

        try:
            current_size = os.stat(inf).st_size
            shutil.copy(inf, temp_in)

            tool_process = subprocess.Popen([self._name] + self._options + [temp_in] +
                                            ([temp_out] if not self._inplace else []),
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

            tool_process.wait()

            if tool_process.returncode:
                output = tool_process.stdout.read()
                if 'more than 256 colors' not in output:
                    raise RuntimeError('%s failed!: %s' %
                                       (self._name, tool_process.stdout.read()))
                log('NO ')
            else:  # New file was written
                new_size = os.stat(
                    temp_out if not self._inplace else temp_in).st_size

                if new_size < current_size:
                    log('YES ')
                    shutil.copy(
                        temp_out if not self._inplace else temp_in, inf)
                else:
                    log('NO ')

        finally:
            os.unlink(temp_in)
            os.unlink(temp_out)


def collect_pngs(directory, prefix):
    """Search for all pngs in the subdir.

    Restrict to filename prefix if it is not empty.
    """
    pngs = []
    for root, dirs, files in os.walk(directory):
        dirs.sort()
        pngs.extend(os.path.join(root, f)
                    for f in sorted(files)
                    if os.path.splitext(f)[-1].lower() == '.png'
                    and (not prefix or f.startswith(prefix)))

    return pngs


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description='Recurses into the given directory (cwd by default) and runs PNG optimization tools on all pngs found.')

    parser.add_argument('-d', '--dir', metavar='DIR', dest='directory',
                        help="Recursively search this directory for PNG's [%default]",
                        default='.')

    parser.add_argument('-p', '--prefix', metavar='PREFIX', dest='prefix',
                        help='Only optimize the files where the filename starts with the given prefix',
                        default='')

    return parser.parse_args()


def main():
    """Walk a directory and use command line tools to optimize all png files in
    it."""
    if sys.version_info[0] < 3:
        sys.exit("At least python version 3 is needed.")

    args = parse_args()

    # The filter function returns an iterator only, so we stick the results into a list for iterating multiple times.
    tools = list(filter(lambda t: t.found, [
        # This tool is destroying deadtree5 after it was cropped
        # Tool('optipng', '-q -zc1-9 -zm1-9 -zs0-3 -f0-5', True),
        Tool('advdef', '-z4', True),
        Tool('advpng', '-z4', True),
        Tool('pngcrush', '-reduce -brute'),
    ]))

    pngs = collect_pngs(args.directory, args.prefix)

    for pidx, png in enumerate(pngs):
        if len(tools):
            log('(%i/%i) Who improves %s? ' % (pidx + 1, len(pngs), png))
            for tool in tools:
                tool(png)
            log('\n')
        else:
            sys.exit("\nPlease install at least one of the above png-tools!")
                

if __name__ == '__main__':
    main()
