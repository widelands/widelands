#!/usr/bin/env python3
# encoding: utf-8

from glob import glob
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
import concurrent.futures as cf
import time
import datetime
import multiprocessing

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

try:
    import psutil
except:
    psutil = None

def datadir():
    return os.path.join(os.path.dirname(__file__), "data")

def datadir_for_testing():
    return os.path.relpath(".", os.path.dirname(__file__))

ansi_colors = {
    'black':   '\033[30m',
    'red':     '\033[31m',
    'green':   '\033[32m',
    'yellow':  '\033[33m',
    'blue':    '\033[34m',
    'purple':  '\033[35m',
    'cyan':    '\033[36m',
    'white':   '\033[37m',
    'default': '\033[39m'
}

info_color      = 'cyan'
success_color   = 'green'
warning_color     = 'yellow'
error_color     = 'red'
separator_color = 'purple'

log_colors = {
    # order in decreasing priority
    'ERROR':   error_color,
    'WARNING': warning_color,
}

use_colors = False

def colorize(text, color):
    if not use_colors:
        return text
    return f'{ansi_colors[color]}{text}{ansi_colors["default"]}'

def colorize_log(text):
    if not use_colors:
        return text
    for key,color in iteritems(log_colors):
        if key in text:
            return colorize(text, color)
    return text

class WidelandsTestCase():
    do_use_random_directory = True
    path_to_widelands_binary = None
    keep_output_around = False
    ignore_error_code = False
    timeout = 600
    total_tests = 0

    statuses = {
        'Starting':  'Start',
        'Loading':   'Load ',
        'Passed':    'Done ',
        'FAILED':    'FAIL ',
        'TIMED OUT': 'TMOUT',
        'SKIPPED':   'SKIP ',
        'IGNORING':  ' IGN ',
        'Info':      'Info '
    }

    status_colors = {
        'Done ': success_color,
        'FAIL ': error_color,
        'TMOUT': error_color,
        'SKIP ': warning_color,
        ' IGN ': warning_color,
        'Info ': info_color
    }

    def __init__(self, test_script, **wlargs):
        self.test_script = test_script
        self._shortname = os.path.basename(test_script)
        self._wlargs = wlargs
        self.success = None
        self.result = "NOT EXECUTED"
        self.report_header = None
        # 'which_time' is an integer defining the number of times Widelands has run this test
        # case (i.e. because we might load a saved game from an earlier run). This will impact
        # the filenames for stdout.txt.
        self.which_time = 0
        self.outputs = []
        WidelandsTestCase.total_tests += 1
        self._test_number = WidelandsTestCase.total_tests

    def run_widelands(self, wlargs):
        """Run Widelands with the given 'wlargs'.

        Returns the stdout filename."""
        stdout_filename = os.path.join(self.run_dir, "stdout_{:02d}.txt".format(self.which_time))
        if (os.path.exists(stdout_filename)):
            os.unlink(stdout_filename)

        with open(stdout_filename, 'a') as stdout_file:
            args = [self.path_to_widelands_binary,
                    '--verbose',
                    '--datadir={}'.format(datadir()),
                    '--datadir_for_testing={}'.format(datadir_for_testing()),
                    '--homedir={}'.format(self.run_dir),
                    '--nosound',
                    '--fail-on-lua-error',
                    '--language=en' ]
            args += [ "--{}={}".format(key, value) for key, value in iteritems(wlargs) ]

            stdout_file.write("Running widelands binary: ")
            for anarg in args:
              stdout_file.write(anarg)
              stdout_file.write(" ")
            stdout_file.write("\n")
            stdout_file.flush()

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
                    stdout_file.write('\n')
                    stdout_file.write(colorize('Timed out.', error_color))
                    stdout_file.write('\n')
            else:
                widelands.communicate()
            end_time = get_time()
            stdout_file.flush()
            self.duration = datetime.timedelta(seconds = end_time - start_time)
            stdout_file.write('\n')
            stdout_file.write(
                colorize(f'{self.step_name()}: Returned from Widelands in {self.duration}, ' \
                f'return code is {widelands.returncode:d}', info_color))
            stdout_file.write('\n')
            self.widelands_returncode = widelands.returncode
        self.outputs.append(stdout_filename)
        return stdout_filename

    def run(self):
        num_w = len(str(self.total_tests))
        self._progress = str(self._test_number).rjust(num_w) + '/' + str(self.total_tests)

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
        self.current_step = ""

        self.out_status('Start', f'{self.test_script} starting ...')
        stdout_filename = self.run_widelands(self._wlargs)
        stdout = open(stdout_filename, "r").read()
        self.verify_success(stdout, stdout_filename)

        find_saves = lambda stdout: re.findall(r'Script requests save to: (\w+)$', stdout, re.M)
        savegame_done = { fn: False for fn in find_saves(stdout) }
        while self.success and not all(savegame_done.values()):
            self.which_time += 1
            for savegame in sorted(savegame_done):
                if not savegame_done[savegame]: break
            self.current_step = "{}.wgf".format(savegame)
            self.out_status('Load ', f'loading savegame ...')
            stdout_filename = self.run_widelands({ "loadgame": os.path.join(
                self.run_dir, "save", "{}.wgf".format(savegame))})
            stdout = open(stdout_filename, "r").read()
            for new_save in find_saves(stdout):
                if new_save not in savegame_done:
                    savegame_done[new_save] = False
            savegame_done[savegame] = True
            self.verify_success(stdout, stdout_filename)

        if self.report_header == None and not self.keep_output_around:
            shutil.rmtree(self.run_dir)

    def step_name(self):
        if self.which_time == 0:
            return self._shortname
        return f'{self._shortname} / {self.current_step}'

    def out_status(self, status, message):
        # Force writing to main test log immediately
        if use_colors and status in self.status_colors.keys():
            color = self.status_colors[status]
            status = colorize(status, color)
            message = colorize(message, color)
        sys.stdout.write(f'{self._progress} {status} {self.step_name()}: {message}\n')
        sys.stdout.flush()

    def get_result_color(self):
        if not use_colors:
            return 'default'
        status = self.statuses[self.result]
        if status in self.status_colors.keys():
            return self.status_colors[status]
        return 'default'

    def out_result(self, stdout_filename):
        self.out_status(self.statuses[self.result], f'{self.result} in {self.duration}')
        if self.keep_output_around:
            self.out_status('Info ', f'stdout: {stdout_filename}')

    def fail(self, short, long, stdout_filename):
        self.success = False
        self.result = short
        self.out_result(stdout_filename)
        long = colorize(long, self.status_colors['FAIL '])
        self.report_header = f'{long} Analyze the files in {self.run_dir} to see why this test case failed.\n'
        self.report_header += colorize(f'Stdout is: {stdout_filename}', info_color)

    def step_success(self, stdout_filename):
        old_result = self.result
        self.result = "Passed"
        self.out_result(stdout_filename)
        if self.which_time == 0:
            self.success = True
        else:
            self.result = old_result

    def verify_success(self, stdout, stdout_filename):
        # Catch instabilities with SDL in CI environment
        if self.widelands_returncode == 2:
            # Print stdout in the final summary with this header
            self.result = "SKIPPED"
            self.report_header = 'SDL initialization failed. TEST SKIPPED.'
            self.out_status('SKIP ', self.report_header)
            if use_color:
                self.report_header = colorize(self.report_header, self.status_colors['SKIP '])
            if self.which_time == 0:  # must set it for the first run, later just ignore
                self.success = True
        else:
            if self.wl_timed_out:
                self.fail("TIMED OUT", "The test timed out.", stdout_filename)
                return
            if self.widelands_returncode != 0:
                if self.widelands_returncode == 1 and self.ignore_error_code:
                    self.out_status(' IGN ', f'IGNORING error code 1')
                else:
                    self.fail("FAILED", "Widelands exited abnormally.", stdout_filename)
                    return
            if not "All Tests passed" in stdout or "lua_errors.cc" in stdout:
                self.fail("FAILED", "Not all tests pass.", stdout_filename)
                return
            self.step_success(stdout_filename)


# For parallel execution of tests
def recommended_workers(binary):
    cpu_count = multiprocessing.cpu_count()

    # Widelands uses 2 threads, but the logic thread is much more heavy,
    # so we give each Widelands instance one full CPU for the logic thread
    # while we let the graphic threads of up to 3 instances share one CPU.
    max_threads_cpu = 1
    if cpu_count > 2 :
        max_threads_cpu = cpu_count - 1 - (cpu_count - 1) // 4

    # Also test memory as a limiting factor
    if psutil != None:
        mem_per_instance = 1500 # MB - default for debug builds

        widelands = subprocess.run([binary, '--version'], shell=False, encoding='utf-8', capture_output=True)
        firstline = widelands.stdout.splitlines(keepends=False)[0]
        firstline_OK = False
        if firstline.startswith('This is Widelands version'):
            if firstline.endswith('Release'):
                mem_per_instance = 800 # MB
                firstline_OK = True
            elif firstline.endswith('Debug'):
                firstline_OK = True
        if not firstline_OK:
            print('Cannot parse build type from stdout:', firstline)

        max_threads_mem = max(1, psutil.virtual_memory().available // (mem_per_instance * 1000 * 1000))
        print("{} CPUs, {}/{} bytes memory free".format(cpu_count, psutil.virtual_memory().available, psutil.virtual_memory().total))
        return min(max_threads_cpu, max_threads_mem)
    else:
        return max_threads_cpu

def find_binary():
    # Prefer binary from source directory
    for potential_binary in (
        glob(os.path.join(os.curdir, "widelands")) +
        glob(os.path.join(os.path.dirname(__file__), "widelands")) +
        glob(os.path.join("src", "widelands")) +
        glob(os.path.join("..", "*", "src", "widelands"))
    ):
        if os.access(potential_binary, os.X_OK):
            return potential_binary

    # Fall back to binary in $PATH if possible
    if "which" in dir(shutil):
        return shutil.which("widelands")

    return None

def check_binary(binary):
    if "which" in dir(shutil) and shutil.which(binary) != None:
        return binary

    if os.path.dirname(binary) != '' and os.access(binary, os.X_OK):
        return binary

    for potential_path in [ os.curdir, os.path.dirname(__file__) ]:
        fullpath = os.path.join(potential_path, binary)
        if os.access(fullpath, os.X_OK):
            return fullpath

    return None

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
    p.add_argument("-j", "--workers", type=int, default = 1,
        help = "Use this many parallel workers."
    )
    p.add_argument("-c", "--color", "--colour", action="store_true", default = False,
        help = "Colorize the output with ANSI color sequences."
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
        args.binary = find_binary()
        if args.binary is None:
            p.error("No widelands binary found. Please specify with -b.")
    else:
        args.binary = check_binary(args.binary)
        if args.binary is None:
            p.error("The specified widelands binary is not found.")

    if args.nonrandom:
        if args.workers != 1:
            args.workers = 1
            print("Only one worker is possible with --nonrandom!")
        if args.keep_around:
            print("--nonrandom is not recommended with --keep-around, some files will be overwritten!")

    if args.workers == 0:
        args.workers = recommended_workers(args.binary)

    return args


def discover_loadgame_tests(regexp, suite):
    """Add all tests using --loadgame to the 'suite'."""
    # Savegames with custom scripts
    for fixture in sorted(glob(os.path.join("test", "save", "*"))):
        if not os.path.isdir(fixture):
            continue
        savegame = sorted(glob(os.path.join(fixture, "*.wgf")))[0]
        for test_script in sorted(glob(os.path.join(fixture, "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.append(
                    WidelandsTestCase(test_script,
                        loadgame=savegame, script=test_script))
    # Savegames without custom script, just test loading
    test_script = os.path.join("test", "scripting", "load_and_quit.lua")
    for savegame in sorted(glob(os.path.join("test", "save", "*.wgf"))):
        if regexp is not None and not re.search(regexp, savegame):
            continue
        suite.append(WidelandsTestCase(savegame, loadgame=savegame, script=test_script))

def discover_scenario_tests(regexp, suite):
    """Add all tests using --scenario to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.append(
                    WidelandsTestCase(test_script,
                        scenario=wlmap, script=test_script))

def discover_game_template_tests(regexp, suite):
    """Add all tests using --new_game_from_template to the 'suite'."""
    for templ in sorted(glob(os.path.join("test", "templates", "test*.wgt"))):
        if not os.path.isfile(templ):
            continue
        test_script = templ[:-3] + 'lua'
        if not os.path.isfile(test_script):
            print(f"WARNING: Game template test { templ }: corresponding script { test_script } not found - Skipping.")
            continue
        if regexp is not None and not re.search(regexp, test_script):
            continue
        suite.append(
                WidelandsTestCase(test_script,
                    new_game_from_template=templ, script=test_script))

def discover_editor_tests(regexp, suite):
    """Add all tests needing --editor to the 'suite'."""
    for wlmap in sorted(glob(os.path.join("test", "maps", "*"))):
        if not os.path.isdir(wlmap):
            continue
        for test_script in sorted(glob(os.path.join(wlmap, "scripting", "editor_test*.lua"))):
            if regexp is not None and not re.search(regexp, test_script):
                continue
            suite.append(
                    WidelandsTestCase(test_script,
                        editor=wlmap, script=test_script))

def main():
    args = parse_args()

    global use_colors
    use_colors = args.color
    WidelandsTestCase.path_to_widelands_binary = args.binary
    print("Using '{}' binary.".format(args.binary))
    WidelandsTestCase.do_use_random_directory = not args.nonrandom
    WidelandsTestCase.keep_output_around = args.keep_around
    WidelandsTestCase.ignore_error_code = args.ignore_error_code
    if has_timeout:
        WidelandsTestCase.timeout = args.timeout * 60
    else:
        print("Python version does not support timeout on subprocesses,\n"
            "test cases may run indefinitely.\n")

    test_cases = []
    discover_loadgame_tests(args.regexp, test_cases)
    discover_scenario_tests(args.regexp, test_cases)
    discover_game_template_tests(args.regexp, test_cases)
    discover_editor_tests(args.regexp, test_cases)

    print(f"Will run { len(test_cases) } tests with { args.workers } workers.\n")

    start_time = get_time()

    if args.workers == 1:
        # Single-threaded execution is special-cased for nicer grouping.
        for test_case in test_cases:
            test_case.run()
    else:
        # Parallel execution
        with cf.ThreadPoolExecutor(max_workers = args.workers) as executor:
            futures = {executor.submit(test_case.run): test_case for test_case in test_cases}
            for future in cf.as_completed(futures):
                if future.exception():
                    raise future.exception()

    end_time = get_time()

    separator = '\n' + \
        colorize('---------------------------------------------------------------------------',
            separator_color) + '\n'

    group_start = '\n'
    group_end = ''
    if os.getenv('GITHUB_ACTION'):
        group_start = '\n::group::'
        group_end = '\n::endgroup::\n'

    nr_errors = 0
    results = dict()
    for test_case in test_cases:
        # Skipped test cases are logged, but don't count as failure
        if test_case.report_header != None:
            print(separator)
            print(f'{colorize(test_case.result, test_case.get_result_color())}: {test_case.test_script}\n')
            print(test_case.report_header)
            print(group_start, end='')
            print(colorize("stdout:", info_color))
            for stdout_fn in test_case.outputs:
                with open(stdout_fn, "r") as stdout:
                    for line in stdout:
                        line = colorize_log(line)
                        print(line, end='')
            print(group_end, end='')
            if test_case.result in results.keys():
                results[test_case.result].append(test_case.test_script)
            else:
                results[test_case.result] = [ test_case.test_script ]
        if not test_case.success:
            nr_errors += 1

    print(separator)

    summary_common = f'Ran {len(test_cases)} test cases in {(end_time - start_time):.3f} s,'

    if nr_errors == 0:
        print(summary_common, colorize('all tests passed.', success_color))
        return True

    for result,tests in iteritems(results):
        print(f'{len(tests)} tests {result}:')
        for test_name in tests:
            print("     {}".format(test_name))
    print(separator)
    print(summary_common, f'{len(test_cases) - nr_errors} tests passed,',
          colorize(f'{nr_errors} tests failed!', error_color))
    return False

if __name__ == '__main__':
    sys.exit(0 if main() else 1)
