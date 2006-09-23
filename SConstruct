import os
import shutil
import fnmatch
import SCons
import time
import glob
from SCons.Script.SConscript import SConsEnvironment
import string

import sys
sys.path.append("build/scons-tools")
from scons_configure import *

#Speedup. If you have problems with inconsistent or wrong builds, look here first
SetOption('max_drift', 1)
SetOption('implicit_cache', 1)

# Pretty output
print

########################################################################### Glob
# glob.glob does not work with BuildDir(), so use the following replacement from
# http://www.scons.org/cgi-bin/wiki/BuildDirGlob?highlight=%28glob%29
# which I modified slightly to return a list of filenames instead of nodes
def Glob(match):
	"""Similar to glob.glob, except globs SCons nodes, and thus sees
	generated files and files from build directories.  Basically, it sees
	anything SCons knows about.  A key subtlety is that since this function
	operates on generated nodes as well as source nodes on the filesystem,
	it needs to be called after builders that generate files you want to
	include.
	"""

	def fn_filter(node):
		fn = str(node)
		return fnmatch.fnmatch(os.path.basename(fn), match)

	here = Dir('.')

	children = here.all_children()
	nodes = map(File, filter(fn_filter, children))
	node_srcs = [n.srcnode() for n in nodes]
	filenames=[]

	src = here.srcnode()
	if src is not here:
		src_children = map(File, filter(fn_filter, src.all_children()))
		for s in src_children:
			 if s not in node_srcs:
			 	filenames.append(os.path.basename(str(s)))

	return filenames

########################################################################### find $ROOT -name $GLOB

def find(root, glob):
	files=[]
	for file in os.listdir(root):
		file=os.path.join(root, file)
		if fnmatch.fnmatch(file, glob):
			files.append(file)
		if os.path.isdir(file):
			files+=find(file, glob)
	return files

########################################################################### Create a phony target (not (yet) a feature of scons)

# taken from scons' wiki
def PhonyTarget(alias, action):
	"""Returns an alias to a command that performs the
	   action.  This is implementated by a Command with a
	   nonexistant file target.  This command will run on every
	   build, and will never be considered 'up to date'. Acts
	   like a 'phony' target in make."""

	from tempfile import mktemp
	from os.path import normpath

	phony_file = normpath(mktemp(prefix="phony_%s_" % alias, dir="."))
	return Alias(alias, Command(target=phony_file, source=None, action=action))

########################################################################### Functions for setting permissions when installing
# don't forget to set umask
try:
	os.umask(022)
except OSError:     # ignore on systems that don't support umask
	pass

def InstallPerm(env, dest, files, perm):
	obj = env.Install(dest, files)
	for i in obj:
		env.AddPostAction(i, env.Chmod(str(i), perm))

SConsEnvironment.InstallPerm = InstallPerm
SConsEnvironment.InstallProgram = lambda env, dest, files: InstallPerm(env, dest, files, 0755)
SConsEnvironment.InstallData = lambda env, dest, files: InstallPerm(env, dest, files, 0644)

################################################################################
# CLI options setup

def cli_options():
	opts=Options('build/scons-config.py', ARGUMENTS)
	opts.Add('build', 'debug-no-parachute / debug-slow / debug(default) / release / profile', 'debug')
	opts.Add('build_id', 'To get a default value(timestamp), leave this empty or set to \'date\'', '')
	opts.Add('sdlconfig', 'On some systems (e.g. BSD) this is called sdl12-config', 'sdl-config')
	opts.Add('paraguiconfig', '', 'paragui-config')
	opts.Add('install_prefix', '', '/usr/local')
	opts.Add('bindir', '(either absolut or relative to install_prefix)', 'games')
	opts.Add('datadir', '(either absolute or relative to install_prefix)', 'games/share/widelands')
	opts.Add('extra_include_path', '', '')
	opts.Add('extra_lib_path', '', '')
	opts.AddOptions(
		BoolOption('use_ggz', 'Use the GGZ Gamingzone?', 0),
		BoolOption('cross', 'Is this a cross compile? (developer use only)', 0)
		)
	return opts

################################################################################
# Environment setup
#
# Create configuration objects

opts=cli_options()

env=Environment(options=opts)
env.Help(opts.GenerateHelpText(env))

conf=env.Configure(conf_dir='#/build/sconf_temp',log_file='#build/config.log',
		   custom_tests={
				'CheckPKGConfig' : CheckPKGConfig,
				'CheckPKG': CheckPKG,
				'CheckSDLConfig': CheckSDLConfig,
				'CheckSDLVersionAtLeast': CheckSDLVersionAtLeast,
				'CheckParaguiConfig': CheckParaguiConfig
		   }
)

################################################################################
# Environment setup
#
# Register tools

env.Tool("ctags", toolpath=['build/scons-tools'])
env.Tool("PNGShrink", toolpath=['build/scons-tools'])
env.Tool("astyle", toolpath=['build/scons-tools'])

################################################################################
# Environment setup
#
# Initial debug info

#This makes LIBPATH work correctly - I just don't know why :-(
#Obviously, env.LIBPATH must be forced to be a list instead of a string. Is this
#a scons problem? Or rather our problem???
env.Append(LIBPATH=[])
env.Append(CPPPATH=[])
env.Append(PATH=[])

print 'Platform:         ', env['PLATFORM']
print 'Build type:       ', env['build']

if env['PLATFORM']!='win32':
	env.Append(PATH=['/usr/bin', '/usr/local/bin'])

if env['PLATFORM']=='darwin':
	# this is where DarwinPorts puts stuff by default
	env.Append(CPPPATH='/opt/local/include')
	env.Append(LIBPATH='/opt/local/lib')
	env.Append(PATH='/opt/local/bin')
	# and here's for fink
	env.Append(CPPPATH='/sw/include')
	env.Append(LIBPATH='/sw/lib')
	env.Append(PATH='/sw/bin')

################################################################################
# Autoconfiguration
#
# Parse build type

DEBUG=0
PROFILE=0
OPTIMIZE=0
SDL_PARACHUTE=1
if env['build']=='debug-no-parachute':
	DEBUG=1
	OPTIMIZE=1
	SDL_PARACHUTE=0

if env['build']=='debug-slow':
	DEBUG=1

if env['build']=='debug':
	DEBUG=1
	OPTIMIZE=1

if env['build']=='profile':
	DEBUG=1
	PROFILE=1
	OPTIMIZE=1

if env['build']=='release':
	OPTIMIZE=1
	# !!!! -fomit-frame-pointer breaks execeptions !!!!
	env.Append(CCFLAGS=['-finline-functions', '-ffast-math', '-funroll-loops'])
	env.Append(CCFLAGS='-fexpensive-optimizations')
	env.Append(LINKFLAGS='-s')

if DEBUG:
	env.Append(CCFLAGS=['-g', '-DDEBUG', '-fmessage-length=0'])
else:
	env.Append(CCFLAGS='-DNDEBUG')

if PROFILE:
	env.Append(CCFLAGS=['-pg', '-fprofile-arcs'])
	env.Append(LINKFLAGS=['-pg', '-fprofile-arcs'])

if OPTIMIZE:
	# heavy optimization
	#ADD_CCFLAGS:=$(ADD_CCFLAGS) -fomit-frame-pointer -finline-functions
	#                -ffast-math -funroll-loops -fexpensive-optimizations
	# !!!! -fomit-frame-pointer breaks execeptions !!!!
	env.Append(CCFLAGS='-O3')
else:
	env.Append(CCFLAGS='-O0')

if not SDL_PARACHUTE:
	env.Append(CCFLAGS='-DNOPARACHUTE')

################################################################################

TARGET=parse_cli(env)

env.Append(CPPPATH=env['extra_include_path'])
env.Append(LIBPATH=env['extra_lib_path'])

BINDIR= os.path.join(env['install_prefix'], env['bindir'])
DATADIR=os.path.join(env['install_prefix'], env['datadir'])
PACKDIR='widelands-'+env['build_id']

#TODO: make sure that build type is valid !!!

################################################################################

opts.Save('build/scons-config.py',env) #build_id must be saved *before* it might be set to a fixed date

#This is just a default, do not change it here. Use the option 'build_id' instead.
if (env['build_id']=='') or (env['build_id']=='date'):
	env['build_id']=time.strftime("%Y.%m.%d-%H%M%S", time.gmtime())
print 'Build ID:          '+env['build_id']

config_h=write_configh_header()
do_configure(config_h, conf, env)
write_configh_footer(config_h, env['install_prefix'], BINDIR, DATADIR)
#load_configuration(conf)

env.Append(CCFLAGS=['-pipe', '-Wall', '-Wextra', '-Wno-comment'])
#env.Append(LINKFLAGS='-lasprintf') #TODO: *check* for this instead of assuming it's there

env=conf.Finish()

# Pretty output
print

########################################################################### Use distcc if available

# not finished yet
#if os.path.exists('/usr/lib/distcc/bin'):
#	env['ENV']['DISTCC_HOSTS'] = os.environ['DISTCC_HOSTS']
#	env['ENV']['PATH'] = '/usr/lib/distcc/bin:'+env['ENV']['PATH']
#	env['ENV']['HOME'] = os.environ['HOME']

############################################################################ Build things

############### Build setup

SConsignFile('build/scons-signatures')
BUILDDIR='build/'+TARGET+'-'+env['build']
Export('env', 'Glob', 'BUILDDIR', 'PhonyTarget')

############### buildcat

buildcat=SConscript('locale/SConscript')

############### The binary

thebinary=SConscript('src/SConscript', build_dir=BUILDDIR, duplicate=0)
Default(thebinary)

############### tags

S=find('src', '*.h')
S+=find('src', '*.cc')
Alias('tags', env.ctags(source=S, target='tags'))
if env['build'] == 'release':
	Default('tags')

############### PNG shrinking

# findfiles takes quite long, so don't execute it if it's unneccessary
if ('shrink' in BUILD_TARGETS) or ('dist' in BUILD_TARGETS):
	shrink=env.PNGShrink(find('.', '*.png'))
	Alias("shrink", shrink)

############### Install and uninstall

INSTDIRS=[
	'campaigns',
	'fonts',
	'game_server',
	'maps',
	'music',
	'pics',
	'sound',
	'tribes',
	'txts',
	'worlds'
	]
INSTDIRS+=glob.glob("locale/??_??")

#TODO: need to install stuff like licenses - where do they go?
def do_inst(target, source, env):
	if not os.path.exists(BINDIR):
		os.makedirs(BINDIR, 0755)
	print 'Installing ', os.path.join(BINDIR, 'widelands')
	shutil.copy(os.path.join(BUILDDIR, 'widelands'), BINDIR)

	shutil.rmtree(DATADIR, ignore_errors=1)
	os.makedirs(DATADIR, 0755)

	for f in INSTDIRS:
		print 'Installing ', os.path.join(DATADIR, f)
		shutil.copytree(f, os.path.join(DATADIR, f))

	RMFILES=find(DATADIR, '*/.cvsignore')
	RMDIRS =find(DATADIR, '*/CVS')

	print 'Removing superfluous files ...'
	for f in RMFILES:
		os.remove(f)
	for f in RMDIRS:
		shutil.rmtree(f)

def do_uninst(target, source, env):
	print 'Removing ', DATADIR
	shutil.rmtree(DATADIR, ignore_errors=1)

	if os.path.exists(os.path.join(BINDIR, 'widelands')):
		print 'Removing ', os.path.join(BINDIR, 'widelands')
		os.remove(os.path.join(BINDIR, 'widelands'))

install=PhonyTarget("install", do_inst)
uninstall=PhonyTarget("uninstall", do_uninst)

############### Distribute

DISTFILES=[
	'COPYING',
	'README-compiling.txt',
	'README.developers',
	'SConstruct',
	'ChangeLog',
	'Doxyfile',
	'build-widelands.sh',
	]
DISTDIRS=[
	'build',
	'campaigns',
	'fonts',
	'maps',
	'music',
	'pics',
	'src',
	'sound',
	'txts',
	'tribes',
	'utils',
	'worlds',
	]

def do_dist(target, source, env):
	shutil.rmtree(PACKDIR, ignore_errors=1)
	os.mkdir(PACKDIR, 0755)

	for f in DISTFILES:
		shutil.copy(f, PACKDIR)
	for f in DISTDIRS:
		shutil.copytree(f, os.path.join(PACKDIR, f))

	RMFILES=find(PACKDIR, '*/.cvsignore')
	#TODO: do include a scons-config.py with suitable content, esp. build number
	RMFILES+=[PACKDIR+'/build/scons-config.py']
	RMFILES+=[PACKDIR+'/build/scons-signatures.dblite']

	RMDIRS =find(PACKDIR, '*/CVS')
	RMDIRS+=glob.glob(PACKDIR+'/build/*-debug*')
	RMDIRS+=glob.glob(PACKDIR+'/build/*-release')
	RMDIRS+=glob.glob(PACKDIR+'/build/*-profile')
	RMDIRS+=[PACKDIR+'/build/sconf_temp']

	for f in RMFILES:
		os.remove(f)
	for f in RMDIRS:
		shutil.rmtree(f)

	#TODO: use sth. else on systems where tar is not supported
	os.system('tar -czf %s %s' % (PACKDIR+'.tar.gz', PACKDIR))
	shutil.rmtree(PACKDIR)

	return None

dist=PhonyTarget("dist", do_dist)

############### longlines

longlines=PhonyTarget("longlines", 'utils/count-longlines.py')

############### precommit

Alias('precommit', 'indent')
Alias('precommit', buildcat)
Alias('precommit', 'longlines')

############### Documentation

PhonyTarget('doc', 'doxygen Doxyfile')

