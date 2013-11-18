#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class CompatibilityBuild15(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(
            loadgame="test/save/build15-bigeconomy.wgf",
            script="test/scripts/exit_after_100_seconds.lua"
        )
        self.assert_all_lunit_tests_passed()
