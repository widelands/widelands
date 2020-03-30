include "scripting/lunit.lua"
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "test/scripting/stable_save.lua"

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
{ "barbarians_headquarters", 8, 18,
   wares = {
      blackwood = 3,
      grout = 2,
      iron = 2,
      granite = 5,
      reed = 4,
      log = 3,
   },
   workers = {
      barbarians_builder = 1,
   },
},
-- But some wares needed for the expedition into the port.
{ "barbarians_port", 16, 16,
   wares = {
      gold = 2,
   },
   workers = {
   },
   soldiers = {
   }
},
{ "barbarians_rangers_hut", 5, 20, workers = { barbarians_ranger = 1 } },
{ "barbarians_rangers_hut", 4, 10, workers = { barbarians_ranger = 1 } },
{ "barbarians_lumberjacks_hut", 7, 20, workers = { barbarians_lumberjack = 1 } },
{ "barbarians_wood_hardener", 10, 18, workers = { barbarians_lumberjack = 1 } }
)

hq = map:get_field(8, 18).immovable
port = map:get_field(16, 16).immovable

connected_road("normal", p1, map:get_field(8,19).immovable, "r,r|r,r|r,r|r,tr,tr|", true)
connected_road("normal", p1, map:get_field(7,21).immovable, "tr,tr|", true)
first_ship = nil
second_ship = nil

function click_on_ship(which_ship)
   local mv = wl.ui.MapView()
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob == which_ship then
               if not mv:is_visible(field) then
                  scroll_to_field(field)
                  click_on_ship(which_ship)
               end
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
   assert_equal(5, wares.granite)
   assert_equal(4, wares.reed)
   -- We do not check for logs here as they might be carried out of the
   -- warehouse already when we check (because they might get requested by the
   -- hardener).
   assert_equal(1, port:get_workers("barbarians_builder"))
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

--function cancel_expedition_or_sink_in_shipwindow()
    --if second_ship then
   --ship_to_click=second_ship
   --elseif first_ship then
   --ship_to_click=first_ship
   --else
      --assert(false)
      --end

   --click_on_ship(ship_to_click)
   --if click_button("cancel_expedition") then
      --sleep(100)
      --assert_true(click_button("ok"))
      --sleep(100)
      --close_windows()
      --sleep(100)
      --print (" DEBUG expedition cancelled")
   --else
      --click_on_ship(ship_to_click)
      --assert_true(click_button("sink"))
      --sleep(100)
      --assert_true(click_button("ok"))
      --sleep(100)
      --close_windows()
      --sleep(100)
      --print (" DEBUG ship sunk")
   --end
--end


function dismantle_hardener()
   assert_true(click_building(p1, "barbarians_wood_hardener"))
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
   while true do
      while #p1.inbox == 0 do
         sleep(313)
      end
      local message_title = p1.inbox[1].title
      archive_messages()
      if title == message_title then
         break
      end
   end
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
   first_ship = p1:place_ship(map:get_field(10, 10))
end

function create_second_ship()
   second_ship = p1:place_ship(map:get_field(14, 10))
end

function create_two_ships()
   create_one_ship()
   create_second_ship()
end

function test_cancel_started_expedition_on_ship(needs_second_ship)
   sleep(100)
   game.desired_speed = 10 * 1000

   -- Start a new expedition.
   port:start_expedition()
   wait_for_message("Expedition")

   --if current test requires second ship...
   if needs_second_ship then
      create_second_ship()
   end

   game.desired_speed = 10 * 1000
   sleep(10000)

   stable_save(game, "ready_to_sail", 10 * 1000)

   sleep(10000)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   -- Now cancel the expedition before it even got send out.
   cancel_expedition_in_shipwindow()
   sleep(40000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
   sleep(8000)  -- ship needs a while to get wares back.
   check_wares_in_port_are_all_there()
   assert_equal(1, p1:get_workers("barbarians_builder"))

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
   port:start_expedition()
   wait_for_message("Expedition")
   game.desired_speed = 10 * 1000
   sleep(10000)

   if first_ship.state=="exp_waiting" then
      expedition_ship=first_ship
   elseif second_ship.state=="exp_waiting" then
      expedition_ship=second_ship
   else
      assert(false)
   end

   expedition_ship.island_explore_direction="ccw"
   sleep(2000)
   assert_equal("ccw",expedition_ship.island_explore_direction)
   sleep(6000)

   stable_save(game, "sailing", 10 * 1000)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   cancel_expedition_in_shipwindow(expedition_ship)
   sleep(50000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
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
   port:start_expedition()
   wait_for_message("Expedition")
   assert_equal(1, p1:get_workers("barbarians_builder"))
   sleep(500)

   first_ship.island_explore_direction="ccw"
   sleep(2000)
   assert_equal("ccw",first_ship.island_explore_direction)
   wait_for_message("Port Space")
   assert_equal("exp_found_port_space", first_ship.state)
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   stable_save(game, "reached_port_space", 10 * 1000)
   sleep(5000)
   ships = p1:get_ships()
   --ships table should contain 1-2 items (1-2 ships)
   assert_true(#ships >= 1)
   assert_true(#ships <= 2)
   assert_equal("exp_found_port_space", first_ship.state)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   cancel_expedition_in_shipwindow(first_ship)
   sleep(50000)
   assert_equal(1, p1:get_workers("barbarians_builder"))
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
   p1:get_buildings("barbarians_wood_hardener")[1]:remove()
   hq:set_wares("log", 100)
   port:set_wares("blackwood", 100)

   port:start_expedition()
   wait_for_message("Expedition")

   if first_ship.state=="exp_waiting" then
      expedition_ship=first_ship
   elseif second_ship.state=="exp_waiting" then
      expedition_ship=second_ship
   else
      assert(false)
   end

   expedition_ship.island_explore_direction="ccw"
   sleep(2000)
   assert_equal("ccw",expedition_ship.island_explore_direction)
   wait_for_message("Port Space")
   expedition_ship:build_colonization_port()
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))
   wait_for_message("Port")
   sleep(500)
   assert_equal(1, p1:get_workers("barbarians_builder"))

   stable_save(game, "port_done", 25 * 1000)

   -- build a lumberjack and see if the ship starts transporting stuff
   p1:place_building("barbarians_lumberjacks_hut", map:get_field(17, 1), true)
   connected_road("normal", p1, map:get_field(18,2).immovable, "bl,l|", true)
   while map:get_field(17, 1).immovable.descr.name ~= "barbarians_lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("barbarians_builder"))

   -- build a lumberjack and see if the builder gets transported
   p1:place_building("barbarians_lumberjacks_hut", map:get_field(12, 18), true)
   while map:get_field(12, 18).immovable.descr.name ~= "barbarians_lumberjacks_hut" do
      sleep(3222)
   end
   assert_equal(1, p1:get_workers("barbarians_builder"))

   -- Check that the first lumberjack house got his worker.
   assert_equal(1, map:get_field(17, 1).immovable:get_workers("barbarians_lumberjack"))

   print("# All Tests passed.")
   wl.ui.MapView():close()
end
