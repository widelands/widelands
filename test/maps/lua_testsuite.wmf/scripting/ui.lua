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


-- =============================
-- Dropdowns and toolbar buttons
-- =============================

--[[These are tested in plain.wmf/test_ui.lua, because we need to call sleep()]]
