#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class Test(WidelandsTestCase):
    def runTest(self):
        self.run_widelands(scenario="test/maps/disappearing_ports.wmf",
                script="test/maps/disappearing_ports.wmf/scripting/test_disappearing_ports_destination_disappears_worker_in_dock.lua")
        self.assert_all_lunit_tests_passed()
