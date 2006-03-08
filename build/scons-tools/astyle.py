import SCons.Builder
import SCons.Action

def complain_astyle(target, source, env):
	print 'INFORMATION: astyle binary was not found (see above). Source indenting was not performed.'

def generate(env):
	env['ASTYLE']=find_astyle(env)

	if env['ASTYLE']!=None:
		env['ASTYLEFLAGS']='--mode=c --indent=tab=3 --indent-preprocessor --one-line=keep-statements --one-line=keep-blocks'
		env['ASTYLECOM']='$ASTYLE $ASTYLEFLAGS $SOURCES'
		env['BUILDERS']['astyle']=SCons.Builder.Builder(action=env['ASTYLECOM'])
	else:
		env['BUILDERS']['astyle']=SCons.Builder.Builder(action=env.Action(complain_astyle))

def find_astyle(env):
	b=env.WhereIs('astyle')
	if b==None:
		print 'WARNING: Could not find astyle. Source indenting will not be performed.'
	else:
		print 'Found astyle:     ', b
	return b

def exists(env):
	if find_astyle(env)==None:
		return 0
	return 1

