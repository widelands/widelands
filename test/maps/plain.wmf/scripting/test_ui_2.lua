-- =======================================================================
--                      User Interface scripting tests
-- =======================================================================
-- These UI tests used to live in lua_testsuite.wmf/ui.lua,
-- but now we need to call sleep() everywhere so I moved them here

local function setup_UI()
   mv = wl.ui.MapView()

   for name,win in pairs(mv.windows) do
      win:close()
   end
   sleep(2000)
end

-- ======
-- Panel
-- ======
local function _cnt_PANEL(t)
   local rv = 0
   for k,v in pairs(t) do
      print(v.name)
      rv = rv + 1
   end
   return rv
end

local function test_buttons_property()
   assert_not_nil(mv.buttons.help)
end

local function test_window_property()
   -- No window to start with
   assert_equal(0, _cnt_PANEL(mv.windows))

   mv.buttons.messages:click()
   sleep(2000)
   assert_equal(1, _cnt_PANEL(mv.windows))

   assert_not_nil(mv.windows.messages)
end

local function test_window_property1()
   assert_equal(0, _cnt_PANEL(mv.windows))

   mv.buttons.objectives:click()
   sleep(2000)
   assert_equal(1, _cnt_PANEL(mv.windows))

   assert_not_nil(mv.windows.objectives)
   assert_nil(mv.windows.messages)
end

local function test_position_x()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages

   w.position_x = 50
   assert_equal(50, w.position_x)
end
local function test_position_y()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages

   w.position_y = 60
   assert_equal(60, w.position_y)
end

local function test_descendant_position()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages
   local b = w.buttons.toggle_between_inbox_or_archive

   w.position_x = 50
   w.position_y = 50

   local abs_x, abs_y = mv:get_descendant_position(b)

   assert_equal(w.position_x + b.position_x, abs_x)
   assert_equal(w.position_y + b.position_y, abs_y)
end

local function test_descendant_position_not_child()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages
   local b = mv.buttons.help

   assert_error("Not a descendant!", function()
      w:get_descendant_position(b)
   end)
end

local function test_width()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages

   w.width = 300
   assert_equal(300, w.width)
end
local function test_height()
   mv.buttons.messages:click()
   sleep(2000)
   local w = mv.windows.messages

   w.height = 200
   assert_equal(200, w.height)
end

-- ========
-- Buttons
-- ========
local function setup_BUTTON()
   b = wl.ui.MapView().buttons.help
   for n,w in pairs(wl.ui.MapView().windows) do w:close() end
end

local function test_name()
   assert_equal("help", b.name)
end
local function test_click()
   b:click()
   sleep(2000)

   assert_not_nil(wl.ui.MapView().windows.encyclopedia)
end

-- =========
-- TabPanel
-- =========
local function _cnt_TABPANEL(t)
   local rv = 0
   for k,v in pairs(t) do
      rv = rv + 1 end
   return rv
end

local function setup_TABPANEL()
   local mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   mv:click(map:get_field(10,10))
   sleep(2000)
   w = mv.windows.field_action
end
local function teardown_TABPANEL()
   local mv = wl.ui.MapView()
   for n,w in pairs(mv.windows) do w:close() end
   w = nil
end

local function test_tabs()
   assert_equal(5, _cnt_TABPANEL(w.tabs))
end
local function test_active()
   assert_equal(true, w.tabs.big.active)
end
local function test_activate()
   w.tabs.small:click()
   sleep(2000)
   assert_equal(true, w.tabs.small.active)
end

-- ========
-- MapView
-- ========
local function setup_MAPVIEW()
   mv = wl.ui.MapView()
   mv.census = false
   mv.statistics = false
   for n,w in pairs(mv.windows) do w:close() end
end

local function test_click()
   mv:click(map:get_field(10,10))
   sleep(2000)
   assert_not_nil(mv.windows.field_action)
end

local function test_census()
   mv.census = 1
   assert_equal(true, mv.census)
   assert_equal(false, mv.statistics)
end

local function test_statistics()
   mv.statistics = 1
   assert_equal(true, mv.statistics)
   assert_equal(false, mv.census)
end


run(function()
   game.desired_speed = 1000

   for i,fn in pairs({
         test_buttons_property,
         test_window_property,
         test_window_property1,
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
         test_click}) do
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
         test_click,
         test_census,
         test_statistics,
         test_window_property,
         test_window_property1,
         test_position_x,
         test_position_y,
         test_descendant_position,
         test_descendant_position_not_child,
         test_width,
         test_height}) do
      print("Starting MAPVIEW test #" .. i)
      setup_UI()
      setup_MAPVIEW()
      fn()
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
