include "data/scripting/lunit.lua"
include "data/scripting/coroutine.lua"
include "data/scripting/ui.lua"

game = wl.Game()
map = game.map
p1 = game.players[1]
p1.see_all = true

p1:allow_workers("all")
prefilled_buildings(p1, { "lumberjacks_hut", 10, 10, }, { "headquarters", 22, 25, })

game.desired_speed = 50000
