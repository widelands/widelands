include "test/scripting/stable_save.lua"

-- Test saveloading of input queues
run(function()

   local hq = p1:get_buildings("barbarians_headquarters")[1]
   hq:set_wares{
      log = 100,
      blackwood = 100,
      granite = 100,
      reed = 100,
      grout = 100,
   }
   hq:set_workers{
      barbarians_carrier = 10,
      barbarians_ox = 10,
      barbarians_innkeeper = 10,
      barbarians_builder = 10,
      barbarians_soldier = 10,
   }
   local cs1 = p1:place_building("barbarians_tavern", map:get_field(24, 25), true, true)
   local cs2 = p1:place_building("barbarians_sentry", map:get_field(26, 25), true, true)
   sleep(1000)
   assert_equal("constructionsite", cs1.descr.name)
   assert_equal("constructionsite", cs2.descr.name)
   p1:place_road("normal", cs2.flag, "w", "w")
   p1:place_road("busy", cs1.flag, "w", "w")
   sleep(1000)

   assert_equal("normal", cs1:get_priority("log", false))
   assert_equal("normal", cs1:get_priority("fish", true))
   assert_equal(3, cs1:get_desired_fill("log", false))
   assert_equal(4, cs1:get_desired_fill("fish", true))
   assert_equal(2, cs2:get_desired_fill("blackwood", false))
   assert_equal(2, cs2.setting_soldier_capacity)
   assert_equal(false, cs2.setting_prefer_heroes)
   sleep(1000)

   cs1:set_desired_fill("fish", 1, true)
   cs1:set_priority("meat", "low", true)
   cs1:set_priority("fish", "high", true)
   cs2.setting_soldier_capacity = 1
   cs2.setting_prefer_heroes = true
   sleep(1000)

   assert_equal(1, cs1:get_desired_fill("fish", true))
   assert_equal("high", cs1:get_priority("fish", true))
   assert_equal("low", cs1:get_priority("meat", true))
   assert_equal(1, cs2.setting_soldier_capacity)
   assert_true(cs2.setting_prefer_heroes)

   sleep(300000)

   local b1 = map:get_field(24, 25).immovable
   local b2 = map:get_field(26, 25).immovable
   assert_equal("barbarians_tavern", b1.descr.name)
   assert_equal("barbarians_sentry", b2.descr.name)
   assert_equal(1, b1:get_desired_fill("fish"))
   assert_equal(4, b1:get_desired_fill("meat"))
   assert_equal("high", b1:get_priority("fish"))
   assert_equal("low", b1:get_priority("meat"))
   assert_true(b2.prefer_heroes)
   assert_equal(1, b2.capacity)
   sleep(1000)

   b1:set_desired_fill("fish", 3)
   b1:set_desired_fill("meat", 0)
   b1:set_priority("fish", "normal")
   b2.capacity = 2
   b2.prefer_heroes = false
   sleep(1000)

   assert_equal(3, b1:get_desired_fill("fish"))
   assert_equal(0, b1:get_desired_fill("meat"))
   assert_equal("normal", b1:get_priority("fish"))
   assert_equal(false, b2.prefer_heroes)
   assert_equal(2, b2.capacity)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
