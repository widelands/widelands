#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import argparse
import os
import re
import unittest

import test

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

    return p.parse_args()

def main():
    args = parse_args()

    test.WidelandsTestCase.do_use_random_directory = not args.nonrandom

    all_files = [os.path.basename(filename) for filename in glob("test/test_*.py") ]
    if args.regexp:
        all_files = [filename for filename in all_files if re.search(args.regexp, filename) ]

    all_modules = [ "test.%s" % filename[:-3] for filename in all_files ]

    test_loader = unittest.TestLoader()
    all_tests = test_loader.loadTestsFromNames(all_modules)

    unittest.TextTestRunner(verbosity=2).run(all_tests)

if __name__ == '__main__':
    main()
