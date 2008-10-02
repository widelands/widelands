#!/usr/bin/python -tt

# Tries to find out the repository revision of the current working directory
# using svn or svk.
#
# For svk: must be in checkout's root path, will not work otherwise
#          (but could be made to)

import os
# Support for bzr local branches
try:
    from bzrlib.branch import Branch
    from bzrlib.bzrdir import BzrDir
    from bzrlib.errors import NotBranchError
    __has_bzr = True
except ImportError:
    __has_bzr = False

def detect_revision():
    revstring='UNKNOWN-VERSION'

    if os.path.exists('.svn'):
        has_svn = os.system('svn >/dev/null 2>&1')==256
        if has_svn:
            svn_revnum=os.popen('LANG=C svn info|grep Revision:|cut -d" " -f 2').read().rstrip()
            revstring='svn%s' % (svn_revnum,)

    has_svk = os.system('svk >/dev/null 2>&1')==0

    if has_svk:
        cwd=os.getcwd()
        is_svk_workdir = os.system('svk co -l|grep '+cwd+' >/dev/null 2>&1')==0

        if is_svk_workdir:
            svk_revnum=os.popen('LANG=C svk info|grep Revision|cut -d" " -f 2').read().rstrip()
            svn_revnum=os.popen('LANG=C svk info|grep Mirrored|cut -d" " -f 5').read().rstrip()

            if svn_revnum=='':
                revstring='unofficial-svk%s' % (svk_revnum,)
            else:
                revstring='unofficial-svk%s(=svn%s)' % (svk_revnum, svn_revnum)

    is_git_workdir=os.system('git show >/dev/null 2>&1')==0
    if is_git_workdir:
        git_revnum=os.popen('git show --pretty=format:%h | head -n 1').read().rstrip()
        is_pristine=os.popen('git show --pretty=format:%b | grep ^git-svn-id\\:').read().find("git-svn-id:") == 0
        common_ancestor=os.popen('git show-branch --sha1-name refs/remotes/git-svn HEAD | tail -n 1 | sed "s@++ \\[\\([0-9a-f]*\\)\\] .*@\\1@"').read().rstrip()
        svn_revnum=os.popen('git show --pretty=format:%b%n '+common_ancestor+' | grep ^git-svn-id\\: -m 1 | sed "sM.*@\\([0-9]*\\) .*M\\1M"').read().rstrip()

        if svn_revnum=='':
            revstring='unofficial-git-%s' % (git_revnum,)
        elif is_pristine:
            revstring='unofficial-git-%s(svn%s)' % (git_revnum, svn_revnum)
        else:
            revstring='unofficial-git-%s(svn%s+changes)' % (git_revnum, svn_revnum)
    
    if __has_bzr and not is_git_workdir:
        try:
            revno = Branch().open(".").revno()
            revstring = "unofficial-bzr-%s(svn%s)" % (revno,svn_revnum)
        except NotBranchError:
            pass


    return revstring

if __name__ == "__main__":
    print detect_revision()

