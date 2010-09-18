-- =============================
-- Test functionality in wl.Game 
-- =============================

test_game = lunit.TestCase("Game functions test")
function test_game:setup()
   self.g = wl.Game()
end

function test_game:test_players_len()
   assert_equal(3, #self.g.players)
end

function test_game:test_players()
   local p = self.g.players[1]
   assert_equal(1, p.number)
   assert_equal("Barbarian Bouncer", p.name)
   local p = self.g.players[2]
   assert_equal(2, p.number)
   assert_equal("Epic Empire", p.name)
   local p = self.g.players[3]
   assert_equal(3, p.number)
   assert_equal("Awesome Atlantean", p.name)
end
   

