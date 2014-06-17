#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import argparse
import os
import tempfile
import unittest
import time

from PIL import Image
from numpy.testing import assert_equal
import numpy as np

import pyrt_render

def parse_args():
    p = argparse.ArgumentParser(description=
        "Run test using render against tests in tests/"
    )

    p.add_argument("name", default=["tests/*/*.txt"], nargs="*",
        help = "The names of the tests to run. Can be globbed")
    p.add_argument("-v", "--verbose",  action="store_true", default=False,
        help = "Verbosity of test runner.")
    p.add_argument("-p", "--profile",  action="store_true", default=False,
        help = "Profile the tests and show running time after testing.")

    return p.parse_args()

def _compare(r, infile, profiles):
    dname = os.path.dirname(infile)
    width = 0
    if os.path.exists(dname + "/width"):
        width = int(open(dname + "/width").read())
    correct = dname + "/correct.png"
    ref_map_fn = dname + "/ref_map"

    st = time.time()
    a, refmap = r.render(open(infile).read(), width)
    et = time.time()
    b = np.asarray(Image.open(correct))
    try:
        assert_equal(a, b)
    except:
        Image.fromarray(a).save(dname + "/new.png")
        raise
    profiles[infile] = (et-st)

    if os.path.exists(ref_map_fn):
        for line in open(ref_map_fn):
            x, y, refn = line.split(" ", 2)
            refn = refn.strip()[1:-1] # remove "
            assert_equal(refmap.query(int(x),int(y)), refn, "RefMap query for %s, %s was %r (WANTED: %r))" %(
                x, y, refmap.query(int(x), int(y)), refn))


def main():
    args = parse_args()
    tests = reduce(lambda a,b: a|b, (set(glob(tn)) for tn in args.name), set())
    tests = sorted(tests)

    suite = unittest.TestSuite()

    r = pyrt_render.Renderer()

    profiles = {}
    def _make_test(infile):
        def testfunc():
            return _compare(r, infile, profiles)
        testfunc.__name__ = infile
        return testfunc

    for t in tests:
        suite.addTest(unittest.FunctionTestCase(_make_test(t)))
    unittest.TextTestRunner(verbosity=2 if args.verbose else 1).run(suite)

    if args.profile:
        sorted_list = sorted(((v,k) for k,v in profiles.items()), reverse=True)
        for time, name in sorted_list:
            name = name[6:] # Strip tests/
            print "%10s ms %s" % ("%.4f" % (time*1000), name)

if __name__ == '__main__':
    main()


