#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class LuaPersistence(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario="test/maps/lua_persistence.wmf")
        self.run_widelands(loadgame=self.run_dir + "/save/lua_persistence.wgf")
        self.assert_all_lunit_tests_passed()
