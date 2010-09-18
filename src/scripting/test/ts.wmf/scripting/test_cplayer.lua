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
   assert_equal("barbarians", wl.game.Player(1).tribe)
   assert_equal("empire", wl.game.Player(2).tribe)
   assert_equal("atlanteans", wl.game.Player(3).tribe)
end

function player_tests:test_number_property()
   assert_equal(1, wl.game.Player(1).number)
   assert_equal(2, wl.game.Player(2).number)
   assert_equal(3, wl.game.Player(3).number)
end
function player_tests:test_negativ_val_for_player()
   assert_error("Illegal plr",  function() wl.game.Player(-1) end)
end
function player_tests:test_too_high_val()
   assert_error("Illegal plr",  function() wl.game.Player(10) end)
end
function player_tests:test_ok_val_for_non_existing()
   assert_error("non existing plr",  function() wl.game.Player(5) end)
end



