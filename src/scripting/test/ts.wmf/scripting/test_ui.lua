-- =======================================================================
--                      User Interface scripting tests                      
-- =======================================================================

ui_tests = lunit.TestCase("Game User Interface tests")
function ui_tests:setup()
   self.mv = wl.ui.MapView()

   for name,win in pairs(self.mv.windows) do
      win:close()
   end

   if wl.editor then
      self.b1 = "menu"
      self.w1 = "main_menu"
      self.b2 = "players"
      self.w2 = "players_menu"
   else
      self.b1 = "messages"
      self.w1 = "messages"
      self.b2 = "objectives"
      self.w2 = "objectives"
   end
end

-- ======
-- Panel 
-- ======
function ui_tests:_cnt(t) 
   local rv = 0
   for k,v in pairs(t) do
      rv = rv + 1 end
   return rv
end

function ui_tests:test_buttons_property()
   assert_not_equal(nil, self.mv.buttons.buildhelp)

   if wl.editor then
      assert_equal(6, self:_cnt(self.mv.buttons)) -- Editor has 6 buttons
   else
      assert_equal(7, self:_cnt(self.mv.buttons)) -- a scenario game, so 7
   end
end

function ui_tests:test_window_property()
   -- No window to start with
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons[self.b1]:click()
   assert_equal(1, self:_cnt(self.mv.windows))
   
   assert_not_equal(nil, self.mv.windows[self.w1])
end

function ui_tests:test_window_property1()
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons[self.b2]:click()
   assert_equal(1, self:_cnt(self.mv.windows))
   
   assert_not_equal(nil, self.mv.windows[self.w2])
   assert_equal(nil, self.mv.windows[self.w1])
end

function ui_tests:test_position_x()
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]

   w.position_x = 50
   assert_equal(50, w.position_x)
end
function ui_tests:test_position_y()
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]

   w.position_y = 60
   assert_equal(60, w.position_y)
end

function ui_tests:test_descendant_position()
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]
   local b
   if not wl.editor then 
      b = w.buttons.invert_selection
   else
      b = w.buttons.exit
   end

   w.position_x = 50
   w.position_y = 50

   local abs_x, abs_y = self.mv:get_descendant_position(b)

   assert_equal(w.position_x + b.position_x, abs_x)
   assert_equal(w.position_y + b.position_y, abs_y)
end

function ui_tests:test_descendant_position_not_child()
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]
   local b = self.mv.buttons.buildhelp

   assert_error("Not a descendant!", function()
      w:get_descendant_position(b)
   end)
end

function ui_tests:test_width(x)
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]

   w.width = 300
   assert_equal(300, w.width)
end
function ui_tests:test_height(x)
   self.mv.buttons[self.b1]:click()
   local w = self.mv.windows[self.w1]

   w.height = 200
   assert_equal(200, w.height)
end

function ui_tests:test_mouse_pos_x(x)
   self.mv.mouse_position_x = 200
   assert_equal(200, self.mv.mouse_position_x)
end
function ui_tests:test_mouse_pos_y(x)
   self.mv.mouse_position_y = 100
   assert_equal(100, self.mv.mouse_position_y)
end

-- ========
-- Buttons
-- ========
button_tests = lunit.TestCase("Button tests")
function button_tests:setup()
   self.b = wl.ui.MapView().buttons.buildhelp
   wl.ui.MapView().buildhelp = false
end

function button_tests:test_name()
   assert_equal("buildhelp", self.b.name)
end
function button_tests:test_click()
   self.b:click()

   assert_equal(true, wl.ui.MapView().buildhelp)
end

-- TODO: this is not proper, all tests should run and editor/game seperation should be cleaner
if not wl.editor then 
-- =========
-- TabPanel
-- =========
tab_panel_tests = lunit.TestCase("TabPanel Unit Tests")
function tab_panel_tests:_cnt(t)
   local rv = 0
   for k,v in pairs(t) do
      rv = rv + 1 end
   return rv
end

function tab_panel_tests:setup()
   local mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   mv:click(wl.Map():get_field(10,10))
   self.w = mv.windows.field_action
end
function tab_panel_tests:teardown()
   local mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   self.w = nil
end

function tab_panel_tests:test_tabs()
   assert_equal(4, self:_cnt(self.w.tabs))
end
function tab_panel_tests:test_active()
   assert_equal(true, self.w.tabs.big.active)
end
function tab_panel_tests:test_activate()
   self.w.tabs.small:click()
   assert_equal(true, self.w.tabs.small.active)
end

-- ========
-- MapView
-- ========
mv_tests = lunit.TestCase("MapView tests")
function mv_tests:setup()
   self.mv = wl.ui.MapView()
   self.mv.census = false
   self.mv.buildhelp = false
   for n,w in pairs(self.mv.windows) do w:close() end
end

function mv_tests:test_click()
   self.mv:click(wl.Map():get_field(10,10))
   assert_not_equal(nil, self.mv.windows.field_action)
end

function mv_tests:test_census()
   self.mv.census = 1
   assert_equal(true, self.mv.census)
   assert_equal(false, self.mv.statistics)
end

function mv_tests:test_statistics()
   self.mv.statistics = 1
   assert_equal(true, self.mv.statistics)
   assert_equal(false, self.mv.census)
end

end

