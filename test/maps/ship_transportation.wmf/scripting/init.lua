include "scripting/lunit.lua"
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "test/scripting/stable_save.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]

p1:allow_workers("all")

function create_southern_port()
   prefilled_buildings(p1,
   { "barbarians_port", 16, 16,
      wares = {},
      workers = {},
      soldiers = {}
   })
end

function southern_port()
   return map:get_field(16, 16).immovable
end

function create_northern_port()
   prefilled_buildings(p1,
   { "barbarians_port", 16, 2,
      wares = {},
      workers = {},
      soldiers = {}
   })
end

function northern_port()
   local o = map:get_field(16, 2).immovable
   if o and o.descr.name == "barbarians_port" then
      return o
   end
   return nil
end

function start_building_farm()
   p1:place_building("barbarians_farm", map:get_field(18, 4), true, true)
   connected_road("normal", p1, map:get_field(18,5).immovable, "l,l|tl,tr|", true)
end

function farm()
   return map:get_field(18, 4).immovable
end

ship = p1:place_ship(map:get_field(10, 10))
p1.see_all = true
