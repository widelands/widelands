-- ====================
-- Tests for the editor 
-- ====================

test_editor = lunit.TestCase("Editor functions test")

function test_editor:test_no_game_module()
   assert_equal(nil, wl.game)
end
function test_editor:test_no_game_class()
   assert_equal(nil, wl.Game)
end

