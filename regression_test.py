#!/usr/bin/env python
# encoding: utf-8

import os

from test import WidelandsTestCase

class LuaTestSuiteInGame(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario=os.path.join(self.maps_dir, "lua_testsuite.wmf"))

class LuaTestSuiteInEditor(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(editor=os.path.join(self.maps_dir, "lua_testsuite.wmf"))

