-- =======================================================================
--                      User Interface scripting tests                      
-- =======================================================================

ui_tests = lunit.TestCase("Game User Interface tests")
function ui_tests:test_interface()
   local mv = wl.ui.MapView()

   assert_not_equal(nil, mv.buttons.buildhelp)
end


-- ========
-- Buttons 
-- ========
button_tests = lunit.TestCase("Button tests")
function button_tests:setup()
   self.b = wl.ui.MapView().buttons.buildhelp
   self.p = wl.game.Player(1)
   self.p.buildhelp = false
end

function button_tests:test_name()
   assert_equal("buildhelp", self.b.name)
end
function button_tests:test_click()
   self.b:click()
   
   assert_equal(true, self.p.buildhelp)
end

