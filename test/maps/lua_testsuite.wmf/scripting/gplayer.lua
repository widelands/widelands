-- ======================================================
-- Testing Player functionality that is only in the game
-- ======================================================

function player_tests:test_name_property()
   assert_equal("Barbarian Bouncer", egbase.players[1].name)
   assert_equal("Epic Empire", egbase.players[2].name)
   assert_equal("Awesome Atlantean", egbase.players[3].name)
end

-- =========================
-- Forbid & Allow buildings
-- =========================
player_allow_buildings_tests = lunit.TestCase("PlayerAllowed Buildings")
function player_allow_buildings_tests:setup()
   player1:allow_buildings "all"
end
function player_allow_buildings_tests:teardown()
   player1:allow_buildings "all"
end
function player_allow_buildings_tests:test_property()
   assert_equal(true, player1.allowed_buildings.barbarians_lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["barbarians_quarry"])
end
function player_allow_buildings_tests:test_forbid_all_buildings()
   player1:forbid_buildings("all")
   for b,v in pairs(player1.allowed_buildings) do
      assert_equal(false, v, b .. " was not forbidden!")
   end
end
function player_allow_buildings_tests:test_forbid_some_buildings()
   player1:forbid_buildings{"barbarians_lumberjacks_hut"}
   assert_equal(false, player1.allowed_buildings.barbarians_lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["barbarians_quarry"])
   player1:forbid_buildings{"barbarians_quarry", "barbarians_sentry"}
   assert_equal(false, player1.allowed_buildings["barbarians_quarry"])
   assert_equal(false, player1.allowed_buildings["barbarians_sentry"])
end
function player_allow_buildings_tests:test_allow_some()
   player1:forbid_buildings("all")
   player1:allow_buildings{"barbarians_quarry", "barbarians_sentry"}
   assert_equal(false, player1.allowed_buildings.barbarians_lumberjacks_hut)
   assert_equal(true, player1.allowed_buildings["barbarians_quarry"])
   assert_equal(true, player1.allowed_buildings["barbarians_sentry"])
end
function player_allow_buildings_tests:test_forbid_illegal_buildings()
   function a() player1:forbid_buildings{"lumberjacksjkdhfs_hut"} end
   assert_error("Illegal building!", a)
end
function player_allow_buildings_tests:test_forbid_string_not_all()
   function a() player1:forbid_buildings "notall"  end
   assert_error("String argument must be all", a)
end

-- ================
-- Access to players buildings
-- ================
player_building_access = lunit.TestCase("Access to Player buildings")
function player_building_access:teardown()
   for temp,b in ipairs(self.bs) do
      pcall(function() b.fields[1].brn.immovable:remove() end)
   end
end
function player_building_access:test_single()
   self.bs = {
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(10,10)),
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(13,10)),
      player1:place_building("barbarians_quarry", map:get_field(8,10)),
   }
   assert_equal(2, #player1:get_buildings("barbarians_lumberjacks_hut"))
   assert_equal(1, #player1:get_buildings("barbarians_quarry"))
end
function player_building_access:test_multi()
   self.bs = {
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(10,10)),
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(13,10)),
      player1:place_building("barbarians_quarry", map:get_field(8,10)),
   }
   rv = player1:get_buildings{"barbarians_lumberjacks_hut", "barbarians_quarry"}

   assert_equal(2, #rv.barbarians_lumberjacks_hut)
   assert_equal(1, #rv.barbarians_quarry)
end
function player_building_access:test_access()
   local b1 = player1:place_building("barbarians_lumberjacks_hut", map:get_field(10,10))
   local b2 = player1:place_building("barbarians_lumberjacks_hut", map:get_field(13,10))
   local b3 = player1:place_building("barbarians_quarry", map:get_field(8,10))
   self.bs = { b1, b2, b3 }
   rv = player1:get_buildings{"barbarians_lumberjacks_hut", "barbarians_quarry"}

   assert_equal(b3, rv.barbarians_quarry[1])
   b1.fields[1].brn.immovable:remove()
   assert_equal(1, #player1:get_buildings("barbarians_lumberjacks_hut"))
end
-- ================
-- Players production statistics
-- ================
player_production_statistics = lunit.TestCase("Players production statistics")
function player_building_access:test_single()
   self.bs = {
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(10,10)),
      player1:place_building("barbarians_lumberjacks_hut", map:get_field(13,10)),
      player1:place_building("barbarians_quarry", map:get_field(8,10)),
   }

   assert_equal((player1:get_produced_wares_count('all'))['log'], player1:get_produced_wares_count('log'))
end
