#!/usr/bin/env python
# encoding: utf-8

import os

from test import WidelandsTestCase

class LuaTestsuiteInGame(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario=os.path.join(self.maps_dir, "lua_testsuite.wmf"))

class LuaTestsuiteInEditor(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(editor=os.path.join(self.maps_dir, "lua_testsuite.wmf"))


