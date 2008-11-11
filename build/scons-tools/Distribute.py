import SCons
from SCons.Script import *
import os, shutil, glob,distutils.filelist
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

def instadd(env, sourcestring, targetdir=None, compress=False, filetype='data'):
	sourcebase=os.path.basename(sourcestring)
	sourcedir=os.path.dirname(sourcestring)

	if targetdir==None:
		targetdir=sourcedir

	for filename in glob.glob(sourcebase):
		if filename=='SConscript':
			continue

		env['INSTFILES']+=[(os.path.join(sourcedir,filename), os.path.join(targetdir, filename), compress, filetype)]

def doinst(target, source, env):
	tmpdir=tempfile.mkdtemp(prefix='widelands-inst.')

	for (source, target, compress, filetype) in set(env['INSTFILES']):  #the set is there to ensure uniqueness
		sourcebase=os.path.basename(source)
		sourcedir=os.path.dirname(source)
		targetbase=os.path.basename(target)
		targetdir=os.path.dirname(target)

		if compress:
			try:
				shutil.rmtree(tmpdir)
			except:
				pass
			os.makedirs(tmpdir)

			zipfilename=os.path.join(tmpdir, sourcebase)
			zipfile=ZipFile(zipfilename, mode='w')

			if os.path.isfile(source):
				zipfile.write(source)

			if os.path.isdir(source):
				for root, dirs, files in os.walk(source):
					for f in files:
						realname=os.path.join(root, f)
						realdir,realbase=realname.split(os.sep, 1)
						zipfile.write(realname, realbase)
					if '.svn' in dirs:
						dirs.remove('.svn')  # don't visit subversion directories

			zipfile.close()

			source=zipfilename
			sourcebase=os.path.basename(source)
			sourcedir=os.path.dirname(source)

		if filetype=='data':
			targetprefix=os.path.join(env['install_prefix'], env['datadir'], targetdir)
		if filetype=='binary':
			targetprefix=os.path.join(env['install_prefix'], env['bindir'], targetdir)
		if filetype=='locale':
			targetprefix=os.path.join(env['install_prefix'], env['localedir'])

		if not os.path.exists(targetprefix):
			os.makedirs(targetprefix, 0755)

		if os.path.isfile(source):
			shutil.copy(source, targetprefix)
			if filetype=='binary':
				os.chmod(os.path.join(targetprefix, sourcebase), 0755)
			else:
				os.chmod(os.path.join(targetprefix, sourcebase), 0644)
		elif os.path.isdir(source):
			
			fl = distutils.filelist.findall(source)
			for file in fl:
				if '.svn' in file:
					continue
					
				tlp = file.split(targetbase + "/")
				dir = os.path.join(targetprefix,targetbase,os.path.dirname(tlp[1]))
				if not os.path.exists(dir):
					os.makedirs(dir,0755)
				shutil.copyfile(file,os.path.join(dir,os.path.split(tlp[1])[1]))

	shutil.rmtree(tmpdir)

def douninst(target, source, env):
	datadir=os.path.join(env['install_prefix'], env['datadir'])
	executable=os.path.join(env['install_prefix'], env['bindir'], 'widelands')

	print "Removing data directory", env['datadir']
	if os.path.exists(env['datadir']):
		shutil.rmtree(env['datadir'])
	else:
		print "        %s does not exist" % (env['datadir'],)
	print "If you installed your locales somewhere else than here, they have not been deleted!"

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
