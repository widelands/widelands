include "scripting/lunit.lua"
include "scripting/coroutine.lua"
include "scripting/ui.lua"

-- This is a test case for bug 1234058: there is constant demand for logs,
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
      log = 3,
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
{ "rangers_hut", 5, 20, workers = { ranger = 1 } },
{ "rangers_hut", 4, 10, workers = { ranger = 1 } },
{ "lumberjacks_hut", 7, 20, workers = { lumberjack = 1 } },
{ "hardener", 10, 18, workers = { lumberjack = 1 } }
)

hq = map:get_field(8, 18).immovable
port = map:get_field(16, 16).immovable

connected_road(p1, map:get_field(8,19).immovable, "r,r|r,r|r,r|r,tr,tr|", true)
connected_road(p1, map:get_field(7,21).immovable, "tr,tr|", true)
first_ship = nil
second_ship = nil

-- Save the game so that reloading does not skip
function stable_save(safename)
   local old_speed = game.desired_speed
   game.desired_speed = 1000
   sleep(100)
   game:save(safename)
   game.desired_speed = 1000
   sleep(2000)  -- Give the loaded game a chance to catch up
   game.desired_speed = old_speed
   sleep(1000)
end

function click_on_ship(which_ship)
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob == which_ship then
               mouse_smoothly_to(field, 1)
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
   -- We do not check for logs here as they might be carried out of the
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

function cancel_expedition_in_shipwindow(which_ship)
   click_on_ship(which_ship or first_ship)
   assert_true(click_button("cancel_expedition"))
   sleep(100)
   assert_true(click_button("ok"))
   sleep(100)
   close_windows()
   sleep(100)
end

function dismantle_hardener()
   assert_true(click_building(p1, "hardener"))
   assert_true(click_button("dismantle"))
   assert_true(click_button("ok"))
   close_windows()
   while map:get_field(10, 18).immovable do
      sleep(317)
   end
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

function create_one_ship()
   first_ship = p1:place_bob("ship", map:get_field(10, 10))
end

function create_two_ships()
   create_one_ship()
   second_ship = p1:place_bob("ship", map:get_field(14, 10))
end

function test_cancel_started_expedition_on_ship()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start a new expedition.
   start_expedition()
   wait_for_message("Expedition Ready")
   game.desired_speed = 10 * 1000
   sleep(10000)

   stable_save("ready_to_sail")

   sleep(10000)
   assert_equal(1, p1:get_workers("builder"))

   -- Now cancel the expedition before it even got send out.
   cancel_expedition_in_shipwindow()
   sleep(100)
   assert_equal(1, p1:get_workers("builder"))
   sleep(8000)  -- ship needs a while to get wares back.
   check_wares_in_port_are_all_there()
   assert_equal(1, p1:get_workers("builder"))

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()
end

function test_cancel_started_expedition_underway()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start a new expedition.
   start_expedition()
   wait_for_message("Expedition Ready")
   game.desired_speed = 10 * 1000
   sleep(10000)

   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   sleep(8000)

   stable_save("sailing")
   assert_equal(1, p1:get_workers("builder"))

   cancel_expedition_in_shipwindow(first_ship)
   sleep(20000)
   assert_equal(1, p1:get_workers("builder"))
   check_wares_in_port_are_all_there()

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()
end

function test_cancel_when_port_space_was_reached()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Send expedition to port space.
   start_expedition()
   wait_for_message("Expedition Ready")
   assert_equal(1, p1:get_workers("builder"))
   sleep(500)

   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   wait_for_message("Port Space Found")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   stable_save("reached_port_space")
   assert_equal(1, p1:get_workers("builder"))

   cancel_expedition_in_shipwindow(first_ship)
   sleep(20000)
   assert_equal(1, p1:get_workers("builder"))
   check_wares_in_port_are_all_there()

   -- Dismantle the hardener to make sure that the builder is able to do his work.
   game.desired_speed = 50 * 1000
   dismantle_hardener()

   print("# All Tests passed.")
   wl.ui.MapView():close()
end

function test_transporting_works()
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Some optimization. No need to feed the hardener and to wait for logs.
   p1:get_buildings("hardener")[1]:remove()
   hq:set_wares("log", 100)
   port:set_wares("blackwood", 100)


   start_expedition()
   wait_for_message("Expedition Ready")
   click_on_ship(first_ship)
   assert_true(click_button("expccw"))
   wait_for_message("Port Space Found")
   assert_true(click_button("buildport"))
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))
   wait_for_message("Port")
   sleep(500)
   assert_equal(1, p1:get_workers("builder"))

   stable_save("port_done")
   game.desired_speed = 25 * 1000

   -- build a lumberjack and see if the ship starts transporting stuff
   p1:place_building("lumberjacks_hut", map:get_field(17, 1), true)
   connected_road(p1, map:get_field(18,2).immovable, "bl,l|", true)
   while map:get_field(17, 1).immovable.name ~= "lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("builder"))

   -- build a lumberjack and see if the builder gets transported
   p1:place_building("lumberjacks_hut", map:get_field(12, 18), true)
   while map:get_field(12, 18).immovable.name ~= "lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("builder"))

   -- Check that the first lumberjack house got his worker.
   assert_equal(1, map:get_field(17, 1).immovable:get_workers("lumberjack"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end
