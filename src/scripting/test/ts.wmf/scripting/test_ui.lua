-- =======================================================================
--                      User Interface scripting tests                      
-- =======================================================================

ui_tests = lunit.TestCase("Game User Interface tests")
function ui_tests:setup()
   self.mv = wl.ui.MapView()

   for name,win in pairs(self.mv.windows) do
      win:close()
   end
end

-- ======
-- Panel 
-- ======
function ui_tests:_cnt(t) 
   local rv = 0
   for k,v in pairs(t) do
      print("## ", k)
      rv = rv + 1 end
   return rv
end

function ui_tests:test_buttons_property()
   assert_not_equal(nil, self.mv.buttons.buildhelp)
   assert_equal(7, self:_cnt(self.mv.buttons)) -- a scenario game, so 7 buttons
end

function ui_tests:test_window_property()
   -- No window to start with
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons.messages:click()
   assert_equal(1, self:_cnt(self.mv.windows))
   
   assert_not_equal(nil, self.mv.windows.messages)
end

function ui_tests:test_window_property1()
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons.objectives:click()
   assert_equal(1, self:_cnt(self.mv.windows))
   
   assert_not_equal(nil, self.mv.windows.objectives)
   assert_equal(nil, self.mv.windows.messages)
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

