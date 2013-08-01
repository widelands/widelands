#!/usr/bin/env python
# encoding: utf-8

import unittest
from glob import glob

def main():
    # NOCOM(#sirver): hardcoded - not good.
    tests = [ filename for filename in glob("test/lua_pers*.py") if not "__" in filename ]
    print "#sirver tests: %r\n" % (tests)
    test_loader = unittest.TestLoader()
    tests = test_loader.discover("test", "*.py")
    print "#sirver tests: %r\n" % (tests)

    runner = unittest.TextTestRunner()
    runner.run(tests)

if __name__ == '__main__':
    main()
