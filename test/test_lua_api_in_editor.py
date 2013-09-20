#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class LuaTestsuiteInEditor(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(editor="test/maps/lua_testsuite.wmf")
        self.assert_all_lunit_tests_passed()
