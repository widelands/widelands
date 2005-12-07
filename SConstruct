import os
import sys
import fnmatch
import subprocess
import SCons
from SCons.Script.SConscript import SConsEnvironment

#Speedup. If you have problems with inconsistent or wrong builds, look here first
SetOption('max_drift', 1)
SetOption('implicit_cache', 1)

##########################################################
# glob.glob does not work with BuildDir(), so use the following replacement from
# http://www.scons.org/cgi-bin/wiki/BuildDirGlob?highlight=%28glob%29
# which I modified slightly to return a list of filenames instead of nodes
def Glob(match):
	"""Similar to glob.glob, except globs SCons nodes, and thus sees
	generated files and files from build directories.  Basically, it sees
	anything SCons knows about.  A key subtlety is that since this function
	operates on generated nodes as well as source nodes on the filesystem,
	it needs to be called after builders that generate files you want to
	include."""
	
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
	
########################################################################### Functions for setting permissions when installing
# don't forget to set umask
try:
	os.umask(022)
except OSError:     # ignore on systems that don't support umask
	pass

SConsEnvironment.Chmod = SCons.Action.ActionFactory(os.chmod,
			lambda dest, mode: 'Chmod("%s", 0%o)' % (dest, mode))

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
	context.Message( 'Checking SDL version >=%s ... ' % (repr(major)+'.'+repr(minor)+'.'+repr(micro)))
	#TODO: this check will always return true!
	version=subprocess.Popen([env['sdlconfig'], "--version"], stdout=subprocess.PIPE).communicate()[0]
	maj=version.split('.')[0]
	min=version.split('.')[1]
	mic=version.split('.')[2]
	if int(maj)>=int(major) and int(min)>=int(minor) and int(mic)>=int(micro): ret=1
	else: ret=0
	context.Result( ret )
	return ret

############################################################################ Options

opts=Options('SConstruct.local')
opts.Add('build', 'debug-no-parachute / debug-slow / debug(default) / release / profile', 'debug')
opts.Add('sdlconfig', 'On some systems (e.g. BSD) this is called sdl12-config', 'sdl-config')
opts.Add('install_prefix', '', '.')
opts.Add('bindir', '(relative to prefix)', 'bin')
opts.Add('datadir', '(relative to prefix)', 'share/widelands')
opts.Add('extra_include_path', '', '')
opts.Add('extra_lib_path', '', '')
opts.AddOptions(
	BoolOption('use_ggz', 'Use the GGZ Gamingzone?', 'no'),
	BoolOption('cross', 'Is this a cross compile? (developer use only)', 'no')
	)
	
env=Environment(options=opts)
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
	print 'Cross-compiling doe not work yet!'
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

opts.Save('build/scons-config.py',env)
	
############################################################################ Configure - Autodetection

if not conf.CheckLibWithHeader('intl', header='locale.h', language='C', autoadd=1):
	if conf.CheckHeader('locale.h'):
		print 'Looks like you do have the gettext library.'
	else:
		print 'Could not find gettext library! Is it installed?' 
		Exit(1)

if not conf.CheckSDLConfig():
	print 'Could not find sdl-config! Is SDL installed?'
	Exit(1)

if not conf.CheckSDLVersionAtLeast(1, 2, 8):
	print 'Could not find an SDL version >= 1.2.8!'
	Exit(1)

env.ParseConfig(env['sdlconfig']+' --libs')
env.ParseConfig(env['sdlconfig']+' --cflags')

if not conf.CheckLibWithHeader('z', header='zlib.h', language='C', autoadd=1):
	print 'Could not find the zlib library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('png', header='png.h', language='C', autoadd=1):
	print 'Could not find the png library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('SDL_image', header='SDL/SDL_image.h', language='C', autoadd=1):
	print 'Could not find the SDL_image library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('SDL_ttf', header='SDL/SDL_ttf.h', language='C', autoadd=1):
	print 'Could not find the SDL_ttf library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('SDL_net', header='SDL/SDL_net.h', language='C', autoadd=1):
	print 'Could not find the SDL_net library! Is it installed?'
	Exit(1)

if not conf.CheckLibWithHeader('SDL_mixer', header='SDL/SDL_mixer.h', language='C', autoadd=1):
	print 'Could not find the SDL_mixer library! Is it installed?'
	Exit(1)

if not conf.TryLink(""" #include <SDL/SDL.h>
			#include <SDL/SDL_mixer.h>
			main(){
				Mix_LoadMUS("foo.ogg");
			}
			""", '.c'):
	env.Append(CCFLAGS=' -DOLD_SDL_MIXER')

env.Append(CCFLAGS=' -pipe -Wall')

env=conf.Finish()

########################################################################### Use distcc if available

# not finished yet

#if os.path.exists('/usr/lib/distcc/bin'):
#	env['ENV']['DISTCC_HOSTS'] = os.environ['DISTCC_HOSTS']
#	env['ENV']['PATH'] = '/usr/lib/distcc/bin:'+env['ENV']['PATH']
#	env['ENV']['HOME'] = os.environ['HOME']

############################################################################ Build things

SConsignFile('build/scons-signatures')

TARGETDIR='build/'+TARGET+'-'+env['build']

Export('env', 'Glob')
SConscript('src/SConscript', build_dir='#'+TARGETDIR, duplicate=0)

env.Alias("copythebinary", Command('thephonyfile1', '', 'cp '+TARGETDIR+'/widelands .'))
env.Depends('copythebinary', TARGETDIR+'/widelands')

env.Alias("tags", Command('thephonyfile2', '', 'ctags --recurse=yes src/'))

Default('tags')
Default('copythebinary')

############################################################################ Install and Distribute

env.Alias("install", env.Install(env['install_prefix']+'/'+env['bindir'], TARGETDIR+'/widelands'))
DATADIR=env['install_prefix']+'/'+env['datadir']
env.Alias("install", Command('thephonyfile3', '', [
						Delete(DATADIR, must_exist=0),
						Mkdir(DATADIR),
						Copy(DATADIR+'/campaigns', 'campaigns'),
						Copy(DATADIR+'/fonts', 'fonts'),
						Copy(DATADIR+'/locale', 'locale'),
						Copy(DATADIR+'/maps', 'maps'),
						Copy(DATADIR+'/music', 'music'),
						Copy(DATADIR+'/pics', 'pics'),
						Copy(DATADIR+'/sound', 'sound'),
						Copy(DATADIR+'/worlds', 'worlds'),
						"find "+DATADIR+" -name CVS -exec rm -rf {} \;",
						"find "+DATADIR+" -name .cvsignore -exec rm -rf {} \;"
						]))

env.Alias("uninstall", Command('thephonyfile4', '', [
						Delete(env['install_prefix']+'/'+env['bindir']+'/widelands', must_exist=0),
						Delete(DATADIR)
						]))

env.Append(TARFLAGS=' -z')
env.Alias("dist", Command('thephonyfile5', '', [
						Delete('widelands-b9half', must_exist=0),
						Mkdir('widelands-b9half'),
						"cp -r ChangeLog fonts maps pics README-compiling.txt txts widelands-b9half",
						"cp -r COPYING Makefile README.developers tribes worlds widelands-b9half",
						"cp -r Doxyfile SConstruct widelands-b9half",
						"cp -r src widelands-b9half"
						]))
env.Alias("dist", Command('thephonyfile6', '', [
						"find widelands-b9half -name .cvsignore -exec rm -rf {} \;",
						"find widelands-b9half -name .sconsign -exec rm -rf {} \;",
						"find widelands-b9half -name CVS/ -exec rm -rf {} \;"
						]))
env.Alias("dist", env.Tar('widelands-b9half.tar.gz', 'widelands-b9half'))
env.Alias("dist", Command('thephonyfile7', '', Delete('widelands-b9half')))

############################################################################ Extra targets

env.Alias("up", Command('thephonyfile8', '', 'cvs -q up -APd'))
env.Alias("update", Command('thephonyfile8', '', 'cvs -q up -APd'))
env.Alias("doc", Command('thephonyfile9', '', 'doxygen Doxyfile'))

