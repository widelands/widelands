-- ================
-- PlayerBase tests
-- ================

-- NOTE: Most of these tests conquer some area for the player and even though
-- all buildings are destroyed after each test, the area is not unconquered
-- since this functionality is not defined inside the game. This means that the
-- state of these fields bleed into others, therefore for the three players,
-- there are individual fields which are used only for these tests here, so
-- this bleeding doesn't matter.
player_tests = lunit.TestCase("Player tests")
function player_tests:test_tribe_property()
   assert_equal("barbarians", player1.tribe)
   assert_equal("empire", player2.tribe)
   assert_equal("atlanteans", player3.tribe)
end

function player_tests:test_number_property()
   assert_equal(1, player1.number)
   assert_equal(2, player2.number)
   assert_equal(3, player3.number)
end

-- =================
-- place_flag tests 
-- =================
function player_tests:test_create_flag()
   local k = player1:place_flag(map:get_field(10,10), true)
   assert_equal(k.owner.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing()
   local f = map:get_field(10,10)
   -- First force the flag, then remove them again
   local k = player1:place_flag(f, true)
   k:remove()
   -- Now, try again, but non forcing
   local k = player1:place_flag(f)
   assert_equal(k.owner.number, 1)
   k:remove()
end
function player_tests:test_create_flag_non_forcing_too_close()
   local f = map:get_field(10,10)
   -- First force the flag, then remove them again
   player1:place_flag(f, true):remove()
   player1:place_flag(f.rn, true):remove()

   -- Now, try again, but non forcing
   local k = player1:place_flag(f)
   assert_error("Too close to other!", function() player1:place_flag(f.rn) end)
   k:remove()
end
-- This test is currently disabled because of bug lp:536366
-- function player_tests:test_create_flag2()
--    local f = map:get_field(20,10)
--    local k = wl.Game().players[2]:place_flag(f, true)
--    assert_equal(k.owner.number, 2)
--    k:remove()
-- end

-- =================
-- place_road tests
-- =================
road_construction_tests = lunit.TestCase("road construction tests")
function road_construction_tests:setup()
   self.f = map:get_field(10,10)
   player1:conquer(self.f, 6)

   self.start_flag = player1:place_flag(self.f)
   self.flags = {
      self.start_flag
   }
end
function road_construction_tests:teardown()
   pcall(function()
      for temp,f in ipairs(self.flags) do
         f:remove()
      end
   end)
end
function road_construction_tests:test_simple()
   r = player1:place_road(self.start_flag, "r", "br", "r")
   self.flags[#self.flags+1] = self.f.rn.brn.rn.immovable
   assert_equal(3, r.length)
end
function road_construction_tests:test_too_short()
   assert_error("can't place flag", function()
      player1:place_road(self.start_flag, "r")
   end)
end
function road_construction_tests:test_connect_two_flags()
   self.flags[#self.flags+1] = player1:place_flag(self.f.rn.rn)
   r = player1:place_road(self.start_flag,"r", "r")
   assert_equal(2, r.length)
end
function road_construction_tests:test_road_crosses_another()
   self.flags[#self.flags+1] = player1:place_flag(self.f.rn.rn.rn.rn)
   r = player1:place_road(self.start_flag,"r", "r", "r", "r")
   assert_equal(4, r.length)
   self.flags[#self.flags+1] = player1:place_flag(self.f.rn.rn.tln.tln)
   self.flags[#self.flags+1] = player1:place_flag(self.f.rn.rn.brn.brn)
   assert_error("Immovable in the way", function()
      r = player1:place_road(self.flags[3],"br", "br", "br", "br")
   end)
end
function road_construction_tests:test_road_is_closed_loop()
   assert_error("Cannot build closed loop", function()
      player1:place_road(self.start_flag,"r", "r", "tl", "tl", "bl", "bl")
   end)
end
function road_construction_tests:test_road_crosses_itself()
   assert_error("The road crosses itself", function()
      player1:place_road(self.start_flag,"r", "r", "r", "tl", "bl", "bl")
   end)
end

-- =====================
-- place_building tests 
-- =====================
function player_tests:test_force_building()
   local f = map:get_field(10,10)
   local k = player1:place_building("headquarters", f)
   assert_equal(1, k.owner.number)
   assert_equal("warehouse", k.building_type)
   f.brn.immovable:remove() -- removing flag also removes building
end
function player_tests:test_force_building_illegal_name()
   assert_error("Illegal building", function()
      player1:place_building("kjhsfjkh", map:get_field(10,10))
   end)
end


