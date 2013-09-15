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

    def setUp(self):
        if self.do_use_random_directory:
            self.run_dir = tempfile.mkdtemp(prefix="widelands_regression_test")
        else:
            self.run_dir = os.path.join(tempfile.gettempdir(), "widelands_regression_test", self.__class__.__name__)
            if os.path.exists(self.run_dir):
                shutil.rmtree(self.run_dir)
            os.makedirs(self.run_dir)
        self.widelands_returncode = 0

    def run(self, result=None):
        self.currentResult = result # remember result for use in tearDown
        unittest.TestCase.run(self, result)

    def tearDown(self):
        if self.currentResult.wasSuccessful():
            shutil.rmtree(self.run_dir)

    def run_widelands(self, **kwargs):
        """Runs widelands with the arguments given, catching stdout and stderr in files."""

        maps_dir = os.path.join(".", "test", "maps")
        if "editor" in kwargs:
            kwargs["editor"] = os.path.join(maps_dir, kwargs["editor"])
        if "scenario" in kwargs:
            kwargs["scenario"] = os.path.join(maps_dir, kwargs["scenario"])
        if "loadgame" in kwargs:
            kwargs["loadgame"] = os.path.join(self.run_dir, "save", kwargs["loadgame"])

        stdout_filename = os.path.join(self.run_dir, "stdout.txt")
        stderr_filename = os.path.join(self.run_dir, "stderr.txt")

        with open(stdout_filename, 'a') as stdout_file, open(stderr_filename, 'a') as stderr_file:
            # NOCOM(#sirver): hard coded
            args = ['../widelands.build/src/widelands', '--verbose=true',
                    '--datadir=.', '--homedir=%s' % self.run_dir,
                    '--disable_fx=true', '--disable_music=true' ]
            args += [ "--%s=%s" % (key, value) for key, value in kwargs.iteritems() ]
            stdout_file.write("---- TestRunner: Starting Widelands: %s\n\n" % args)

            widelands = subprocess.Popen(
                    args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (self.stdout, self.stderr) = widelands.communicate()

            stderr_file.write(self.stderr)
            stdout_file.write(self.stdout)

            stdout_file.write("\n\n---- TestRunner: Widelands terminated.\n\n")

            self.widelands_returncode = widelands.returncode

    def assert_all_lunit_tests_passed(self):
        success = (
            self.widelands_returncode == 0 and
            re.search("All Tests passed.", self.stdout, re.M) is not None
        )

        self.assertTrue(success,
            "Not all tests pass. Analyze the files in %s to see why this test case failed." % self.run_dir
        )

