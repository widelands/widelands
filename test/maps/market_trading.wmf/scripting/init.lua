include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/lunit.lua"
include "scripting/ui.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]
p2 = game.players[2]
p1.see_all = true

p1:allow_workers("all")

function full_headquarters(player, x, y)
   prefilled_buildings(player, { "barbarians_headquarters", x, y })
   hq = player:get_buildings("barbarians_headquarters")[1]
   for idx,descr in ipairs(game:get_tribe_description("barbarians").wares) do
      hq:set_wares(descr.name, 350)
   end
   for idx,descr in ipairs(game:get_tribe_description("barbarians").workers) do
      hq:set_workers(descr.name, 50)
   end
end

function place_markets()
   prefilled_buildings(p1, { "barbarians_market", 18, 25 })
   market_p1 = map:get_field(18, 25).immovable
   connected_road(p1, market_p1.flag, "r,r,r,r|", true)

   prefilled_buildings(p2, { "barbarians_market", 35, 25 })
   market_p2 = map:get_field(35, 25).immovable
   connected_road(p2, market_p2.flag, "l,l,l|", true)
end

full_headquarters(p1, 22, 25)
full_headquarters(p2, 32, 25)

game.desired_speed = 50000
