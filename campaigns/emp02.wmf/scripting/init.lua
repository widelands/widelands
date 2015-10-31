-- =======================================================================
--                         Empire Campaign Mission 2
-- =======================================================================
set_textdomain("scenario_emp02.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/objective_utils.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1]
p2 = game.players[2]

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"