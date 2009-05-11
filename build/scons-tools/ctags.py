import SCons.Builder
import SCons.Action

def complain_ctags(target, source, env):
	print 'INFORMATION: ctags binary was not found (see above). Tags have not been built.'

def generate(env):
	env['CTAGS']=find_ctags(env)

	if env['CTAGS']!=None:
		env['CTAGSCOM']='$CTAGS $SOURCES'
		env['BUILDERS']['ctags']=SCons.Builder.Builder(action=env['CTAGSCOM'])
	else:
		env['BUILDERS']['ctags']=SCons.Builder.Builder(action=env.Action(complain_ctags))

def find_ctags(env):
    b = None
    if 'PATH' in env:
        b=env.WhereIs('ctags', path=env['PATH'])
	if b==None:
		print 'Searching for ctags:       not found. Tags will not be built'
	else:
		print 'Searching for ctags:      ', b
	return b

def exists(env):
	if find_ctags(env)==None:
		return 0
	return 1

