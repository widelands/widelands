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
import time
import datetime

#Python2/3 compat code for iterating items
try:
    dict.iteritems
except AttributeError:
    # Python 3
    def itervalues(d):
        return iter(d.values())
    def iteritems(d):
        return iter(d.items())
    def bytes_to_str(bytes):
        return str(bytes, 'utf-8')
else:
    # Python 2
    def itervalues(d):
        return d.itervalues()
    def iteritems(d):
        return d.iteritems()
    def bytes_to_str(bytes):
        return str(bytes)

has_timeout = "TimeoutExpired" in dir(subprocess)

if "monotonic" in dir(time):
    get_time = time.monotonic
elif "perf_counter" in dir(time):
    get_time = time.perf_counter
else:
    get_time = time.time

def datadir():
    return os.path.join(os.path.dirname(__file__), "data")

def datadir_for_testing():
    return os.path.relpath(".", os.path.dirname(__file__))

def out(string):
    sys.stdout.write(string)
    sys.stdout.flush()

class WidelandsTestCase(unittest.TestCase):
    do_use_random_directory = True
    path_to_widelands_binary = None
    keep_output_around = False
    ignore_error_code = False
    timeout = 600

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
        self.wl_timed_out = False

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
        stdout_filename = os.path.join(self.run_dir, "stdout_{:02d}.txt".format(which_time))
        if (os.path.exists(stdout_filename)):
            os.unlink(stdout_filename)

        with open(stdout_filename, 'a') as stdout_file:
            args = [self.path_to_widelands_binary,
                    '--verbose=true',
                    '--datadir={}'.format(datadir()),
                    '--datadir_for_testing={}'.format(datadir_for_testing()),
                    '--homedir={}'.format(self.run_dir),
                    '--nosound',
                    '--fail-on-lua-error',
                    '--language=en_US' ]
            args += [ "--{}={}".format(key, value) for key, value in iteritems(wlargs) ]

            stdout_file.write("Running widelands binary: ")
            for anarg in args:
              stdout_file.write(anarg)
              stdout_file.write(" ")
            stdout_file.write("\n")

            start_time = get_time()
            widelands = subprocess.Popen(
                    args, shell=False, stdout=stdout_file, stderr=subprocess.STDOUT)
            if has_timeout:
                try:
                    widelands.communicate(timeout = self.timeout)
                except subprocess.TimeoutExpired:
                    widelands.kill()
                    widelands.communicate()
                    self.wl_timed_out = True
                    stdout_file.write("\nTimed out.\n")
            else:
                widelands.communicate()
            end_time = get_time()
            stdout_file.flush()
            self.duration = datetime.timedelta(seconds = end_time - start_time)
            stdout_file.write("\nReturned from Widelands in {}, return code is {:d}\n".format(
                self.duration, widelands.returncode))
            self.widelands_returncode = widelands.returncode
        return stdout_filename

    def runTest(self):
        out("\nStarting test case {}\n".format(self._test_script))
        out("  Running Widelands ...\n")
        stdout_filename = self.run_widelands(self._wlargs, 0)
        stdout = open(stdout_filename, "r").read()
        self.verify_success(stdout, stdout_filename)

        find_saves = lambda stdout: re.findall("Script requests save to: (\w+)$", stdout, re.M)
        savegame_done = { fn: False for fn in find_saves(stdout) }
        which_time = 1
        while not all(savegame_done.values()):
            for savegame in sorted(savegame_done):
                if not savegame_done[savegame]: break
            out("  Loading savegame: {} ...\n".format(savegame))
            stdout_filename = self.run_widelands({ "loadgame": os.path.join(
                self.run_dir, "save", "{}.wgf".format(savegame))}, which_time)
            which_time += 1
            stdout = open(stdout_filename, "r").read()
            for new_save in find_saves(stdout):
                if new_save not in savegame_done:
                    savegame_done[new_save] = False
            savegame_done[savegame] = True
            self.verify_success(stdout, stdout_filename)

    def verify_success(self, stdout, stdout_filename):
        out("    Elapsed time: {}\n".format(self.duration))
        # Catch instabilities with SDL in CI environment
        if self.widelands_returncode == 2:
            print("SDL initialization failed. TEST SKIPPED.")
            with open(stdout_filename, 'r') as stdout_file:
                for line in stdout_file.readlines():
                    print(line.strip())
            out("  SKIPPED.\n")
        else:
            common_msg = "Analyze the files in {} to see why this test case failed. Stdout is\n  {}\n\nstdout:\n{}".format(
                    self.run_dir, stdout_filename, stdout)
            if self.wl_timed_out:
                out("  TIMED OUT.\n")
                self.assertTrue(False, "The test timed out. {}".format(common_msg))
            if self.widelands_returncode == 1 and self.ignore_error_code:
                out("  IGNORING error code 1\n")
            else:
                self.assertTrue(self.widelands_returncode == 0,
                    "Widelands exited abnormally. {}".format(common_msg)
                )
            self.assertTrue("All Tests passed" in stdout,
                "Not all tests pass. {}.".format(common_msg)
            )
            self.assertFalse("lua_errors.cc" in stdout,
                "Not all tests pass. {}.".format(common_msg)
            )
            out("  done.\n")
        if self.keep_output_around:
            out("    stdout: {}\n".format(stdout_filename))

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
    p.add_argument("-i", "--ignore-error-code", action="store_true", default = False,
        help = "Assume success on return code 1, to allow running the tests "
        "without ASan reporting false positives."
    )
    if has_timeout:
        p.add_argument("-t", "--timeout", type=float, default = "10",
            help = "Set the timeout duration for test cases in minutes. Default is 10 minutes."
        )
    else:
        p.epilog = "Python version does not support timeout. -t, --timeout is disabled. " \
                   "Python >=3.3 is required for timeout support."

    args = p.parse_args()

    if args.binary is None:
        potential_binaries = (
            glob(os.path.join(os.curdir, "widelands")) +
            glob(os.path.join(os.path.dirname(__file__), "widelands")) +
            glob(os.path.join("src", "widelands")) +
            glob(os.path.join("..", "*", "src", "widelands"))
        )
        if potential_binaries:
            args.binary = potential_binaries[0]
        elif "which" in dir(shutil):
            args.binary = shutil.which("widelands")

        if args.binary is None:
            p.error("No widelands binary found. Please specify with -b.")

    return args


def discover_loadgame_tests(regexp, suite):
    """Add all tests using --loadgame to the 'suite'."""
    for fixture in sorted(glob(os.path.join("test", "save", "*"))):
        if not os.path.isdir(fixture):
            continue
        savegame = sorted(glob(os.path.join(fixture, "*.wgf")))[0]
        for test_script in sorted(glob(os.path.join(fixture, "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        loadgame=savegame, script=test_script))

def discover_scenario_tests(regexp, suite):
    """Add all tests using --scenario to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        scenario=wlmap, script=test_script))

def discover_editor_tests(regexp, suite):
    """Add all tests needing --editor to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "editor_test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.addTest(
                    WidelandsTestCase(test_script,
                        editor=wlmap, script=test_script))

def main():
    args = parse_args()

    WidelandsTestCase.path_to_widelands_binary = args.binary
    print("Using '{}' binary.".format(args.binary))
    WidelandsTestCase.do_use_random_directory = not args.nonrandom
    WidelandsTestCase.keep_output_around = args.keep_around
    WidelandsTestCase.ignore_error_code = args.ignore_error_code
    if has_timeout:
        WidelandsTestCase.timeout = args.timeout * 60
    else:
        out("Python version does not support timeout on subprocesses,\n"
            "test cases may run indefinitely.\n\n")

    suite = unittest.TestSuite()
    discover_loadgame_tests(args.regexp, suite)
    discover_scenario_tests(args.regexp, suite)
    discover_editor_tests(args.regexp, suite)

    return unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful()

if __name__ == '__main__':
    sys.exit(0 if main() else 1)
