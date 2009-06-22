#!/usr/bin/python
# -*- coding: utf-8 -*-

import os, sys, string
sys.path.append("build/scons-tools")
import shutil, fnmatch, time, glob
import SCons
from SCons.Script.SConscript import SConsEnvironment

from ColorizeGcc import overwrite_spawn_function
from distcc import setup_distcc
from ccache import setup_ccache
from scons_configure import *
from Distribute import *

# Sanity checks
EnsurePythonVersion(2, 3)
EnsureSConsVersion(1, 0, 1)

# Speedup. If you have problems with inconsistent or wrong builds, look here first
SetOption('max_drift', 1)
SetOption('implicit_cache', 1)
Decider('MD5-timestamp')

# write only *one* signature file in a place where we don't care
SConsignFile('build/scons-signatures')

# Pretty output
print

########################################## simple glob that works across BUILDDIR

def simpleglob(pattern='*', directory='.', recursive=False):
        entries=[]

	for entry in os.listdir(Dir(directory).srcnode().abspath):
		if fnmatch.fnmatchcase(entry, pattern):
			entries.append(os.path.join(directory, entry))
		if recursive and os.path.isdir(os.path.join(directory, entry)):
                        entries+=(simpleglob(pattern, os.path.join(directory, entry), recursive))

	return entries

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
	opts=Variables('build/scons-config.py', ARGUMENTS)
	opts.Add('cc', 'use this compiler as default c compiler', '')
	opts.Add('cxx', 'use this compiler as default c++ compiler','')
	opts.Add('build', 'debug / profile / release(default)', 'debug')
	opts.Add('build_id', 'To get a default value (SVN revision), leave this empty', '') #change this before/after preparing a release
	opts.Add('sdlconfig', 'On some systems (e.g. BSD) this is called sdl12-config', 'sdl-config')
	opts.Add('paraguiconfig', '', 'paragui-config')
	opts.Add('install_prefix', '', '/usr/local')
	opts.Add('bindir', '(absolute or relative to install_prefix)', 'games')
	opts.Add('datadir', '(absolute or relative to install_prefix)', 'share/games/widelands')
	opts.Add('localedir', '(absolute or relative to install_prefix)', 'share/games/widelands/locale')
	opts.Add('extra_include_path', '', '')
	opts.Add('extra_lib_path', '', '')
	opts.Add('extra_compile_flags', '', '')
	opts.Add('extra_link_flags', '', '')
	opts.Add('check','Enable/Disable checks, DO NOT USE', True),
	opts.AddVariables(
		BoolVariable('distcc', 'use distcc to compile widelands on many computers', False),
		BoolVariable('ccache', 'use ccache to speedup compiling widelands', False),

		BoolVariable('enable_sdl_parachute', 'Enable SDL parachute?', False),
		BoolVariable('enable_efence', 'Use the efence memory debugger?', False),
		BoolVariable('enable_ggz', 'Use the GGZ Gamingzone?', True),
		BoolVariable('enable_opengl', 'use opengl for graphics', False),
		BoolVariable('prefer_localdata', 'Useful for developers. Use data and locales from ./ at runtime', True),
		BoolVariable('pretty_compile_output', 'Suppress link and compile commands, only print a very short info text and occurring errors and warnings.', True),
		BoolVariable('colored_compile_output', 'Show style warnings and (gcc) compile warnings in color. Disabled if output is not a tty', True),
		)
	return opts

################################################################################
# Environment setup
#
# Create configuration objects

opts=cli_options()

env=Environment(ENV=os.environ, options=opts)

BUILDDIR=parse_cli(env, BUILD_TARGETS) # must parse CLI before anything else, PATH might get changed

env.Tool("ctags", toolpath=['build/scons-tools'])
env.Tool("PNGShrink", toolpath=['build/scons-tools'])
env.Tool("astyle", toolpath=['build/scons-tools'])
env.Tool("Distribute", toolpath=['build/scons-tools'])
env.Help(opts.GenerateHelpText(env))
if sys.platform == 'win32':
	Tool('mingw')(env)

opts.Save('build/scons-config.py',env)

conf=env.Configure(conf_dir='#/build/sconf_temp',log_file='#build/config.log',
		   custom_tests={
				'CheckPKGConfig' : CheckPKGConfig,
				'CheckPKG': CheckPKG,
				'CheckSDLConfig': CheckSDLConfig,
				'CheckSDLVersionAtLeast': CheckSDLVersionAtLeast,
				'CheckCompilerAttribute': CheckCompilerAttribute,
				'CheckCompilerFlag': CheckCompilerFlag,
				'CheckLinkerFlag': CheckLinkerFlag,
				#'CheckParaguiConfig': CheckParaguiConfig,
				'CheckBoostVersion': CheckBoostVersion
		   }
)

################################################################################
# Environment setup
#
# Parse commandline and autoconfigure

env.Append(CPPPATH=[os.path.join('#', BUILDDIR)])

print_build_info(env)

if env.enable_configuration:
	# Generate build_info.cc - scons itself will decide whether a recompile is needed
	Command(os.path.join(BUILDDIR, "build_info.cc"), [Value(get_build_id(env)),Value(env['build'])], generate_buildinfo_file)

	if (env['cxx'] != ''):
		env['CXX'] = env['cxx']
		print "Using compiler cxx=" + env['cxx']
	if (env['cc'] != ''):
		env['CC'] = env['cc']
		print "Using compiler cc=" + env['cc']
	num_cpu = 0
	if (env['distcc'] == True):
		setup_distcc(env)
		num_cpu = int(os.environ.get('NUM_CPU', 8))
	else:
		num_cpu = int(os.environ.get('NUM_CPU', 2))
	if (env['ccache'] == True):
			setup_ccache(env)

	SetOption('num_jobs', num_cpu)
	print "running with -j", GetOption('num_jobs')

	do_configure(conf, env)

	# Generate config.h - scons itself will decide whether a recompile is needed
	Command(os.path.join(BUILDDIR, "config.h"), [Value(generate_configh_content(env))], generate_configh_file)




env=conf.Finish()

# We only add this tool now, because it adds an emitter to Object which
# breaks configuration.
env.Tool("CodeCheck", toolpath=['build/scons-tools'])

# Setup pretty compile output
if env['pretty_compile_output']:
    env.Append(CXXCOMSTR="Compiling ==> '$TARGET'");
    env.Append(LINKCOMSTR="Linking ==> '$TARGET'");
    env.Append(ARCOMSTR="ar ==> '$TARGET'");
    env.Append(RANLIBCOMSTR="ranlib ==> '$TARGET'");
env["USE_COLOR"]=False
if env['colored_compile_output']:
    if not os.isatty(1): # No tty, no color. Sorry
        env["USE_COLOR"]=False
    elif "TERM" in os.environ:
        color_terms = ("xterm-color",)
        nocolor_terms = ("dumb",)

        # Try to get terminal settings from environment
        # Note: we do not use scons environment, since
        # we *really* want the users terminal here
        t = os.environ["TERM"]
        if t in nocolor_terms:
            env["USE_COLOR"] = False
        elif t in color_terms:
            env["USE_COLOR"] = True
        else:
            print "\nUnknown TERM. Disabling colors. Please add your TERM in SConstruct"
            env["USE_COLOR"] = True


# We now copy env to get our test Environment
# TODO Doing this after configuration means that all
# test exectutables are linked agains boots,SDL,... and
# have the DEFINES from config. That's bad.
testEnv = env.Clone()
testEnv.Tool("UnitTest", toolpath=['build/scons-tools'],
    LIBS=["boost_unit_test_framework-mt"])
testEnv.Append(CXXFLAGS=["-DBOOST_TEST_DYN_LINK"])

# Overwriting Spawn to provide some color output
overwrite_spawn_function(env)

print # Pretty output

#######################################################################

Export('env', 'testEnv', 'BUILDDIR', 'PhonyTarget', 'simpleglob')

SConscript('build/SConscript')
SConscript('campaigns/SConscript')
SConscript('doc/SConscript')
SConscript('fonts/SConscript')
SConscript('maps/SConscript')
SConscript('music/SConscript')
SConscript('pics/SConscript')
buildlocale=SConscript('po/SConscript')
SConscript('sound/SConscript')
SConscript('src/SConscript.dist')
thebinary=SConscript('src/SConscript', build_dir=BUILDDIR, duplicate=0)
SConscript('tribes/SConscript')
SConscript('txts/SConscript')
SConscript('utils/SConscript')
SConscript('worlds/SConscript')
SConscript('global/SConscript')

Default(thebinary)
if env['build']=='release':
	Default(buildlocale)

# Also build tests by default
if env['build'] == 'debug' or env['build'] == 'profile':
    Default("test")

########################################################################### tags
# Tags
all_code_files=simpleglob('*.h', 'src', recursive=True)+simpleglob('*.cc', 'src', recursive=True)
all_code_files.sort()
Alias('tags', env.ctags(source=all_code_files))

################################################################ C++ style-check
if (env['build'] == 'debug' or env['build'] == 'profile') and env['PLATFORM'] != 'darwin' and env['check'] == True:
        Alias('old-stylecheck', env.Execute('utils/spurious_source_code/detect'))

# Style Checks
PhonyTarget('stylecheck',
    Action('./build/scons-tools/codecheck/CodeCheck.py %s %s' %
        ('-c' if env["USE_COLOR"] else '',
         ' '.join(all_code_files)), lambda *Silence: None)
)

################################################################## PNG shrinking

# finding files takes quite long, so don't execute it if it's unneccessary
if ('shrink' in BUILD_TARGETS):
	print "Assembling file list for image compactification..."
	Alias('shrink', env.PNGShrink(simpleglob('*.png', '.', recursive=True)))

########################################################## Install and uninstall

instadd(env, 'ChangeLog', 'doc')
instadd(env, 'COPYING', 'doc')
instadd(env, 'CREDITS', 'doc')
instadd(env, 'widelands', filetype='binary')

install=env.Install('installtarget', 'COPYING') # the second argument is a (neccessary) dummy
Alias('install', install)
AlwaysBuild(install)
Depends(install, buildlocale)
#env.AddPreAction(install, Action("scons buildlocale"))


uninstall=env.Uninstall('uninstalltarget', 'COPYING') # the second argument is a (neccessary) dummy
Alias('uninstall', uninstall)
Alias('uninst', uninstall)
AlwaysBuild(uninstall)

########################
#version file

print ("GENERATING VERSION FILE")
version_file=open("VERSION","w")
version_file.write(get_build_id(env))
version_file.write('\n\0')
version_file.close()


##################################################################### Distribute

distadd(env, 'ChangeLog')
distadd(env, 'COPYING')
distadd(env, 'CREDITS')
distadd(env, 'Makefile')
distadd(env, 'SConstruct')

dist=env.DistPackage('widelands-'+get_build_id(env), 'COPYING') # the second argument is a (neccessary) dummy
Alias('dist', dist)
AlwaysBuild(dist)

###################################################################### longlines

longlines=PhonyTarget("longlines", 'utils/count-longlines.py')

###################################################################### precommit

#Alias('precommit', 'indent')
Alias('precommit', 'longlines')

################################################################## Documentation

PhonyTarget('doc', 'doxygen doc/Doxyfile')

########################################################################## Clean

distcleanactions=[
	Delete('build/native-debug'),
	Delete('build/native-profile'),
	Delete('build/native-release'),
	Delete('build/sconf_temp'),
	Delete('build/scons-config.py'),
	Delete('build/config.log'),
	Delete('build/scons-tools/scons_configure.pyc'),
	Delete('build/scons-tools/detect_revision.pyc'),
	Delete('build/scons-tools/Distribute.pyc'),
	Delete('build/scons-tools/ctags.pyc'),
	Delete('build/scons-tools/astyle.pyc'),
	Delete('build/scons-tools/distcc.pyc'),
	Delete('build/scons-tools/ccache.pyc'),
	Delete('build/scons-tools/CodeCheck.pyc'),
	Delete('build/scons-tools/PNGShrink.pyc'),
	Delete('build/scons-signatures.dblite'), #have to delete this or problems occur
	Touch('build/scons-signatures.dblite'), #we need this to make scons exit nicely
	Delete('utils/scons.py'),
	Delete('utils/scons-LICENSE'),
	Delete('utils/scons-README'),
	Delete('utils/scons-local-0.96.1'),
	Delete('utils/scons-local-0.97'),
	Delete('utils/scons-local-1.0.1'),
	Delete('utils/scons-time.py'),
	Delete('utils/sconsign.py'),
	Delete('utils/buildcat.pyc'),
	Delete('utils/confgettext.pyc'),
	Delete('tags'),
	Delete('widelands'),
	Delete('locale/sv_SE'),
	Delete('locale/de_DE'),
	Delete('locale/da_DK'),
	Delete('locale/nl_NL'),
	Delete('locale/pl_PL'),
	Delete('locale/cs_CZ'),
	Delete('locale/he_HE'),
	Delete('locale/hu_HU'),
	Delete('locale/gl_ES'),
	Delete('locale/sk_SK'),
	Delete('locale/fi_FI'),
	Delete('locale/ru_RU'),
	Delete('locale/es_ES'),
	Delete('locale/fr_FR'),
	Delete('po/pot'),
	Delete('VERSION')
]

distclean=PhonyTarget("distclean", distcleanactions)

