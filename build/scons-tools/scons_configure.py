# -*- coding: utf-8 -*-
import os, sys
import string

from detect_revision import *

################################################################################

def print_build_info(env):
	print
	print 'Platform:         ', env['PLATFORM']
	print 'Build type:       ', env['build']
	print 'Build ID:         ', get_build_id(env)
	print

def get_build_id(env):
	#This is just a default value, don't change it here in the code.
	#Use the commandline option 'build_id' instead
	if env['build_id']=='':
	        env['build_id']=detect_revision()

	return env['build_id']

def generate_buildinfo_file(env, target, source):
	build_info_file=open(target[0].path, "w")

	build_info_file.write("""#include "build_info.h"

std::string build_id()
{
	return \""""+source[0].get_contents()+"""\";
}

std::string build_type()
{
	return \""""+source[1].get_contents()+"""\";
}
""")

	build_info_file.close()

################################################################################

def parse_cli(env, buildtargets):
        env.enable_configuration=True
	env.debug=0
	env.optimize=0
	env.strip=0
	env.efence=0
	env.profile=0

	TARGET='native'

	BUILDDIR='build/'+TARGET+'-'+env['build']

        if env.GetOption('clean'):
                env.enable_configuration=False
		return BUILDDIR
        if '-h' in sys.argv[1:]:
                env.enable_configuration=False
		return BUILDDIR
        if '-H' in sys.argv[1:]:
                env.enable_configuration=False
		return BUILDDIR

	env.enable_configuration=configure_is_needed(buildtargets)

	#This makes LIBPATH work correctly - I just don't know why :-(
	#Obviously, env.LIBPATH must be forced to be a list instead of a string. Is this
	#a scons problem? Or rather our problem???
	env.Append(LIBPATH=[])
	env.Append(CPPPATH=[])
	env.Append(PATH=[])

	#TODO: should be detected automagically
	env.Append(PATH=['/usr/bin', '/usr/local/bin'])

	if env['PLATFORM']=='win32':
	        env.Append(ENV=os.environ)
	        env.Append(PATH='/mingw/bin')
	        env.Append(CPPPATH=['/mingw/include', '/mingw/include/SDL'])
	        env.Append(LIBPATH='/mingw/lib')
	        env.Append(LIBS=['intl', 'wsock32'])

	#TODO: should be detected automagically
	if env['PLATFORM']=='darwin':
	        if os.path.exists("/opt/local") :
	                # this is where DarwinPorts puts stuff by default
	                env.Append(CPPPATH='/opt/local/include')
	                env.Append(LIBPATH='/opt/local/lib')
	                env.Append(PATH='/opt/local/bin')
	        if os.path.exists("/sw") :
	                # and here's for fink
	                env.Append(CPPPATH='/sw/include')
	                env.Append(LIBPATH='/sw/lib')
	                env.Append(PATH='/sw/bin')

	env.AppendUnique(CPPPATH=string.split(env['extra_include_path']))
	env.AppendUnique(LIBPATH=string.split(env['extra_lib_path']))
	env.AppendUnique(CCFLAGS=string.split(env['extra_compile_flags']))
	env.AppendUnique(LINKFLAGS=string.split(env['extra_link_flags']))

	if env['build'] not in ['debug', 'profile', 'release']:
		print "\nERROR: unknown buildtype:", env['build']
		print "       Please specify a valid build type."
		Exit(1)

	if env['build']=='debug':
		env.debug=1
		env.Append(CCFLAGS='-DNOPARACHUTE')
		env.Append(LINKFLAGS='-rdynamic')

	if env['build']=='profile':
		env.debug=1
		env.optimize=1
		env.profile=1
		env.Append(CCFLAGS='-DNOPARACHUTE')

	if env['build']=='release':
		env.optimize=1
		env.strip=1
		SDL_PARACHUTE=1

	if env.debug==1:
		env.Append(CCFLAGS='-DDEBUG')
	else:
		env.Append(CCFLAGS='-DNDEBUG')

	if env['enable_efence']=='1':
		env.efence=1

	return BUILDDIR

################################################################################

def CheckPKG(context, name):
	context.Message( 'Checking for %s... ' % name )
	ret = context.TryAction('pkg-config --exists \'%s\'' % name)[0]
	context.Result( ret )
	return ret

def CheckPKGConfig(context, version):
	context.Message( 'Checking for pkg-config... ' )
	ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
	context.Result( ret )
	return ret

def CheckSDLConfig(context, env):
	context.Message( 'Checking for sdl-config... ' )
	if env['PLATFORM'] == 'win32':
		for p in env['PATH']:
			ret = context.TryAction('sh.exe '+os.path.join(p, env['sdlconfig'])+' --version')[0]
			if ret==1:
				env['sdlconfig']='sh.exe '+os.path.join(p, env['sdlconfig'])
				context.Result( ret )
				break
	else:
		for p in env['PATH']:
			ret = context.TryAction(os.path.join(p, env['sdlconfig'])+' --version')[0]
			if ret==1:
				env['sdlconfig']=os.path.join(p, env['sdlconfig'])
				context.Result( ret )
				break
	return ret

def CheckParaguiConfig(context, env):
	context.Message( 'Checking for paragui-config... ' )
	for p in env['PATH']:
		ret = context.TryAction(os.path.join(p, env['paraguiconfig'])+' --version')[0]
		if ret==1:
			env['paraguiconfig']=os.path.join(p, env['paraguiconfig'])
			context.Result( ret )
			break
	return ret

def CheckSDLVersionAtLeast(context, major, minor, micro, env):
	context.Message( 'Checking SDL version >= %s ... ' % (repr(major)+'.'+repr(minor)+'.'+repr(micro)))
	version=os.popen(env['sdlconfig']+" --version", "r").read()
	(maj, min, mic)=version.split('.')
	if int(maj)>int(major) or (int(maj)==int(major) and (int(min)>int(minor) or (int(min)==int(minor) and int(mic)>=int(micro)))):
		ret=1
	else: ret=0
	context.Result( ret )
	return ret

def CheckCompilerAttribute(context, compiler_attribute, env):
	context.Message( 'Trying whether __attribute__((%s)) works on your compiler ... ' % compiler_attribute)

	ret=context.TryLink("""class test {
	__attribute__(("""+compiler_attribute+"""))
	   int test_func(const char* fmt, ...) {}
};

int main(int argc, char **argv){return argc==0 && argv==0;}
\n\n""", '.cc') #must _use_ both arguments, otherwise -Werror will break

	context.Result(ret)
	return ret

def CheckCompilerFlag(context, compiler_flag, env):
	context.Message( 'Trying to enable compiler flag %s ... ' % compiler_flag)
	lastCCFLAGS = context.env['CCFLAGS'][:]
	context.env.Append(CCFLAGS = compiler_flag)
	ret = context.TryLink("""int main(int argc, char **argv) {return argc==0 && argv==0;}
			\n""", ".cc") #must _use_ both arguments, otherwise -Werror will break
	if not ret:
		context.env.Replace(CCFLAGS = lastCCFLAGS)
	context.Result( ret )
	return

def CheckLinkerFlag(context, link_flag, env):
	context.Message( 'Trying to enable linker   flag %s ... ' % link_flag)
	lastLINKFLAGS = context.env['LINKFLAGS'][:]
	context.env.Append(LINKFLAGS = link_flag)
	ret = context.TryLink("""int main(int argc, char **argv) {return argc==0 && argv==0;}
			\n""", ".cc") #must _use_ both arguments, otherwise -Werror will break
	if not ret:
		context.env.Replace(LINKFLAGS = lastLINKFLAGS)
	context.Result( ret )
	return

# Shamelessly copied from http://www.scons.org/wiki/CheckBoostVersion
def CheckBoostVersion(context, version):
	# Boost versions are in format major.minor.subminor
	v_arr = version.split(".")
	version_n = 0
	if len(v_arr) > 0:
		version_n += int(v_arr[0])*100000
	if len(v_arr) > 1:
		version_n += int(v_arr[1])*100
	if len(v_arr) > 2:
		version_n += int(v_arr[2])

	context.Message('Checking for Boost version >= %s... ' % (version))
	ret = context.TryCompile("""
#include <boost/version.hpp>

#if BOOST_VERSION < %d
#error Installed boost is too old!
#endif

int main()
{
	return 0;
}
""" % version_n, '.cpp')
	context.Result(ret)
	return ret


################################################################################

def configure_is_needed(targets):
	"""Do we need to run autoconfiguration? If we're building the default target
	   (==no target given at commandline): yes. If we're _only_ building targets
	   that don't need configuration: no."""

	NOCONFTARGETS=["clean", "dist", "distclean", "indent", "install", "longlines", "precommit", "shrink", "stylecheck", "tags", "uninst", "uninstall"]
	is_needed=False

	if targets==[]:
		is_needed=True

	def allowed(x): return x in NOCONFTARGETS
	if filter(allowed, targets) == []:
		is_needed=True

	return is_needed

def do_configure_basic_compiling(conf, env):
	print "Checking for a working C++ compiler ...",
	if not conf.TryLink("""class c{}; int main(){class c the_class;}""", '.cc'):
		print "no"
		print "Can't even compile the simplest C++ program! Your setup is beyond broken. Repair it, then try again."
		env.Exit(1)
	else:
		print "yes"

	if not conf.CheckCompilerAttribute('deprecated', env):
		print "Your compiler does not support __attribute__((deprecated)) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('noreturn', env):
		print "Your compiler does not support __attribute__((noreturn)) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('noinline', env):
		print "Your compiler does not support __attribute__((noinline)) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)

	if not conf.CheckCompilerAttribute('format(printf, 2, 3)', env):
		print "Your compiler does not support __attribute__((format(printf, 2, 3))) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)
	if not conf.CheckCompilerAttribute('hot', env):
		print "Your compiler does not support __attribute__((hot)) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)
	if not conf.CheckCompilerAttribute('pure', env):
		print "Your compiler does not support __attribute__((noinline)) which is necessary for Widelands. Please get a decent compiler."
		env.Exit(1)

def do_configure_locale(conf, env):
	setlocalefound=0
	if (conf.CheckFunc('setlocale') or conf.CheckLibWithHeader('', 'locale.h', 'C', 'setlocale("LC_ALL", "C");', autoadd=0)):
		setlocalefound=1

	textdomainfound=0
	if (conf.CheckFunc('textdomain') or conf.CheckLib(library='intl', symbol='textdomain', autoadd=1)):
		textdomainfound=1

	if setlocalefound and textdomainfound:
		print '--> NLS subsystem found.'
	else:
		#TODO: use dummy replacements that just pass back the original string
		print '--> No usable NLS subsystem found. Please install gettext.'
		env.Exit(1)

	if not conf.CheckFunc('getenv'):
		print '--> Your system does not support getenv(). Tilde expansion in filenames will not work.'
		env.HAS_GETENV=False
	else:
		env.HAS_GETENV=True

def do_configure_libraries(conf, env):
	if not conf.CheckSDLConfig(env):
		print 'Could not find sdl-config! Is SDL installed?'
		env.Exit(1)

	if not conf.CheckSDLVersionAtLeast(1, 2, 8, env):
		print 'Could not find an SDL version >= 1.2.8!'
		env.Exit(1)
	else:
		env.ParseConfig(env['sdlconfig']+' --libs --cflags')

	if not (conf.CheckBoostVersion('1.35')):
		print 'Boost version >= 1.35 needed (boost bug #1278). Make sure Boost development packages are installed.'
		env.Exit(1)

	env.HAS_PARAGUI=False
	#disabled until somebody finds time and courage to actually work on this #fweber
	#if not conf.CheckParaguiConfig(env):
	#	print 'Could not find paragui. That\'s no problem unless you\'re a developer working on this.'
	#	#print 'Could not find paragui-config! Is paragui installed?'
	#	#env.Exit(1)
	#else:
	#	env.ParseConfig(env['paraguiconfig']+' --libs --cflags')
	#	env.HAS_PARAGUI=True

	if not conf.CheckLibWithHeader('z', header='zlib.h', language='C', autoadd=1):
		print 'Could not find the zlib library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLibWithHeader('png', header='png.h', language='C', autoadd=1):
		print 'Could not find the png library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_image', symbol='IMG_Load', autoadd=1):
		print 'Could not find the SDL_image library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_ttf', symbol='TTF_Init', autoadd=1):
		print 'Could not find the SDL_ttf library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_net', symbol='SDLNet_TCP_Open', autoadd=1):
		print 'Could not find the SDL_net library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_gfx', symbol='zoomSurface', autoadd=1):
		print 'Could not find the SDL_gfx library! Is it installed?'
		env.Exit(1)

	if not conf.CheckLib(library='SDL_mixer', symbol='Mix_OpenAudio', autoadd=1):
		print 'Could not find the SDL_mixer library! Is it installed?'
		env.Exit(1)
	if env['enable_opengl']:
		if not conf.CheckLib(library='GL', symbol='glClear', autoadd=1):
			print 'Could not find the GL Library! Is it installed?'
			env.Exit(1)

       		if not conf.CheckLib(library='GLU', symbol='gluOrtho2D', autoadd=1):
			print 'Could not find the GLU Library! Is it installed?'
			env.Exit(1)
		env.Append(CCFLAGS='-DUSE_OPENGL')

	if not conf.CheckLib(library='boost_program_options', symbol='', autoadd=1) and not conf.CheckLib(library='boost_program_options-mt', symbol='', autoadd=1):
		print 'Could not find the boost::program_options Library! Is it installed?'
		env.Exit(1)

	if env['enable_ggz']:
		if not (conf.CheckLib(library='libggzcore', symbol='ggzcore_conf_read_int', autoadd=1)
			and conf.CheckLib(library='libggz', symbol='ggz_read_line', autoadd=1)
			and conf.CheckLib(library='libggzmod', symbol='', autoadd=1)):
			print 'Could not find libggz or ggz-client-libs! Are they BOTH installed?'
			env.Exit(1)
		else:
			env.Append(CCFLAGS='-DUSE_GGZ')
			#env.Append(LIBS=['ggzmod', 'ggzcore', 'ggz'])


	if (not env['PLATFORM'] == 'win32') and not conf.TryLink(""" #define USE_RWOPS
			#include <SDL_mixer.h>
			int main(){
				Mix_LoadMUS_RW("foo.ogg");
			}
			""", '.c'):
		print 'Your SDL_mixer does not support Mix_LoadMUS_RW(). Update SDL_mixer.'
		env.Exit(1)

def do_configure_debugtools(conf, env):
	if conf.CheckLib('efence', symbol='EF_newFrame', language='C', autoadd=0):
		if env.efence:
			conf.CheckCompilerFlag('-include stdlib.h -include string.h -include efence.h', env)
			conf.CheckCompilerFlag('-include new -include fstream -include efencepp.h', env)
			conf.CheckLinkerFlag('-lefence', env)
	else:
		if env.efence:
			print 'Could not find efence, so doing a debug-efence build is impossible !'
			env.Exit(1)

def do_configure_compiler_warnings(conf, env):
	conf.CheckCompilerFlag('-Wall', env)
	conf.CheckCompilerFlag('-Wcast-align', env)
	conf.CheckCompilerFlag('-Werror=cast-align', env)
	conf.CheckCompilerFlag('-Wcast-qual', env)
	conf.CheckCompilerFlag('-Werror=cast-qual', env)
	#conf.CheckCompilerFlag('-Wconversion', env)
	conf.CheckCompilerFlag('-Wdisabled-optimization', env)
	conf.CheckCompilerFlag('-Wextra', env)
	conf.CheckCompilerFlag('-Werror=extra', env)
	conf.CheckCompilerFlag('-Wfloat-equal', env)
	conf.CheckCompilerFlag('-Winvalid-pch', env)
	conf.CheckCompilerFlag('-Werror=invalid-pch', env)
	conf.CheckCompilerFlag('-Wmissing-format-attribute', env)
	#conf.CheckCompilerFlag('-Werror=missing-format-attribute', env) the C++ library on amd64 has this error
	conf.CheckCompilerFlag('-Wmissing-include-dirs', env)
	conf.CheckCompilerFlag('-Werror=missing-include-dirs', env)
	conf.CheckCompilerFlag('-Wmissing-noreturn', env)
	#conf.CheckCompilerFlag('-Werror=missing-noreturn', env)
	conf.CheckCompilerFlag('-Wcomment', env)
	conf.CheckCompilerFlag('-Werror=comment', env)
	conf.CheckCompilerFlag('-Wnormalized=nfc', env)
	conf.CheckCompilerFlag('-Wold-style-cast', env)
	#conf.CheckCompilerFlag('-Werror=old-style-cast', env) Some SDL macros still use old-style casts. Get them fixed!
	conf.CheckCompilerFlag('-Werror=parentheses', env)
	conf.CheckCompilerFlag('-Wpointer-arith', env)
	conf.CheckCompilerFlag('-Werror=pointer-arith', env)
	conf.CheckCompilerFlag('-Werror=sequence-point', env)
	conf.CheckCompilerFlag('-Wshadow', env)
	conf.CheckCompilerFlag('-Werror=shadow', env)
	conf.CheckCompilerFlag('-Wstrict-aliasing=2', env)
	conf.CheckCompilerFlag('-Werror=switch', env)
	conf.CheckCompilerFlag('-Wwrite-strings', env)
	conf.CheckCompilerFlag('-Werror=trigraphs', env)
	conf.CheckCompilerFlag('-Wunsafe-loop-optimizations', env)
	conf.CheckCompilerFlag('-Werror=unused-function', env)
	conf.CheckCompilerFlag('-Werror=unused-label', env)
	conf.CheckCompilerFlag('-Werror=unused-value', env)
	conf.CheckCompilerFlag('-Werror=write-strings', env)

	# Use this to temporarily disable some warnings
	#TODO: make available via commandline parameters
	#conf.CheckCompilerFlag('-Wno-deprecated-declarations', env)
	#conf.CheckCompilerFlag('-Wno-unused-variable', env)
	#conf.CheckCompilerFlag('-Wno-unused-parameter', env)
	#conf.CheckCompilerFlag('-Werror', env)

def do_configure_compiler_features(conf, env):
	conf.CheckCompilerFlag('-fstack-protector-all', env)
	conf.CheckCompilerFlag('-fbounds-check', env)
	conf.CheckCompilerFlag('-pipe', env)

	import platform
	if (platform.architecture()[0] == '64bit'):
		# Some stuff for 64bit architectures:
		conf.CheckCompilerFlag('-m64', env)
	else:
		if env.optimize: # no optimization on 64bit atm - leads to segfaults
			 # !!!! -fomit-frame-pointer breaks exceptions !!!!
			 conf.CheckCompilerFlag('-fexpensive-optimizations', env)
			 conf.CheckCompilerFlag('-finline-functions', env)
			 conf.CheckCompilerFlag('-ffast-math', env)
			 conf.CheckCompilerFlag('-funroll-loops', env)
			 conf.CheckCompilerFlag('-O3', env)
		else:
			 conf.CheckCompilerFlag('-O0', env)
			 conf.CheckCompilerFlag('-funit-at-a-time', env)

	if env.profile:
		conf.CheckCompilerFlag('-pg', env)
		conf.CheckCompilerFlag('-fprofile-arcs', env)

	if env.debug:
		conf.CheckCompilerFlag('-g', env)
		conf.CheckCompilerFlag('-fmessage-length=0', env)

def do_configure_linker_features(conf, env):
	if env.profile:
		conf.CheckLinkerFlag('-pg', env)
		conf.CheckLinkerFlag('-fprofile-arcs', env)

	if env.strip:
		conf.CheckLinkerFlag('-s', env)

def do_configure(conf, env):
	do_configure_basic_compiling(conf, env)
	do_configure_locale(conf, env)
	do_configure_libraries(conf, env)
	do_configure_debugtools(conf, env)

	do_configure_compiler_warnings(conf, env)
	do_configure_compiler_features(conf, env)
	do_configure_linker_features(conf, env)

# Previously BOOST_SP_DISABLE_THREADS was defined in some header file. But that
# caused segmentation fault in 64bit optimized builds. Try to obey the
# documented use by defining it for all translation units. Here is a quote from
# the manual:
#
# If your program is single-threaded and does not link to any libraries that
# might have used shared_ptr in its default configuration, you can  #define the
# macro BOOST_SP_DISABLE_THREADS on a project-wide basis to switch to ordinary
# non-atomic reference count updates.
#
# (Defining BOOST_SP_DISABLE_THREADS in some, but not all, translation units is
# technically a violation of the One Definition Rule and undefined behavior.
# Nevertheless, the implementation attempts to do its best to accommodate the
# request to use non-atomic updates in those translation units. No guarantees,
# though.)
	env.Append(CCFLAGS = "-DBOOST_SP_DISABLE_THREADS")


################################################################################

def generate_configh_content(env):
	data=""

	if os.path.isabs(env['bindir']):
		bindir=env['bindir']
	else:
		bindir=os.path.join(env['install_prefix'], env['bindir'])

	if os.path.isabs(env['datadir']):
		datadir=env['datadir']
	else:
		datadir=os.path.join(env['install_prefix'], env['datadir'])

	if os.path.isabs(env['localedir']):
		localedir=env['localedir']
	else:
		localedir=os.path.join(env['install_prefix'], env['localedir'])

	if env['build'] != 'release' and env['prefer_localdata']:
		data+="#define INSTALL_PREFIX \".\"\n"
		data+="#define INSTALL_BINDIR \".\"\n"
		data+="#define INSTALL_DATADIR \".\"\n"
		data+="#define INSTALL_LOCALEDIR \"locale\"\n\n"
	else:
		data+="#define INSTALL_PREFIX \""+env['install_prefix']+"\"\n"
		data+="#define INSTALL_BINDIR \""+env['bindir']+"\"\n"
		data+="#define INSTALL_DATADIR \""+env['datadir']+"\"\n"
		data+="#define INSTALL_LOCALEDIR \""+env['localedir']+"\"\n\n"

	if env.HAS_GETENV==True:
		data+="#define HAS_GETENV\n\n"

	if env.HAS_PARAGUI==True:
		data+="#define HAS_PARAGUI\n\n"

	return data

def generate_configh_file(env, target, source):
	config_h_file=open(target[0].path, "w")

	config_h_file.write(""" // This file is autogenerated. It will be overwritten by the next run of scons

#ifndef CONFIG_H
#define CONFIG_H

""")
	config_h_file.write(source[0].get_contents())
	config_h_file.write("\n#endif\n\n")

	config_h_file.close()

