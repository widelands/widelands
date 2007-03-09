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
from Distribute import *
from detect_revision import *

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

######################################################### find $ROOT -name $GLOB

def find(root, glob):
	files=[]
	for file in os.listdir(root):
		file=os.path.join(root, file)
		if fnmatch.fnmatch(file, glob):
			files.append(file)
		if os.path.isdir(file):
			files+=find(file, glob)
	return files

########################### Create a phony target (not (yet) a feature of scons)

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

############################## Functions for setting permissions when installing
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
	opts.Add('build', 'debug-no-parachute / debug-slow / debug-efence / debug / release(default) / profile', 'release')
	opts.Add('build_id', 'To get a default value (SVN revision), leave this empty', '')
	opts.Add('sdlconfig', 'On some systems (e.g. BSD) this is called sdl12-config', 'sdl-config')
	opts.Add('paraguiconfig', '', 'paragui-config')
	opts.Add('install_prefix', '', '/usr/local')
	opts.Add('bindir', '(relative to install_prefix)', 'games')
	opts.Add('datadir', '(relative to install_prefix)', 'share/games/widelands')
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
				'CheckCompilerAttribute': CheckCompilerAttribute,
				'CheckCompilerFlag': CheckCompilerFlag,
				'CheckLinkerFlag': CheckLinkerFlag,
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
env.Tool("Distribute", toolpath=['build/scons-tools'])

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

#TODO: should be detected automagically
if env['PLATFORM']!='win32':
	env.Append(PATH=['/usr/bin', '/usr/local/bin'])

#TODO: should be detected automagically
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
STRIP=0
EFENCE=0
if env['build']=='debug-no-parachute':
	DEBUG=1
	OPTIMIZE=1
	SDL_PARACHUTE=0

if env['build']=='debug-slow':
	DEBUG=1

if env['build']=='debug':
	DEBUG=1
	OPTIMIZE=1

if env['build']=='debug-efence':
	DEBUG=1
	OPTIMIZE=1
	EFENCE=1

if env['build']=='profile':
	DEBUG=1
	OPTIMIZE=1
	PROFILE=1

if env['build']=='release':
	OPTIMIZE=1
	STRIP=1

if DEBUG:
	env.debug=1
	env.Append(CCFLAGS='-DDEBUG')
else:
	env.debug=0
	env.Append(CCFLAGS='-DNDEBUG')

if PROFILE:
	env.profile=1
else:
	env.profile=0

if OPTIMIZE:
	env.optimize=1
else:
	env.optimize=0

if EFENCE:
	env.efence=1
else:
	env.efence=0

if not SDL_PARACHUTE:
	env.Append(CCFLAGS='-DNOPARACHUTE')

if STRIP:
	env.strip=1
else:
	env.strip=0

################################################################################

TARGET=parse_cli(env)

env.Append(CPPPATH=env['extra_include_path'])
env.Append(LIBPATH=env['extra_lib_path'])

BINDIR= os.path.join(env['install_prefix'], env['bindir'])
DATADIR=os.path.join(env['install_prefix'], env['datadir'])

#TODO: make sure that build type is valid !!!

################################################################################

#This is just a default, don't change it here in the code.
#Use the commandline option 'build_id' instead
if env['build_id']=='':
	env['build_id']='svn'+detect_revision()
print 'Build ID:          '+env['build_id']

config_h=write_configh_header()
do_configure(config_h, conf, env)
write_configh_footer(config_h, env['install_prefix'], BINDIR, DATADIR)
write_buildid(env['build_id'])

env=conf.Finish()

# Pretty output
print

######################################################## Use distcc if available

# not finished yet
#if os.path.exists('/usr/lib/distcc/bin'):
#	env['ENV']['DISTCC_HOSTS'] = os.environ['DISTCC_HOSTS']
#	env['ENV']['PATH'] = '/usr/lib/distcc/bin:'+env['ENV']['PATH']
#	env['ENV']['HOME'] = os.environ['HOME']

################################################################### Build things

SConsignFile('build/scons-signatures')
BUILDDIR='build/'+TARGET+'-'+env['build']
Export('env', 'Glob', 'BUILDDIR', 'PhonyTarget')

####################################################################### buildcat

SConscript('build/SConscript')
SConscript('campaigns/SConscript')
SConscript('doc/SConscript')
SConscript('fonts/SConscript')
SConscript('game_server/SConscript')
buildcat=SConscript('locale/SConscript')
SConscript('maps/SConscript')
SConscript('music/SConscript')
SConscript('pics/SConscript')
SConscript('sound/SConscript')
SConscript('src/SConscript.dist')
thebinary=SConscript('src/SConscript', build_dir=BUILDDIR, duplicate=0)
SConscript('tribes/SConscript')
SConscript('txts/SConscript')
SConscript('utils/SConscript')
SConscript('worlds/SConscript')

Default(thebinary)

########################################################################### tags

S=find('src', '*.h')
S+=find('src', '*.cc')
Alias('tags', env.ctags(source=S, target='tags'))
if env['build'] == 'release':
	Default('tags')

################################################################## PNG shrinking

# findfiles takes quite long, so don't execute it if it's unneccessary
if ('shrink' in BUILD_TARGETS):
	print "Assembling file list for image compactification..."
	shrink=env.PNGShrink(find('.', '*.png'))
	Alias("shrink", shrink)

########################################################## Install and uninstall

instadd(env, 'ChangeLog', 'doc')
instadd(env, 'COPYING', 'doc')
instadd(env, 'CREDITS', 'doc')
instadd(env, 'widelands', filetype='binary')

install=env.Install('installtarget', '')
Alias('install', install)
AlwaysBuild(install)

uninstall=env.Uninstall('uninstalltarget', '')
Alias('uninstall', uninstall)
AlwaysBuild(uninstall)

##################################################################### Distribute

distadd(env, 'ChangeLog')
distadd(env, 'COPYING')
distadd(env, 'CREDITS')
distadd(env, 'Doxyfile')
distadd(env, 'Makefile')
distadd(env, 'README-compiling.txt')
distadd(env, 'README.developers')
distadd(env, 'SConstruct')
distadd(env, 'build-widelands.sh')
distadd(env, 'macos')

dist=env.DistPackage('widelands-'+env['build_id'], '')
Alias('dist', dist)
AlwaysBuild(dist)

###################################################################### longlines

longlines=PhonyTarget("longlines", 'utils/count-longlines.py')

###################################################################### precommit

#Alias('precommit', 'indent')
Alias('precommit', buildcat)
Alias('precommit', 'longlines')

################################################################## Documentation

PhonyTarget('doc', 'doxygen Doxyfile')
