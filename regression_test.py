#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
import platform

def out(string):
    sys.stdout.write(string)
    sys.stdout.flush()

class WidelandsTestCase(unittest.TestCase):
    do_use_random_directory = True
    path_to_widelands_binary = None
    keep_output_around = False

    def __init__(self, test_script, **wlargs):
        unittest.TestCase.__init__(self)
        self._test_script = test_script
        self._wlargs = wlargs

    def __str__(self):
        return self._test_script

    def setUp(self):
        if self.do_use_random_directory:
            self.run_dir = tempfile.mkdtemp(prefix="widelands_regression_test")
        else:
            self.run_dir = os.path.join(tempfile.gettempdir(), "widelands_regression_test", self.__class__.__name__)
            if os.path.exists(self.run_dir):
                if not self.keep_output_around:
                    shutil.rmtree(self.run_dir)
                    os.makedirs(self.run_dir)
            else:
                os.makedirs(self.run_dir)
        self.widelands_returncode = 0

    def run(self, result=None):
        self.currentResult = result # remember result for use in tearDown
        unittest.TestCase.run(self, result)

    def tearDown(self):
        if self.currentResult.wasSuccessful() and not self.keep_output_around:
            shutil.rmtree(self.run_dir)

    def run_widelands(self, wlargs, which_time):
        """Run Widelands with the given 'wlargs'. 'which_time' is an integer
        defining the number of times Widelands has been run this test case
        (i.e. because we might load a saved game from an earlier run. This will
        impact the filenames for stdout.txt.

        Returns the stdout filename."""
        stdout_filename = os.path.join(self.run_dir, "stdout_%02i.txt" % which_time)
        if (os.path.exists(stdout_filename)):
            os.unlink(stdout_filename)

        with open(stdout_filename, 'a') as stdout_file:
            args = [self.path_to_widelands_binary, '--verbose=true',
                    '--datadir=.', '--homedir=%s' % self.run_dir,
                    '--disable_fx=true', '--disable_music=true' ]
            args += [ "--%s=%s" % (key, value) for key, value in wlargs.iteritems() ]

            widelands = subprocess.Popen(
                    args, shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            while 1:
                line = widelands.stdout.readline()
                if not line:
                    break
                stdout_file.write(line)
                stdout_file.flush()
            widelands.communicate()
            if platform.system() == "Windows":
                 win_stdout = self.path_to_widelands_binary.replace("widelands.exe","stdout.txt")
                 win_stderr = self.path_to_widelands_binary.replace("widelands.exe","stderr.txt")
                 with open(win_stdout,"r") as f:
                     for line in f:
                         stdout_file.write(line)
                     stdout_file.flush()
                 if (os.path.exists(win_stderr)):
                     with open(win_stderr,"r") as f:
                         for line in f:
                             stdout_file.write(line)
                         stdout_file.flush()

            self.widelands_returncode = widelands.returncode
        return stdout_filename

    def runTest(self):
        out("\n  Running Widelands ... ")
        stdout_filename = self.run_widelands(self._wlargs, 0)
        stdout = open(stdout_filename, "r").read()
        self.verify_success(stdout, stdout_filename)

        find_saves = lambda stdout: re.findall("Script requests save to: (\w+)$", stdout, re.M)
        savegame_done = { fn: False for fn in find_saves(stdout) }
        which_time = 1
        while not all(savegame_done.values()):
            for savegame in sorted(savegame_done):
                if not savegame_done[savegame]: break
            out("  Loading savegame: %s ... " % savegame)
            stdout_filename = self.run_widelands({ "loadgame": os.path.join(
                self.run_dir, "save", "%s.wgf" % savegame) }, which_time)
            which_time += 1
            stdout = open(stdout_filename, "r").read()
            for new_save in find_saves(stdout):
                if new_save not in savegame_done:
                    savegame_done[new_save] = False
            savegame_done[savegame] = True
            self.verify_success(stdout, stdout_filename)

    def verify_success(self, stdout, stdout_filename):
        common_msg = "Analyze the files in %s to see why this test case failed. Stdout is\n  %s\n" % (
                self.run_dir, stdout_filename)
        self.assertTrue(self.widelands_returncode == 0,
            "Widelands exited abnormally. %s" % common_msg
        )
        self.assertTrue("All Tests passed" in stdout,
            "Not all tests pass. %s." % common_msg
        )
        out("done.\n")
        if self.keep_output_around:
            out("    stdout: %s\n" % stdout_filename)

def parse_args():
    p = argparse.ArgumentParser(description=
        "Run the regression tests suite."
    )

    p.add_argument("-r", "--regexp", type=str,
        help = "Run only the tests from the files which filename matches."
    )
    p.add_argument("-n", "--nonrandom", action="store_true", default = False,
        help = "Do not randomize the directories for the tests. This is useful "
        "if you want to run a test more often than once and not reopen stdout.txt "
        "in your editor."
    )
    p.add_argument("-k", "--keep-around", action="store_true", default = False,
        help = "Keep the output files around even when a test terminates successfully."
    )
    p.add_argument("-b", "--binary", type=str,
        help = "Run this binary as Widelands. Otherwise some default paths are searched."
    )

    args = p.parse_args()

    if args.binary is None:
        potential_binaries = (
            glob("widelands") +
            glob("src/widelands") +
            glob("../*/src/widelands")
        )
        if not potential_binaries:
            p.error("No widelands binary found. Please specify with -b.")
        args.binary = potential_binaries[0]
    return args


def discover_loadgame_tests(regexp, suite):
    """Add all tests using --loadgame to the 'suite'."""
    for fixture in glob(os.path.join("test", "save", "*")):
        if not os.path.isdir(fixture):
            continue
        savegame = glob(os.path.join(fixture, "*.wgf"))[0]
        for test_script in glob(os.path.join(fixture, "test*.lua")):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        loadgame=savegame, script=test_script))

def discover_scenario_tests(regexp, suite):
    """Add all tests using --scenario to the 'suite'."""
    for wlmap in glob(os.path.join("test", "maps", "*")):
        if not os.path.isdir(wlmap):
            continue
        for test_script in glob(os.path.join(wlmap, "scripting", "test*.lua")):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        scenario=wlmap, script=test_script))

def discover_editor_tests(regexp, suite):
    """Add all tests needing --editor to the 'suite'."""
    for wlmap in glob(os.path.join("test", "maps", "*")):
        if not os.path.isdir(wlmap):
            continue
        for test_script in glob(os.path.join(wlmap, "scripting", "editor_test*.lua")):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        editor=wlmap, script=test_script))

def main():
    args = parse_args()

    WidelandsTestCase.path_to_widelands_binary = args.binary
    print "Using '%s' binary." % args.binary
    WidelandsTestCase.do_use_random_directory = not args.nonrandom
    WidelandsTestCase.keep_output_around = args.keep_around

    all_files = [os.path.basename(filename) for filename in glob("test/test_*.py") ]
    if args.regexp:
        all_files = [filename for filename in all_files if re.search(args.regexp, filename) ]

    all_modules = [ "test.%s" % filename[:-3] for filename in all_files ]

    suite = unittest.TestSuite()
    discover_loadgame_tests(args.regexp, suite)
    discover_scenario_tests(args.regexp, suite)
    discover_editor_tests(args.regexp, suite)

    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()

if __name__ == '__main__':
    sys.exit(0 if main() else 1)
