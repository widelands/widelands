include "scripting/lunit.lua"
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]
p1.see_all = true

p1:allow_workers("all")
prefilled_buildings(p1, { "amazons_headquarters", 0, 0, })
