-- ==================================================
-- Test common functionality in wl.Game and wl.Editor
-- ==================================================

test_egbase = lunit.TestCase("EditorGameBase functions test")
function test_egbase:test_players_len()
   assert_equal(3, #egbase.players)
end

function test_egbase:test_player_numbers()
   assert_equal(1, egbase.players[1].number)
   assert_equal(2, egbase.players[2].number)
   assert_equal(3, egbase.players[3].number)
end
   
