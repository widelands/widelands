import os
import shutil
import fnmatch
import SCons
import time
import glob
from SCons.Script.SConscript import SConsEnvironment
import string

#Speedup. If you have problems with inconsistent or wrong builds, look here first
SetOption('max_drift', 1)
SetOption('implicit_cache', 1)

########################################################################### verbatim copy from env.ParseConfig for parsing `sdl-config`
########################################################################### it's a nested function there, so we can't use it directly
#TODO: this can be dropped once we use scons-0.97

def parse_conf(env, output):
            dict = {
                'ASFLAGS'       : [],
                'CCFLAGS'       : [],
                'CPPFLAGS'      : [],
                'CPPPATH'       : [],
                'LIBPATH'       : [],
                'LIBS'          : [],
                'LINKFLAGS'     : [],
            }
            static_libs = []

            params = string.split(output)
            for arg in params:
                if arg[0] != '-':
                    static_libs.append(arg)
                elif arg[:2] == '-L':
                    dict['LIBPATH'].append(arg[2:])
                elif arg[:2] == '-l':
                    dict['LIBS'].append(arg[2:])
                elif arg[:2] == '-I':
                    dict['CPPPATH'].append(arg[2:])
                elif arg[:4] == '-Wa,':
                    dict['ASFLAGS'].append(arg)
                elif arg[:4] == '-Wl,':
                    dict['LINKFLAGS'].append(arg)
                elif arg[:4] == '-Wp,':
                    dict['CPPFLAGS'].append(arg)
                elif arg == '-pthread':
                    dict['CCFLAGS'].append(arg)
                    dict['LINKFLAGS'].append(arg)
                else:
                    dict['CCFLAGS'].append(arg)
            apply(env.Append, (), dict)
            return static_libs



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

########################################################################### Configure functions

def CheckPKGConfig(context, version):
	context.Message( 'Checking for pkg-config... ' )
	ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
	context.Result( ret )
	return ret

def CheckPKG(context, name):
	context.Message( 'Checking for %s... ' % name )
	ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
	context.Result( ret )
	return ret

def CheckSDLConfig(context):
	context.Message( 'Checking for sdl-config... ' )
	ret = context.TryAction(env['sdlconfig']+' --version')[0]
	context.Result( ret )
	return ret

def CheckSDLVersionAtLeast(context, major, minor, micro):
	context.Message( 'Checking SDL version >= %s ... ' % (repr(major)+'.'+repr(minor)+'.'+repr(micro)))
	version=os.popen(env['sdlconfig']+" --version", "r").read()
	maj=version.split('.')[0]
	min=version.split('.')[1]
	mic=version.split('.')[2]
	if int(maj)>=int(major) and int(min)>=int(minor) and int(mic)>=int(micro): ret=1
	else: ret=0
	context.Result( ret )
	return ret

#TODO: this can be dropped once we use scons-0.97
def ParseSDLConfig(env, confstring):
	words=confstring.split()

	print words
	print env['LINKFLAGS']

	for i, w in enumerate(words):
		print i, w
		if w=='-framework':
			#remove implicitly causes the new element i to be the former i+1, so the next two lines remove two consecutive tokens
			words.remove(w)
			w2=words.pop(i)
			env['LINKFLAGS']+=w+' '+w2

	print words
	print env['LINKFLAGS']

	print string.join(words)

	# problematic flags have been taken care of, call the standard parser
	parse_conf(env, string.join(words))

	return

############################################################################ Options

opts=Options('build/scons-config.py', ARGUMENTS)
opts.Add('build', 'debug-no-parachute / debug-slow / debug(default) / release / profile', 'debug')
opts.Add('build_id', 'To get a default value(timestamp), leave this empty or set to \'date\'', '')
opts.Add('sdlconfig', 'On some systems (e.g. BSD) this is called sdl12-config', 'sdl-config')
opts.Add('install_prefix', '', '.')
opts.Add('bindir', '(relative to install_prefix)', 'bin')
opts.Add('datadir', '(relative to install_prefix)', 'share/widelands')
opts.Add('extra_include_path', '', '')
opts.Add('extra_lib_path', '', '')
opts.AddOptions(
	BoolOption('use_ggz', 'Use the GGZ Gamingzone?', 0),
	BoolOption('cross', 'Is this a cross compile? (developer use only)', 0)
	)

env=Environment(options=opts)
print 'Platform:', env['PLATFORM']

if env['PLATFORM']=='darwin':
	# this is where DarwinPorts puts stuff by default
	env.Append(CPPPATH='/opt/local/include')
	env.Append(LIBPATH='/opt/local/lib')
	env.Append(PATH='/opt/local/bin')

env.Help(opts.GenerateHelpText(env))
env.Append(CPPPATH=env['extra_include_path'])
env.Append(LIBPATH=env['extra_lib_path'])

############################################################################ Configure - Command Line Options

conf=env.Configure(conf_dir='#/build/sconf_temp',log_file='#build/config.log',custom_tests={
				'CheckPKGConfig' : CheckPKGConfig,
				'CheckPKG': CheckPKG,
				'CheckSDLConfig': CheckSDLConfig,
				'CheckSDLVersionAtLeast': CheckSDLVersionAtLeast
				})
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
	env.Append(CCFLAGS=' -finline-functions -ffast-math -funroll-loops -funroll-all-loops -fexpensive-optimizations')
	env.Append(LINKFLAGS=[' -s'])

if DEBUG:
	env.Append(CCFLAGS=' -g -DDEBUG -fmessage-length=0')
else:
	env.Append(CCFLAGS=' -DNDEBUG')

if PROFILE:
	env.Append(CCFLAGS=' -pg -fprofile-arcs')

if OPTIMIZE:
	# heavy optimization
	#ADD_CCFLAGS:=$(ADD_CCFLAGS) -fomit-frame-pointer -finline-functions -ffast-math -funroll-loops -funroll-all-loops -fexpensive-optimizations
	# !!!! -fomit-frame-pointer breaks execeptions !!!!
	env.Append(CCFLAGS=' -O3')
else:
	env.Append(CCFLAGS=' -O0')

if not SDL_PARACHUTE:
	env.Append(CCFLAGS=' -DNOPARACHUTE')

if env['use_ggz']:
	env.Append(CCFLAGS=' -DGGZ')
	env.Append(LIBS=['ggzmod', 'ggzcore', 'ggz'])

if env['cross']:
	print 'Cross-compiling does not work yet!'
	Exit(1)
	#TARGET='i586-mingw32msvc'
	#PREFIX='/usr/local/cross-tools'
	#env['ENV']['PATH']=PREFIX+'/'+TARGET+'/bin:'+PREFIX+'/bin'+env['ENV']['PATH']
	#env['CXX']=TARGET+'-g++'
	### manually overwrite
	###env['sdlconfig']=PREFIX+'/bin/'+TARGET+'-sdl_config'
	#env['sdlconfig']=PREFIX+'/'+TARGET+'/bin/'+TARGET+'-sdl-config'
else:
	TARGET='native'

BINDIR= os.path.join(env['install_prefix'], env['bindir'])
DATADIR=os.path.join(env['install_prefix'], env['datadir'])
PACKDIR='widelands-'+env['build_id']

############################################################################ Configure - build_id.h

opts.Save('build/scons-config.py',env) #build_id must be saved *before* it might be set to a fixed date

#This is just a default, do not change it here. Use the option 'build_id' instead.
if (env['build_id']=='') or (env['build_id']=='date'):
	env['build_id']=time.strftime("%Y.%m.%d-%H%M%S", time.gmtime())

print 'Build ID: '+env['build_id']
build_id_file=open('src/build_id.h', "w")

build_id_file.write("""
#ifndef BUILD_ID
#define BUILD_ID """+env['build_id']+"""

const char *g_build_id=\""""+env['build_id']+"""\";

#endif

""")

build_id_file.close()

config_h_file=open('src/config.h', "w")
config_h_file.write("""
#ifndef CONFIG_H
#define CONFIG_H

""")

############################################################################ Configure - Tool autodetection

env.Tool("ctags", toolpath=['build/scons-tools'])
env.Tool("PNGShrink", toolpath=['build/scons-tools'])
env.Tool("astyle", toolpath=['build/scons-tools'])

############################################################################ Configure - Library autodetection

#if not conf.CheckLibWithHeader('intl', header='libintl.h', language='C', call='gettext("test");', autoadd=1):
#	if conf.CheckHeader('locale.h'):
#		print 'Looks like gettext is included in your libc.'
#	else:
#		print 'Could not find gettext library! Is it installed?'
#		Exit(1)

if conf.CheckFunc('setlocale') and (conf.CheckFunc('textdomain') or conf.CheckLib(library='intl', symbol='textdomain', autoadd=1)):
	print '   NLS subsystem found.'
else:
	#TODO: use dummy replacements that just pass back the original string
	print '   No usable NLS subsystem found. Please install gettext.'
	Exit(1)

if not conf.CheckFunc('getenv'):
	print 'Your system does not support getenv(). Tilde epansion in filenames will not work.'
else:
	config_h_file.write("#define HAS_GETENV\n\n");

if not conf.CheckSDLConfig():
	print 'Could not find sdl-config! Is SDL installed?'
	Exit(1)

if not conf.CheckSDLVersionAtLeast(1, 2, 8):
	print 'Could not find an SDL version >= 1.2.8!'
	Exit(1)

env.ParseConfig(env['sdlconfig']+' --libs --cflags', ParseSDLConfig)

if not conf.CheckLibWithHeader('z', header='zlib.h', language='C', autoadd=1):
	print 'Could not find the zlib library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('png', header='png.h', language='C', autoadd=1):
	print 'Could not find the png library! Is it installed?'
	Exit(1)

if not conf.CheckLib(library='SDL_image', symbol='IMG_Load', autoadd=1):
	print 'Could not find the SDL_image library! Is it installed?'
	Exit(1)

if not conf.CheckLib(library='SDL_ttf', symbol='TTF_Init', autoadd=1):
	print 'Could not find the SDL_ttf library! Is it installed?'
	Exit(1)

if not conf.CheckLib(library='SDL_net', symbol='SDLNet_TCP_Open', autoadd=1):
	print 'Could not find the SDL_net library! Is it installed?'
	Exit(1)

if not conf.CheckLib(library='SDL_mixer', symbol='Mix_OpenAudio', autoadd=1):
	print 'Could not find the SDL_mixer library! Is it installed?'
	Exit(1)

if conf.TryLink(""" #include <SDL.h>
			#define USE_RWOPS
			#include <SDL_mixer.h>
			main(){
				Mix_LoadMUS_RW("foo.ogg");
			}
			""", '.c'):
	config_h_file.write("//second line is needed by SDL_mixer\n");
	config_h_file.write("#define NEW_SDL_MIXER 1\n");
	config_h_file.write("#define USE_RWOPS\n\n");
	print 'SDL_mixer supports Mix_LoadMUS_RW(). Good'
else:
	config_h_file.write("#define NEW_SDL_MIXER 0\n\n");
	print 'SDL_mixer does not support Mix_LoadMUS_RW(). Widelands will run without problems, but consider updating SDL_mixer anyway.'

env.Append(CCFLAGS=' -pipe -Wall')

env=conf.Finish()

########################################################################### Use distcc if available

# not finished yet
#if os.path.exists('/usr/lib/distcc/bin'):
#	env['ENV']['DISTCC_HOSTS'] = os.environ['DISTCC_HOSTS']
#	env['ENV']['PATH'] = '/usr/lib/distcc/bin:'+env['ENV']['PATH']
#	env['ENV']['HOME'] = os.environ['HOME']

############################################################################ Configure - finish config.h

config_h_file.write("#define INSTALL_DATADIR \""+DATADIR+"\"\n\n")

config_h_file.write("\n#endif\n")
config_h_file.close()

############################################################################ Build things


############### Build setup

SConsignFile('build/scons-signatures')

BUILDDIR='build/'+TARGET+'-'+env['build']

Export('env', 'Glob', 'BUILDDIR')

############### The binary

thebinary=SConscript('src/SConscript', build_dir=BUILDDIR, duplicate=0)

############### tags

#we'd only need the first directory component - but directories cannot be passed via source=
S=find('src', '*.h')
S+=find('src', '*.cc')
Alias('tags', env.ctags(source=S, target='tags'))
Default('tags')

############### PNG shrinking

# findfiles takes quite long, so don't execute it if it's unneccessary
if ('shrink' in BUILD_TARGETS) or ('dist' in BUILD_TARGETS):
	shrink=env.PNGShrink(find('.', '*.png'))
	Alias("shrink", shrink)

############### Install and uninstall

DISTDIRS=[
	'campaigns',
	'fonts',
	'game-server',
	'locale',
	'maps',
	'music',
	'pics',
	'sound',
	'tribes',
	'txts',
	'worlds'
	]

#TODO: need to install stuff like licenses - where do they go?
def do_inst(target, source, env):
	if not os.path.exists(BINDIR):
		os.makedirs(BINDIR, 0755)
	print 'Installing ', os.path.join(BINDIR, 'widelands')
	shutil.copy(os.path.join(BUILDDIR, 'widelands'), BINDIR)

	shutil.rmtree(DATADIR, ignore_errors=1)
	os.makedirs(DATADIR, 0755)

	for f in DISTDIRS:
		if f=='build':
			continue
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
	'fonts',
	'maps',
	'music',
	'pics',
	'src',
	'sound',
	'txts',
	'tribes',
	'utils',
	'worlds'
	]

def do_dist(target, source, env):
	shutil.rmtree(PACKDIR, ignore_errors=1)
	os.mkdir(PACKDIR, 0755)

	for f in DISTFILES:
		shutil.copy(f, PACKDIR)
	for f in DISTDIRS:
		shutil.copytree(f, os.path.join(PACKDIR, f))

	RMFILES=find(PACKDIR, '*/.cvsignore')
	#TODO: do include a scons-config with suitable content, esp. build number
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

	os.system('tar -czf %s %s' % (PACKDIR+'.tar.gz', PACKDIR))
	shutil.rmtree(PACKDIR)

	return None

dist=PhonyTarget("dist", do_dist)

############### CVS

cvsup=PhonyTarget('up', 'cvs -q up -APd')
Alias('update', cvsup)

############### Documentation

PhonyTarget('doc', 'doxygen Doxyfile')

