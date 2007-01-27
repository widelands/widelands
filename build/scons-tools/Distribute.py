import SCons
import os, shutil, glob
from tempfile import mkdtemp
import tarfile
from zipfile import *
from os.path import *

def distadd(env, source, compress=False):
	head,tail=os.path.split(source)

	for s in glob.glob(tail):
		env['DISTFILES']+=[(os.path.join(head, s), compress)]

def dodist(target, source, env):
	tmpdir=mkdtemp(prefix='widelands-dist.')
	tarbz2file=tarfile.open(str(target[0]),'w:bz2')

	for (name, compress) in env['DISTFILES']:
		if compress:
			head,tail=os.path.split(name)
			try:
				os.makedirs(os.path.join(tmpdir, head))
			except:
				pass
			zipfilename=os.path.join(tmpdir, name)
			zipfile=ZipFile(zipfilename, mode='w')

			if os.path.isfile(name):
				zipfile.write(name)

			if os.path.isdir(name):
				for root, dirs, files in os.walk(name):
					for f in files:
						realname=os.path.join(root, f)
						head,virtualname=realname.split(os.sep, 1)
						zipfile.write(realname, virtualname)
					if '.svn' in dirs:
						dirs.remove('.svn')  # don't visit subversion directories

			zipfile.close()
			tarbz2file.add(zipfilename, name)

		else:
			tarbz2file.add(name)

	tarbz2file.close()
	shutil.rmtree(tmpdir)

def dosnapshot(target, source, env):
	tmpdir=mkdtemp(prefix='widelands-dist.')
	tarbz2file=tarfile.open(str(target[0]),'w:bz2')

	for (name,compress) in env['DISTFILES']:
		tarbz2file.add(name)

	tarbz2file.close()
	shutil.rmtree(tmpdir)

def generate(env):
	env['DISTFILES']=[]

	try:
		bld = env['BUILDERS']['DistPackage']
	except KeyError:
		env['BUILDERS']['DistPackage'] = SCons.Builder.Builder(action=dodist)

	try:
		bld = env['BUILDERS']['SnapshotPackage']
	except KeyError:
		env['BUILDERS']['SnapshotPackage'] = SCons.Builder.Builder(action=dosnapshot)

def exists(env):
	return env.Detect('zip')
