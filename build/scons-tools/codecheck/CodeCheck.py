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

import os
from glob import glob
import re
from collections import defaultdict
from time import time

class CheckingRule(object):
    """
    Represents one test to check the sourcecode for
    """
    def __init__(self, name, vars ):
        self.name = name
        self._error_msg = vars["error_msg"]
        self._is_multiline = vars.get('is_multiline',False)
        if self._is_multiline:
            self._evaluate_matches = vars['evaluate_matches']
        else:
            self._regexp = re.compile(vars["regexp"])
       
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

    def check_text(self, data):
        if not self._is_multiline:
            raise RuntimeError("I am not a Multiline rule. Call check_line!")
        
        # Delegate work to Rule
        matches = self._evaluate_matches(data)
        
        return matches

    def check_line(self,line):
        if self._is_multiline:
            raise RuntimeError("I am a Multiline rule. Call check_text!")
        
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
        for e in errors:
            fn,l,msg = e
            if self._color:
                fn = '%s%s%s' % (ansicolor["green"], fn, ansicolor["reset"])
                l = '%s%s%s' % (ansicolor["cyan"], l, ansicolor["reset"])
                msg = '%s%s%s%s' % (ansicolor["yellow"], ansicolor["bold"], msg, ansicolor["reset"])

            print "%s:%s: %s" % (fn,l,msg)
    
    def check_file(self,fn, print_errors = True):
        errors = []
       
        bm = defaultdict(lambda: 0.)
         
        # Check line by line (currently)
        data = open(fn).read()
        for lidx,line in enumerate(data.splitlines(False)):
            for c in self._checkers:
                if self._benchmark:
                    start = time()
                    if c.check_line(line):
                        errors.append( (fn,lidx+1,c.error_msg) )
                    bm[c.name] += time()-start
                else:
                    if c.check_line(line):
                        errors.append( (fn,lidx+1,c.error_msg) )

        for c in self._mlcheckers:
            if self._benchmark:
                start = time()
                e =  c.check_text( data )
                errors.extend( [ (fn,lidx,em) for lidx,em in e ] )
                bm[c.name] += time()-start
            else:
                e =  c.check_text( data )
                errors.extend( [ (fn,lidx,em) for lidx,em in e ] )
       
        errors.sort(key=lambda a: a[1])

        if len(errors) and print_errors:
            self._print_errors(errors)
       
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

