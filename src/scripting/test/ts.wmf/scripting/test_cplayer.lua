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

