include "scripting/lunit.lua"
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
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

full_headquarters(p1, 22, 25)
full_headquarters(p2, 32, 25)

game.desired_speed = 50000
