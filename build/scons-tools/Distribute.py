import SCons
from SCons.Script import *
import os, shutil, glob, distutils.dir_util
import tempfile, tarfile
from zipfile import *

def distadd(env, source, compress=False):
	head,tail=os.path.split(source)

	for s in glob.glob(tail):
		env['DISTFILES']+=[(os.path.join(head, s), compress)]

def dodist(target, source, env):
	try:
		os.remove(str(target[0])+'.tar.bz2')
	except:
		pass
	tmpdir=tempfile.mkdtemp(prefix='widelands-dist.')
	tarbz2file=tarfile.open(str(target[0])+'.tar.bz2','w:bz2')

	for (name, compress) in set(env['DISTFILES']):  #the set is there to ensure uniqueness
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
			tarbz2file.add(zipfilename, os.path.join(str(target[0]), name))

		else:
			tarbz2file.add(name, os.path.join(str(target[0]), name))

	tarbz2file.close()
	shutil.rmtree(tmpdir)

def instadd(env, source, prefix=None, compress=False, filetype='data'):
	if prefix==None:
		if os.path.isdir(source):
			prefix=source
		else:
			prefix=''

	for s in glob.glob(os.path.basename(source)):
		if s=='SConscript':
			continue
		head,tail=os.path.split(source)
		env['INSTFILES']+=[(os.path.join(head, s), os.path.join(prefix, tail), compress, filetype)]

def doinst(target, source, env):
	tmpdir=tempfile.mkdtemp(prefix='widelands-inst.')

	for (name, location, compress, filetype) in set(env['INSTFILES']):  #the set is there to ensure uniqueness
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
			name=zipfilename

		if filetype=='data':
			prefix=os.path.join(env['datadir'], os.path.dirname(location))
		if filetype=='binary':
			prefix=os.path.join(env['bindir'], os.path.dirname(location))

		if not os.path.exists(prefix):
				os.makedirs(prefix, 0755)

		if os.path.isfile(name):
			shutil.copy(name, prefix)
			if filetype=='binary':
				os.chmod(os.path.join(prefix, name), 0755)
		elif os.path.isdir(name):
			distutils.dir_util.copy_tree(name, os.path.join(prefix, os.path.basename(name)))

	shutil.rmtree(tmpdir)

def douninst(target, source, env):
	datadir=os.path.join(env['install_prefix'], env['datadir'])
	executable=os.path.join(env['install_prefix'], env['bindir'], 'widelands')

	print "Removing data directory", env['datadir']
	if os.path.exists(env['datadir']):
		shutil.rmtree(env['datadir'])
	else:
		print "        %s does not exist" % (env['datadir'],)

	print "Removing executable", executable
	if os.path.exists(executable):
		os.remove(executable)
	else:
		print "        %s does not exist" % (executable,)

def generate(env):
	env['DISTFILES']=[]
	env['INSTFILES']=[]

	try:
		bld = env['BUILDERS']['DistPackage']
	except KeyError:
		env['BUILDERS']['DistPackage'] = SCons.Builder.Builder(action=dodist)

	try:
		bld = env['BUILDERS']['Install']
	except KeyError:
		env['BUILDERS']['Install'] = SCons.Builder.Builder(action=doinst)

	try:
		bld = env['BUILDERS']['Uninstall']
	except KeyError:
		env['BUILDERS']['Uninstall'] = SCons.Builder.Builder(action=douninst)

def exists(env):
	return env.Detect('zip')
