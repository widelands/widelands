include "scripting/lunit.lua"
include "scripting/coroutine.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]

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

function farm()
   return map:get_field(18, 4).immovable
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
