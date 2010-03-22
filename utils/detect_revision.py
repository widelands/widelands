#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Tries to find out the repository revision of the current working directory
# using bzr or git

###############################################
#
# ATTENTION CMAKE-TRANSITION
# This (in utils directory) is the future location of this file.
# The other file (in build/scons-utils) is for scons only and can be deleted after transition
#
###############################################

import os
import sys
import os.path as p

# Support for bzr local branches
try:
    from bzrlib.branch import Branch
    from bzrlib.bzrdir import BzrDir
    from bzrlib.errors import NotBranchError
    __has_bzr = True
except ImportError:
    __has_bzr = False

def check_for_explicit_version():
    """
    Checks for a file WL_RELEASE in the root directory. It then defaults to
    this version without further trying to find which revision we're on
    """
    if os.path.exists("WL_RELEASE"):
        return open("WL_RELEASE").read().strip()

def detect_git_revision():
    if not sys.platform.startswith('linux') and \
       not sys.platform.startswith('darwin'):
        return None

    is_git_workdir=os.system('git show >/dev/null 2>&1')==0
    if is_git_workdir:
        git_revnum=os.popen('git show --pretty=format:%h | head -n 1').read().rstrip()
        is_pristine=os.popen('git show --pretty=format:%b | grep ^git-svn-id\\:').read().find("git-svn-id:") == 0
        common_ancestor=os.popen('git show-branch --sha1-name refs/remotes/git-svn HEAD | tail -n 1 | sed "s@++ \\[\\([0-9a-f]*\\)\\] .*@\\1@"').read().rstrip()
        svn_revnum=os.popen('git show --pretty=format:%b%n '+common_ancestor+' | grep ^git-svn-id\\: -m 1 | sed "sM.*@\\([0-9]*\\) .*M\\1M"').read().rstrip()

        if svn_revnum=='':
            return 'unofficial-git-%s' % (git_revnum,)
        elif is_pristine:
            return 'unofficial-git-%s(svn%s)' % (git_revnum, svn_revnum)
        else:
            return 'unofficial-git-%s(svn%s+changes)' % (git_revnum, svn_revnum)


def detect_bzr_revision():
    path = p.join(p.dirname(__file__),p.pardir)

    b = BzrDir.open(path).open_branch()
    revno, nick = b.revno(), b.nick
    return "bzr%s[%s] " % (revno,nick)



def detect_revision():
    for func in (
        check_for_explicit_version,
        detect_git_revision,
        detect_bzr_revision):
        rv = func()
        if rv:
            return rv

    return 'REVDETECT-BROKEN-PLEASE-REPORT-THIS'


    return revstring

if __name__ == "__main__":
    print detect_revision()

