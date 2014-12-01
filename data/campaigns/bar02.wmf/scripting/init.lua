-- =======================================================================
--                       Barbarians Campaign Mission 2
-- =======================================================================

set_textdomain("scenario_bar02.wmf")

include "data/scripting/coroutine.lua"
include "data/scripting/objective_utils.lua"
include "data/scripting/infrastructure.lua"
include "data/scripting/table.lua"
include "data/scripting/ui.lua"

game = wl.Game()
p1 = game.players[1]
p2 = game.players[2]
p3 = game.players[3]
p4 = game.players[4]

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
