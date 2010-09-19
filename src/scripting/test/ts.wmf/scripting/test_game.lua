-- ===================
-- Tests for the game 
-- ===================

test_game = lunit.TestCase("Game functions test")

function test_game:test_no_editor_module()
   assert_equal(nil, wl.editor)
end
function test_game:test_no_editor_class()
   assert_equal(nil, wl.Editor)
end

