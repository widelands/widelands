#!/usr/bin/env python
# encoding: utf-8
#

import unittest

from CodeCheck import CodeChecker, Preprocessor

class RuleTest(unittest.TestCase):
    "Base class for RuleTesting"
    def __init__( self, checker, val ):
        unittest.TestCase.__init__(self)
        self.preprocessor = Preprocessor()
        self.c = checker
        self.tv = val

    def runTest(self):
        self.rv = self.c.check_text(self.preprocessor, "testdir/test.h", self.tv)
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

class CommentsTest(unittest.TestCase):
    """
    Tests that comment stripping is working correctly
    """
    def __init__(self, before, after):
        unittest.TestCase.__init__(self)
        self.before = before
        self.after = after
        
    def runTest(self):
        preprocessor = Preprocessor()
        after = preprocessor.get_preprocessed_data("test", self.before, True, False)
        self.assertTrue(after == self.after,
            "Stripping comments from %r failed. Expected %r, got %r"
            % (self.before, self.after, after)
            )
        
comment_tests = [
    # Let's get the basics right.
    ("a b c",
     "a b c"),
    # Whitespace before comments should be stripped
    ("a b c       // a",
     "a b c"),
    # Single line comment shouldn't affect the next line
    ("a b c       // a\nd e",
     "a b c\nd e"),
    # Multi-line comments should retain original line numbering
    ("a /* \n b \n */ c",
     "a \n\n c"),
    # Multiple comments on one line should work
    ("int32_t estimate(Map & /* map */, FCoords /* pos */) const {return 0;}\ntest",
     "int32_t estimate(Map & "       ", FCoords "       ") const {return 0;}\ntest"),
]

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

    for before, after in comment_tests:
        suite.addTest(CommentsTest(before, after.splitlines(True)))

    unittest.TextTestRunner(verbosity=1).run(suite)
