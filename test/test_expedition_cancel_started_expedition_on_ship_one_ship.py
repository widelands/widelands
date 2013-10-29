#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class Expedition(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario="test/maps/expedition.wmf",
                script="test/maps/expedition.wmf/scripting/test_cancel_started_expedition_on_ship_one_ship.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/ready_to_sail.wgf")
        self.assert_all_lunit_tests_passed()
