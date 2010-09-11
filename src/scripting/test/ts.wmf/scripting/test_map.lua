-- ============================
-- Test functionality in wl.map 
-- ============================

test_map = lunit.TestCase("Map functions test")
function test_map:setup()
   self.m = wl.Map()
end
function test_map:test_width()
   assert_equal(64, self.m.width)
end
function test_map:test_height()
   assert_equal(80, self.m.height)
end

function test_map:test_playerslots_name()
   assert_equal("Barbarian Bouncer", self.m.player_slots[1].name)
   assert_equal("Epic Empire", self.m.player_slots[2].name)
   assert_equal("Awesome Atlantean", self.m.player_slots[3].name)
end

function test_map:test_playerslots_tribes()
   assert_equal("barbarians", self.m.player_slots[1].tribe)
   assert_equal("empire", self.m.player_slots[2].tribe)
   assert_equal("atlanteans", self.m.player_slots[3].tribe)
end

function test_map:test_playerslots_starting_field()
   assert_equal(self.m:get_field(10,10), self.m.player_slots[1].starting_field)
   assert_equal(self.m:get_field(30,10), self.m.player_slots[2].starting_field)
   assert_equal(self.m:get_field(50,10), self.m.player_slots[3].starting_field)
end


