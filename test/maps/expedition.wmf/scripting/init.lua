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
first_ship = nil
second_ship = nil

-- Save the game so that reloading does not skip
function stable_save(safename)
   local old_speed = game.desired_speed
   game:save(safename)
   game.desired_speed = 1000
   sleep(8000)  -- Give the loaded game a chance to catch up
   game.desired_speed = old_speed
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

-- Setup the game for everybody.
run(function()
   stable_save("no_ship")

   -- Wait for the ship
   game.desired_speed = 100 * 1000
   sleep(25 * minutes)

   game.desired_speed = 1000

   sleep(2000)
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob.name == "ship" then
               first_ship = bob
               break
            end
         end
      end
   end
   stable_save("one_ship")
   sleep(2000)

   -- Give the builder more materials again.
   p1:get_buildings("shipyard")[1]:set_wares{
      blackwood = 10,
      trunk = 2,
      cloth = 4,
   }

   -- Wait for the ship
   game.desired_speed = 100 * 1000
   sleep(25 * minutes)

   game.desired_speed = 1000
   sleep(2000)
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob.name == "ship" and bob ~= first_ship then
               second_ship = bob
               break
            end
         end
      end
   end
   stable_save("two_ships")

   -- Give the tests that load this game some time to pass too.
   game.desired_speed = 1000 * 1000
   sleep(60 * minutes)

   wl.ui.MapView():close()
end)
