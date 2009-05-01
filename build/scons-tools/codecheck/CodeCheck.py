#!/usr/bin/env python
# encoding: utf-8
#

"""
Yet another code checker for the widelands project. This one's intention is to get
rid of the ADA whitespace checker (while keeping it's functionality or improving on it)
and supersede the old detect_spurious_indentation.py script (while also keeping it's cases
around). En plus, we also replace the spurious code checking done with grep
which is currently also around. 
"""

from collections import defaultdict
from glob import glob
from time import time
import os
import re

class TokenStripper(object):
    """
    This class knows how to remove certain
    strings from given lines
    """
    _literal_strings = re.compile(r'''(?x)
".*?
(
    (\\\\")|
    ([^\\]")   |       # End of string
$)
    ''')

    def __init__(self):
        self.reset()

    def reset(self):
        self._in_comment = False
        self._comm_cache = {}
        self._str_cache = {}

    def strip_comments(self,given_line):
        cached_line = self._comm_cache.get(given_line,None)
        if cached_line:
            return cached_line
        line = given_line

        # Multiline comments
        start_idx = given_line.find('/*')
        if not self._in_comment:
            if start_idx != -1:
                stop_idx = line.find("*/") 
                if stop_idx != -1:
                    line = line[stop_idx+2:]
                else:
                    self._in_comment = True
                
                line = line[:start_idx].strip()

        if self._in_comment:
            stop_idx = line.find('*/')
            if stop_idx == -1:
                line = "" 
            else:
                line = line[stop_idx+2:].strip()
                self._in_comment = False

        # Single line comments
        idx = line.find('//')
        if idx != -1:
            line = line[:idx].strip()
       
        self._comm_cache[given_line] = line

        return line
        
    def strip_strings(self,line):
        r"""
        Strips all cstring literals from line. String contents
        are replace by spaces. 

        >>> _do_strip_strings(r'"Hallo"')
        '"     "'
        >>> _do_strip_strings(r'blah "string with \"escaped\" text" more blah')
        'blah "                            " more blah'
        >>> _do_strip_strings(r'blah "string with \\"upsy" \\"upsyagain"\\text" more blah')
        'blah "              "upsy"   "upsyagain"      " more blah'
        """
        cached_line = self._str_cache.get(line,None)
        if cached_line:
            return cached_line

        # Strings are replaced with blanks 
        newline = self._literal_strings.sub(lambda k: '"%s"' % ((len(k.group(0))-2)*" "),line)
         
        self._str_cache[line] = newline

        return newline

class CheckingRule(object):
    """
    Represents one test to check the sourcecode for
    """
    def __init__(self, name, vars ):
        self.name = name
        self._is_multiline = vars.get('is_multiline',False)
        if self._is_multiline:
            self._evaluate_matches = vars['evaluate_matches']
            self._error_msg = None
            self._strip_comments = False
            self._strip_strings = False
        else:
            self._regexp = re.compile(vars["regexp"])
            self._error_msg = vars["error_msg"]
            self._strip_comments = vars.get('strip_comments',True)
            self._strip_strings = vars.get('strip_strings',True)
       
        def _to_tuple(a):
            if isinstance(a,str):
                return (a,)
            else:
                return tuple(a)

        self.allowed = _to_tuple(vars["allowed"])
        self.forbidden  = _to_tuple(vars["forbidden"])
    
    @property
    def multiline(self):
        return self._is_multiline

    @property
    def error_msg(self):
        return self._error_msg

    def check_text(self, token_stripper, data):
        if not self._is_multiline:
            raise RuntimeError("I am not a Multiline rule. Call check_line!")
        
        token_stripper = TokenStripper()

        # Delegate work to Rule
        matches = self._evaluate_matches(token_stripper, data)
        
        return matches

    def check_line(self,token_stripper,line):
        if self._is_multiline:
            raise RuntimeError("I am a Multiline rule. Call check_text!")
        
        if self._strip_comments:
            line = token_stripper.strip_comments(line)
        if self._strip_strings:
            line = token_stripper.strip_strings(line)

        m = self._regexp.search( line )
        if m:
            return True
        return False

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
    mlcheckers = []

    for filename in rule_files:
        variables = {}
        execfile(filename,variables)
        
        rule = CheckingRule(os.path.basename(filename), variables)
        if rule.multiline:
            mlcheckers.append(rule)
        else:
            checkers.append( rule )
    
    return checkers, mlcheckers

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
    _checkers,_mlcheckers = _parse_rules()
    
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

        print output.rstrip()
        
        return output
    
    def check_file(self,fn, print_errors = True):
        if print_errors and fn in self._cache:
            print self._cache[fn].rstrip()
            return 
        errors = []
       
        bm = defaultdict(lambda: 0.)
            
        token_stripper = TokenStripper()

        # Check line by line (currently)
        data = open(fn).read()
        for lidx,line in enumerate(data.splitlines(False)):
            for c in self._checkers:
                if self._benchmark:
                    start = time()
                    if c.check_line(token_stripper,line):
                        errors.append( (fn,lidx+1,c.error_msg) )
                    bm[c.name] += time()-start
                else:
                    if c.check_line(token_stripper,line):
                        errors.append( (fn,lidx+1,c.error_msg) )

        for c in self._mlcheckers:
            if self._benchmark:
                start = time()
                e =  c.check_text( token_stripper, data )
                errors.extend( [ (fn,lidx,em) for lidx,em in e ] )
                bm[c.name] += time()-start
            else:
                e =  c.check_text( token_stripper, data )
                errors.extend( [ (fn,lidx,em) for lidx,em in e ] )
       
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
        print "Usage: %s <options> <files>" % os.path.basename(sys.argv[0])
        print """
 -h,   --help            Print help and exit
 -b,   --benchmark       Benchmark each rule
 -c,   --color           Print warnings in color
"""

    def main():
        opts, files = getopt.getopt(sys.argv[1:], "hbc", ["help", "benchmark","color", "colour"])
       
        benchmark = False
        color = False
        for o,a in opts:
            if o in ('-h','--help'):
                usage()
                sys.exit(0)
            if o in ('-b','--benchmark'):
                benchmark = True
            if o in ('-c','--colour','--color'):
                color = True
        
        if not len(files):
            usage()
            sys.exit(0)

        d = CodeChecker( benchmark = benchmark, color = color )
        for filename in files:
            print "Checking %s ..." % filename
            errors = d.check_file(filename)
        
        # Print benchmark results
        if benchmark:
            print
            print "Benchmark results:"
            
            res = d.benchmark_results
            ctime = sum( l[0] for l in res )

            for time,n in res:
                per = time/ctime * 100.
                percentage = ("%.2f%%"% per).rjust(8)
                time = ("%4.2fms"% (time*1000.)).rjust(8)
                print "%s %s    %s" % (percentage,time,n)

    # import cProfile
    # cProfile.runctx("main()",globals(),locals(),"Profile.prof")

    main()

