#!/usr/bin/env python
# encoding: utf-8
#

import unittest
import new

from CodeCheck import CodeChecker, TokenStripper

class RuleTest(unittest.TestCase):
    "Base class for RuleTesting"
    def __init__( self, checker, val ):
        unittest.TestCase.__init__(self)
        self.ts = TokenStripper()
        self.c = checker
        self.tv = val
        
    def runTest(self):
        if self.c.multiline:
            self.rv = self.c.check_text(self.ts, self.tv)
        else:
            self.rv = self.c.check_line(self.ts, self.tv)
        self._do_test()

class AllowedTest(RuleTest):
    """
    Tests allowed code. The rule checker shouldn't find anything
    in this test
    """
    def _do_test(self):
        if self.c.multiline:
            condition = (len(self.rv)==0)
        else:
            condition = (self.rv == False)
        
        self.assertTrue( condition,
            "Rule '%s' failed. Example '%s' should be ok, but wasn't" % (self.c.name,self.tv) )
class ForbiddenTest(RuleTest):
    """
    Tests forbidden constructs. The rule checker should find errors
    in the example code.
    """
    def _do_test(self):
        if self.c.multiline:
            condition = (len(self.rv)!=0)
        else:
            condition = (self.rv == True)
        self.assertTrue( self.rv,
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
        for checker in (d._checkers + d._mlcheckers):
            if checker.name in sys.argv:
                suite.addTests( _make_tests_from_checker(checker) )
    else:
        for checker in d._checkers:
            suite.addTests( _make_tests_from_checker(checker) )
        for checker in d._mlcheckers:
            suite.addTests( _make_tests_from_checker(checker) )

    unittest.TextTestRunner(verbosity=1).run(suite)
