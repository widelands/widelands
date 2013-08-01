#!/usr/bin/env python
# encoding: utf-8

"""
Classes and utilities for Widelands Regression testing.
"""

import os
import subprocess
import tempfile
import unittest

class WidelandsTestCase(unittest.TestCase):
    def setUp(self):
        self.maps_dir = os.path.join(".", "test", "__maps__")
        self.home_dir = tempfile.mkdtemp(prefix="widelands_regression_test")

    def tearDown(self):
        # NOCOM(#sirver): should clean up on success, bark on error.
        print "Tear down called!"

    def run_widelands(self, **kwargs):
        """Runs widelands with the arguments given, catching stdout and stderr in files."""

        stdout_filename = os.path.join(self.home_dir, "stdout.txt")
        stderr_filename = os.path.join(self.home_dir, "stderr.txt")

        print "#sirver stdout_filename: %r\n" % (stdout_filename)
        with open(stdout_filename, 'w') as stdout_file, open(stderr_filename, 'w') as stderr_file:
            # NOCOM(#sirver): shoud ignore config files.
            # NOCOM(#sirver): hard coded
            args = ["../widelands.build/src/widelands", '--datadir=.', '--homedir=%s' % self.home_dir]
            args += [ "--%s=%s" % (key, value) for key, value in kwargs.iteritems() ]

            widelands = subprocess.Popen(args, shell=False, stdout=stdout_file, stderr=stderr_file)
            widelands.communicate()
        self.assertEqual(widelands.returncode, 0)
