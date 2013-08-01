#!/usr/bin/env python
# encoding: utf-8

import os

from test import WidelandsTestCase

class LuaPersistence(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario=os.path.join(self.maps_dir, "lua_persistence.wmf"))
        self.run_widelands(loadgame=os.path.join("save", "lua_persistence.wgf"))
        pass



