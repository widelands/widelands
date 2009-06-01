#!/usr/bin/env python 
#
# File: build/scons-tools/distcc.py
#
# Created by Victor Pelt on 2009-06-02
# Copyright (c) 2009 Victor Pelt. All rights reserved
# Licensed under the GPL-2
#

import os

def setup_ccache(env):
        binary = None
        if 'PATH' in env:
            binary = env.WhereIs('ccache', path=env['PATH'])
            if binary == None:
                print 'ccache not found'
                return 1
            else:
                print 'ccache found at ' + binary
        env['CXX'] = binary + ' ' + env['CXX']
	env['CC'] = binary + ' ' + env['CC']
        list = ('CCACHE_DIR',
                'CCACHE_TEMPDIR',
                'CCACHE_LOGFILE',
                'CCACHE_PATH',
                'CCACHE_CC',
                'CCACHE_PREFIX',
                'CCACHE_DISABLE',
                'CCACHE_READONLY',
                'CCACHE_CPP2',
                'CCACHE_NOSTATS',
                'CCACHE_NLEVELS',
                'CCACHE_HARDLINK',
                'CCACHE_RECACHE',
                'CCACHE_UMASK',
                'CCACHE_HASHDIR',
                'CCACHE_UNIFY',
                'CCACHE_EXTENSION')


        
        for var in list:
		if var in os.environ:
			env['ENV'][var] = os.environ[var]
        return 0
        

def exists(env):
	return True
