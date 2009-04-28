#!/usr/bin/env python -tt
# encoding: utf-8
# 
import os
import SCons
from SCons.Script.SConscript import SConsEnvironment
from codecheck.CodeCheck import CodeChecker

d = CodeChecker()

def codeCheckAction(target, source, env):
        '''
        Checks the given source, 
        reporting failure to scons status.

        We write a cache file to remember when this check run the last time. 
        '''
        if len(target)!=1 and len(source)!=1:
            raise RuntimeError, "CodeChecker Builder was called with more than one target or source"

        for s,t in zip(source,target):
            app = str(s.path)
            d.check_file(app)
            open(str(t),'w').write("")

def codeCheckActionStr(target, source, env):
        '''
        Return output string which will be seen when running unit tests.
        '''
        return 'Checking for style crimes in ' + str(source[0])

# Inform SCons about our CodeChecker
def generate(env):
        env['BUILDERS']['CodeCheck'] = env.Builder(
                        action = env.Action(codeCheckAction, codeCheckActionStr),
                        suffix='.codecheck')
def exists(env):
        return 1

