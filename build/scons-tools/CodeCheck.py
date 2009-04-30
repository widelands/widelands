#!/usr/bin/env python -tt
# encoding: utf-8
# 
import os
import SCons
from SCons.Script.SConscript import SConsEnvironment
from codecheck.CodeCheck import CodeChecker

d = CodeChecker()

class SConsCodeChecker(object):
    def __init__(self,check_includes=False):
        self._check_includes = check_includes

    def _find_includes(self,source_node,env):
        # We are blocked to use SConsCPPScannerWrapper here, because of SCons bug
        # http://scons.tigris.org/issues/show_bug.cgi?id=2410
        #
        # And there are other problems in SConsCPPScannerWrapper too.
        #
        # This is the only scanner I could find that does recursive include analysis though, 
        # I have no idea of SCons handles this internally, but it doesn't offer this as
        # tool to 3rd parties, that's for sure. 
        #
        # Therefore we use this 'old' scanner class, which only returns the immediate includes. 
        # We therefore do no complete stylecheck of all include files while compiling. This is
        # a bad bug. Hope to fix it someday....
        # scanner = SCons.Scanner.C.SConsCPPScannerWrapper("CScanner", "CPPPATH")
        scanner = SCons.Scanner.C.CScanner()
        path = scanner.path(env)
       

        local_includes = [ h for h in scanner(source_node,env,path) if not os.path.isabs(h.path) ]

        return local_includes

    def __call__(self, target, source, env):
            '''
            Checks the given source, 
            reporting failure to scons status.

            We write a cache file to remember when this check run the last time. 
            '''
            if len(target)!=1 and len(source)!=1:
                raise RuntimeError, "CodeChecker Builder was called with more than one target or source"

            d.use_color = env["USE_COLOR"]
            
            for s,t in zip(source,target):
                app = str(s.path)
                if self._check_includes:
                    include_nodes = self._find_includes(s,env)
                    for i in include_nodes:
                        d.check_file(i.path)
                
                d.check_file(app)
                
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
    
    cpp_check = env.CodeCheckInc(sources[0])
    env.Depends( target[0], cpp_check )

    return (target,sources)


from SCons.Tool import SourceFileScanner

# Inform SCons about our CodeChecker
def generate(env):
        cc = SConsCodeChecker(True)
        env['BUILDERS']['CodeCheckInc'] = env.Builder(
                        action = env.Action(cc,cc.actionString),
                        suffix='.codecheck',
                        source_scanner = SourceFileScanner
                    )
        env['BUILDERS']['Object'].add_emitter(".cc", codecheck_emitter)

def exists(env):
        return 1

