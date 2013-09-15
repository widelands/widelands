#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class LuaTestsuiteInGame(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario="lua_testsuite.wmf")
        self.assert_all_lunit_tests_passed()
