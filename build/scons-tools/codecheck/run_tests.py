#!/usr/bin/env python
# encoding: utf-8
#

import unittest
import new

from CodeCheck import CodeChecker, Preprocessor

class RuleTest(unittest.TestCase):
    "Base class for RuleTesting"
    def __init__( self, checker, val ):
        unittest.TestCase.__init__(self)
        self.preprocessor = Preprocessor()
        self.c = checker
        self.tv = val
        
    def runTest(self):
        self.rv = self.c.check_text(self.preprocessor, "test.cc", self.tv)
        self._do_test()

class AllowedTest(RuleTest):
    """
    Tests allowed code. The rule checker shouldn't find anything
    in this test
    """
    def _do_test(self):
        self.assertTrue( len(self.rv)==0,
            "Rule '%s' failed. Example '%s' should be ok, but wasn't" % (self.c.name,self.tv) )
class ForbiddenTest(RuleTest):
    """
    Tests forbidden constructs. The rule checker should find errors
    in the example code.
    """
    def _do_test(self):
        self.assertTrue( len(self.rv)!=0,
            "Rule '%s' failed. Example '%s' should fail, but passed" % (self.c.name,self.tv) )
        
def _make_tests_from_checker( c ):
    """
    Takes a checker class and turns it's allowed and forbidden variables into
    TestCases.
    """
    allowed_tests = [ AllowedTest(c,allowed) for allowed in c.allowed ]
    forbidden_tests = [ ForbiddenTest(c,forbidden) for forbidden in c.forbidden ]

    return allowed_tests + forbidden_tests

if __name__ == '__main__':
    import sys
    
    d = CodeChecker()
    
    suite = unittest.TestSuite()
    
    if len(sys.argv) > 1:
        for checker in (d._checkers):
            if checker.name in sys.argv:
                suite.addTests( _make_tests_from_checker(checker) )
    else:
        for checker in d._checkers:
            suite.addTests( _make_tests_from_checker(checker) )

    unittest.TextTestRunner(verbosity=1).run(suite)
