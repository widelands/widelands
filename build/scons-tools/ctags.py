import string
import os
import SCons.Builder
import SCons.Action

#directories can't be passed to a builder via source= :(
#so we pass files (which also gives us dependency tracking)
#and use the pathnames to find out the source directories

def complain_ctags(target, source, env):
	print 'ctags binary was not found (see above). Tags have not been built.'

def ctags_command(source, target, env, for_signature):
	paths=[]

	if env['CTAGS']==None:
		return env.Action(complain_ctags)
	
	# create a list with the (unique) _first_ path component(s) only
	for src in source:
		s=src.path.split('/')[0]
		if s not in paths:
			paths.append(s)
			
	return env.Action("%s %s" % (env['CTAGSCOM'], " ".join(paths)))

def generate(env):
	env['CTAGS']=find_ctags(env)
	
	if env['CTAGS']!=None:
		env['CTAGSFLAGS']=' --recurse=yes'
		env['CTAGSCOM']='$CTAGS $CTAGSFLAGS' 
	
	env['BUILDERS']['ctags']=SCons.Builder.Builder(generator=ctags_command)
	
def find_ctags(env):
	b=env.WhereIs('ctags')
	if b==None:
		print 'WARNING: Could not find ctags. Tags will not be built.'
	else:
		print 'Found Exuberant ctags: ', b
	return b

def exists(env):
	if find_ctags(env)==None:
		return 0
	return 1

