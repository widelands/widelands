-- =======================================================================
--                         Empire Campaign Mission 2
-- =======================================================================
push_textdomain("scenario_emp02.wmf")

include "scripting/coroutine.lua"
include "scripting/field_animations.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

game = wl.Game()
p1 = game.players[1]
p2 = game.players[2]
game.allow_diplomacy = false

include "map:scripting/starting_conditions.lua"
include "map:scripting/texts.lua"
include "map:scripting/mission_thread.lua"
