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
      print(v.name)
      rv = rv + 1
   end
   return rv
end

function ui_tests:test_buttons_property()
   assert_not_nil(self.mv.buttons.help)
end

function ui_tests:test_window_property()
   -- No window to start with
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons.messages:click()
   assert_equal(1, self:_cnt(self.mv.windows))

   assert_not_nil(self.mv.windows.messages)
end

function ui_tests:test_window_property1()
   assert_equal(0, self:_cnt(self.mv.windows))

   self.mv.buttons.objectives:click()
   assert_equal(1, self:_cnt(self.mv.windows))

   assert_not_nil(self.mv.windows.objectives)
   assert_nil(self.mv.windows.messages)
end

function ui_tests:test_position_x()
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages

   w.position_x = 50
   assert_equal(50, w.position_x)
end
function ui_tests:test_position_y()
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages

   w.position_y = 60
   assert_equal(60, w.position_y)
end

function ui_tests:test_descendant_position()
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages
   local b = w.buttons.toggle_between_inbox_or_archive

   w.position_x = 50
   w.position_y = 50

   local abs_x, abs_y = self.mv:get_descendant_position(b)

   assert_equal(w.position_x + b.position_x, abs_x)
   assert_equal(w.position_y + b.position_y, abs_y)
end

function ui_tests:test_descendant_position_not_child()
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages
   local b = self.mv.buttons.help

   assert_error("Not a descendant!", function()
      w:get_descendant_position(b)
   end)
end

function ui_tests:test_width(x)
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages

   w.width = 300
   assert_equal(300, w.width)
end
function ui_tests:test_height(x)
   self.mv.buttons.messages:click()
   local w = self.mv.windows.messages

   w.height = 200
   assert_equal(200, w.height)
end

-- ========
-- Buttons
-- ========
button_tests = lunit.TestCase("Button tests")
function button_tests:setup()
   self.b = wl.ui.MapView().buttons.help
   for n,w in pairs(wl.ui.MapView().windows) do w:close() end
end

function button_tests:test_name()
   assert_equal("help", self.b.name)
end
function button_tests:test_click()
   self.b:click()

   assert_not_nil(wl.ui.MapView().windows.encyclopedia)
end

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
   mv:click(map:get_field(10,10))
   self.w = mv.windows.field_action
end
function tab_panel_tests:teardown()
   local mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   self.w = nil
end

function tab_panel_tests:test_tabs()
   assert_equal(5, self:_cnt(self.w.tabs))
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
   self.mv.statistics = false
   for n,w in pairs(self.mv.windows) do w:close() end
end

function mv_tests:test_click()
   self.mv:click(map:get_field(10,10))
   assert_not_nil(self.mv.windows.field_action)
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


-- =========
-- Dropdowns
-- =========

--[[Dropdowns are tested in plain.wmf/test_ui.lua, because we need to call sleep()]]
