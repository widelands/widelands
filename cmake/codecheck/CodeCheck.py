#!/usr/bin/env python
# encoding: utf-8
#

"""
Yet another code checker for the widelands project. This one's
intention is to get rid of the ADA whitespace checker (while keeping
it's functionality or improving on it) and supersede the old
detect_spurious_indentation.py script (while also keeping it's cases
around). En plus, we also replace the spurious code checking done
with grep which is currently also around.
"""

from collections import defaultdict
from glob import glob
from time import time
import os
import re

def strip_multi_line_c_comments(input_lines):
    """
    Remove multi-line C comments from a one-line-per-entry list of strings,
    but preserve new lines so that line-numbering is not affected.
    """
    new_lines = "".join(input_lines)

    def fixup(match):
        "Remove everything except newlines"
        orig_string = match.string[match.start():match.end()]
        return "\n" * orig_string.count("\n")

    # Strip multi-line c-style comments
    new_lines = re.sub(r"/\*.*?\*/", fixup, new_lines, flags=re.DOTALL | re.M)

    return new_lines.splitlines(True)

class Preprocessor(object):
    """
    This class knows how to remove certain
    strings from given lines
    """
    _literal_strings = re.compile(r'''(?x)
".*?
(
    (\\\\")|
    ((?<!\\)")   |       # End of string
$)
    ''')
    _literal_chars = re.compile(r"'[\\]?.'")

    def __init__(self):
        self._plain_cache = {}
        self._stripped_comments_and_strings = {}
        self._stripped_all = {}

    def _get_plain(self,fn,data):
        if fn in self._plain_cache:
            return self._plain_cache[fn]

        lines = data.splitlines(True)
        self._plain_cache[fn] = lines

        return lines

    def _get_stripped_comments_and_strings(self,fn,lines):
        """
        Strips all cstring literals from the text. String contents
        are replace by spaces.
        Removes comments from lines. Comments are completely
        stripped, including // and /**/ symbols
        """
        if fn in self._stripped_comments_and_strings:
            return self._stripped_comments_and_strings[fn]

        in_comment = False
        new_lines = []
        for line in lines:
            # Strings are replaced with blanks
            line = self._literal_chars.sub(lambda k: "'%s'" % ((len(k.group(0))-2)*" "),line)
            line = self._literal_strings.sub(lambda k: '"%s"' % ((len(k.group(0))-2)*" "),line)

            # Remove whitespace followed by single-line comment (old behaviour)
            line = re.sub(r"\s*//.*$", "", line)

            new_lines.append( line )

        new_lines = strip_multi_line_c_comments(new_lines)

        self._stripped_comments_and_strings[fn] = new_lines

        return new_lines

    def _get_stripped_macros(self,fn,lines):
        """
        Remove macros definitions. Also multiline macros. They are replaced with
        empty lines
        """
        if fn in self._stripped_all:
            return self._stripped_all[fn]

        new_lines = []
        in_macro = False
        for given_line in lines:
            line = given_line
            if in_macro or (len(given_line) and given_line[0] == '#'):
                in_macro = True
                line = ""

            if len(given_line) > 1 and given_line[-2] != '\\':
                in_macro = False

            new_lines.append(line)

        self._stripped_all[fn] = new_lines

        return new_lines


    def get_preprocessed_data(self,fn, data, strip_strings_and_comments, strip_macros):
        """
        Return an array of lines where data has been stripped off
        """
        if not strip_macros and not strip_strings_and_comments:
            return self._get_plain(fn,data)
        elif not strip_macros and strip_strings_and_comments:
            return self._get_stripped_comments_and_strings(fn,self._get_plain(fn,data))
        elif strip_strings_and_comments and strip_macros:
            return self._get_stripped_macros(fn,self._get_stripped_comments_and_strings(fn,self._get_plain(fn,data)))

        # Error checking, we should never be here
        raise RuntimeError("strip_macros can't be true when strip_strings_and_comments isn't!")


class CheckingRule(object):
    """
    Represents one test to check the sourcecode for
    """
    def __init__(self, name, vars ):
        self.name = name
        self._strip_comments_and_strings = vars.get('strip_comments_and_strings',False)
        self._strip_macros = vars.get('strip_macros',False)

        self._evaluate_matches = vars.get('evaluate_matches',None)

        if self._evaluate_matches == None:
            self._regexp = re.compile(vars["regexp"])
            self._error_msg = vars["error_msg"]

        def _to_tuple(a):
            if isinstance(a,str):
                return (a,)
            else:
                return tuple(a)

        self.allowed = _to_tuple(vars["allowed"])
        self.forbidden  = _to_tuple(vars["forbidden"])

    def check_text(self, preprocessor, fn, data):
        """
        Data must be a complete file data as returned by .read()

        preprocessor - Tool to preprocess the text (strip it from unwanted tokens)
        fn           - File name of current file to check
        data         - File contents
        """
        lines = preprocessor.get_preprocessed_data(fn,data,self._strip_comments_and_strings, self._strip_macros)

        matches = []
        if self._evaluate_matches is not None:
            # Rule has it's own checking function
            matches = self._evaluate_matches(lines,fn)
        else:
            # Regular expression rule
            for lidx,line in enumerate(lines):
                if self._regexp.search(line):
                    matches.append( (fn,lidx+1,self._error_msg) )

        return matches

###################
# Helper function #
###################
def _find_rule_files():
    """
    Searches the rules/ directory for rule files and returns
    an array with them.
    """
    p = os.path.dirname(__file__)
    if not len(p):
        p = "./"
    rule_files = [ f for f in glob(p + '/rules/*') if os.path.isfile(f) ]

    return rule_files

def _parse_rules():
    rule_files = _find_rule_files()

    checkers = []

    for filename in rule_files:
        variables = {}
        fh = open(filename, "r")
        exec(fh.read()+"\n", variables)
        fh.close()

        rule = CheckingRule(os.path.basename(filename), variables)
        checkers.append( rule )

    return checkers

ansicolor = {
    "default" : '\033[0m',

    "black" : '\033[30m',
    "red" : '\033[31m',
    "green" : '\033[32m',
    "yellow" : '\033[33m',
    "blue" : '\033[34m',
    "magenta" : '\033[35m',
    "purple" : '\033[35m',
    "cyan" : '\033[36m',
    "white" : '\033[37m',

    "reset" : '\033[0;0m',
    "bold" : '\033[1m',

    "blackbg" : '\033[40m',
    "redbg" : '\033[41m',
    "greenbg" : '\033[42m',
    "yellowbg" : '\033[43m',
    "bluebg" : '\033[44m',
    "magentabg" : '\033[45m',
    "cyanbg" : '\033[46m',
    "whitebg" : '\033[47m',
}

class CodeChecker(object):
    _checkers = _parse_rules()

    def __init__(self, benchmark = False, color = False):
        """
        benchmark - Run benchmarks on each rule. Print milliseconds after run
        """
        self._benchmark = benchmark
        self._color = color

        # We keep a cache of file names/error strings
        # so that (e.g.) a header is requested twice in one
        # run of the program, we just return the cached errors.
        # They will not have changed while the program was running
        self._cache = {}

    @property
    def benchmark_results(self):
        return self._bench_results

    def use_color():
        def fget(self):
            return self._color
        def fset(self, value):
            self._color = value
        return locals()
    use_color = property(**use_color())

    def _print_errors(self,errors):
        output = ""
        for e in errors:
            fn,l,msg = e
            if self._color:
                fn = '%s%s%s' % (ansicolor["green"], fn, ansicolor["reset"])
                l = '%s%s%s' % (ansicolor["cyan"], l, ansicolor["reset"])
                msg = '%s%s%s%s' % (ansicolor["yellow"], ansicolor["bold"], msg, ansicolor["reset"])

            output += "%s:%s: %s\n" % (fn,l,msg)

        print(output.rstrip())

        return output

    def check_file(self,fn, print_errors = True):
        if print_errors and fn in self._cache:
            print(self._cache[fn].rstrip())
            return
        errors = []

        bm = defaultdict(lambda: 0.)

        preprocessor = Preprocessor()

        # Check line by line (currently)
        data = open(fn).read()
        for c in self._checkers:
            if self._benchmark:
                start = time()
                e =  c.check_text( preprocessor, fn, data )
                errors.extend( e )
                bm[c.name] += time()-start
            else:
                e =  c.check_text( preprocessor, fn, data )
                errors.extend( e )

        errors.sort(key=lambda a: a[1])

        if len(errors) and print_errors:
            self._cache[fn] = self._print_errors(errors)

        if self._benchmark:
            self._bench_results = [ (v,k) for k,v in bm.items() ]
            self._bench_results.sort(reverse=True)

        return errors


if __name__ == '__main__':
    import sys
    import os
    import getopt

    def usage():
        print("Usage: %s <options> <files>" % os.path.basename(sys.argv[0]))
        print("""
 -h,   --help            Print help and exit
 -c,   --color           Print warnings in color
 -b,   --benchmark       Benchmark each rule
 -p,   --profile         Run with cProfile. Creates "Profile.prof" file
""")

    def check_files(files,color,benchmark):
        d = CodeChecker( benchmark = benchmark, color = color )
        for filename in files:
            errors = d.check_file(filename)

        # Print benchmark results
        if benchmark:
            print("\nBenchmark results:")

            res = d.benchmark_results
            ctime = sum( l[0] for l in res )

            for time,n in res:
                per = time/ctime * 100.
                percentage = ("%.2f%%"% per).rjust(8)
                time = ("%4.2fms"% (time*1000.)).rjust(8)
                print("%s %s    %s" % (percentage,time,n))

    def main():
        opts, given_paths = getopt.getopt(sys.argv[1:], "hbcp", ["help", "benchmark","color", "colour", "profile"])

        benchmark = False
        color = False
        profile = False
        for o,a in opts:
            if o in ('-h','--help'):
                usage()
                sys.exit(0)
            if o in ('-b','--benchmark'):
                benchmark = True
            if o in ('-c','--colour','--color'):
                term = os.environ.get("TERM", "dumb")
                if term != "dumb":
                    color = True
            if o in ('-p','--profile'):
                profile = True

        if not len(given_paths):
            usage()
            sys.exit(0)

        given_paths = set(given_paths)
        files = set()
        for f in given_paths:
            if os.path.isdir(f):
                for (dirpath, dirnames, filenames) in os.walk(f):
                    for fn in filenames:
                        files.add(os.path.abspath(os.path.join(dirpath, fn)))
                continue
            files.add(os.path.abspath(f))

        source_files = []
        for f in files:
            extension = os.path.splitext(f)[-1].lower()
            if extension in ('.cc', '.h'):
                source_files.append(f)

        if profile:
            import cProfile
            cProfile.runctx("check_files(source_files,color,benchmark)",globals(),locals(),"Profile.prof")
        else:
            check_files(source_files,color,benchmark)

    main()

