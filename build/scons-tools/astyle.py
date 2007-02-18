import SCons.Builder
import SCons.Action

def complain_astyle(target, source, env):
	print 'INFORMATION: Astyle produces malformed indentation (see for example [https://sourceforge.net/tracker/index.php?func=detail&aid=1642489&group_id=2319&atid=102319]) and is disabled whilst waiting for repair. If you really want to use it, execute "cd /usr/bin && ln -s astyle buggy-astyle" and try again.'#astyle binary was not found (see above). Source indenting was not performed.'

def generate(env):
	env['ASTYLE']=find_astyle(env)

	if env['ASTYLE']!=None:
		env['ASTYLEFLAGS']='--mode=c --indent=tab=3 --indent-preprocessor --one-line=keep-statements --one-line=keep-blocks'
		env['ASTYLECOM']='$ASTYLE $ASTYLEFLAGS $SOURCES'
		env['BUILDERS']['astyle']=SCons.Builder.Builder(action=env['ASTYLECOM'])
	else:
		env['BUILDERS']['astyle']=SCons.Builder.Builder(action=env.Action(complain_astyle))

def find_astyle(env):
	b=env.WhereIs('buggy-astyle')
	if b==None:
		complain_astyle(None, None, env)
	else:
		print 'Found astyle:     ', b
	return b

def exists(env):
	if find_astyle(env)==None:
		return 0
	return 1

