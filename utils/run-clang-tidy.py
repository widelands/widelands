#!/usr/bin/env python
#
# ===- run-clang-tidy.py - Parallel clang-tidy runner ---------*- python -*--===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# ===------------------------------------------------------------------------===#
# FIXME: Integrate with clang-tidy-diff.py
# ===------------------------------------------------------------------------===#
#
# Downloaded from https://github.com/llvm-mirror/clang-tools-extra/blob/master/clang-tidy/tool/run-clang-tidy.py
# Version 1c8cadde7ea4ca20a449edcffe10d23b612fe5d6
#
# ===------------------------------------------------------------------------===#
# HOW TO RUN THIS TOOL
#
# 1. Install the needed libraries. On Ubuntu, this is:
#
#       sudo apt install clang clang-tidy python-yaml
#
#    If you wish to use the autofix, you'll need the clang-tools package too.
#
# 2. Run compile.sh to create the build directory. You can abort this pretty quick.
#
# 3. Run the following commands:
#
#        cd build
#        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
#
#    This will give you a file 'build/compile_commands.json'
#
# 4. Run the tool from the 'build' directory, for example:
#
#        python ../utils/run-clang-tidy.py -checks=*,-android*,-fuchsia* > ../clang-tidy.log
#
#    Results will then be in 'clang-tidy.log'
#
# 5. You can pick which warnings you want with the '-checks=...' parameter like
#    in the above example. A '.clang-tidy' config file is provided, so normally
#    you should not use this parameter.
#
# 6. For seeing only warnings that were cleared previously, run from the
#    widelands main directory:
#
#        utils/check_clang_tidy_results.py clang-tidy.log
#
# 7. The optional cache directory 'clang-tidy-cache' is created and searched in
#    the build directory.
#
# 8. Documentation is available at:
#
#        https://www.kdab.com/clang-tidy-part-1-modernize-source-code-using-c11c14/
#
#        http://clang.llvm.org/extra/clang-tidy/
#
# ===------------------------------------------------------------------------===#
#


"""
Parallel clang-tidy runner
==========================

Runs clang-tidy over all files in a compilation database. Requires clang-tidy
and clang-apply-replacements in $PATH.

Example invocations.
- Run clang-tidy on all files in the current working directory with a default
  set of checks and show warnings in the cpp files and all project headers.
    run-clang-tidy.py $PWD

- Fix all header guards.
    run-clang-tidy.py -fix -checks=-*,llvm-header-guard

- Fix all header guards included from clang-tidy and header guards
  for clang-tidy headers.
    run-clang-tidy.py -fix -checks=-*,llvm-header-guard extra/clang-tidy \
                      -header-filter=extra/clang-tidy

Compilation database setup:
http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html
"""

from __future__ import print_function

import argparse
import glob
import hashlib
import json
import multiprocessing
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
import threading
import traceback

try:
    import yaml
except ImportError:
    yaml = None

is_py2 = sys.version[0] == '2'

if is_py2:
    import Queue as queue
else:
    import queue as queue


def find_compilation_database(path):
    """Adjusts the directory until a compilation database is found."""
    result = './'
    while not os.path.isfile(os.path.join(result, path)):
        if os.path.realpath(result) == '/':
            print('Error: could not find compilation database.')
            sys.exit(1)
        result += '../'
    return os.path.realpath(result)


def make_absolute(f, directory):
    if os.path.isabs(f):
        return f
    return os.path.normpath(os.path.join(directory, f))


cache = None

class Cache:
    """Implements the cache.

    Acknowledgements:

    The basic inspiration and some ideas for hashing the sources came from
    https://github.com/freedick/cltcache.

    Some more ideas for hashing and ideas for parsing compile_commands.json
    came from https://github.com/matus-chochlik/ctcache.

    No code is copied directly from either.
    """
    failed_files_regexp = re.compile(r'^([0-9a-f]{64})\.std(out|err)$')

    def __init__(self, dir, tidy_version, config, checks, custom):
        self.dir = os.path.join(dir, 'clang-tidy-cache')
        if not os.path.isdir(self.dir):
            os.mkdir(self.dir)

        self.config_hash = get_hash(tidy_version + config + checks)

        # We only put caches of custom runs in separate subdirs, so we can clean
        # up the main directory on upgrades or when the default config changes.
        # Custom subdirs have to be cleaned up manually on upgrades.
        if custom:
            self.dir = os.path.join(self.dir, '__custom_args', self.config_hash)
            if not os.path.isdir(self.dir):
                os.makedirs(self.dir)

        self.passed_file = os.path.join(self.dir, 'passed')
        self.failed_dir = os.path.join(self.dir, 'failed')
        if not os.path.isdir(self.failed_dir):
            os.mkdir(self.failed_dir)

        # We only store the hash of the files that passed the checks and the
        # time of its last access
        self.passed = dict()

        # For files failing the checks, we store 2 files: <hash>.std{out,err}
        # Their mtime is always set on access, so no need to keep track of the
        # last access time internally
        self.failed = set()

        if self.check_version(tidy_version, config, checks, custom):
            self.load_cache()

        # Statistics counters
        self.hits = 0
        self.misses = 0

    def check_version(self, tidy_version, config, checks, custom):
        self.version_file = os.path.join(self.dir, 'clang-tidy_version')
        self.config_dump_file = os.path.join(self.dir, 'clang-tidy_config')
        self.checks_file = os.path.join(self.dir, 'enabled_checks')
        self.config_hash_file = os.path.join(self.dir, 'config_hash')
        old_version = ''
        old_config = ''
        old_checks = ''
        if os.path.isfile(self.version_file):
            with open(self.version_file, 'r', encoding = 'utf-8') as vf:
                old_version = vf.read()
        if os.path.isfile(self.config_dump_file):
            with open(self.config_dump_file, 'r', encoding = 'utf-8') as cf:
                old_config = cf.read()
        if os.path.isfile(self.checks_file):
            with open(self.checks_file, 'r', encoding = 'utf-8') as cf:
                old_checks = cf.read()

        # For customized runs (when the run-time configuration is changed by
        # the command line arguments), the directory name already contains the
        # hash.
        old_config_hash = self.config_hash
        if not custom:
            if os.path.isfile(self.config_hash_file):
                with open(self.config_hash_file, 'r', encoding = 'utf-8') as hf:
                    old_config_hash = hf.read()
            else:
                old_config_hash = ''
        if tidy_version == old_version and config == old_config and \
           checks == old_checks and self.config_hash == old_config_hash:
            return True
        else:
            self.clear()
            with open(self.version_file, 'w', encoding = 'utf-8') as vf:
                 vf.write(tidy_version)
            with open(self.config_dump_file, 'w', encoding = 'utf-8') as cf:
                 cf.write(config)
            with open(self.checks_file, 'w', encoding = 'utf-8') as cf:
                 cf.write(checks)
            if not custom:
                 with open(self.config_hash_file, 'w', encoding = 'utf-8') as hf:
                     hf.write(self.config_hash)
            return False

    def load_cache(self):
        if os.path.isfile(self.passed_file):
            with open(self.passed_file, 'r', encoding = 'utf-8') as pf:
                self.passed = json.load(pf)
                assert type(self.passed) == dict, 'Wrong format of passed hashes cache'
        else:
            self.passed = dict()

        self.failed = {match.group(1)
                       for match in [self.failed_files_regexp.match(name)
                           for name in os.listdir(self.failed_dir)]
                       if match}

    def update(self):
        # 2 months (we don't expect the cache to grow big)
        retention = 62 * 24 * 3600

        old = int(time.time()) - retention
        self.passed = dict((hash, mtime) for hash, mtime in self.passed.items() if mtime > old)
        with open(self.passed_file, 'w', encoding = 'utf-8') as pf:
            json.dump(self.passed, pf, sort_keys = True, indent = 2)
            pf.write('\n')
        # failed entries are updated on the fly, we only need to delete old ones
        with os.scandir(self.failed_dir) as dir:
            for entry in dir:
                if entry.is_file() and self.failed_files_regexp.match(entry.name) \
                   and entry.stat().st_mtime < old:
                    os.remove(entry)

    def clear(self):
        if os.path.isfile(self.version_file):
            os.remove(self.version_file)
        if os.path.isfile(self.config_dump_file):
            os.remove(self.config_dump_file)
        if os.path.isfile(self.checks_file):
            os.remove(self.checks_file)
        if os.path.isfile(self.config_hash_file):
            os.remove(self.config_hash_file)
        if os.path.isfile(self.passed_file):
            os.remove(self.passed_file)
        with os.scandir(self.failed_dir) as dir:
            for entry in dir:
                if entry.is_file() and self.failed_files_regexp.match(entry.name):
                    os.remove(entry)
        self.passed = dict()
        self.failed = set()

    def lookup(self, hash):
        """Look up hash in the cache.

        Meaning of return values:

        found (bool), passed (bool), stdout (string), stderr (string)
        """
        if hash in self.passed:
            self.hits += 1
            self.passed[hash] = int(time.time())
            return (True, True, '', 'Cache hit as OK\n')
        if hash in self.failed:
            self.hits += 1
            out = None
            err = None
            out_fn = os.path.join(self.failed_dir, hash + '.stdout')
            err_fn = os.path.join(self.failed_dir, hash + '.stderr')
            try:
                if os.path.isfile(out_fn):
                    with open(out_fn, 'r', encoding = 'utf-8') as of:
                        out = of.read()
                    os.utime(out_fn)
                if os.path.isfile(err_fn):
                    with open(err_fn, 'r', encoding = 'utf-8') as ef:
                        err = ef.read()
                    os.utime(err_fn)
            except OSError:
                pass
            if out and err:
                return (True, False, out, 'Retrieved from the cache.\n' + err)
            # This shouldn't happen, but if it does, there's no harm in making
            # it a cache miss, so fall through.
        self.misses += 1
        return (False, False, None, None)

    def add_passed(self, hash):
        self.passed[hash] = int(time.time())

    def add_failed(self, hash, stdout, stderr):
        self.failed.add(hash)
        out_fn = os.path.join(self.failed_dir, hash + '.stdout')
        err_fn = os.path.join(self.failed_dir, hash + '.stderr')
        try:
            with open(out_fn, 'w', encoding = 'utf-8') as of:
                of.write(stdout)
            with open(err_fn, 'w', encoding = 'utf-8') as ef:
                ef.write(stderr)
            return True
        except OSError as error:
            print(error, file=sys.stderr)
            print(f'WARNING: Could not store outputs in cache for {hash}\n',
                  file=sys.stderr)
            return False


def get_hash(text_to_hash):
    return hashlib.sha256(text_to_hash.encode('utf-8')).hexdigest()


extra_before = []
extra_after = []

def pp_hash(file, command, dir):
    """Run the preprocessor for a source file and return the hash of the
    result.

    We use this hash to identify files in the cache. We don't do anything else
    with the preprocessed results, clang-tidy does its own thing independently.

    The three arguments are the entries for the source file 'file' in
    compile_commands.json that is generated by

    'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON'

    'command' is the compiler command line for compiling it, and 'directory'
    is the base directory for relative paths in either 'file' or 'command'.
    """
    preprocessor_args = []
    pp_args_loaded = shlex.split(command, posix = True)
    skip = False
    for arg in pp_args_loaded[0:1] + extra_before + pp_args_loaded[1:] + extra_after:
        if skip:
            skip = False
            continue
        if arg == '-o':
            # Compiler output file: we clear it and read output from stdout
            # instead
            skip = True
            continue
        if arg == '-c':
            # Replace compilation with preprocessing only
            preprocessor_args.append('-E')
            # Preserve comments: We need this unfortunately, because NOLINT
            # comments would be removed too.
            arg = '-C'
        preprocessor_args.append(arg)

    proc = subprocess.Popen(
       preprocessor_args, cwd=dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = proc.communicate()
    if proc.returncode != 0 or len(err) > 0:
        print('Preprocessor exited with error:\n' +
              ' '.join(preprocessor_args) + '\n' +
              err.decode('utf-8'), file=sys.stderr)
        sys.stderr.flush()
        raise Exception('Preprocessor error')

    # Mix clang-tidy configuration into the hash to prevent mixing outputs
    # from runs that may produce different outputs
    hash = hashlib.sha256(cache.config_hash.encode('utf-8'))

    ### Would be nice (not very important), but we need the NOLINT comments...
    # # Try to make it more resistant to comment and formatting changes
    # # by removing empty lines and included file and line number markers,
    # # and joining lines.
    # for line in output.decode('utf-8').splitlines():
    #     l = line.strip()
    #     # We don't need the line numbers here
    #     if l == '' or l.startswith('#'):
    #         continue
    #     if l.endswith('('):
    #         l += ' '
    #     hash.update(l.encode('utf-8'))
    #
    # return hash.hexdigest()
    ###

    hash.update(output)
    return hash.hexdigest()


tidy_common_args = []

def set_tidy_common_args(clang_tidy_binary, checks, build_path, header_filter,
                         extra_arg, extra_arg_before, quiet, config):
    """Gets the global arguments for clang-tidy."""
    global tidy_common_args
    tidy_common_args = [clang_tidy_binary]
    if header_filter is not None:
        tidy_common_args.append('-header-filter=' + header_filter)
    if checks:
        tidy_common_args.append('-checks=' + checks)
    tidy_common_args.append('-p=' + build_path)
    if quiet:
        tidy_common_args.append('-quiet')
    if config:
        tidy_common_args.append('-config=' + config)
    for arg in extra_arg:
        tidy_common_args.append('-extra-arg=%s' % arg)
        extra_after.append(arg)
    for arg in extra_arg_before:
        tidy_common_args.append('-extra-arg-before=%s' % arg)
        extra_before.append(arg)
    return tidy_common_args

def get_tidy_file_args(file, tmpdir):
    """Gets per file arguments for clang-tidy."""
    f_args = []
    if tmpdir is not None:
        f_args.append('-export-fixes')
        # Get a temporary file. We immediately close the handle so clang-tidy can
        # overwrite it.
        (handle, name) = tempfile.mkstemp(suffix='.yaml', dir=tmpdir)
        os.close(handle)
        f_args.append(name)
    f_args.append(file)
    return f_args


def merge_replacement_files(tmpdir, mergefile):
    """Merge all replacement files in a directory into a single file."""
    # The fixes suggested by clang-tidy >= 4.0.0 are given under
    # the top level key 'Diagnostics' in the output yaml files
    mergekey = 'Diagnostics'
    merged = []
    for replacefile in glob.iglob(os.path.join(tmpdir, '*.yaml')):
        content = yaml.safe_load(open(replacefile, 'r'))
        if not content:
            continue  # Skip empty files.
        merged.extend(content.get(mergekey, []))

    if merged:
        # MainSourceFile: The key is required by the definition inside
        # include/clang/Tooling/ReplacementsYaml.h, but the value
        # is actually never used inside clang-apply-replacements,
        # so we set it to '' here.
        output = {'MainSourceFile': '', mergekey: merged}
        with open(mergefile, 'w') as out:
            yaml.safe_dump(output, out)
    else:
        # Empty the file:
        open(mergefile, 'w').close()


def check_clang_apply_replacements_binary(args):
    """Checks if invoking supplied clang-apply-replacements binary works."""
    try:
        subprocess.check_call(
            [args.clang_apply_replacements_binary, '--version'])
    except:
        print('Unable to run clang-apply-replacements. Is clang-apply-replacements '
              'binary correctly specified?', file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)


def apply_fixes(args, tmpdir):
    """Calls clang-apply-fixes on a given directory."""
    invocation = [args.clang_apply_replacements_binary]
    if args.format:
        invocation.append('-format')
    if args.style:
        invocation.append('-style=' + args.style)
    invocation.append(tmpdir)
    subprocess.call(invocation)


# List of files with a non-zero return code.
failed_files = []

def run_tidy(tmpdir, build_path, quiet, queue, lock):
    """Takes filenames out of queue and runs clang-tidy on them."""
    global failed_files
    while True:
        name, command, dir = queue.get()
        hash = None
        hit = False
        passed = True
        output = ''
        err = ''
        tidy_error = False

        invocation = tidy_common_args + get_tidy_file_args(name, tmpdir)

        if cache:
            try:
                hash = pp_hash(name, command, dir)
            except:
                print('Preprocessing failed for {}, skipping cache lookup.'.format(name),
                      file=sys.stderr)
                hash = None
            if hash:
                with lock:
                    try:
                        hit, passed, output, err = cache.lookup(hash)
                    except:
                        print('Cache lookup error for {} with hash {}.'.format(name, hash),
                              file=sys.stderr)
                        traceback.print_exc()
                        hit = False

        if not hit:
            proc = subprocess.Popen(
                invocation, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output_r, err_r = proc.communicate()
            output = output_r.decode('utf-8')
            err = err_r.decode('utf-8')
            tidy_error = proc.returncode != 0
            passed = len(output) == 0

        with lock:
            if tidy_error:
                failed_files.append(name)
            sys.stderr.write('\n' + ' '.join(invocation) + '\n')
            sys.stderr.flush()
            if len(output) > 0:
                sys.stdout.write(output + '\n')
                sys.stdout.flush()
            if (tidy_error or not quiet) and len(err) > 0:
                sys.stderr.write(err)
                sys.stderr.flush()

            if cache and not hit and hash and not tidy_error:
                if passed:
                    cache.add_passed(hash)
                else:
                    try:
                        cache.add_failed(hash, output, err)
                    except:
                        print('Error saving output to the cache.',
                              file=sys.stderr)
                        traceback.print_exc()
                        sys.stderr.flush()

        queue.task_done()

def query_tidy(query, quiet):
    invocation = tidy_common_args + [query]
    if not quiet:
        print(invocation)
    proc = subprocess.Popen(invocation, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output_r, err_r = proc.communicate()
    output = output_r.decode('utf-8')
    if not quiet and len(output) > 0:
        sys.stdout.write(output)
        sys.stdout.flush()
    if len(err_r) > 0:
        sys.stderr.write(err_r.decode('utf-8'))
        sys.stderr.flush()
    if proc.returncode != 0:
        raise Exception('Unable to run clang-tidy.')
    return output

def filter_config(config):
    filtered_config = ''
    for line in config.splitlines(keepends = True):
        if not line.startswith(('Checks:', 'User:', 'FormatStyle:')):
            filtered_config += line
    return filtered_config

def filter_version(tidy_version):
    for line in tidy_version.splitlines():
        if 'version' in line:
            return line.strip()
    return None


default_tidy_binary = 'clang-tidy'

def main():
    parser = argparse.ArgumentParser(description='Runs clang-tidy over all files '
                                     'in a compilation database. Requires '
                                     'clang-tidy and clang-apply-replacements in '
                                     '$PATH.')
    parser.add_argument('-clang-tidy-binary', metavar='PATH',
                        default=default_tidy_binary,
                        help='path to clang-tidy binary')
    parser.add_argument('-clang-apply-replacements-binary', metavar='PATH',
                        default='clang-apply-replacements',
                        help='path to clang-apply-replacements binary')
    parser.add_argument('-checks', default=None,
                        help='checks filter, when not specified, use clang-tidy '
                        'default')
    parser.add_argument('-config', default=None,
                        help='Specifies a configuration in YAML/JSON format: '
                        '  -config="{Checks: \'*\', '
                        '                       CheckOptions: [{key: x, '
                        '                                       value: y}]}" '
                        'When the value is empty, clang-tidy will '
                        'attempt to find a file named .clang-tidy for '
                        'each source file in its parent directories.')
    parser.add_argument('-header-filter', default=None,
                        help='regular expression matching the names of the '
                        'headers to output diagnostics from. Diagnostics from '
                        'the main file of each translation unit are always '
                        'displayed.')
    if yaml:
        parser.add_argument('-export-fixes', metavar='filename', dest='export_fixes',
                            help='Create a yaml file to store suggested fixes in, '
                            'which can be applied with clang-apply-replacements.')
    parser.add_argument('-j', type=int, default=0,
                        help='number of tidy instances to be run in parallel.')
    parser.add_argument('files', nargs='*', default=['.*'],
                        help='files to be processed (regex on path)')
    parser.add_argument('-fix', action='store_true', help='apply fix-its')
    parser.add_argument('-format', action='store_true', help='Reformat code '
                        'after applying fixes')
    parser.add_argument('-style', default='file', help='The style of reformat '
                        'code after applying fixes')
    parser.add_argument('-p', dest='build_path',
                        help='Path used to read a compile command database.')
    parser.add_argument('-extra-arg', dest='extra_arg',
                        action='append', default=[],
                        help='Additional argument to append to the compiler '
                        'command line.')
    parser.add_argument('-extra-arg-before', dest='extra_arg_before',
                        action='append', default=[],
                        help='Additional argument to prepend to the compiler '
                        'command line.')
    parser.add_argument('-quiet', action='store_true',
                        help='Run clang-tidy in quiet mode')
    parser.add_argument('-cache', action='store_true',
                        help='Use a cache of clang-tidy outputs to speed up '
                        'repeated checks. Cache is always disabled when -fix '
                        'is used.')
    args = parser.parse_args()

    db_path = 'compile_commands.json'

    if args.build_path is not None:
        build_path = args.build_path
    else:
        # Find our database
        build_path = find_compilation_database(db_path)

    tmpdir = None
    if args.fix or (yaml and args.export_fixes):
        check_clang_apply_replacements_binary(args)
        tmpdir = tempfile.mkdtemp()
        if args.cache:
            print('Caching is disabled because of -fix\n', file=sys.stderr)
            args.cache = False

    custom = False
    default_version = ''
    default_config = ''
    default_checks = ''

    if args.cache:
        # -quiet only changes stderr, not the actual checks, so we
        # suppress stderr completely with quiet as long as return code
        # is 0, so we can use the same cache if the only difference
        # is -quiet.
        custom = args.checks or args.config or args.header_filter or \
                 args.clang_tidy_binary != default_tidy_binary

        # Maybe the command-line arguments don't actually change the config
        # and the enabled checks. To find out, we need to get them for the
        # default setup.
        if custom:
            set_tidy_common_args(default_tidy_binary, None, build_path,
                None, [], [], False, None)
            try:
                default_version = filter_version(query_tidy('-version', True))
                assert default_version
                default_config = filter_config(query_tidy('-dump-config', True))
                default_checks = query_tidy('-list-checks', True)
            except:
                # Ignore errors. Maybe the args are there to make it work.
                pass

    set_tidy_common_args(args.clang_tidy_binary, args.checks, build_path,
        args.header_filter, args.extra_arg, args.extra_arg_before, args.quiet,
        args.config)

    global cache

    try:
        version = query_tidy('-version', args.quiet)
        checks = query_tidy('-list-checks', args.quiet)

        if args.cache:
            try:
                version_f = filter_version(version)
                assert version_f, 'Failed to detect clang-tidy version.'
                config = filter_config(query_tidy('-dump-config', True))
                custom = custom and (version_f != default_version or
                         config != default_config or checks != default_checks)
                cache = Cache(build_path, version_f, config, checks, custom)
            except Exception as error:
                print(error, file=sys.stderr)
                traceback.print_exc()
                print('Cannot create cache directory, caching is disabled.\n', file=sys.stderr)
                args.cache = False
    except Exception as e:
        print(e)
        print('Unable to run clang-tidy.', file=sys.stderr)
        sys.exit(1)

    max_task = args.j
    if max_task == 0:
        max_task = multiprocessing.cpu_count()

    # Build up a big regexy filter from all command line arguments.
    file_name_re = re.compile('|'.join(args.files))

    # Load the database
    files = []
    for entry in json.load(open(os.path.join(build_path, db_path))):
        name = make_absolute(entry['file'], entry['directory'])
        if file_name_re.search(name) and not 'src/third_party' in name:
            files.append((name, entry['command'], entry['directory']))

    return_code = 0
    lock = threading.Lock()
    task_queue = queue.Queue(max_task)

    try:
        # Spin up a bunch of tidy-launching threads.
        for _ in range(max_task):
            t = threading.Thread(target=run_tidy,
                                 args=(tmpdir, build_path, args.quiet,
                                       task_queue, lock))
            t.daemon = True
            t.start()

        # Fill the queue
        for entry in files:
            task_queue.put(entry)

        # Wait for all threads to be done.
        task_queue.join()
        if len(failed_files):
            print('\nclang-tidy errors: {}\n'.format(len(failed_files)),
                  file=sys.stderr)
            return_code = 1

    except KeyboardInterrupt:
        # This is a sad hack. Unfortunately subprocess goes
        # bonkers with ctrl-c and we start forking merrily.
        print('\nCtrl-C detected, goodbye.')
        if tmpdir:
            shutil.rmtree(tmpdir)
        os.kill(0, 9)

    if yaml and args.export_fixes:
        print('Writing fixes to ' + args.export_fixes + ' ...')
        try:
            merge_replacement_files(tmpdir, args.export_fixes)
        except:
            print('Error exporting fixes.\n', file=sys.stderr)
            traceback.print_exc()
            return_code = 1

    if args.fix:
        print('Applying fixes ...')
        try:
            apply_fixes(args, tmpdir)
        except:
            print('Error applying fixes.\n', file=sys.stderr)
            traceback.print_exc()
            return_code = 1

    if args.cache:
        cache.update()
        if not args.quiet:
            print('\nCache statistics:\n'
                  f'   Total files:  {len(files):5}\n'
                  f'   Cache hits:   {cache.hits:5}\n'
                  f'   Cache misses: {cache.misses:5}',
                  file=sys.stderr)
            other = len(files) - cache.hits - cache.misses
            if other != 0:
                print(f'   Lookup failed: {other:4}', file=sys.stderr)
            print('', file=sys.stderr)

    if tmpdir:
        shutil.rmtree(tmpdir)
    sys.exit(return_code)


if __name__ == '__main__':

    if sys.version_info[0] < 3:
        sys.exit('At least python version 3 is needed.')

    main()
