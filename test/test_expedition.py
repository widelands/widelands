#!/usr/bin/env python
# encoding: utf-8

from test import WidelandsTestCase

class Expedition(WidelandsTestCase):
    def runTest(self):
        # This will create a bunch of savegames that we can work with.
        self.run_widelands(scenario="test/maps/expedition.wmf")

        self.run_widelands(loadgame=self.run_dir + "/save/no_ship.wgf",
                script="test/maps/expedition.wmf/scripting/00_test_starting_and_immediately_canceling.lua")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/no_ship.wgf",
                script="test/maps/expedition.wmf/scripting/01_starting_wait_a_while_cancel.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/cancel_in_port.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/one_ship.wgf",
                script="test/maps/expedition.wmf/scripting/02_cancel_started_expedition.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/ready_to_sail.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/two_ships.wgf",
                script="test/maps/expedition.wmf/scripting/02_cancel_started_expedition.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/ready_to_sail.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/one_ship.wgf",
                script="test/maps/expedition.wmf/scripting/03_cancel_started_expedition_underway.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/sailing.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/two_ships.wgf",
                script="test/maps/expedition.wmf/scripting/03_cancel_started_expedition_underway.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/sailing.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/one_ship.wgf",
                script="test/maps/expedition.wmf/scripting/04_cancel_when_port_space_was_reached.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/reached_port_space.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/two_ships.wgf",
                script="test/maps/expedition.wmf/scripting/04_cancel_when_port_space_was_reached.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/reached_port_space.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/one_ship.wgf",
                script="test/maps/expedition.wmf/scripting/05_check_transporting_works.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/port_done.wgf")
        self.assert_all_lunit_tests_passed()

        self.run_widelands(loadgame=self.run_dir + "/save/two_ships.wgf",
                script="test/maps/expedition.wmf/scripting/05_check_transporting_works.lua")
        self.assert_all_lunit_tests_passed()
        self.run_widelands(loadgame=self.run_dir + "/save/port_done.wgf")
        self.assert_all_lunit_tests_passed()
