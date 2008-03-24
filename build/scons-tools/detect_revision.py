#!/usr/bin/python -tt

# Tries to find out the repository revision of the current working directory
# using svn or svk.
#
# For svk: must be in checkout's root path, will not work otherwise
#          (but could be made to)

import os

def detect_revision():
	revstring='UNKNOWN-VERSION'

	if os.path.exists('.svn'):
		has_svn = os.system('svn >/dev/null 2>&1')==256
		if has_svn:
			svn_revnum=os.popen('svn info|grep Revision:|cut -d" " -f 2').read().rstrip()
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

	return revstring

if __name__ == "__main__":
	print detect_revision()

