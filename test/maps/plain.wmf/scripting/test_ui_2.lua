-- =======================================================================
--                      User Interface scripting tests
-- =======================================================================
-- These UI tests used to live in lua_testsuite.wmf/ui.lua,
-- but now we need to call sleep() everywhere so I moved them here

function setup_UI()
   mv = wl.ui.MapView()

   for name,win in pairs(mv.windows) do
      win:close()
   end
   sleep(2000)
end

-- ======
-- Panel
-- ======
function _cnt_PANEL(t)
   rv = 0
   for k,v in pairs(t) do
      print(v.name)
      rv = rv + 1
   end
   return rv
end

function test_buttons_property()
   assert_not_nil(mv.buttons.help)
end

function test_window_property1()
   -- No window to start with
   assert_equal(0, _cnt_PANEL(mv.windows))

   mv.buttons.messages:click()
   sleep(2000)
   assert_equal(1, _cnt_PANEL(mv.windows))

   assert_not_nil(mv.windows.messages)
end

function test_window_property2()
   assert_equal(0, _cnt_PANEL(mv.windows))

   mv.buttons.objectives:click()
   sleep(2000)
   assert_equal(1, _cnt_PANEL(mv.windows))

   assert_not_nil(mv.windows.objectives)
   assert_nil(mv.windows.messages)
end

function test_position_x()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages

   w.position_x = 50
   assert_equal(50, w.position_x)
end
function test_position_y()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages

   w.position_y = 60
   assert_equal(60, w.position_y)
end

function test_descendant_position()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages
   b = w.buttons.toggle_between_inbox_or_archive

   w.position_x = 50
   w.position_y = 50

   abs_x, abs_y = mv:get_descendant_position(b)

   assert_equal(w.position_x + b.position_x, abs_x)
   assert_equal(w.position_y + b.position_y, abs_y)
end

function test_descendant_position_not_child()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages
   b = mv.buttons.help

   assert_error("Not a descendant!", function()
      w:get_descendant_position(b)
   end)
end

function test_width()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages

   w.width = 300
   assert_equal(300, w.width)
end
function test_height()
   mv.buttons.messages:click()
   sleep(2000)
   w = mv.windows.messages

   w.height = 200
   assert_equal(200, w.height)
end

-- ========
-- Buttons
-- ========
function setup_BUTTON()
   b = wl.ui.MapView().buttons.help
   for n,w in pairs(wl.ui.MapView().windows) do w:close() end
end

function test_name()
   assert_equal("help", b.name)
end
function test_click1()
   b:click()
   sleep(2000)

   assert_not_nil(wl.ui.MapView().windows.encyclopedia)
end

-- =========
-- TabPanel
-- =========
function _cnt_TABPANEL(t)
   rv = 0
   for k,v in pairs(t) do
      rv = rv + 1 end
   return rv
end

function setup_TABPANEL()
   mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
end
function teardown_TABPANEL()
   mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   w = nil
end

function test_tabs()
   mv:click(map:get_field(23, 29))
   sleep(2000)
   assert_equal(5, _cnt_TABPANEL(mv.windows.field_action.tabs))
end
function test_active()
   mv:click(map:get_field(23, 29))
   sleep(2000)
   assert_equal(true, mv.windows.field_action.tabs.big.active)
end
function test_activate()
   mv:click(map:get_field(23, 29))
   sleep(2000)
   mv.windows.field_action.tabs.small:click()
   sleep(2000)
   assert_equal(true, mv.windows.field_action.tabs.small.active)
end

-- ========
-- MapView
-- ========
function setup_MAPVIEW()
   mv = wl.ui.MapView()
   mv.census = false
   mv.statistics = false
   for n,w in pairs(mv.windows) do w:close() end
end

function test_click2()
   mv:click(map:get_field(23,29))
   sleep(2000)
   assert_not_nil(mv.windows.field_action)
end

function test_census()
   mv.census = 1
   assert_equal(true, mv.census)
   assert_equal(false, mv.statistics)
end

function test_statistics()
   mv.statistics = 1
   assert_equal(true, mv.statistics)
   assert_equal(false, mv.census)
end


run(function()
   game.desired_speed = 1000

   for i,fn in pairs({
         test_buttons_property,
         test_window_property1,
         test_window_property2,
         test_position_x,
         test_position_y,
         test_descendant_position,
         test_descendant_position_not_child,
         test_width,
         test_height}) do
      print("Starting UI test #" .. i)
      setup_UI()
      fn()
   end

   for i,fn in pairs({
         test_name,
         test_click1}) do
      print("Starting BUTTON test #" .. i)
      setup_UI()
      setup_BUTTON()
      fn()
   end

   for i,fn in pairs({
         test_tabs,
         test_active,
         test_activate}) do
      print("Starting TABPANEL test #" .. i)
      setup_UI()
      setup_TABPANEL()
      fn()
      teardown_TABPANEL()
   end

   for i,fn in pairs({
         test_click2,
         test_census,
         test_statistics}) do
      print("Starting MAPVIEW test #" .. i)
      setup_UI()
      setup_MAPVIEW()
      fn()
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
