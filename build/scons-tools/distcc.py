#!/usr/bin/env python 
#
# File: build/scons-tools/distcc.py
#
# Created by Victor Pelt on 2009-06-02
# Copyright (c) 2009 Victor Pelt. All rights reserved
# Licensed under the GPL-2
#

import os, sys, string
import SCons.Action

def setup_distcc(env):
	print 'setting up distcc'
	env['ENV']['HOME'] = os.environ['HOME']
        binary = None
        if 'PATH' in env:
            binary = env.WhereIs('distcc', path=env['PATH'])
            if binary == None:
                print 'distcc not found'
                return 1
            else:
                print 'distcc found at ' + binary
        env['CXX'] = binary + ' ' + env['CXX']
	env['CC'] = binary + ' ' + env['CC']
        if 'DISTCC_HOSTS' in os.environ:
            env['ENV']['DISTCC_HOSTS'] = os.environ['DISTCC_HOSTS']
        else:
            env['ENV']['DISTCC_HOSTS'] = '+zeroconf'
        list = ('DISTCC_VERBOSE',
                'DISTCC_LOG',
                'DISTCC_FALLBACK',
                'DISTCC_SAVE_TEMPS',
                'DISTCC_TCP_CORK',
                'DISTCC_SSH',
                'DISTCC_DIR',
                'TMPDIR',
                'UNCACHED_ERR_FD',
                'DISTCC_ENABLE_DISCREPANCY_EMAIL',
                'DCC_EMAILLOGW_WHOM_TO_BLAME')

        
        for var in list:
		if var in os.environ:
			env['ENV'][var] = os.environ[var]
        return 0
        
def exists(env):
	return True
