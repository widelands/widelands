#!/usr/bin/python

import os, sys, string
sys.path.append("build/scons-tools")
import shutil, fnmatch, time, glob
import SCons
from SCons.Script.SConscript import SConsEnvironment

from scons_configure import *
from Distribute import *

# Sanity checks
EnsurePythonVersion(2, 3)
EnsureSConsVersion(1, 0, 0)

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
	opts=Options('build/scons-config.py', ARGUMENTS)
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
	opts.Add('extra_compile_flags', '(does not work with build-widelands.sh!)', '')
	opts.Add('extra_link_flags', '(does not work with build-widelands.sh!)', '')
	opts.AddOptions(
		BoolOption('enable_sdl_parachute', 'Enable SDL parachute?', False),
		BoolOption('enable_efence', 'Use the efence memory debugger?', False),
		BoolOption('enable_ggz', 'Use the GGZ Gamingzone?', False),
		BoolOption('prefer_localdata', 'Useful for developers. Use data and locales from ./ at runtime', True),
		)
	return opts

################################################################################
# Environment setup
#
# Create configuration objects

opts=cli_options()

env=Environment(options=opts)
env.Tool("ctags", toolpath=['build/scons-tools'])
env.Tool("PNGShrink", toolpath=['build/scons-tools'])
env.Tool("astyle", toolpath=['build/scons-tools'])
env.Tool("Distribute", toolpath=['build/scons-tools'])
env.Help(opts.GenerateHelpText(env))

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

BUILDDIR=parse_cli(env, BUILD_TARGETS)
env.Append(CPPPATH=[os.path.join('#', BUILDDIR)])

print_build_info(env)

if env.enable_configuration:
	# Generate build_id.cc - scons itself will decide whether a recompile is needed
	Command(os.path.join(BUILDDIR, "build_id.cc"), [Value(get_build_id(env))], generate_buildid_file)

	do_configure(conf, env)

	# Generate config.h - scons itself will decide whether a recompile is needed
	Command(os.path.join(BUILDDIR, "config.h"), [Value(generate_configh_content(env))], generate_configh_file)

env=conf.Finish()
print # Pretty output

#######################################################################

Export('env', 'BUILDDIR', 'PhonyTarget', 'simpleglob')

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

########################################################################### tags

Alias('tags', env.ctags(source=simpleglob('*.h', 'src', recursive=True)+simpleglob('*.cc', 'src', recursive=True)))

################################################################ C++ style-check

if env['build'] == 'debug' or env['build'] == 'profile':
        Alias('stylecheck', env.Execute('utils/spurious_source_code/detect'))

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

install=env.Install('installtarget', 'build-widelands.sh') # the second argument is a (neccessary) dummy
Alias('install', install)
AlwaysBuild(install)
env.AddPreAction(install, Action(buildlocale))

uninstall=env.Uninstall('uninstalltarget', 'build-widelands.sh') # the second argument is a (neccessary) dummy
Alias('uninstall', uninstall)
Alias('uninst', uninstall)
AlwaysBuild(uninstall)

##################################################################### Distribute

distadd(env, 'ChangeLog')
distadd(env, 'COPYING')
distadd(env, 'CREDITS')
distadd(env, 'Makefile')
distadd(env, 'SConstruct')
distadd(env, 'build-widelands.sh')

dist=env.DistPackage('widelands-'+get_build_id(env), 'build-widelands.sh') # the second argument is a (neccessary) dummy
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
	Delete('build/scons-tools/PNGShrink.pyc'),
#	Delete('build/scons-signatures.dblite')) # This can not work, how to get rid of this file?
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
]

distclean=PhonyTarget("distclean", distcleanactions)

