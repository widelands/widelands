use("aux", "lunit")
use("aux", "coroutine")
use("aux", "ui")
lunit.import "assertions"

-- This is a test case for bug 1234058: there is constant demand for trunks,
-- so the expedition initially never got any.
-- We also exercise the expedition feature and send the ship around, build a
-- port and construct a building in the colony.

minutes = 60 * 1000

game = wl.Game()
map = game.map
p1 = game.players[1]

p1:allow_workers("all")
prefilled_buildings(p1,
{ "headquarters", 8, 18,
   wares = {
      blackwood = 3,
      grout = 2,
      iron = 2,
      raw_stone = 5,
      thatchreed = 4,
      trunk = 3,
   },
   workers = {
      builder = 1,
   },
},
-- But some wares needed for the expedition into the port.
{ "port", 16, 16,
   wares = {
      gold = 2,
   },
   workers = {
   },
   soldiers = {
   }
},
-- We need a shipyard, because there is no way to make ships via scripting yet.
{ "shipyard", 10, 15,
   wares = {
      blackwood = 10,
      trunk = 2,
      cloth = 4,
   },
   workers = {
      shipwright = 1,
   }
},
{ "rangers_hut", 5, 20, workers = { ranger = 1 } },
{ "rangers_hut", 4, 10, workers = { ranger = 1 } },
{ "lumberjacks_hut", 7, 20, workers = { lumberjack = 1 } },
{ "hardener", 10, 18, workers = { lumberjack = 1 } }
)

hq = map:get_field(8, 18).immovable
port = map:get_field(16, 16).immovable

connected_road(p1, map:get_field(8,19).immovable, "r,r|r,r|r,r|r,tr,tr|", true)
connected_road(p1, map:get_field(7,21).immovable, "tr,tr|", true)

function click_on_ship()
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob.name == "ship" then
               mouse_smoothly_to(field)
               wl.ui.MapView():click(field)
               return
            end
         end
      end
   end
end

function check_wares_in_port_are_all_there(args)
   local wares = port:get_wares("all")
   assert_equal(3, wares.blackwood)
   assert_equal(2, wares.gold)
   assert_equal(2, wares.grout)
   assert_equal(2, wares.iron)
   assert_equal(5, wares.raw_stone)
   assert_equal(4, wares.thatchreed)
   -- We do not check for trunks here as they might be carried out of the
   -- warehouse already when we check (because they might get requested by the
   -- hardener).
   assert_equal(1, port:get_workers("builder"))
end

function start_expedition()
   assert_true(click_building(p1, "port"))
   sleep(100)
   assert_true(click_button("start_expedition"))
   sleep(100)
   close_windows()
   sleep(100)
end

function cancel_expedition_in_port()
   assert_true(click_building(p1, "port"))
   sleep(100)
   assert_true(click_button("cancel_expedition"))
   sleep(100)
   close_windows()
   sleep(100)
end

function cancel_expedition_in_shipwindow()
   click_on_ship()
   assert_true(click_button("cancel_expedition"))
   sleep(100)
   assert_true(click_button("ok"))
   sleep(100)
   close_windows()
   sleep(100)
end

function archive_messages()
   for idx, m in ipairs(p1.inbox) do
      m.status = "archived"
   end
end

function wait_for_message(title)
   archive_messages()
   sleep(5000)
   local old_speed = game.desired_speed
   game.desired_speed = 100 * 1000
   sleep(5000)
   while #p1.inbox == 0 do
      sleep(313)
   end
   assert_equal(title, p1.inbox[1].title)
   archive_messages()
   game.desired_speed = old_speed
   sleep(5000)
end

-- This keeps eating blackwood, so that the burner will never stop working.
run(function()
   while true do
      hq:set_wares("blackwood", 0)
      sleep(1000)
   end
end)

-- Main logic.
run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start and immediately cancel an expedition.
   print("---- 1 -----")
   start_expedition()
   cancel_expedition_in_port()
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   -- Start an expedition, but let them carry some wares into it. This also
   -- gives the builder enough time to walk over.
   print("---- 2 -----")
   start_expedition()
   sleep(70000)
   cancel_expedition_in_port()
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   -- Wait till the expedition is ready (we need to build a ship).
   print("---- 3 -----")
   start_expedition()
   wait_for_message("Expedition ready")

   -- Now cancel the expedition before it even got send out.
   print("---- 4 -----")
   cancel_expedition_in_shipwindow()
   sleep(100)
   assert_equal(1, p1:get_workers("builder"))
   sleep(8000)  -- ship needs a while to get wares back.
   check_wares_in_port_are_all_there()
   assert_equal(1, p1:get_workers("builder"))

   print("---- 5 -----")
   -- Dismantle the hardener to make sure that the builder is able to do his work.
   assert_true(click_building(p1, "hardener"))
   assert_true(click_button("dismantle"))
   assert_true(click_button("ok"))
   close_windows()
   while map:get_field(10, 18).immovable ~= nil do
      sleep(317)
   end
   sleep(1000)

   print("---- 6 -----")
   -- Start a new one and cancel again in port.
   start_expedition()
   sleep(45000)
   cancel_expedition_in_port()
   sleep(80000)  -- let the workers some time to carry wares back in.
   assert_equal(1, p1:get_workers("builder"))
   check_wares_in_port_are_all_there()

   -- Start a new expedition and cancel it while the ship is underway.
   print("---- 7 -----")
   start_expedition()
   wait_for_message("Expedition ready")

   click_on_ship()
   assert_true(click_button("expccw"))
   sleep(8000)
   cancel_expedition_in_shipwindow()
   sleep(20000)
   assert_equal(1, p1:get_workers("builder"))
   check_wares_in_port_are_all_there()
   print("---- 8 -----")

   -- Start an expedition and let it go to the port place.
   print("---- 9 -----")
   start_expedition()
   wait_for_message("Expedition ready")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("---- 10 -----")
   click_on_ship()
   assert_true(click_button("expccw"))
   wait_for_message("Port space found")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("---- 11 -----")
   assert_true(click_button("buildport"))
   wait_for_message("Port")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   -- Build a lumberjack and see if the ship starts transporting stuff
   print("---- 12 -----")
   p1:place_building("lumberjacks_hut", map:get_field(17, 1), true)
   connected_road(p1, map:get_field(18,2).immovable, "bl,l|", true)
   sleep(5 * minutes)  -- wait for the lumberjack to complete.
   assert_equal("lumberjacks_hut", map:get_field(17, 1).immovable.name)
   assert_equal(1, map:get_field(17, 1).immovable:get_workers("lumberjack"))
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   print("---- Build a lumberjack and see if the builder gets transported")
   p1:place_building("lumberjacks_hut", map:get_field(12, 18), true)
   sleep(5 * minutes)  -- wait for the lumberjack to complete.
   assert_equal("lumberjacks_hut", map:get_field(12, 18).immovable.name)
   assert_equal(1, p1:get_workers("builder"))

   print("# All Tests passed.")

   wl.ui.MapView():close()
end)
