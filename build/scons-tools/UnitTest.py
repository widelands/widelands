#!/usr/bin/env python -tt
# encoding: utf-8
# 
# This was stolen and slightly modified from the scons wiki:
# http://www.scons.org/wiki/UnitTests
#

import os
import SCons
from SCons.Script.SConscript import SConsEnvironment

def unitTestAction(target, source, env):
        '''
        Action for a 'UnitTest' builder object.
        Runs the supplied executable, reporting failure to scons via the test exit
        status.
        When the test succeeds, the file target.passed is created to indicate that
        the test was successful and doesn't need running again unless dependencies
        change.
        '''
        app = str(source[0].abspath)
        if os.spawnle(os.P_WAIT, app, env['ENV'])==0:
                open(str(target[0]),'w').write("PASSED\n")
        else:
                return 1
def unitTestActionString(target, source, env):
        '''
        Return output string which will be seen when running unit tests.
        '''
        return 'Running tests in ' + str(source[0])
def addUnitTest(env, target=None, source=None, *args, **kwargs):
        '''
        Add a unit test
        Parameters:
                target - If the target parameter is present, it is the name of the test
                                executable
                source - list of source files to create the test executable.
                any additional parameters are passed along directly to env.Program().
        Returns:
                The scons node for the unit test.
        Any additional files listed in the env['UTEST_MAIN_SRC'] build variable are
        also included in the source list.
        All tests added with addUnitTest can be run with the test alias:
                "scons test"
        Any test can be run in isolation from other tests, using the name of the
        test executable provided in the target parameter:
                "scons target"
        '''
        if source is None:
                source = target
                target = None
        source = [source, env['UTEST_MAIN_SRC']]
        program = env.Program(target, source, *args, **kwargs)
        utest = env.UnitTest(program)
        # add alias to run all unit tests.
        env.AlwaysBuild(utest)
        env.Alias('test', utest)
        # make an alias to run the test in isolation from the rest of the tests.
        env.Alias(str(program[0]), utest)
        return utest
#-------------------------------------------------------------------------------
# Functions used to initialize the unit test tool.
def generate(env, UTEST_MAIN_SRC=[], LIBS=[], CXXFLAGS=[]):
        env['BUILDERS']['UnitTest'] = env.Builder(
                        action = env.Action(unitTestAction, unitTestActionString),
                        suffix='.passed')
        env['UTEST_MAIN_SRC'] = UTEST_MAIN_SRC
        env.AppendUnique(LIBS=LIBS)
        env.AppendUnique(CXXFLAGS=CXXFLAGS)
        # The following is a bit of a nasty hack to add a wrapper function for the
        # UnitTest builder, see http://www.scons.org/wiki/WrapperFunctions
        SConsEnvironment.addUnitTest = addUnitTest
def exists(env):
        return 1

