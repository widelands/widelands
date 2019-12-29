#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Tries to find out the repository revision of the current working directory
# using bzr or debian/changelog

import os
import sys
import os.path as p
import subprocess
import re
import string

# Support for bzr local branches
try:
    from bzrlib.branch import Branch
    from bzrlib.bzrdir import BzrDir
    from bzrlib.errors import NotBranchError
    __has_bzrlib = True
except ImportError:
    __has_bzrlib = False

base_path = p.abspath(p.join(p.dirname(__file__), p.pardir))


def _communicate_utf8(cmd, **kwargs):
    """Runs the 'cmd' with 'kwargs' and returns its output which is assumed to
    be utf-8."""
    if sys.version_info >= (3, 0):
        output = subprocess.check_output(cmd, **kwargs)
        return output.decode('utf-8')
    return subprocess.check_output(cmd, **kwargs)


def detect_debian_version():
    """Parse bzr revision and branch information from debian/changelog."""
    if sys.platform.startswith('win'):
        return None
    fname = p.join(base_path, 'debian/changelog')
    if not p.exists(fname):
        return None
    f = open(fname)
    version = f.readline()
    # bzr5905-210307251546
    pattern = re.compile('bzr[0-9]+-[0-9]+')
    m = pattern.search(version)
    if m == None:
        return None
    version = version[m.start():m.end()]
    return version


def detect_git_revision():
    try:
        stdout = _communicate_utf8(
            ['git', 'rev-list', '--count', 'HEAD'], cwd=base_path)
        git_count = stdout.rstrip()
        stdout = _communicate_utf8(
            ['git', 'rev-parse', '--short=7', 'HEAD'], cwd=base_path)
        git_revnum = stdout.rstrip()
        stdout = _communicate_utf8(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'], cwd=base_path)
        git_abbrev = stdout.rstrip()
        if git_count and git_revnum and git_abbrev:
            return 'r%s[%s@%s]' % (git_count, git_revnum, git_abbrev)
    except Exception as e:
        pass
    return None


def check_for_explicit_version():
    """Checks for a file WL_RELEASE in the root directory.

    It then defaults to this version without further trying to find
    which revision we're on
    """
    fname = p.join(base_path, 'WL_RELEASE')
    if os.path.exists(fname):
        return open(fname).read().strip()


def detect_bzr_revision():
    if __has_bzrlib:
        try:
            b = BzrDir.open(base_path).open_branch()
            revno, nick = b.revno(), b.nick
            return 'bzr%s[%s]' % (revno, nick)
        except:
            return None
    else:
        # Windows stand alone installer do not come with bzrlib. We try to
        # parse the output of bzr then directly
        try:
            def run_bzr(subcmd):
                return _communicate_utf8(['bzr', subcmd], cwd=base_path).strip()
            revno = run_bzr('revno')
            nick = run_bzr('nick')
            return 'bzr%s[%s]' % (revno, nick)
        except OSError:
            return None
    return None


def detect_revision():
    for func in (
            check_for_explicit_version,
            detect_git_revision,
            detect_bzr_revision,
            detect_debian_version):
        rv = func()
        if rv:
            return rv

    return 'REVDETECT-BROKEN-PLEASE-REPORT-THIS'


if __name__ == '__main__':
    print(detect_revision())
