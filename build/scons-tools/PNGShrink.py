import SCons.Builder
import SCons.Action
import os

def complain_pngshrink(target, source, env):
	print 'INFORMATION: pngrewrite, optipng or both have not been found (see above). PNGs have not been compacted.'

def del_temp_png():
	if os.path.exists('temp.png'):
		os.remove('temp.png')

def move_temp_png(dest):
	if os.path.exists('temp.png'):
		os.rename('temp.png', dest)

def shrink_command(source, target, env, for_signature):
	commands=[]

	if (env['PNGREWRITE']==None) or (env['OPTIPNG']==None) or (env['ADVDEF']==None):
		return env.Action(complain_pngshrink)

	s=source[0].path
	commands.append(env.Action(del_temp_png()))
	commands.append(env.Action('$PNGREWRITE '+s+' temp.png||echo skipped'))
	commands.append(env.Action(move_temp_png(s)))
	commands.append(env.Action('$OPTIPNG $OPTIPNGFLAGS '+s))
	commands.append(env.Action('$ADVDEF $ADVDEFFLAGS '+s))

	return commands

def generate(env):
	env['PNGREWRITE']=find_pngrewrite(env)

	env['OPTIPNG']=find_optipng(env)
	env['OPTIPNGFLAGS']='-q -zc1-9 -zm1-9 -zs0-3 -f0-5'

	env['ADVDEF']=find_advdef(env)
	env['ADVDEFFLAGS']='-z4'

	env['BUILDERS']['PNGShrink']=SCons.Builder.Builder(generator=shrink_command, single_source=1)

def find_pngrewrite(env):
	b=env.WhereIs('pngrewrite')
	if b==None:
		print 'Searching for pngrewrite:  not found. PNG compatification disabled.'
	else:
		print 'Searching for pngrewrite: ', b
	return b

def find_optipng(env):
	b=env.WhereIs('optipng')
	if b==None:
		print 'Searching for optipng:     not found. PNG compatification disabled.'
	else:
		print 'Searching for optipng:    ', b
	return b

def find_advdef(env):
	b=env.WhereIs('advdef')
	if b==None:
		print 'Searching for advdef:      not found. PNG compatification disabled.'
	else:
		print 'Searching for advdef:     ', b
	return b

def exists(env):
	if (find_advdef(env)==None) or (find_optipng(env)==None) or (find_pngrewrite(env)==None):
		return 0
	return 1
