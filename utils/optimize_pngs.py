#!/usr/bin/env python
# encoding: utf-8

from glob import glob
from optparse import OptionParser
import tempfile
import subprocess
import os
import shutil
import os.path
import sys


def log(s):
    sys.stdout.write(s)
    sys.stdout.flush()


class Tool(object):

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
            b = subprocess.call(self._name,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

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
        fd, temp_in = tempfile.mkstemp(self._name, 'png')
        os.close(fd)
        fd1, temp_out = tempfile.mkstemp(self._name, 'png')
        os.close(fd1)

        try:
            current_size = os.stat(inf).st_size
            shutil.copy(inf, temp_in)

            sp = subprocess.Popen([self._name] + self._options +
                                  [temp_in] +
                                  ([temp_out] if not self._inplace else []),
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

            sp.wait()

            if sp.returncode:
                output = sp.stdout.read()
                if not 'more than 256 colors' in output:
                    raise RuntimeError('%s failed!: %s' %
                                       (self._name, sp.stdout.read()))
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


def collect_pngs(d, prefix):
    """Search for all pngs in the subdir.

    Restrict to filename prefix if it is not empty.
    """
    pngs = []
    for root, dirs, files in os.walk(d):
        dirs.sort()
        pngs.extend(os.path.join(root, f)
                    for f in sorted(files)
                    if os.path.splitext(f)[-1].lower() == '.png'
                    and (not prefix or f.startswith(prefix)))

    return pngs


def parse_args():
    p = OptionParser('Recurses into the given directory (cwd by default) and '
                     'runs PNG optimization tools on all pngs found.')

    p.add_option('-d', '--dir', metavar='DIR', dest='directory',
                 help="Recursively search this directory for PNG's [%default]",
                 default='.')

    p.add_option('-p', '--prefix', metavar='PREFIX', dest='prefix',
                 help='Only optimize the files where the filename starts with the given prefix',
                 default='')

    o, a = p.parse_args()

    return o, a


def main():
    o, a = parse_args()

    tools = filter(lambda t: t.found,  [
        # This tool is destroying deadtree5 after it was cropped
        # Tool('optipng', '-q -zc1-9 -zm1-9 -zs0-3 -f0-5', True),
        Tool('advdef', '-z4', True),
        Tool('advpng', '-z4', True),
        Tool('pngcrush', '-reduce -brute'),
    ])

    pngs = collect_pngs(o.directory, o.prefix)

    for pidx, p in enumerate(pngs):
        log('(%i/%i) Who improves %s? ' % (pidx + 1, len(pngs), p))
        for t in tools:
            t(p)
        log('\n')


if __name__ == '__main__':
    main()
