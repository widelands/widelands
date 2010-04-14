-- =======================================================================
--                      User Interface scripting tests                      
-- =======================================================================

ui_tests = lunit.TestCase("Game User Interface tests")
function ui_tests:test_interface()
   local i = wl.ui.get()
   assert_equal(5, #i.buttons)
end

