-- ==================================================
-- Test common functionality in wl.Game and wl.Editor
-- ==================================================

test_egbase = lunit.TestCase("EditorGameBase functions test")
function test_egbase:test_players_len()
   assert_equal(3, #egbase.players)
end

function test_egbase:test_players()
   local p = egbase.players[1]
   assert_equal(1, p.number)
   assert_equal("Barbarian Bouncer", p.name)
   local p = egbase.players[2]
   assert_equal(2, p.number)
   assert_equal("Epic Empire", p.name)
   local p = egbase.players[3]
   assert_equal(3, p.number)
   assert_equal("Awesome Atlantean", p.name)
end
   

