use("aux", "lunit")
use("aux", "coroutine")
use("aux", "ui")
lunit.import "assertions"

game = wl.Game()
map = game.map
p1 = game.players[1]

-- NOCOM(#sirver): also saving and loading
-- NOCOM(#sirver): before the ship grabs the ware
-- NOCOM(#sirver): what about expeditions?
-- NOCOM(#sirver): what if the farm disappears? what should happen to the wares then.
-- NOCOM(#sirver): ware in portdock?
-- NOCOM(#sirver): workers in portdock

-- NOCOM(#sirver): most functions in this file are not actually used.

-- NOCOM(#sirver):  Wenn ein schiff, oder ein
-- port verschwindet müssen alle anderen ports geupdated werden und die
-- waren müssen eine chance kriegen ihren weg zu canceln.

p1:allow_workers("all")

function create_first_port()
   prefilled_buildings(p1,
   { "port", 16, 16,
      wares = {},
      workers = {},
      soldiers = {}
   })
end

function port1()
   return map:get_field(16, 16).immovable
end

function create_second_port()
   prefilled_buildings(p1,
   { "port", 16, 2,
      wares = {},
      workers = {},
      soldiers = {}
   })
end

function port2()
   local o = map:get_field(16, 2).immovable
   if o and o.name == "port" then
      return o
   end
   return nil
end

function start_building_farm()
   p1:place_building("farm", map:get_field(18, 4), true, true)
   connected_road(p1, map:get_field(18,5).immovable, "l,l|tl,tr|", true)
end

ship = p1:place_bob("ship", map:get_field(10, 10))
p1.see_all = true

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

function click_on_ship()
   for x=0,map.width-1 do
      for y=0,map.height-1 do
         local field = map:get_field(x,y)
         for idx, bob in ipairs(field.bobs) do
            if bob == ship then
               mouse_smoothly_to(field, 1)
               wl.ui.MapView():click(field)
               return
            end
         end
      end
   end
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
   click_on_ship(ship)
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
