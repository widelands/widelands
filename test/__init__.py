#!/usr/bin/env python
# encoding: utf-8

"""
Classes and utilities for Widelands Regression testing.
"""

import os
import re
import shutil
import subprocess
import tempfile
import unittest

class WidelandsTestCase(unittest.TestCase):
    do_use_random_directory = True
    path_to_widelands_binary = None

    def setUp(self):
        if self.do_use_random_directory:
            self.run_dir = tempfile.mkdtemp(prefix="widelands_regression_test")
        else:
            self.run_dir = os.path.join(tempfile.gettempdir(), "widelands_regression_test", self.__class__.__name__)
            if os.path.exists(self.run_dir):
                shutil.rmtree(self.run_dir)
            os.makedirs(self.run_dir)
        print "stdout: %s/stdout.txt" % self.run_dir
        self.widelands_returncode = 0

    def run(self, result=None):
        self.currentResult = result # remember result for use in tearDown
        unittest.TestCase.run(self, result)

    def tearDown(self):
        if self.currentResult.wasSuccessful():
            shutil.rmtree(self.run_dir)

    def run_widelands(self, **kwargs):
        """Runs widelands with the arguments given, catching stdout and stderr in files."""

        stdout_filename = os.path.join(self.run_dir, "stdout.txt")

        with open(stdout_filename, 'a') as stdout_file:
            args = [self.path_to_widelands_binary, '--verbose=true',
                    '--datadir=.', '--homedir=%s' % self.run_dir,
                    '--disable_fx=true', '--disable_music=true', '--maxfps=25', ]
            args += [ "--%s=%s" % (key, value) for key, value in kwargs.iteritems() ]
            stdout_file.write("---- TestRunner: Starting Widelands: %s\n\n" % args)

            widelands = subprocess.Popen(
                    args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            while 1:
                line = widelands.stdout.readline()
                if not line:
                    break
                stdout_file.write(line)
                stdout_file.flush()
            widelands.communicate()

            stdout_file.write("\n\n---- TestRunner: Widelands terminated.\n\n")
            self.widelands_returncode = widelands.returncode
        self.stdout = open(stdout_filename, "r").read()

    def assert_all_lunit_tests_passed(self):
        success = self.widelands_returncode == 0 and "All Tests passed." in self.stdout

        self.assertTrue(success,
            "Not all tests pass. Analyze the files in %s to see why this test case failed." % self.run_dir
        )
