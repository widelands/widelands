#!/usr/bin/env python -tt
# encoding: utf-8
# 
import os
import SCons
from SCons.Script.SConscript import SConsEnvironment
from codecheck.CodeCheck import CodeChecker

d = CodeChecker()

class SConsCodeChecker(object):
    def __call__(self, target, source, env):
            '''
            Checks the given source, 
            reporting failure to scons status.

            We write a cache file to remember when this check ran the last time. 
            '''
            if len(target)!=1 and len(source)!=1:
                raise RuntimeError, "CodeChecker Builder was called with more than one target or source"

            d.use_color = env["USE_COLOR"]
            
            for s,t in zip(source,target):
                d.check_file(str(s.path))

                # Only cache that this was run for main source
                open(str(t),'w').write("")
                
            return 0

    def actionString(self, target, source, env):
            '''
            Return output string which will be seen when running checks
            '''
            # SILENCE!
            return None
            # return 'Checking for style crimes in ' + str(source[0])

def codecheck_emitter( target,sources,env ):
    """
    A simple emitter that makes sure that code checking
    is done before compiling
    """
    assert( len(sources) == 1)
    
    cpp_check = env.CodeCheck(sources[0])
    env.Depends( target[0], cpp_check )

    return (target,sources)


from SCons.Tool import SourceFileScanner

# Inform SCons about our CodeChecker
def generate(env):
        cc = SConsCodeChecker()
        env['BUILDERS']['CodeCheck'] = env.Builder(
                        action = env.Action(cc,cc.actionString),
                        suffix='.codecheck',
                        source_scanner = SourceFileScanner
                    )
        env['BUILDERS']['Object'].add_emitter(".cc", codecheck_emitter)

def exists(env):
        return 1

