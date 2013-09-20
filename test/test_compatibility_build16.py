#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class CompatibilityBuild16(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(
            loadgame="test/save/Build16.wgf",
            script="test/scripts/exit_after_100_seconds.lua"
        )
        self.assert_all_lunit_tests_passed()
